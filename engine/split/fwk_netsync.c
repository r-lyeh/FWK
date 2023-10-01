typedef void* (*rpc_function)();

typedef struct rpc_call {
    char *method;
    rpc_function function;
    uint64_t function_hash;
} rpc_call;

#define RPC_SIGNATURE_i_iii    UINT64_C(0x78409099752fa48a)
#define RPC_SIGNATURE_i_ii     UINT64_C(0x258290edf43985a5)
#define RPC_SIGNATURE_i_s      UINT64_C(0xf7b73162829ed667)
#define RPC_SIGNATURE_s_s      UINT64_C(0x97deedd17d9afb12)
#define RPC_SIGNATURE_s_v      UINT64_C(0x09c16a1242049b80)
#define RPC_SIGNATURE_v_s      UINT64_C(0xc1746990ab73ed24)

static
rpc_call rpc_new_call(const char *signature, rpc_function function) {
    if( signature && function ) {
        array(char*)tokens = strsplit(signature, "(,)");
        if( array_count(tokens) >= 1 ) {
            char *method = strrchr(tokens[0], ' ')+1;
            char *rettype = va("%.*s", (int)(method - tokens[0] - 1), tokens[0]);
            int num_args = array_count(tokens) - 1;
            char* hash_sig = va("%s(%s)", rettype, num_args ? (array_pop_front(tokens), strjoin(tokens, ",")) : "void");
            uint64_t hash = hash_str(hash_sig);
            method = va("%s%d", method, num_args );
#if RPC_DEBUG
            printf("%p %p %s `%s` %s(", function, (void*)hash, rettype, hash_sig, method); for(int i = 0, end = array_count(tokens); i < end; ++i) printf("%s%s", tokens[i], i == (end-1)? "":", "); puts(");");
#endif
            return (rpc_call) { STRDUP(method), function, hash }; // LEAK
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
            case RPC_SIGNATURE_i_iii: return va("%d %d", id, (int)(intptr_t)found->function(atoi(args[0]), atoi(args[1]), atoi(args[2])) );
            case RPC_SIGNATURE_i_ii:  return va("%d %d", id, (int)(intptr_t)found->function(atoi(args[0]), atoi(args[1])) );
            case RPC_SIGNATURE_i_s:   return va("%d %d", id, (int)(intptr_t)found->function(args[0]) );
            case RPC_SIGNATURE_s_s:   return va("%d %s", id, (char*)found->function(args[0]) );
            case RPC_SIGNATURE_s_v:   return va("%d %s", id, (char*)found->function() );
            case RPC_SIGNATURE_v_s:   return found->function(args[0]), va("%d", id);
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

struct peer_node_t {
    int64_t id;
    struct peer_node_t *next;
};

static ENetHost *Server;
static map(ENetPeer *, int64_t) clients;
static map(int64_t, ENetPeer *) peers;
static int64_t next_client_id = 1; // assumes ID 0 is server
static struct peer_node_t *next_free_id = NULL;
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
    struct peer_node_t *node = C_CAST(struct peer_node_t *, CALLOC(sizeof(struct peer_node_t), 1));
    node->id = *(int64_t *)map_find(clients, peer);
    if (!next_free_id) {
        next_free_id = node;
    } else {
        node->next = next_free_id;
        next_free_id = node;
    }
    map_erase(peers, *(int64_t *)map_find(clients, peer));
    map_erase(clients, peer);
}

void server_broadcast_bin_flags(const void *msg, int len, uint64_t flags) {
    ENetPacket *packet = enet_packet_create(msg, len, flags&NETWORK_UNRELIABLE ? ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT : ENET_PACKET_FLAG_RELIABLE | flags&(NETWORK_UNRELIABLE|NETWORK_UNORDERED) ? ENET_PACKET_FLAG_UNSEQUENCED : 0);
    enet_host_broadcast(Server, 0, packet);
}

void server_broadcast_bin(const void *msg, int len) {
    ENetPacket *packet = enet_packet_create(msg, len, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(Server, 0, packet);
    //enet_host_flush(Server); // flush if needed
}
void server_broadcast_flags(const char *msg, uint64_t flags) {
    server_broadcast_bin_flags(msg, strlen(msg)+1, flags);
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
    enet_address_set_host(&address, !strcmp(ip, "localhost") ? "127.0.0.1" : ip);
    address.port = port;

    ENetHost *host = enet_host_create(NULL, 1 /*outgoing connections*/, 2 /*channels*/, 0 /*in bandwidth*/, 0 /*out bandwidth*/);
    if(!host) return -1;
    ENetPeer *peer = enet_host_connect(host, &address, 2, 0);
    if(!peer) return -1;
    Server = host;

    ENetEvent event;
    bool client_join_connected = enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT;
    if(!client_join_connected) { enet_peer_reset(peer); return -1; }

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
        enet_packet_destroy( event.packet );
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
    uint64_t flags;
} netbuffer_t;

static array(char*) events; // @todo: make event 128 bytes max?
static array(int64_t) values; // @todo: map<key,values> instead?
static map( int64_t, array(netbuffer_t) ) buffers; // map<client,array<netbuffer>>
static double msg_send_cooldown = 0.0;
static double network_dt = 0.0;
static double last_netsync = 0.0;

void network_create(unsigned max_clients, const char *ip, const char *port_, unsigned flags) {
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
    network_put(NETWORK_COUNT, 0);
    network_put(NETWORK_CAPACITY, max_clients);
    network_put(NETWORK_BUF_CLEAR_ON_JOIN, 1);

    if( !(flags&NETWORK_CONNECT) || flags&NETWORK_BIND ) {
        // server, else client
        PRINTF("Trying to bind server, else we connect as a client...\n");
        network_put(NETWORK_RANK, 0);
        if( server_bind(max_clients, port) ) {
            network_put(NETWORK_LIVE, 1);
            PRINTF("Server bound\n");
        } else {
            network_put(NETWORK_RANK, -1); /* unassigned until we connect successfully */
            int64_t socket = client_join(ip, port);
            if( socket >= 0 ) {
                PRINTF("Client connected, id %d\n", (int)socket);
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
            PRINTF("Client connected, id %d\n", (int)socket);
            network_put(NETWORK_LIVE, 1);
            network_put(NETWORK_RANK, socket);
        } else {
            PRINTF("!Client conn failed\n");
            network_put(NETWORK_LIVE, 0);
            if (!(flags&NETWORK_NOFAIL))
                PANIC("cannot connect to server %s:%d", ip, port);
        }
    }

    PRINTF("Network rank:%u ip:%s port:%d\n", (unsigned)network_get(NETWORK_RANK), ip, (int)network_get(NETWORK_PORT));
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

void* network_buffer(void *ptr, unsigned sz, uint64_t flags, int64_t rank) {
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

static
int enet_event_to_netsync(int ev) {
    switch (ev) {
        case ENET_EVENT_TYPE_CONNECT: return NETWORK_EVENT_CONNECT;
        case ENET_EVENT_TYPE_DISCONNECT: return NETWORK_EVENT_DISCONNECT;
        case ENET_EVENT_TYPE_RECEIVE: return NETWORK_EVENT_RECEIVE;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: return NETWORK_EVENT_DISCONNECT_TIMEOUT;
    }

    /* passthrough for our own events */
    return ev;
}

char** network_sync(unsigned timeout_ms) {
    int64_t whoami = network_get(NETWORK_RANK);
    bool is_server = whoami == 0;
    bool is_client = !is_server;
    if(timeout_ms < 2) timeout_ms = 2;

    network_dt = time_ss() - last_netsync;
    last_netsync = time_ss();

    // Split buffers into clients @todo
    // clients need to do this before network polling; servers should do this after polling.
    if (msg_send_cooldown <= 0.0) {
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
        msg_send_cooldown = (double)network_get(NETWORK_SEND_MS)/1000.0;
    } else {
        msg_send_cooldown -= network_dt;
    }

    if (is_server) {
        return server_poll(timeout_ms);
    } else {
        return client_poll(timeout_ms);
    }
}


char** server_poll(unsigned timeout_ms) {
    array_clear(events);
    if(timeout_ms < 2) timeout_ms = 2;

    // network poll
    for( ENetEvent event; Server && enet_host_service(Server, &event, timeout_ms) > 0; ) {
        char *msg = 0;
        char ip[128]; enet_peer_get_ip(event.peer, ip, 128);

        switch (event.type) {
            default: // case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:;
                /* ensure we have free slot for client */
                if (map_count(clients) >= network_get(NETWORK_CAPACITY)-1) {
                    msg = va("%d Server is at maximum capacity, disconnecting the peer (::%s:%u)...", 1, ip, event.peer->address.port);
                    enet_peer_disconnect_now(event.peer, 1);
                    break;
                }

                int64_t client_id = -1;

                if (next_free_id) {
                    struct peer_node_t *node = next_free_id;
                    client_id = next_free_id->id;
                    next_free_id = next_free_id->next;
                    FREE(node);
                }
                else client_id = next_client_id++;

                // if (network_get(NETWORK_BUF_CLEAR_ON_JOIN)) {
                //     array(netbuffer_t) *list = map_find(buffers, client_id);

                //     if (list)
                //     for(int i = 0, end = array_count(list); i < end; ++i) {
                //         netbuffer_t *nb = &list[i];
                //         memset(nb->ptr, 0, nb->sz);
                //     }
                // }

                map_find_or_add(clients, event.peer, client_id);
                map_find_or_add(peers, client_id, event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)+1);


                // send server slot
                char init_msg[12];
                *(uint32_t*)&init_msg[0] = MSG_INIT;
                *(int64_t*)&init_msg[4] = client_id;
                server_send_bin(client_id, init_msg, 12);
                PRINTF("Client rank %u for peer ::%s:%u\n", (unsigned)client_id, ip, event.peer->address.port);
                msg = va( "%d new client rank:%u from ::%s:%u", 0, (unsigned)client_id, ip, event.peer->address.port );
                event.peer->data = (void*)client_id;
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
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
                case MSG_BUF: {
                    uint64_t *flags = (uint64_t*)(ptr + 0);
                    uint32_t *idx = (uint32_t*)(ptr + 8);
                    uint32_t *len = (uint32_t*)(ptr + 12);
                    uint64_t *who = (uint64_t*)(ptr + 16);
                    ptr += 24;

                    // validate if peer owns the buffer
                    int64_t *cid = map_find(clients, event.peer);
                    uint8_t client_valid = cid ? *cid == *who : 0;

                    // apply incoming packet.
                    if( client_valid ) {
                        array(netbuffer_t) *list = map_find(buffers, *who);
                        assert( list );
                        assert( *idx < array_count(*list) );
                        netbuffer_t *nb = &(*list)[*idx];
                        assert( *len == nb->sz );
                        memcpy(nb->ptr, ptr, *len);
                    }
                } break;
                case MSG_RPC: {
                    event.type = NETWORK_EVENT_RPC;
                    unsigned id = *(uint32_t*)ptr; ptr += 4;
                    char *cmdline = ptr;
                    char *resp = rpc(id, cmdline);
                    char *resp_msg = va("%*.s%s", 4, "", resp);
                    *(uint32_t*)&resp_msg[0] = MSG_RPC_RESP;
                    ENetPacket *packet = enet_packet_create(resp_msg, strlen(resp) + 5, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    msg = va("%d req:%s res:%s", 0, cmdline, resp);
                } break;
                case MSG_RPC_RESP: {
                    event.type = NETWORK_EVENT_RPC_RESP;
                    msg = va("%d %s", 0, va("%s", ptr));
                } break;
                default:
                    msg = va("%d unk msg len:%u from rank:%u ::%s:%u", -1, sz, (unsigned)(uintptr_t)event.peer->data, ip, event.peer->address.port); /* @TODO: hexdump? */
                    break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
            } break;

            case ENET_EVENT_TYPE_DISCONNECT:
                msg = va( "%d disconnect rank:%u", 0, (unsigned)(uintptr_t)event.peer->data);
                /* Reset the peer's client information. */
                FREE(event.peer->data);
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)-1);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                msg = va( "%d timeout rank:%u", 0, (unsigned)(uintptr_t)event.peer->data);
                FREE(event.peer->data);
                event.peer->data = NULL;
                server_drop_client_peer(event.peer);
                network_put(NETWORK_COUNT, network_get(NETWORK_COUNT)-1);
                break;
        }

        if(msg) array_push(events, va("%d %s", enet_event_to_netsync(event.type), msg));
    }

    array_push(events, NULL);
    return events;
}

char** client_poll(unsigned timeout_ms) {
    array_clear(events);
    int64_t whoami = network_get(NETWORK_RANK);
    if(timeout_ms < 2) timeout_ms = 2;

    // network poll
    for( ENetEvent event; Server && enet_host_service(Server, &event, timeout_ms) > 0; ) {
        char *msg = 0;
        char ip[128]; enet_peer_get_ip(event.peer, ip, 128);

        switch (event.type) {
            default: // case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                char *dbg = (char *)event.peer->data;
                char *ptr = (char *)event.packet->data;
                unsigned sz = (unsigned)event.packet->dataLength;
                unsigned id = (unsigned)event.channelID;

                // decapsulate incoming packet.
                uint32_t mid = *(uint32_t*)(ptr + 0);
                ptr += 4;

                switch (mid) {
                case MSG_INIT:
                    /* handled by client_join */
                    break;
                case MSG_BUF: {
                    uint64_t *flags = (uint64_t*)(ptr + 0);
                    uint32_t *idx = (uint32_t*)(ptr + 8);
                    uint32_t *len = (uint32_t*)(ptr + 12);
                    uint64_t *who = (uint64_t*)(ptr + 16);
                    ptr += 24;

                    // apply incoming packet.
                    if( *who != whoami ) {
                        array(netbuffer_t) *list = map_find(buffers, *who);
                        assert( list );
                        assert( *idx < array_count(*list) );
                        netbuffer_t *nb = &(*list)[*idx];
                        assert( *len == nb->sz );
                        memcpy(nb->ptr, ptr, *len);
                    }
                } break;
                case MSG_RPC: {
                    event.type = NETWORK_EVENT_RPC;
                    unsigned id = *(uint32_t*)ptr; ptr += 4;
                    char *cmdline = ptr;
                    char *resp = rpc(id, cmdline);
                    char *resp_msg = va("%*.s%s", 4, "", resp);
                    *(uint32_t*)&resp_msg[0] = MSG_RPC_RESP;
                    ENetPacket *packet = enet_packet_create(resp_msg, strlen(resp) + 5, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    msg = va("%d req:%s res:%s", 0, cmdline, resp);
                } break;
                case MSG_RPC_RESP: {
                    event.type = NETWORK_EVENT_RPC_RESP;
                    msg = va("%d %s", 0, ptr);
                } break;
                default:
                    msg = va("%d unk msg len:%u from server", -1, sz); /* @TODO: hexdump? */
                    break;
                }
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy( event.packet );
            } break;

            case ENET_EVENT_TYPE_DISCONNECT:
                msg = va( "%d disconnect", 0 );
                /* Reset the peer's client information. */
                FREE(event.peer->data);
                event.peer->data = NULL;
                network_put(NETWORK_RANK, -1);
                network_put(NETWORK_LIVE, 0);
                break;

            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                msg = va( "%d timeout", 0);
                FREE(event.peer->data);
                event.peer->data = NULL;
                network_put(NETWORK_RANK, -1);
                network_put(NETWORK_LIVE, 0);
                break;
        }

        if(msg) array_push(events, va("%d %s", enet_event_to_netsync(event.type), msg));
    }

    array_push(events, NULL);
    return events;
}

int network_event(const char *msg, int *errcode, char **errstr) {
    int evid = -1;
    int err = 0;
    char errbuf[128] = {0};
    sscanf(msg, "%d %d %127[^\r\n]", &evid, &err, errbuf);
    if (errcode) *errcode = err;
    if (errstr) *errstr = va("%s", errbuf);
    return evid;
}

void network_rpc(const char *signature, void *function) {
    rpc_insert(signature, function);
}

void network_rpc_send_to(int64_t rank, unsigned id, const char *cmdline) {
    assert(network_get(NETWORK_RANK) == 0); /* must be a host */
    char *msg = va("%*.s%s", 8, "", cmdline);
    unsigned sz = strlen(cmdline) + 9;
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_send_bin(rank, msg, sz);
}

void network_rpc_send(unsigned id, const char *cmdline) {
    char *msg = va("%*.s%s", 8, "", cmdline);
    unsigned sz = strlen(cmdline) + 9;
    *(uint32_t*)&msg[0] = MSG_RPC;
    *(uint32_t*)&msg[4] = id;
    server_broadcast_bin(msg, sz);
}
