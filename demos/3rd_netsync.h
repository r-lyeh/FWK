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
enum { MAX_CLIENTS = 32 };
API void   network_create(const char *ip, const char *port, unsigned flags); // both ip and port can be null

//enum { NETWORK_LOSSY, NETWORK_COMPRESS }; // post-processes
//enum { NETWORK_UNRELIABLE, NETWORK_UNORDERED, NETWORK_PRIORITY }; // how
//enum { NETWORK_PREDICT, NETWORK_RECONCILE, NETWORK_INTERPOLATE, NETWORK_COMPENSATE }; // time authority, when
//enum { NETWORK_LAGS, NETWORK_DROPS, NETWORK_THROTTLES, NETWORK_DUPES }; // quality sim, how much
//enum { NETWORK_CONST = 1, NETWORK_64,NETWORK_32,NETWORK_16,NETWORK_8, NETWORK_FLT, NETWORK_STR, NETWORK_BLOB }; // type, what
enum { NETWORK_SEND = 2, NETWORK_RECV = 4 };
API void*   network_buffer(void *ptr, unsigned sz, unsigned flags, int64_t rank); // configures a shared/networked buffer
API char**  network_sync(unsigned timeout_ms); // syncs all buffers & returns null-terminated list of network events

enum { NETWORK_RANK = 0 }; // [0..N] where 0 is server
enum { NETWORK_PING = 1 }; // NETWORK_BANDWIDTH, NETWORK_QUALITY };
enum { NETWORK_PORT = 2, NETWORK_IP, NETWORK_LIVE };
//enum {  NETWORK_USERID, NETWORK_SALT, NETWORK_COUNT/*N users*/ /*...*/,
API int64_t network_get(uint64_t key);
API int64_t network_put(uint64_t key, int64_t value);

API void network_rpc(const char *signature, void *function);
API void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline);
API void network_rpc_send(unsigned id, const char *cmdline);

// -----------------------------------------------------------------------------
// low-level api (sockets based)

API bool server_bind(int max_clients, int port);
API void server_poll();
API void server_broadcast_bin(const void *ptr, int len);
API void server_broadcast(const char *msg);
API void server_terminate();
API void server_send(int64_t handle, const char *msg);
API void server_send_bin(int64_t handle, const void *ptr, int len);
API void server_drop(int64_t handle);

API int64_t  client_join(const char *ip, int port);
#define      client_send(msg) server_broadcast(msg)
#define      client_send_bin(ptr,len) server_broadcast_bin(ptr, len)
#define      client_terminate() server_terminate()

#define ANYHOST_IPV4 "0.0.0.0"
#define ANYHOST_IPV6 "::0"

#define LOCALHOST_IPV4 "127.0.0.1"
#define LOCALHOST_IPV6 "::1"

// -----------------------------------------------------------------------------
// implementation

typedef void* (*rpc_function)();

typedef struct rpc_call {
    char *method;
    rpc_function function;
    uint64_t function_hash;
} rpc_call;

#define RPC_SIGNATURE_i_iii UINT64_C(0x78409099752fa48a) // printf("%llx\n, HASH_STR("int(int,int,int)"));
#define RPC_SIGNATURE_i_ii  UINT64_C(0x258290edf43985a5) // printf("%llx\n, HASH_STR("int(int,int)"));
#define RPC_SIGNATURE_s_s   UINT64_C(0x97deedd17d9afb12) // printf("%llx\n, HASH_STR("char*(char*)"));
#define RPC_SIGNATURE_s_v   UINT64_C(0x09c16a1242049b80) // printf("%llx\n, HASH_STR("char*(void)"));

static
rpc_call rpc_new_call(const char *signature, rpc_function function) {
    if( signature && function ) {
        array(char*)tokens = strsplit(signature, "(,)");
        if( array_count(tokens) >= 1 ) {
            char *method = strrchr(tokens[0], ' ')+1;
            char *rettype = va("%.*s", (int)(method - tokens[0] - 1), tokens[0]);
            int num_args = array_count(tokens) - 1;
            char* hash_sig = va("%s(%s)", rettype, num_args ? ((void)array_pop_front(tokens), strjoin(tokens, ",")) : "void");
            uint64_t hash = hash_str(hash_sig);
            method = va("%s%d", method, num_args );
#if RPC_DEBUG
            printf("%p %p %s `%s` %s(", function, (void*)hash, rettype, hash_sig, method); for(int i = 0, end = array_count(tokens); i < end; ++i) printf("%s%s", tokens[i], i == (end-1)? "":", "); puts(");");
#endif
            return (rpc_call) { strdup(method), function, hash }; // LEAK
        }
    }
    return (rpc_call) {0};
}

