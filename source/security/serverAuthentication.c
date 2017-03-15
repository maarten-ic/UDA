#include "serverAuthentication.h"

#include <libpq-fe.h>

#include <server/udaServer.h>
#include <clientserver/errorLog.h>
#include <logging/logging.h>
#ifndef TESTIDAMSECURITY
#  include <clientserver/udaErrors.h>
#  include <clientserver/protocol.h>
#  include <clientserver/xdrlib.h>
#  include <clientserver/printStructs.h>
#endif

#include "authenticationUtils.h"
#include "x509Utils.h"

static const ENCRYPTION_METHOD encryptionMethod = ASYMMETRICKEY;
static const unsigned short tokenByteLength = NONCEBYTELENGTH;        // System problem when >~ 110 !
static const TOKEN_TYPE tokenType = NONCESTRONGRANDOM; // NONCESTRONGRANDOM NONCESTRINGRANDOM NONCEWEAKRANDOM NONCETEST //

/**
 * Receive the client block, respecting earlier protocol versions
 * @param client_block
 * @return
 */
static int receiveSecurityBlock(CLIENT_BLOCK* client_block)
{
    int err = 0;

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Waiting for Initial Client Block\n");

    if (!xdrrec_skiprecord(serverInput)) {
        IDAM_LOG(LOG_DEBUG, "xdrrec_skiprecord error!\n");
        THROW_ERROR(PROTOCOL_ERROR_5, "Protocol 5 Error (Client Block #2)");
    }

    if ((err = protocol2(serverInput, PROTOCOL_CLIENT_BLOCK, XDR_RECEIVE, NULL, client_block)) != 0) {
        IDAM_LOG(LOG_DEBUG, "protocol error! Client Block not received!\n");
        THROW_ERROR(err, "Protocol 10 Error (Client Block #2)");
    }

    if ((err = protocol2(serverInput, PROTOCOL_SECURITY_BLOCK, XDR_RECEIVE, NULL, &client_block->securityBlock)) != 0) {
        IDAM_LOG(LOG_DEBUG, "protocol error! Client Security Block not received!\n");
        THROW_ERROR(err, "Protocol 10 Error (Client Security Block #2)");
    }

    if (err == 0) {
        IDAM_LOG(LOG_DEBUG, "Initial Client Block received\n");
        printClientBlock(*client_block);
    }

    // Flush (mark as at EOF) the input socket buffer (not all client state data may have been read - version dependent)

    xdrrec_eof(serverInput);
#endif

    return err;
}

static int validateDistinguishedName(const DISTINGUISHED_NAME* dn)
{
    PGconn* conn;

    const char* host = "idam3.mast.ccfe.ac.uk";
    const char* port = "60001";
    const char* dbname = "idam";
    const char* user = "idam";
    const char* pswrd = NULL;

    if ((conn = PQsetdbLogin(host, port, "", "", dbname, user, pswrd)) == NULL) {
        PQfinish(conn);
        THROW_ERROR(999, "SQL Server Connect Error");
    }

    if (PQstatus(conn) == CONNECTION_BAD) {
        PQfinish(conn);
        THROW_ERROR(999, "Bad SQL Server Connect Status");
    }

    const char* sql = "SELECT valid_to FROM uda_authentication WHERE name = $1";

    const char* params[1];
    params[0] = dn->commonName;

    PGresult* res = PQexecParams(conn, sql, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        addIdamError(&idamerrorstack, CODEERRORTYPE, __func__, PQresultStatus(res), PQresultErrorMessage(res));
        THROW_ERROR(999, "Authentication query failed");
    }

    if (PQntuples(res) == 0) {
        THROW_ERROR(999, "Authentication query returned no rows");
    }

    if (PQntuples(res) > 1) {
        THROW_ERROR(999, "Authentication query returned multiple rows");
    }

    const char* buf = PQgetvalue(res, 0, 0);
    IDAM_LOGF(LOG_DEBUG, "timestamp: %s\n", buf);

    struct tm ts;
    if (strptime(buf, "%Y-%m-%d %H:%M:%S", &ts) == NULL) {
        THROW_ERROR(999, "Failed to parse database timestamp");
    }

    time_t valid_to = mktime(&ts);
    time_t now = time(NULL);

    if (now > valid_to) {
        IDAM_LOGF(LOG_INFO, "Certificate expired on %s\n", buf);
        THROW_ERROR(999, "Certificate is no longer valid");
    }

    sql = "UPDATE uda_authentication SET last_login = now() WHERE name = $1;";

    PQexecParams(conn, sql, 1, NULL, params, NULL, NULL, 0);

    return 0;
}

