#include "clientAuthentication.h"

#include <clientserver/errorLog.h>
#include <clientserver/protocol.h>
#include <clientserver/printStructs.h>
#include <logging/logging.h>
#ifndef TESTIDAMSECURITY
#  include <clientserver/xdrlib.h>
#  include <clientserver/udaErrors.h>
#  include <client/udaClient.h>
#endif

#include "authenticationUtils.h"
#include "x509Utils.h"

static ENCRYPTION_METHOD encryptionMethod = ASYMMETRICKEY;
static unsigned short tokenByteLength = NONCEBYTELENGTH;        // System problem when >~ 110 !
static TOKEN_TYPE tokenType = NONCESTRONGRANDOM; // NONCESTRONGRANDOM NONCESTRINGRANDOM NONCEWEAKRANDOM NONCETEST; //

/**
 * Read the User's Private Key (from a PEM format file) and the Server's Public Key (from a DER format x509 cert)
 * Convert keys from PEM to S-Expressions representation.
 *
 * Key locations are identified from an environment variable
 * Server public key are from a x509 certificate (to check date validity - a key file isn't sufficient)
 */
static int initialiseKeys(CLIENT_BLOCK* client_block, gcry_sexp_t* server_publickey_out, gcry_sexp_t* client_privatekey_out)
{
    SECURITY_BLOCK* securityBlock = NULL;

    static gcry_sexp_t client_privatekey = NULL;    // Client's private key - maintain state for future en/decryption
    static gcry_sexp_t server_publickey = NULL;    // Server's public key

    static short initialised = FALSE;    // Input keys and certificates at startup

    if (initialised) {
        *client_privatekey_out = client_privatekey;
        *server_publickey_out = server_publickey;
        return 0;
    }

    char* env = NULL;

    char* clientPrivateKeyFile = NULL;
    char* serverPublicKeyFile = NULL;
    char* clientX509File = NULL;      // Authentication with the first server in a server chain
    char* client2X509File = NULL;     // Delivered to the final host in a server chain where the data resides
    char* serverX509File = NULL;      // Certificate of the first server host

    if ((env = getenv("UDA_CLIENT_CERTIFICATE")) != NULL) {    // Directory with certificates and key files
        size_t len = strlen(env) + 56;
        clientPrivateKeyFile = (char*)malloc(len * sizeof(unsigned char));
        serverPublicKeyFile = (char*)malloc(len * sizeof(unsigned char));
        clientX509File = (char*)malloc(len * sizeof(unsigned char));
        serverX509File = (char*)malloc(len * sizeof(unsigned char));

        sprintf(clientPrivateKeyFile, "%s/clientskey.pem", env);    // Client's
        sprintf(serverPublicKeyFile, "%s/serverpkey.pem", env);    // Server's
        sprintf(clientX509File, "%s/clientX509.der", env);
        sprintf(serverX509File, "%s/serverX509.der", env);
    } else {
        char* home = getenv("HOME");
        size_t len = 256 + strlen(home);

        clientPrivateKeyFile = (char*)malloc(len * sizeof(unsigned char));
        serverPublicKeyFile = (char*)malloc(len * sizeof(unsigned char));
        clientX509File = (char*)malloc(len * sizeof(unsigned char));
        serverX509File = (char*)malloc(len * sizeof(unsigned char));

        sprintf(clientPrivateKeyFile, "%s/.UDA/client/clientskey.pem", home);
        sprintf(serverPublicKeyFile, "%s/.UDA/client/serverpkey.pem", home);
        sprintf(clientX509File, "%s/.UDA/client/clientX509.der", home);
        sprintf(serverX509File, "%s/.UDA/client/serverX509.der", home);
    }

    if ((env = getenv("UDA_CLIENT2_CERTIFICATE")) != NULL) {
        // X509 certificate to authenticate with the final server in a chain
        size_t len = strlen(env) + 56;
        client2X509File = (char*)malloc(len * sizeof(unsigned char));
        sprintf(client2X509File, "%s/client/client2X509.der", env);
    }

    ksba_cert_t clientCert = NULL;
    ksba_cert_t client2Cert = NULL;
    ksba_cert_t serverCert = NULL;
    int err = 0;

    // Read the Client's certificates and check date validity

    securityBlock = &client_block->securityBlock;
    initSecurityBlock(securityBlock);

    if (clientX509File != NULL) {
        if ((err = importSecurityDoc(clientX509File, &securityBlock->client_X509,
                                     &securityBlock->client_X509Length)) != 0) {
            return err;
        }
        if ((err = makeX509CertObject(securityBlock->client_X509, securityBlock->client_X509Length,
                                      &clientCert)) != 0) {
            return err;
        }
        if ((err = testX509Dates(clientCert)) != 0) {
            return err;
        }
        ksba_cert_release(clientCert);
        clientCert = NULL;
    }

    if (client2X509File != NULL) {
        if ((err = importSecurityDoc(client2X509File, &securityBlock->client2_X509,
                                     &securityBlock->client2_X509Length)) != 0) {
            return err;
        }
        if ((err = makeX509CertObject(securityBlock->client2_X509, securityBlock->client2_X509Length,
                                      &client2Cert)) != 0) {
            return err;
        }
        if ((err = testX509Dates(client2Cert)) != 0) {
            return err;
        }
        ksba_cert_release(client2Cert);
        client2Cert = NULL;
    }

    // Test the private key file and its directory directory have permissions set to owner read only

    if ((err = testFilePermissions(clientPrivateKeyFile)) != 0) {
        return err;
    }

    if (client2X509File != NULL && (err = testFilePermissions(client2X509File)) != 0) {
        return err;
    }

    if ((env = getenv("UDA_CLIENT_CERTIFICATE")) != NULL) {
        if ((err = testFilePermissions(env)) != 0) {
            return err;
        }
    } else {
        const char* home = getenv("HOME");
        char work[1024];
        sprintf(work, "%s/.UDA/client", home);
        if ((err = testFilePermissions(work)) != 0) {
            return err;
        }
    }

    // get the user's Private key from a PEM file (for decryption) and convert to S-Expression

    if ((err = importPEMPrivateKey(clientPrivateKeyFile, &client_privatekey)) != 0) {
        return err;
    }

    // get the server's Public key (for encryption of exchanged tokens) from a certificate or a file
    // If from a PEM file, convert to S-Expression

    if (serverX509File != NULL) {
        unsigned char* serverCertificate;        // Server's X509 authentication certificate
        unsigned short serverCertificateLength;

        if ((err = importSecurityDoc(serverX509File, &serverCertificate, &serverCertificateLength)) != 0) {
            return err;
        }
        if ((err = makeX509CertObject(serverCertificate, serverCertificateLength, &serverCert)) != 0) {
            return err;
        }
        if ((err = testX509Dates(serverCert)) != 0) {
            return err;
        }
        if ((err = extractX509SExpKey(serverCert, &server_publickey)) != 0) {
            return err;
        }        // get the server's Public key from an X509 certificate
        ksba_cert_release(serverCert);
        serverCert = NULL;
        if (serverCertificate != NULL) free(serverCertificate);
        serverCertificate = NULL;

    // get the server's Public key from a file
    } else if ((err = importPEMPublicKey(serverPublicKeyFile, &server_publickey)) != 0) {
        return err;
    }

    // Test the user's private key for consistency
    // User keys also have a lifetime - automatically checked if there is a x509 certificate
    // Stale keys must be renewed by a utility (separate system) requiring strong authentication - proof of identity
    // Server may also renew it's public key at that time (may be different for each user!)

    if (gcry_pk_testkey(client_privatekey) != 0) {
        err = 999;
        addIdamError(&idamerrorstack, CODEERRORTYPE, "idamClientAuthentication", err,
                     "The User's Private Authentication Key is Invalid!");
        return err;
    }

    free(clientPrivateKeyFile);
    free(serverPublicKeyFile);
    free(clientX509File);
    free(serverX509File);
    free(client2X509File);

    if (err != 0) {
        free((void*)securityBlock->client_X509);
        free((void*)securityBlock->client2_X509);

        securityBlock->client_X509 = NULL;
        securityBlock->client2_X509 = NULL;
        securityBlock->client_X509Length = 0;
        securityBlock->client2_X509Length = 0;

        if (clientCert != NULL) ksba_cert_release(clientCert);
        if (client2Cert != NULL) ksba_cert_release(client2Cert);
        if (serverCert != NULL) ksba_cert_release(serverCert);
        clientCert = NULL;
        client2Cert = NULL;
        serverCert = NULL;

        if (client_privatekey != NULL) gcry_sexp_release(client_privatekey);
        if (server_publickey != NULL) gcry_sexp_release(server_publickey);
        // These are declared as static so ensure they are reset when an error occurs
        client_privatekey = NULL;
        server_publickey = NULL;
        return err;
    }

    initialised = TRUE;
    *server_publickey_out = server_publickey;
    *client_privatekey_out = client_privatekey;

    return 0;
}