static map(char*, rpc_call) rpc_calls = 0;

static
void rpc_insert(const char *signature, void *function ) {
    rpc_call call = rpc_new_call(signature, function);
    if( call.method ) {
        if( !rpc_calls ) map_init(rpc_calls, less_str, hash_str);
        if( map_find(rpc_calls, call.method)) {
            map_erase(rpc_calls, call.method);
        }
        map_insert(rpc_calls, call.method, call);
    }
}

static
char *rpc_full(unsigned id, const char* method, unsigned num_args, char *args[]) {
#if RPC_DEBUG
    printf("id:%x method:%s args:", id, method );
    for( int i = 0; i < num_args; ++i ) printf("%s,", args[i]); puts("");
#endif

    method = va("%s%d", method, num_args);
    rpc_call *found = map_find(rpc_calls, (char*)method);
    if( found ) {
        switch(found->function_hash) {
            case RPC_SIGNATURE_i_iii: return va("%d %d", id, (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1]), atoi(args[2])) );
            case RPC_SIGNATURE_i_ii:  return va("%d %d", id, (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1])) );
            case RPC_SIGNATURE_s_s:   return va("%d %s", id, (char*)found->function(args[0]) );
            case RPC_SIGNATURE_s_v:   return va("%d %s", id, (char*)found->function() );
            default: break;
        }
    }
    return va("%d -1", id);
}

static
array(char*) rpc_parse_args( const char *cmdline, bool quote_whitespaces ) { // parse cmdline arguments. must array_free() after use
    // - supports quotes: "abc" "abc def" "abc \"def\"" "abc \"def\"""ghi" etc.
    // - #comments removed
    array(char*) args = 0; // LEAK
    for( int i = 0; cmdline[i]; ) {
        char buf[256] = {0}, *ptr = buf;
        while(cmdline[i] && isspace(cmdline[i])) ++i;
        bool quoted = cmdline[i] == '\"';
        if( quoted ) {
            while(cmdline[++i]) {
                char ch = cmdline[i];
                /**/ if (ch == '\\' && cmdline[i + 1] == '\"') *ptr++ = '\"', ++i;
                else if (ch == '\"' && cmdline[i + 1] == '\"') ++i;
                else if (ch == '\"' && (!cmdline[i + 1] || isspace(cmdline[i + 1]))) {
                    ++i; break;
                }
                else *ptr++ = ch;
            }
        } else {
            while(cmdline[i] && !isspace(cmdline[i])) *ptr++ = cmdline[i++];
        }
        if (buf[0] && buf[0] != '#') { // exclude empty args + comments
            if( quote_whitespaces && quoted )
            array_push(args, va("\"%s\"",buf));
            else
            array_push(args, va("%s",buf));
        }
    }
    return args;
}

static
char* rpc(unsigned id, const char* cmdline) {
    array(char*) args = rpc_parse_args(cmdline, false);
    int num_args = array_count(args);
    char *ret = num_args ? rpc_full(id, args[0], num_args - 1, &args[1]) : rpc_full(id, "", 0, NULL);
    array_free(args);
    return ret;
}

static void enet_quit(void) {
    do_once {
        // enet_deinitialize();
    }
}
static void enet_init() {
    do_once {
        if( enet_initialize() != 0 ) {
            PANIC("cannot initialize enet");
        }
        atexit( enet_quit );
    }
}

static ENetHost *Server;
static map(ENetPeer *, int64_t) clients;
static map(int64_t, ENetPeer *) peers;
static int64_t next_client_id = 1; // assumes ID 0 is server
enum { MSG_INIT, MSG_BUF, MSG_RPC, MSG_RPC_RESP };