/**
 * Read the Server's Private Key (from a PEM file) and the User's Public Key (from the passed x509 cert)
 * Read the Certificate Authority's Public Key to check signature of the client's certificate
 * Keys have S-Expressions format
 *
 * Key locations are identified from an environment variable
 * The client's public key is from a x509 certificate (check date validity + CA signature)
 *
 * @param client_block
 * @param publickey_out
 * @param privatekey_out
 * @return
 */
static int initialiseKeys(CLIENT_BLOCK* client_block, gcry_sexp_t* client_publickey_out, gcry_sexp_t* server_privatekey_out)
{
    int err = 0;

    static int initialised = FALSE;

    static gcry_sexp_t server_privatekey = NULL;       // Server's
    static gcry_sexp_t client_publickey = NULL;        // Client's

    if (initialised) {
        *client_publickey_out = client_publickey;
        *server_privatekey_out = server_privatekey;
        return 0;
    }

    //---------------------------------------------------------------------------------------------------------------
    // Read the CLIENT_BLOCK and client x509 certificate
    err = receiveSecurityBlock(client_block);
    if (err != 0) {
        return err;
    }

    char* env = NULL;
    size_t len = 0;

    char* serverPrivateKeyFile = NULL;
    char* CACertFile = NULL;

    if ((env = getenv("UDA_SERVER_CERTIFICATE")) != NULL) {    // Directory with certificates and key files
        len = strlen(env) + 56;
        serverPrivateKeyFile = (char*)malloc(len * sizeof(unsigned char));
        CACertFile = (char*)malloc(len * sizeof(unsigned char));

        sprintf(serverPrivateKeyFile, "%s/serverskey.pem", env);  // Server's
        sprintf(CACertFile, "%s/carootX509.der", env); // CA Certificate for signature verification
    } else {
        char* home = getenv("HOME");
        len = 256 + strlen(home);

        serverPrivateKeyFile = (char*)malloc(len * sizeof(unsigned char));
        CACertFile = (char*)malloc(len * sizeof(unsigned char));

        sprintf(serverPrivateKeyFile, "%s/.UDA/server/serverskey.pem", home);
        sprintf(CACertFile, "%s/.UDA/server/carootX509.der", home);
    }

    ksba_cert_t clientCert = NULL;
    ksba_cert_t CACert = NULL;

    do {    // Error Trap

        SECURITY_BLOCK* securityBlock = &client_block->securityBlock;

        // Read the Client's certificate, check validity, extract the public key

        if (securityBlock->client_X509 != NULL) {
            if ((err = makeX509CertObject(securityBlock->client_X509, securityBlock->client_X509Length,
                                          &clientCert)) != 0) {
                break;
            }
            const char* idn = ksba_cert_get_issuer(clientCert, 0);
            IDAM_LOGF(LOG_INFO, "issuer distinguished name: %s\n", idn);

            const char* sdn = ksba_cert_get_subject(clientCert, 0);
            IDAM_LOGF(LOG_INFO, "subject distinguished name: %s\n", sdn);

            DISTINGUISHED_NAME dn = unpackDistinguishedName(sdn);
            printDistinguishedName(&dn);

            int validation = validateDistinguishedName(&dn);

            destroyDistinguishedName(&dn);
            free((void*)idn);
            free((void*)sdn);

            if (validation) {
                addIdamError(&idamerrorstack, CODEERRORTYPE, __func__, err, "Certificate is invalid");
                break;
            }

            // Check the Certificate Validity
            if ((err = testX509Dates(clientCert)) != 0) {
                break;
            }

            // get the Public key from an X509 certificate
            if ((err = extractX509SExpKey(clientCert, &client_publickey)) != 0) {
                break;
            }
        } else {
            addIdamError(&idamerrorstack, CODEERRORTYPE, __func__, err, "Failed to receive the Client's certificate");
            break;
        }

        // get the server's Private key from a PEM file (for decryption) and convert to S-Expression
        if ((err = importPEMPrivateKey(serverPrivateKeyFile, &server_privatekey)) != 0) {
            addIdamError(&idamerrorstack, CODEERRORTYPE, __func__, err, "Failed to load Server's Private Key File");
            break;
        }

        // Read the CA's certificate, check date validity
        if (CACertFile != NULL) {
            if ((err = importX509Reader(CACertFile, &CACert)) != 0) break;
            if ((err = testX509Dates(CACert)) != 0) break;
        }

        // Test the server's private key for consistency
        if (gcry_pk_testkey(server_privatekey) != 0) {
            err = 999;
            addIdamError(&idamerrorstack, CODEERRORTYPE, __func__, err,
                         "The Server's Private Authentication Key is Invalid!");
            break;
        }

        // Verify the client certificate's signature using the CA's public key
        if ((err = checkX509Signature(CACert, clientCert)) != 0) {
            break;
        }

        ksba_cert_release(clientCert);
        ksba_cert_release(CACert);
        clientCert = NULL;
        CACert = NULL;

    } while (0);

    free(serverPrivateKeyFile);
    free(CACertFile);

    if (err != 0) {
        if (server_privatekey != NULL) gcry_sexp_release(server_privatekey);
        if (client_publickey != NULL) gcry_sexp_release(client_publickey);
        if (clientCert != NULL) ksba_cert_release(clientCert);
        if (CACert != NULL) ksba_cert_release(CACert);
        server_privatekey = NULL;
        client_publickey = NULL;
        return err;
    }

    *client_publickey_out = client_publickey;
    *server_privatekey_out = server_privatekey;
    initialised = TRUE;

    return err;
}

