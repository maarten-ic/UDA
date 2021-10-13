#include "memcache.h"
#include "cache.h"

#ifdef NOLIBMEMCACHED

struct UdaCache {
    int dummy_;
};

UDA_CACHE* udaOpenCache()
{ return nullptr; }

void udaFreeCache()
{}

char* udaCacheKey(const REQUEST_BLOCK* request_block, ENVIRONMENT environment)
{ return nullptr; }

int udaCacheWrite(UDA_CACHE* cache, const REQUEST_BLOCK* request_block, DATA_BLOCK* data_block,
                   LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist, ENVIRONMENT environment,
                   int protocolVersion)
{ return 0; }

DATA_BLOCK* udaCacheRead(UDA_CACHE* cache, const REQUEST_BLOCK* request_block, LOGMALLOCLIST* logmalloclist,
                          USERDEFINEDTYPELIST* userdefinedtypelist, ENVIRONMENT environment, int protocolVersion)
{ return nullptr; }

#else

#include <libmemcached/memcached.h>
#include <openssl/ssl.h>

#include <logging/logging.h>
#include <clientserver/initStructs.h>
#include <clientserver/memstream.h>
#include <clientserver/xdrlib.h>
#include <tuple>
#include <clientserver/errorLog.h>

#define HASHXDR 1
#ifdef HASHXDR
#  define PARTBLOCKINIT        1
#  define PARTBLOCKUPDATE      2
#  define PARTBLOCKOUTPUT      3
#endif // HASHXDR

#define MAXELEMENTSHA1 20

struct UdaCache {
    memcached_st memcache;
};

static UDA_CACHE* global_cache = nullptr;    // scope limited to this code module

UDA_CACHE* udaOpenCache()
{
    auto cache = (UDA_CACHE*)malloc(sizeof(UDA_CACHE));
    memcached_return_t rc;
    memcached_server_st* servers;

    const char* host = getenv("UDA_CACHE_HOST");   // Overrule the default settings
    const char* port = getenv("UDA_CACHE_PORT");

    if (host == nullptr && port == nullptr) {
        servers = memcached_server_list_append(nullptr, UDA_CACHE_HOST, (in_port_t)UDA_CACHE_PORT, &rc);
    } else if (host != nullptr && port != nullptr) {
        servers = memcached_server_list_append(nullptr, host, (in_port_t)atoi(port), &rc);
    } else if (host != nullptr) {
        servers = memcached_server_list_append(nullptr, host, (in_port_t)UDA_CACHE_PORT, &rc);
    } else {
        servers = memcached_server_list_append(nullptr, UDA_CACHE_HOST, (in_port_t)atoi(port), &rc);
    }

    //memcached_create(&cache->memcache);       // Causes a segmentation Violation!
    cache->memcache = *memcached_create(nullptr);
    rc = memcached_server_push(&cache->memcache, servers);

    if (rc == MEMCACHED_SUCCESS) {
        UDA_LOG(UDA_LOG_DEBUG, "%s\n", "Added server successfully");
    } else {
        UDA_LOG(UDA_LOG_DEBUG, "Couldn't add server: %s\n", memcached_strerror(&cache->memcache, rc));
        free(cache);
        return nullptr;
    }

    global_cache = cache;   // Copy the pointer
    return cache;
}

void udaFreeCache() // Will be called by the idamFreeAll function
{
    memcached_free(&global_cache->memcache);
    free(global_cache);
    global_cache = nullptr;
}

// Use the requested signal and source with client specified properties to create a unique key
// All parameters that may effect the data state, e.g. flags, host, port, properties, etc. must be included in the key
// There is a 250 character limit - use SHA1 hash if it exceeds 250
// The local cache should only be used to record data returned from a server after a GET method - Note: Put methods may be disguised in a GET call!
// How to validate the cached data?

char* generate_cache_key(const REQUEST_DATA* request, ENVIRONMENT environment, int flags)
{
    // Check Properties for permission and requested method
    if (!(flags & CLIENTFLAG_CACHE)) {
        return nullptr;
    }

    // **** TODO **** if(!(clientFlags & CLIENTFLAG_CACHE) || request_block->put) return nullptr;
    const char* delimiter = "&&";
    size_t len = strlen(request->source) + strlen(request->signal) +
                 strlen(environment.server_host) + 128;
    char* key = (char*)malloc(len * sizeof(char));
    sprintf(key, "%s%s%s%s%s%s%d%s%d%s%d", request->signal, delimiter, request->source, delimiter,
            environment.server_host, delimiter, environment.server_port, delimiter, environment.clientFlags, delimiter,
            privateFlags);

    char* p = nullptr;
    while ((p = strchr(key, ' ')) != nullptr) {
        *p = '_';
    }

    // *** TODO: Add server properties (set by the client) to the key - planned to use clientFlags (bit settings) but not implemented yet! ***
    // *** which server is the client connected to .... may not be the default in the ENVIRONMENT structure! - Investigate! ***
    // *** privateFlags is a global also in the CLIENT_BLOCK structure passed to the server (with clientFlags)

    if (len < 250) {
        return key;
    }

#ifndef HASHXDR
    free(key);
    return nullptr;
    // No Hash function to create the key
#else
    // Need a compact hash - use SHA1 as always 20 bytes (40 bytes when printable)
    unsigned char md[MAXELEMENTSHA1 + 1];      // SHA1 Hash
    md[MAXELEMENTSHA1] = '\0';
    strcpy((char*)md, "                    ");
    SHA1((unsigned char*)key, len, md);
    // Convert to a printable string (40 characters) for the key (is this necessary?)
    key[40] = '\0';

    for (int j = 0; j < 20; j++) {
        sprintf(&key[2 * j], "%2.2x", md[j]);
    }

    return key;
#endif
}