bool server_bind(int max_clients, int port) {
    map_init(clients, less_64, hash_64);
    map_init(peers, less_64, hash_64);
    assert(port == 0 || (port > 1024 && port < 65500));
    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = port;
    Server = enet_host_create(&address, max_clients, 2 /*channels*/, 0 /*in bandwidth*/, 0 /*out bandwidth*/);
    return Server != NULL;
}

static
void server_drop_client(int64_t handle) {
    map_erase(clients, *(ENetPeer **)map_find(peers, handle));
    map_erase(peers, *(int64_t *)handle);
}

static
void server_drop_client_peer(ENetPeer *peer) {
    map_erase(peers, *(int64_t *)map_find(clients, peer));
    map_erase(clients, peer);
}

void server_poll() {
    ENetEvent event;
    while( enet_host_service(Server, &event, 2 /*timeout,ms*/) > 0 ) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:;
                char ip[128]; enet_peer_get_ip(event.peer, ip, 128);
                PRINTF( "A new client connected from ::%s:%u.\n", ip, event.peer->address.port );
                /* Store any relevant client information here. */
                event.peer->data = "Client information";

                int64_t client_id = next_client_id++;
                map_find_or_add(clients, event.peer, client_id);
                map_find_or_add(peers, client_id, event.peer);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                PRINTF( "A packet of length %zu containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        (char *)event.peer->data,
                        event.channelID );

                char *dbg = (char *)event.peer->data;
                char *ptr = event.packet->data;
                unsigned sz = event.packet->dataLength;

                uint32_t mid = *(uint32_t*)ptr;
                ptr += 4;

                // @todo: propagate event to user
                switch (mid) {
                case MSG_INIT: {
                    uint64_t *cid = map_find(clients, event.peer);
                    if (cid) {
                        char init_msg[12];
                        *(uint32_t*)&init_msg[0] = MSG_INIT;
                        *(uint64_t*)&init_msg[4] = *cid;
                        ENetPacket *packet = enet_packet_create(init_msg, 12, ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(event.peer, 0, packet);
                    } else {
                        PRINTF("ignoring unk MSG_INIT client packet.\n");
                    }
                } break;
                case MSG_RPC:
                case MSG_RPC_RESP:
                    // @todo: process and send a response back
                    break;
                default:
                    PRINTF("recving unk %d sz %d from peer %s\n", mid, sz, dbg);
                }

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                PRINTF( "%s disconnected.\n", (char *)event.peer->data );
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PRINTF( "%s timeout.\n", (char *)event.peer->data );
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                break;

            case ENET_EVENT_TYPE_NONE: break;
        }
    }
}

void client_poll() {
    ENetEvent event;
    while( enet_host_service(Server, &event, 2 /*timeout,ms*/) > 0 ) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                PRINTF( "A packet of length %zu containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        (char *)event.peer->data,
                        event.channelID );

                char *dbg = (char *)event.peer->data;
                char *ptr = event.packet->data;
                unsigned sz = event.packet->dataLength;

                uint32_t mid = *(uint32_t*)ptr;
                ptr += 4;

                // @todo: propagate event to user
                switch (mid) {
                case MSG_INIT:
                    /* handled during client_join */
                    break;
                case MSG_RPC:
                case MSG_RPC_RESP:
                    // @todo: process and send a response back
                    break;
                default:
                    PRINTF("recving unk %d sz %d from peer %s\n", mid, sz, dbg);
                    }

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                PRINTF( "%s disconnected.\n", (char *)event.peer->data );
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PRINTF( "%s timeout.\n", (char *)event.peer->data );
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                break;

            case ENET_EVENT_TYPE_NONE: break;
        }
    }
}

void server_broadcast_bin(const void *msg, int len) {
    ENetPacket *packet = enet_packet_create(msg, len, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server, 0, packet);
    //enet_host_flush(Server); // flush if needed
}
void server_broadcast(const char *msg) {
    server_broadcast_bin(msg, strlen(msg)+1);
}
void server_terminate() {
    enet_host_destroy(Server);
    Server = 0;
}

volatile int client_join_connected = 0;
static int client_join_threaded(void *userdata) {
    ENetHost *host = (ENetHost *)userdata;

    ENetPacket *packet = enet_packet_create("", 1, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server, 0, packet);

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    ENetEvent event;
    client_join_connected = 0;
    client_join_connected = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
    return 0;
}