static int decryptClientToken(CLIENT_BLOCK* client_block, gcry_sexp_t client_publickey, gcry_sexp_t server_privatekey,
                              gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Step 2: Receive the Client's token cipher (EASP) and decrypt with the server's private key (->A)

    SECURITY_BLOCK* securityBlock = &client_block->securityBlock;

    // Already received the encrypted token (A) from the client

    if (securityBlock->authenticationStep != SERVER_DECRYPT_CLIENT_TOKEN - 1) {
        THROW_ERROR(999, "Authentication Step #2 Inconsistency!");
    }

    unsigned char* client_ciphertext = securityBlock->client_ciphertext;
    unsigned char* server_ciphertext = securityBlock->server_ciphertext;

    size_t client_ciphertextLength = securityBlock->client_ciphertextLength;
    size_t server_ciphertextLength = securityBlock->server_ciphertextLength;

    // Decrypt token (A)

    err = udaAuthentication(SERVER_DECRYPT_CLIENT_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            client_publickey, server_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Decryption Step #2!");
    }

    free((void*)client_ciphertext);
    client_ciphertext = NULL;
    client_ciphertextLength = 0;
    free((void*)server_ciphertext);
    server_ciphertext = NULL;
    server_ciphertextLength = 0;

    return err;
}

static int encryptClientToken(SERVER_BLOCK* server_block, gcry_sexp_t client_publickey, gcry_sexp_t server_privatekey,
                              gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Step 3: Server encrypts the client token (A) with the client's public key (->EACP)

    // Encrypt token (A)

    unsigned char* client_ciphertext = NULL;
    unsigned char* server_ciphertext = NULL;
    size_t client_ciphertextLength = 0;
    size_t server_ciphertextLength = 0;

    err = udaAuthentication(SERVER_ENCRYPT_CLIENT_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            client_publickey, server_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Encryption Step #3!");
    }

    SECURITY_BLOCK* securityBlock = &server_block->securityBlock;
    initSecurityBlock(securityBlock);

    securityBlock->client_ciphertext = client_ciphertext;
    securityBlock->client_ciphertextLength = (unsigned short)client_ciphertextLength;

    gcry_mpi_release(*client_mpiToken);

    return err;
}