// Use NON-BLOCKING IO mode for performance?
// Write only with the server's permission - which information should be kept in the cache is the concern of the server only
// All data services should indicate whether or not the data returned is suitable for client side caching (all server plugin get methods must decide!)
// The server should also set a recommmended expiration time (lifetime of the stored object) - overridden by the client if necessary

int memcache_put(UDA_CACHE* cache, const char* key, const char* buffer, size_t bufsize)
{
    // Expiration of the object
    static unsigned int age_max = UDA_CACHE_EXPIRY;
    static int init = 1;

    if (init) {
        char* env = getenv("UDA_CACHE_EXPIRY");

        if (env != nullptr) {
            age_max = (unsigned int)atoi(env);
        }

        init = 0;
    }

    time_t life = time(nullptr);

#ifdef CACHEDEV
    if (data_block->cacheExpiryTime > 0) {
        // Object expiration time is set by the server
        life += data_block->cacheExpiryTime;
    } else {
        // Add the default or client overridden lifetime for the object to the current time
        life += age_max;
    }
#else
    // Add the default or client overridden lifetime for the object to the current time
    life += age_max;
#endif

    memcached_return_t rc = memcached_set(&cache->memcache, key, strlen(key), buffer, bufsize, life, (uint32_t)0);

    if (rc != MEMCACHED_SUCCESS) {
        THROW_ERROR(-1, memcached_strerror(&cache->memcache, rc));
    }

    rc = memcached_flush_buffers(&cache->memcache);

    if (rc != MEMCACHED_SUCCESS) {
        THROW_ERROR(-1, memcached_strerror(&cache->memcache, rc));
    }

    return 0;
}

int
udaCacheWrite(UDA_CACHE* cache, const REQUEST_DATA* request_data, DATA_BLOCK* data_block,
              LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist, ENVIRONMENT environment,
              int protocolVersion, int flags)
{
#ifdef CACHEDEV
    if (!data_block->cachePermission) {
        // Test permission for the Client to cache this structure.
        return -1;
    }
#endif
    int rc = 0;

    char* key = generate_cache_key(request_data, environment, flags);
    UDA_LOG(UDA_LOG_DEBUG, "Caching value for key: %s\n", key);

    if (key == nullptr) {
        return -1;
    }

    char* buffer = nullptr;
    size_t bufsize = 0;

    FILE* memfile = open_memstream(&buffer, &bufsize);

    writeCacheData(memfile, logmalloclist, userdefinedtypelist, data_block, protocolVersion);

    rc = memcache_put(cache, key, buffer, bufsize);

    fclose(memfile);
    free(buffer);
    free(key);

    return rc;
}

FILE* create_mem_file(const char* value, size_t len)
{
    char* buffer;
    size_t bufsize = 0;

    FILE* memfile = open_memstream(&buffer, &bufsize);

    fwrite(value, sizeof(char), len, memfile);
    fseek(memfile, 0L, SEEK_SET);

    return memfile;
}

std::pair<char*, size_t> get_cache_value(UDA_CACHE* cache, const char* key)
{
    UDA_LOG(UDA_LOG_DEBUG, "Retrieving value for key: %s\n", key);
    memcached_return rc;
    size_t len = 0;
    u_int32_t flags = 0;
    char* value = memcached_get(&cache->memcache, key, strlen(key), &len, &flags, &rc);

    if (rc != MEMCACHED_SUCCESS) {
        UDA_LOG(UDA_LOG_ERROR, "Couldn't retrieve key: %s\n", memcached_strerror(&cache->memcache, rc));
        return { nullptr, 0 };
    }

    return { value, len };
}

DATA_BLOCK* udaCacheRead(UDA_CACHE* cache, const REQUEST_DATA* request_data, LOGMALLOCLIST* logmalloclist,
                         USERDEFINEDTYPELIST* userdefinedtypelist, ENVIRONMENT environment, int protocolVersion,
                         int flags)
{
    char* key = generate_cache_key(request_data, environment, flags);
    if (key == nullptr) {
        return nullptr;
    }

    char* value;
    size_t len;
    std::tie(value, len) = get_cache_value(cache, key);
    free(key);
    if (value == nullptr) {
        return nullptr;
    }

    char* buffer = nullptr;
    size_t bufsize = 0;

    FILE* memfile = open_memstream(&buffer, &bufsize);

    fwrite(value, sizeof(char), len, memfile);
    fseek(memfile, 0L, SEEK_SET);

    auto data = readCacheData(memfile, logmalloclist, userdefinedtypelist, protocolVersion);
    fclose(memfile);
    free(buffer);

    return data;
}

#endif // NOLIBMEMCACHED