int64_t client_join(const char *ip, int port) {
    assert(port > 1024 && port < 65500);
    ENetAddress address = {0};
//  address.host = ENET_HOST_ANY;
    enet_address_set_host(&address, !strcmp(ip, "localhost") ? "127.0.0.1" : ip);
    address.port = port;

    ENetHost *host = enet_host_create(NULL, 1 /*outgoing connections*/, 2 /*channels*/, 0 /*in bandwidth*/, 0 /*out bandwidth*/);
    if(!host) return -1;
    ENetPeer *peer = enet_host_connect(host, &address, 2, 0);
    if(!peer) return -1;
    Server = host;

#if 1
#if 0
    // sync wait (not working in localhost, unless threaded)
    thread_ptr_t th = thread_init(client_join_threaded, host, "client_join_threaded()", 0 );
    thread_join( th );
    thread_destroy( th );
#else
    ENetEvent event;
    bool client_join_connected = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
#endif
    if(!client_join_connected) { enet_peer_reset(peer); return -1; }
#endif

    // ask for server slot
    char init_msg[4]; *(uint32_t*)init_msg = MSG_INIT;
    server_broadcast_bin(init_msg, sizeof(init_msg));

    // wait for the response
    bool msg_received = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE;
    if (!msg_received) { enet_peer_reset(peer); return -1; }

    char *ptr = (char *)event.packet->data;
    int64_t cid = -1;

    // decapsulate incoming packet.
    uint32_t mid = *(uint32_t*)(ptr + 0);
    ptr += 4;

    switch (mid) {
    case MSG_INIT:
        cid = *(int64_t*)ptr;
        break;
    default:
        enet_peer_reset(peer);
        return -1;
    }

    /* Clean up the packet now that we're done using it. */
    enet_packet_destroy( event.packet );

    return cid;
}
void server_drop(int64_t handle) {
    enet_peer_disconnect_now(*(ENetPeer **)map_find(peers, handle), 0);
    server_drop_client(handle);
}