static int issueToken(SERVER_BLOCK* server_block, gcry_sexp_t client_publickey, gcry_sexp_t server_privatekey,
                      gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;
    //---------------------------------------------------------------------------------------------------------------
    // Step 4: Server issues a new token (B) also encrypted with the client's public key (->EBCP), passes both to client.

    // Generate new Token and Encrypt (B)

    unsigned char* client_ciphertext = NULL;
    unsigned char* server_ciphertext = NULL;
    size_t client_ciphertextLength = 0;
    size_t server_ciphertextLength = 0;

    err = udaAuthentication(SERVER_ISSUE_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            client_publickey, server_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Encryption Step #4!");
    }

    // Send the encrypted token to the server

    SECURITY_BLOCK* securityBlock = &server_block->securityBlock;

    securityBlock->authenticationStep = SERVER_ISSUE_TOKEN;
    securityBlock->server_ciphertext = server_ciphertext;
    securityBlock->server_ciphertextLength = (unsigned short)server_ciphertextLength;

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Sending initial ServerBlock\n");
    printServerBlock(*server_block);

    if ((err = protocol2(serverOutput, PROTOCOL_SERVER_BLOCK, XDR_SEND, NULL, server_block)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Server Block #4)");
    }

    if ((err = protocol2(serverOutput, PROTOCOL_SECURITY_BLOCK, XDR_SEND, NULL, securityBlock)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Server Security Block #4)");
    }

    if (!xdrrec_endofrecord(serverOutput, 1)) {
        THROW_ERROR(PROTOCOL_ERROR_7, "Protocol 7 Error (Server Block)");
    }
#endif

    return err;
}

static int verifyToken(CLIENT_BLOCK* client_block, gcry_sexp_t client_publickey, gcry_sexp_t server_privatekey,
                       gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Step 7: Server decrypts the passed cipher (EBSP) with the server's private key (->B) and checks
    //	   token (B) => client authenticated

    // Receive the encrypted token (B) from the client

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Waiting for Client Block\n");

    if (!xdrrec_skiprecord(serverInput)) {
        IDAM_LOG(LOG_DEBUG, "xdrrec_skiprecord error!\n");
        THROW_ERROR(PROTOCOL_ERROR_5, "Protocol 5 Error (Client Block #7)");
    }

    if ((err = protocol2(serverInput, PROTOCOL_CLIENT_BLOCK, XDR_RECEIVE, NULL, client_block)) != 0) {
        THROW_ERROR(err, "Protocol 11 Error (securityBlock #7)");
    }

    if ((err = protocol2(serverInput, PROTOCOL_SECURITY_BLOCK, XDR_RECEIVE, NULL, &client_block->securityBlock)) != 0) {
        THROW_ERROR(err, "Protocol 11 Error (securityBlock #7)");
    }

    // Flush (mark as at EOF) the input socket buffer (not all client state data may have been read - version dependent)
    xdrrec_eof(serverInput);
#endif

    SECURITY_BLOCK* securityBlock = &client_block->securityBlock;

    if (securityBlock->authenticationStep != SERVER_VERIFY_TOKEN - 1) {
        THROW_ERROR(999, "Authentication Step Inconsistency!");
    }

    unsigned char* client_ciphertext = securityBlock->client_ciphertext;
    unsigned char* server_ciphertext = securityBlock->server_ciphertext;
    size_t client_ciphertextLength = securityBlock->client_ciphertextLength;
    size_t server_ciphertextLength = securityBlock->server_ciphertextLength;

    // Decrypt token (B) and Authenticate the Client

    err = udaAuthentication(SERVER_VERIFY_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            client_publickey, server_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Authentication Step #7!");
    }

    IDAM_LOG(LOG_INFO, "Client successfully authenticated\n");

//    // Send the encrypted token B to the client
//
//    securityBlock = &server_block->securityBlock;
//
//    securityBlock->authenticationStep = SERVER_VERIFY_TOKEN;
//    securityBlock->server_ciphertext = server_ciphertext;
//    securityBlock->server_ciphertextLength = (unsigned short)server_ciphertextLength;
//    securityBlock->client_ciphertext = client_ciphertext;
//    securityBlock->client_ciphertextLength = (unsigned short)client_ciphertextLength;
//
//#ifndef TESTIDAMSECURITY
//    IDAM_LOG(LOG_DEBUG, "Sending Server Block with encrypted client token\n");
//
//    if ((err = protocol2(serverOutput, PROTOCOL_SERVER_BLOCK, XDR_SEND, NULL, server_block)) != 0) {
//        THROW_ERROR(err, "Protocol 10 Error (securityBlock #7)");
//    }
//
//    if ((err = protocol2(serverOutput, PROTOCOL_SECURITY_BLOCK, XDR_SEND, NULL, securityBlock)) != 0) {
//        THROW_ERROR(err, "Protocol 10 Error (securityBlock #7)");
//    }
//
//    if (!xdrrec_endofrecord(serverOutput, 1)) {
//        THROW_ERROR(PROTOCOL_ERROR_7, "Protocol 7 Error (Server Block #7)");
//    }
//#endif

    return err;
}

int serverAuthentication(CLIENT_BLOCK* client_block, SERVER_BLOCK* server_block, AUTHENTICATION_STEP authenticationStep)
{
    int err = 0;

    gcry_sexp_t server_privatekey = NULL;
    gcry_sexp_t client_publickey = NULL;

    err = initialiseKeys(client_block, &client_publickey, &server_privatekey);
    if (err != 0) {
        return err;
    }

    //---------------------------------------------------------------------------------------------------------------
    // Authenticate both Client and Server

    static gcry_mpi_t client_mpiToken = NULL;
    static gcry_mpi_t server_mpiToken = NULL;

    switch (authenticationStep) {
        case SERVER_DECRYPT_CLIENT_TOKEN:
            err = decryptClientToken(client_block, client_publickey, server_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case SERVER_ENCRYPT_CLIENT_TOKEN:
            err = encryptClientToken(server_block, client_publickey, server_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case SERVER_ISSUE_TOKEN:
            err = issueToken(server_block, client_publickey, server_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case SERVER_VERIFY_TOKEN:
            err = verifyToken(client_block, client_publickey, server_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case HOUSEKEEPING:
            if (server_privatekey != NULL) gcry_sexp_release(server_privatekey);
            if (client_publickey != NULL) gcry_sexp_release(client_publickey);
            if (client_mpiToken != NULL) gcry_mpi_release(client_mpiToken);
            if (server_mpiToken != NULL) gcry_mpi_release(server_mpiToken);
            break;

        default:
            THROW_ERROR(999, "Unknown authentication step");
    }

    if (err != 0) {
        if (client_mpiToken != NULL) gcry_mpi_release(client_mpiToken);
        if (server_mpiToken != NULL) gcry_mpi_release(server_mpiToken);
    }

    return err;
}