static int issueToken(CLIENT_BLOCK* client_block, gcry_sexp_t server_publickey, gcry_sexp_t client_privatekey,
                      gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    // Prepare the encrypted token (A)

    unsigned char* client_ciphertext = NULL;
    unsigned char* server_ciphertext = NULL;
    size_t client_ciphertextLength = 0;
    size_t server_ciphertextLength = 0;

    err = udaAuthentication(CLIENT_ISSUE_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            server_publickey, client_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Preparing Authentication Step #1!");
    }

// Send the encrypted token to the server together with Client's claim of identity

    SECURITY_BLOCK* securityBlock = &client_block->securityBlock;

    securityBlock->authenticationStep = CLIENT_ISSUE_TOKEN;
    securityBlock->client_ciphertext = client_ciphertext;
    securityBlock->server_ciphertext = NULL;

    securityBlock->client_ciphertextLength = (unsigned short)client_ciphertextLength;
    securityBlock->server_ciphertextLength = 0;

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Sending initial ClientBlock\n");
    printClientBlock(*client_block);

    if ((err = protocol2(clientOutput, PROTOCOL_CLIENT_BLOCK, XDR_SEND, NULL, client_block)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Client Block #1)");
    }

    if ((err = protocol2(clientOutput, PROTOCOL_SECURITY_BLOCK, XDR_SEND, NULL, securityBlock)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Client Security Block #1)");
    }

    // Send to server
    if (!xdrrec_endofrecord(clientOutput, 1)) {
        THROW_ERROR(PROTOCOL_ERROR_7, "Protocol 7 Error (Client Block #1)");
    }

    // No need to resend the client's certificates or encrypted token A
    free(securityBlock->client_ciphertext);

    securityBlock->client_ciphertext = NULL;
    securityBlock->client_ciphertextLength = 0;

    free((void*)securityBlock->client_X509);
    free((void*)securityBlock->client2_X509);

    securityBlock->client_X509 = NULL;
    securityBlock->client2_X509 = NULL;
    securityBlock->client_X509Length = 0;
    securityBlock->client2_X509Length = 0;
#endif

    return err;
}

static int decryptServerToken(SERVER_BLOCK* server_block, CLIENT_BLOCK* client_block, gcry_sexp_t server_publickey,
                              gcry_sexp_t client_privatekey, gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Step 5: Client decrypts the passed ciphers (EACP, EBCP) with the client's private key (->A, ->B) and
    //	   checks token (A) => server authenticated

    // Receive the encrypted tokens (A,B) from the server

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Waiting for Initial Server Block\n");

    if (!xdrrec_skiprecord(clientInput)) {
        THROW_ERROR(PROTOCOL_ERROR_7, "Protocol 7 Error (Server Block #5)");
    }

    if ((err = protocol2(clientInput, PROTOCOL_SERVER_BLOCK, XDR_RECEIVE, NULL, server_block)) != 0) {
        THROW_ERROR(err, "Protocol 11 Error (Server Block #5)");
    }

    if ((err = protocol2(clientInput, PROTOCOL_SECURITY_BLOCK, XDR_RECEIVE, NULL, &server_block->securityBlock)) != 0) {
        THROW_ERROR(err, "Protocol 11 Error (Server Security Block #5)");
    }

    // Flush (mark as at EOF) the input socket buffer (not all server state data may have been read - version dependent)

    xdrrec_eof(clientInput);
#endif

    IDAM_LOG(LOG_DEBUG, "Server Block Received\n");
    printServerBlock(*server_block);

    // Protocol Version: Lower of the client and server version numbers
    // This defines the set of elements within data structures passed between client and server
    // Must be the same on both sides of the socket

    if (client_block->version < server_block->version) {
        protocolVersion = client_block->version;
    }

    // Check for FATAL Server Errors

    if (server_block->idamerrorstack.nerrors != 0) {
        THROW_ERROR(999, "Server Side Authentication Failed!");
    }

    // Extract Ciphers

    SECURITY_BLOCK* securityBlock = &server_block->securityBlock;

    if (securityBlock->authenticationStep != CLIENT_DECRYPT_SERVER_TOKEN - 1) {
        THROW_ERROR(999, "Authentication Step Inconsistency!");
    }

    unsigned char* client_ciphertext = securityBlock->client_ciphertext;
    unsigned char* server_ciphertext = securityBlock->server_ciphertext;
    size_t client_ciphertextLength = securityBlock->client_ciphertextLength;
    size_t server_ciphertextLength = securityBlock->server_ciphertextLength;

    // Decrypt tokens (A, B) and Authenticate the Server

    err = udaAuthentication(CLIENT_DECRYPT_SERVER_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            server_publickey, client_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Authentication Step #5!");
    }

    free((void*)client_ciphertext);
    free((void*)server_ciphertext);

    return err;
}

static int encryptServerToken(CLIENT_BLOCK* client_block, gcry_sexp_t server_publickey, gcry_sexp_t client_privatekey,
                              gcry_mpi_t* client_mpiToken, gcry_mpi_t* server_mpiToken)
{
    int err = 0;

    //---------------------------------------------------------------------------------------------------------------
    // Step 6: Client encrypts passed token (B) with the server's public key (->EBSP), passes back to server

    // Encrypt token (B)

    unsigned char* client_ciphertext = NULL;
    unsigned char* server_ciphertext = NULL;
    size_t client_ciphertextLength = 0;
    size_t server_ciphertextLength = 0;

    err = udaAuthentication(CLIENT_ENCRYPT_SERVER_TOKEN, encryptionMethod,
                            tokenType, tokenByteLength,
                            server_publickey, client_privatekey,
                            client_mpiToken, server_mpiToken,
                            &client_ciphertext, &client_ciphertextLength,
                            &server_ciphertext, &server_ciphertextLength);

    if (err != 0) {
        THROW_ERROR(err, "Failed Preparing Authentication Step #6!");
    }

    // Send the encrypted token to the server via the CLIENT_BLOCK data structure

    SECURITY_BLOCK* securityBlock = &client_block->securityBlock;

    securityBlock->authenticationStep = CLIENT_ENCRYPT_SERVER_TOKEN;
    securityBlock->server_ciphertext = server_ciphertext;
    securityBlock->client_ciphertext = NULL;
    securityBlock->server_ciphertextLength = (unsigned short)server_ciphertextLength;
    securityBlock->client_ciphertextLength = 0;

#ifndef TESTIDAMSECURITY
    IDAM_LOG(LOG_DEBUG, "Sending ClientBlock with encrypted server token\n");
    printClientBlock(*client_block);

    if ((err = protocol2(clientOutput, PROTOCOL_CLIENT_BLOCK, XDR_SEND, NULL, client_block)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Client Block #6)");
    }

    if ((err = protocol2(clientOutput, PROTOCOL_SECURITY_BLOCK, XDR_SEND, NULL, securityBlock)) != 0) {
        THROW_ERROR(err, "Protocol 10 Error (Client Security Block #6)");
    }

    // Send to server
    if (!xdrrec_endofrecord(clientOutput, 1)) {
        THROW_ERROR(PROTOCOL_ERROR_7, "Protocol 7 Error (Client Block #6)");
    }

    free((void*)server_ciphertext);
    free((void*)client_ciphertext);
#endif

    return err;
}

int clientAuthentication(CLIENT_BLOCK* client_block, SERVER_BLOCK* server_block, AUTHENTICATION_STEP authenticationStep)
{
    int err = 0;

    gcry_sexp_t client_privatekey = NULL;
    gcry_sexp_t server_publickey = NULL;

    err = initialiseKeys(client_block, &server_publickey, &client_privatekey);
    if (err != 0) {
        return err;
    }

    static gcry_mpi_t client_mpiToken = NULL;
    static gcry_mpi_t server_mpiToken = NULL;

    switch (authenticationStep) {
        case CLIENT_ISSUE_TOKEN:
            err = issueToken(client_block, server_publickey, client_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case CLIENT_DECRYPT_SERVER_TOKEN:
            err = decryptServerToken(server_block, client_block, server_publickey, client_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case CLIENT_ENCRYPT_SERVER_TOKEN:
            err = encryptServerToken(client_block, server_publickey, client_privatekey, &client_mpiToken, &server_mpiToken);
            break;

        case HOUSEKEEPING:
            if (server_publickey != NULL) gcry_sexp_release(server_publickey);
            if (client_privatekey != NULL) gcry_sexp_release(client_privatekey);
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