void server_send_bin(int64_t handle, const void *ptr, int len) {
    ENetPacket *packet = enet_packet_create(ptr, len, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(*(ENetPeer **)map_find(peers, handle), 0, packet);
}

void server_send(int64_t handle, const char *msg) {
    server_send_bin(handle, msg, strlen(msg)+1);
}

// ---

typedef struct netbuffer_t {
    int64_t owner;
    void *ptr;
    unsigned sz;
    unsigned flags;
} netbuffer_t;

static array(char*) events; // @todo: make event 128 bytes max?
static array(int64_t) values; // @todo: map<key,values> instead?
static map( int64_t, array(netbuffer_t) ) buffers; // map<client,array<netbuffer>>

void network_create(const char *ip, const char *port_, unsigned flags) {
    if (buffers) map_clear(buffers);
    do_once {
        array_resize(values, 128);
        map_init(buffers, less_64, hash_64);

        enet_init();
    }

    ip = ip ? ip : "0.0.0.0";
    int port = atoi(port_ ? port_ : "1234");

    // network_put(NETWORK_IP, 0x7F000001); // 127.0.0.1
    network_put(NETWORK_PORT, port);
    network_put(NETWORK_LIVE, -1);

    if( !(flags&NETWORK_CONNECT) || flags&NETWORK_BIND ) {
        // server, else client
        PRINTF("Trying to bind server, else we connect as a client...\n");
        network_put(NETWORK_RANK, 0);
        if( server_bind(MAX_CLIENTS, port) ) {
            network_put(NETWORK_LIVE, 1);
            PRINTF("Server bound\n");
        } else {
            network_put(NETWORK_RANK, -1); /* unassigned until we connect successfully */
            int64_t socket = client_join(ip, port);
            if( socket >= 0 ) {
                PRINTF("Client connected, id %lld\n", socket);
                network_put(NETWORK_LIVE, 1);
                network_put(NETWORK_RANK, socket);
            } else {
                PRINTF("!Client conn failed\n");
                network_put(NETWORK_LIVE, 0);

                if (!(flags&NETWORK_NOFAIL))
                    PANIC("cannot neither connect to %s:%d, nor create a server", ip, port);
            }
        }
    } else {
        // client only
        PRINTF("Connecting to server...\n");
        network_put(NETWORK_RANK, -1); /* unassigned until we connect successfully */
        int64_t socket = client_join(ip, port);
        if( socket > 0 ) {
            PRINTF("Client connected, id %lld\n", socket);
            network_put(NETWORK_LIVE, 1);
            network_put(NETWORK_RANK, socket);
        } else {
            PRINTF("!Client conn failed\n");
            network_put(NETWORK_LIVE, 0);
            if (!(flags&NETWORK_NOFAIL))
                PANIC("cannot connect to server %s:%d", ip, port);
        }
    }

    PRINTF("Network rank:%lld ip:%s port:%lld\n", network_get(NETWORK_RANK), ip, network_get(NETWORK_PORT));
}

int64_t network_put(uint64_t key, int64_t value) {
    int64_t *found = key < array_count(values) ? &values[key] : NULL;
    if(found) *found = value;
    return value;
}
int64_t network_get(uint64_t key) {
    int64_t *found = key < array_count(values) ? &values[key] : NULL;
    return found ? *found : 0;
}

void* network_buffer(void *ptr, unsigned sz, unsigned flags, int64_t rank) {
    assert(flags);
    array(netbuffer_t) *found = map_find_or_add(buffers, rank, NULL);

    netbuffer_t nb;
    nb.owner = rank;
    nb.ptr = ptr;
    nb.sz = sz;
    nb.flags = flags;
    array_push(*found, nb);

    return ptr;
}

char** network_sync(unsigned timeout_ms) {
    array_clear(events);

    int64_t whoami = network_get(NETWORK_RANK);
    bool is_server = whoami == 0;
    bool is_client = !is_server;
    if(timeout_ms < 2) timeout_ms = 2;
    // sleep_ms(timeout_ms); // @fixme. server only?

    // Split buffers into clients @todo
    // clients need to do this before network polling; servers should do this after polling.
    map_foreach(buffers, int64_t, rank, array(netbuffer_t), list) {
        for(int i = 0, end = array_count(list); i < end; ++i) {
            netbuffer_t *nb = &list[i];
            if (!is_server && !(nb->flags & NETWORK_SEND))
                continue;
            static array(char) encapsulate;
            array_resize(encapsulate, nb->sz + 28);
            uint32_t *mid = (uint32_t*)&encapsulate[0]; *mid = MSG_BUF;
            uint64_t *st = (uint64_t*)&encapsulate[4]; *st = nb->flags;
            uint32_t *idx = (uint32_t*)&encapsulate[12]; *idx = i;
            uint32_t *len = (uint32_t*)&encapsulate[16]; *len = nb->sz;
            uint64_t *who = (uint64_t*)&encapsulate[20]; *who = nb->owner;
            // PRINTF("sending %llx %u %lld %u\n", *st, *idx, *who, *len);
            memcpy(&encapsulate[28], nb->ptr, nb->sz);
            server_broadcast_bin(&encapsulate[0], nb->sz + 28);
        }
    }

    // network poll
    for( ENetEvent event; Server && enet_host_service(Server, &event, timeout_ms) > 0; ) {
        char *msg = 0;
        char ip[128]; enet_peer_get_ip(event.peer, ip, 128);

        switch (event.type) {
            default: // case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:;
                msg = va( "A new client connected from ::%s:%u", ip, event.peer->address.port );
                /* Store any relevant client information here. */
                event.peer->data = "Client information";

                /* ensure we have free slot for client */
                if (map_count(clients) >= MAX_CLIENTS) {
                    msg = va("%s\n", "Server is at maximum capacity, disconnecting the peer...");
                    enet_peer_disconnect_now(event.peer, 1);
                    break;
                }

                int64_t client_id = next_client_id++;
                map_find_or_add(clients, event.peer, client_id);
                map_find_or_add(peers, client_id, event.peer);
                break;

            case ENET_EVENT_TYPE_RECEIVE:;
                /*
                msg = va( "A packet of length %u containing %s was received from %s on channel %u",
                        (unsigned)event.packet->dataLength,
                        event.packet->data,
                        (char *)event.peer->data,
                        event.channelID );
                */
                char *dbg = (char *)event.peer->data;
                char *ptr = (char *)event.packet->data;
                unsigned sz = (unsigned)event.packet->dataLength;
                unsigned id = (unsigned)event.channelID;

                // debug
                // puts(dbg);
                // hexdump(ptr, sz);

                // decapsulate incoming packet.
                uint32_t mid = *(uint32_t*)(ptr + 0);
                ptr += 4;

                switch (mid) {
                case MSG_INIT:
                    if (is_server) {
                        uint64_t *cid = map_find(clients, event.peer);
                        if (cid) {
                            char init_msg[12];
                            *(uint32_t*)&init_msg[0] = MSG_INIT;
                            *(int64_t*)&init_msg[4] = *cid;
                            ENetPacket *packet = enet_packet_create(init_msg, 12, ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(event.peer, 0, packet);
                            PRINTF("Client req id %lld for peer ::%s:%u\n", *cid, ip, event.peer->address.port);
                        } else {
                            PRINTF("!Ignoring unk MSG_INIT client packet.\n");
                        }
                    }
                    break;
                case MSG_BUF: {
                    uint64_t *flags = (uint64_t*)(ptr + 0);
                    uint32_t *idx = (uint32_t*)(ptr + 8);
                    uint32_t *len = (uint32_t*)(ptr + 12);
                    uint64_t *who = (uint64_t*)(ptr + 16);
                    // PRINTF("recving %d %llx %u %u %lld\n", mid, *flags, *idx, *len, *who);
                    ptr += 24;

                    // validate if peer owns the buffer
                    uint8_t client_valid = 0;

                    if (is_server) {
                        int64_t *cid = map_find(clients, event.peer);
                        client_valid = cid ? *cid == *who : 0;
                    }

                    // apply incoming packet.
                    if( is_client ? *who != whoami : client_valid ) { // clients merge always foreign packets. servers merge foreign packets.
                        array(netbuffer_t) *list = map_find(buffers, *who);
                        assert( list );
                        assert( *idx < array_count(*list) );
                        netbuffer_t *nb = &(*list)[*idx];
                        assert( *len == nb->sz );
                        memcpy(nb->ptr, ptr, *len);
                    }
                } break;
                case MSG_RPC: {
                    unsigned id = *(uint32_t*)ptr; ptr += 4;
                    char *cmdline = ptr;
                    char *resp = rpc(id, cmdline);
                    char *resp_msg = va("%*.s%s", 4, "", resp); 
                    *(uint32_t*)&resp_msg[0] = MSG_RPC_RESP;
                    ENetPacket *packet = enet_packet_create(resp_msg, 4 + strlen(resp), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                } break;
                case MSG_RPC_RESP: {
                    // @todo: react on response?
                    msg = ptr;
                } break;
                default:
                    // PRINTF("!Receiving unk %d sz %d from peer ::%s:%u\n", mid, sz, ip, event.peer->address.port);
                    break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                msg = va( "%s disconnected", (char *)event.peer->data );
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                if (is_server) server_drop_client_peer(event.peer);
                else {network_put(NETWORK_RANK, -1); network_put(NETWORK_LIVE, 0);}
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                msg = va( "%s timeout", (char *)event.peer->data );
                event.peer->data = NULL;
                if (is_server) server_drop_client_peer(event.peer);
                else {network_put(NETWORK_RANK, -1); network_put(NETWORK_LIVE, 0);}
                break;
        }

        if(msg) array_push(events, va("%d %s", event.type, msg));
//            if(msg) server_broadcast(msg);
    }

    array_push(events, NULL);
    return events;
}

void network_rpc(const char *signature, void *function) {
    rpc_insert(signature, function);
}

void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline) {
    assert(network_get(NETWORK_RANK) == 0); /* must be a host */
    char *msg = va("%*.s%s", 8, "", cmdline); 
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_send_bin(rank, msg, 8 + strlen(cmdline));
}

void network_rpc_send(unsigned id, const char *cmdline) {
    char *msg = va("%*.s%s", 8, "", cmdline); 
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_broadcast_bin(msg, 8 + strlen(cmdline));
}
