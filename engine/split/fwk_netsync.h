// high-level, socket-less networking api. inspired by Quake, MPI and RenderBuckets theories.
// - rlyeh, public domain
//
// Usage:
// 1. configure networked memory buffers with flags (world, player1, player2, etc). network_buffer();
// 2. then during game loop:
//    - modify your buffers as much as needed.
//    - sync buffers at least once per frame. network_sync();
//    - render your world
// 3. optionally, monitor network status & variables. network_get();
//
// @todo: maybe network_send(msg) + msg *network_recv(); instead of event queue of network_sync() ?

//enum { NETWORK_HANDSHAKE, NETWORK_ENCRYPT, NETWORK_VERSIONED, NETWORK_CHECKSUM }; // negotiation
//enum { NETWORK_TCP, NETWORK_UDP, NETWORK_KCP, NETWORK_ENET, NETWORK_WEBSOCKET }; // transport, where

enum { NETWORK_BIND = 2, NETWORK_CONNECT = 4, NETWORK_NOFAIL = 8 };
API void   network_create(unsigned max_clients, const char *ip, const char *port, unsigned flags); // both ip and port can be null

//enum { NETWORK_LOSSY, NETWORK_COMPRESS }; // post-processes
//enum { NETWORK_PREDICT, NETWORK_RECONCILE, NETWORK_INTERPOLATE, NETWORK_COMPENSATE }; // time authority, when
//enum { NETWORK_LAGS, NETWORK_DROPS, NETWORK_THROTTLES, NETWORK_DUPES }; // quality sim, how much
//enum { NETWORK_CONST = 1, NETWORK_64,NETWORK_32,NETWORK_16,NETWORK_8, NETWORK_FLT, NETWORK_STR, NETWORK_BLOB }; // type, what
enum { NETWORK_SEND = 2, NETWORK_RECV = 4 };
enum { NETWORK_UNRELIABLE = 8, NETWORK_UNORDERED = 16/*, NETWORK_PRIORITY = 32*/ };
API void*   network_buffer(void *ptr, unsigned sz, uint64_t flags, int64_t rank); // configures a shared/networked buffer
API char**  network_sync(unsigned timeout_ms); // syncs all buffers & returns null-terminated list of network events

enum {
    NETWORK_EVENT_CONNECT,
    NETWORK_EVENT_DISCONNECT,
    NETWORK_EVENT_RECEIVE,
    NETWORK_EVENT_DISCONNECT_TIMEOUT,

    /* offset from internal networking events */
    NETWORK_EVENT_RPC = 10,
    NETWORK_EVENT_RPC_RESP,
};

/* errcode and errstr are optional arguments, pass NULL to ignore them,
    errstr is filled by va() */
API int network_event(const char *msg, int *errcode, char **errstr);

enum { NETWORK_RANK = 0 }; // [0..N] where 0 is server
enum { NETWORK_PING = 1 }; // NETWORK_BANDWIDTH, NETWORK_QUALITY };
enum { NETWORK_PORT = 2, NETWORK_IP, NETWORK_LIVE };
enum { NETWORK_SEND_MS = 4 };
enum { NETWORK_BUF_CLEAR_ON_JOIN = 5 };
enum { NETWORK_USERID = 7, /*NETWORK_SALT,*/ NETWORK_COUNT/*N users*/ /*...*/, NETWORK_CAPACITY };
API int64_t network_get(uint64_t key);
API int64_t network_put(uint64_t key, int64_t value);

API void network_rpc(const char *signature, void *function);
API void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline);
API void network_rpc_send(unsigned id, const char *cmdline);

// -----------------------------------------------------------------------------
// low-level api (sockets based)

API bool   server_bind(int max_clients, int port);
API char** server_poll(unsigned timeout_ms);
API char** client_poll(unsigned timeout_ms);
API void   server_broadcast_bin_flags(const void *ptr, int len, uint64_t flags);
API void   server_broadcast_bin(const void *ptr, int len);
API void   server_broadcast_flags(const char *msg, uint64_t flags);
API void   server_broadcast(const char *msg);
API void   server_terminate();
API void   server_send(int64_t handle, const char *msg);
API void   server_send_bin(int64_t handle, const void *ptr, int len);
API void   server_drop(int64_t handle);

API int64_t  client_join(const char *ip, int port);
#define      client_send_flags(msg,flags) server_broadcast(msg, flags)
#define      client_send(msg) server_broadcast(msg)
#define      client_send_bin_flags(ptr,len,flags) server_broadcast_bin(ptr, len, flags)
#define      client_send_bin(ptr,len) server_broadcast_bin(ptr, len)
#define      client_terminate() server_terminate()

#define ANYHOST_IPV4 "0.0.0.0"
#define ANYHOST_IPV6 "::0"

#define LOCALHOST_IPV4 "127.0.0.1"
#define LOCALHOST_IPV6 "::1"
