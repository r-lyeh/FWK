
// @fixme: broken with tcc -m64 (our default tcc configuration)
int download_file( FILE *out, const char *url ) {
    int ok = false;
    if( out ) for( https_t *h = https_get(url, NULL); h; https_release(h), h = NULL ) {
        while (https_process(h) == HTTPS_STATUS_PENDING) sleep_ms(1);
        //printf("fetch status%d, %d %s\n\n%.*s\n", https_process(h), h->status_code, h->content_type, (int)h->response_size, (char*)h->response_data);
        if(https_process(h) == HTTPS_STATUS_COMPLETED)
        ok = fwrite(h->response_data, h->response_size, 1, out) == 1;
    }
    return ok;
}

// @fixme: broken with tcc -m64 (our default tcc configuration)
array(char) download( const char *url ) {
    array(char) out = 0;
    for( https_t *h = https_get(url, NULL); h; https_release(h), h = NULL ) {
        while (https_process(h) == HTTPS_STATUS_PENDING) sleep_ms(1);
        //printf("fetch status:%d, %d %s\n\n%.*s\n", https_process(h), h->status_code, h->content_type, (int)h->response_size, (char*)h->response_data);
        if( https_process(h) == HTTPS_STATUS_COMPLETED ) {
            array_resize(out, h->response_size);
            memcpy(out, h->response_data, h->response_size);
        }
    }
    return out;
}

bool network_tests() {
    // network test (https)
    array(char) webfile = download("https://www.google.com/");
    printf("Network test: %d bytes downloaded from google.com\n", array_count(webfile));
    return true;
}

int portname( const char *service_name, unsigned retries ) {
    // Determine port for a given service based on hash of its name.
    // If port cant be reached, client should retry with next hash.
    // Algorithm: fnv1a(name of service) -> splitmix64 num retries -> remap bucket as [min..max] ports.

    // hash64
    uint64_t hash = 14695981039346656037ULL;
    while( *service_name ) {
        hash = ( (unsigned char)*service_name++ ^ hash ) * 0x100000001b3ULL;
    }

    // splitmix64
    for( unsigned i = 0; i < retries; ++i ) {
        uint64_t h = (hash += UINT64_C(0x9E3779B97F4A7C15));
        h = (h ^ (h >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        h = (h ^ (h >> 27)) * UINT64_C(0x94D049BB133111EB);
        h = (h ^ (h >> 31));
        hash = h;
    }

    // See dynamic ports: https://en.wikipedia.org/wiki/Ephemeral_port
    // So, excluded ranges: 32768..60999 (linux), 49152..65535 (freebsd+vista+win7), 1024..5000 (winsrv2003+bsd)
    // Output range: [5001..32724], in 4096 steps
    return ((hash & 0xFFF) * 677 / 100 + 5001);
}

static
void netdump( const void *ptr, int len ) {
    char hexbuf[256] = {0}, strbuf[256] = {0}, *data = (char*)ptr, width = 16;
    for( int jt = 0; jt < len; jt += width ) {
        char *hex = hexbuf, *str = strbuf;
        for( int it = jt, next = it + width; it < len && it < next; ++it, ++data ) {
            hex += sprintf( hex, "%02x ", (unsigned char)*data);
            str += sprintf( str, "%c", *data >= 32 && *data != '\\' ? *data : '.');
        }
        printf("%06x %-*s%s\n", jt, width*3, hexbuf, strbuf);
    }
}

// -----------------------------------------------------------------------------

#define UDP_DEBUG 0

static int udp_init() {
    do_once {
        int rc = swrapInit(); // atexit(swrapTerminate);
        if( rc ) PANIC("udp_init: swrapInit error");
    }
    return 1;
}

int udp_open(const char *address, const char *port) {
    do_once udp_init();
    int fd = swrapSocket(SWRAP_UDP, SWRAP_CONNECT, 0, address, port);
    // if( fd == -1 ) PANIC("udp_open: swrapSocket error");
    return fd;
}

int udp_bind(const char *address, const char *port) {
    do_once udp_init();
    int fd = swrapSocket(SWRAP_UDP, SWRAP_BIND, 0, address, port);
    // if( fd == -1 ) PANIC("udp_bind: swrapSocket error");
    return fd;
}

int udp_send( int fd, const void *buf, int len ) { // returns bytes sent, or -1 if error
    int rc = -1;
    if( fd >= 0 ) for( ;; ) {
        rc = swrapSend(fd, (const char *)buf, len);
#if is(win32)
        if( rc == -1 && WSAGetLastError() == WSAEINTR ) continue;
        else break;
#else
        if( rc == -1 && errno == EINTR ) continue;
        else break;
#endif
    }
#if UDP_DEBUG
    if( rc > 0 ) {
        char host[128], serv[128];
        int rc2 = swrapAddressInfo(&sa, host, 128, serv, 128 );
        if( rc2 != 0 ) PANIC("swrapAddressInfo error");
        printf("udp_send: %d bytes to %s:%s : %.*s\n", rc, host, serv, rc, buf );
        netdump(buf, rc);
    }
#endif
    return rc;
}

int udp_close( int fd ) { // @todo: expose me? needed?
#if is(win32)
    // closesocket(fd);
#else
    // close(fd);
#endif
    fd = -1; // noop
    return 0;
}

#if 0
// use socket to send data to another address
int udp_sendto( int fd, const char *ip, const char *port, const void *buf, int len ) { // return number of bytes sent
#if 0
    int rc = swrapSendTo(fd, struct swrap_addr*, (const char*)buf, len);
    if( rc == -1 ) return -1; //PANIC("udp_send: swrapSend error");
    return rc;
#else
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    // use inet_addr. tcc(win32) wont work otherwise.
    addr.sin_addr.s_addr = inet_addr(ip); // inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(atoi(port));

    int n = sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    return n < 0 ? -1 : n;
#endif
}
#endif

int udp_peek( int fd ) { // <0 error, 0 timeout, >0 data
    int rc = swrapSelect(fd, 0.00001);
    if( rc < 0 ) return -1; // PANIC("udp_peek: swrapSelect error");
    if( rc == 0 ) return 0; // timeout
    return 1; //> 0: new data is available
}

int udp_recv( int fd, void *buf, int len ) { // <0 error, 0 orderly shutdown, >0 received bytes
    struct swrap_addr sa = {0};
    int rc = swrapReceiveFrom(fd, &sa, buf, len);
    if( rc < 0 ) return -1; // PANIC("udp_recv: swrapReceiveFrom error");
    if( rc == 0 ) return 0; // orderly shutdown

#if UDP_DEBUG
    char host[128], serv[128];
    int rc2 = swrapAddressInfo(&sa, host, 128, serv, 128 );
    if( rc2 != 0 ) PANIC("swrapAddressInfo error");
    printf("udp_recv: %d bytes from %s:%s : %.*s\n", rc, host, serv, rc, buf );
    netdump(buf, rc);
#endif

    return rc;
}

// -----------------------------------------------------------------------------

#define TCP_DEBUG 1

#if TCP_DEBUG
static set(int) tcp_set;
#endif

void tcp_init(void) {
    do_once {
        udp_init();
#if TCP_DEBUG
        set_init(tcp_set, less_int, hash_int);
#endif
    }
}
int tcp_open(const char *address, const char *port) {
    do_once tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_CONNECT, 0/*|SWRAP_NODELAY*/, address, port);
    return fd;
}
int tcp_bind(const char *interface_, const char *port, int backlog) {
    do_once tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_BIND, 0/*|SWRAP_NODELAY*//*|SWRAP_NOBLOCK*/, interface_, port);
    if( fd >= 0 ) swrapListen(fd, backlog);
    return fd;
}
int tcp_peek(int fd, int(*callback)(int)) {
    struct swrap_addr sa;
    int fd2 = swrapAccept(fd, &sa);
    if( fd2 >= 0 ) return callback(fd2);
    return -1;
}
int tcp_send(int fd, const void *buf, int len) {
    int rc = swrapSend(fd, (const char *)buf, len);
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        printf("send -> %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
int tcp_recv(int fd, void *buf, int len) {
    int rc = swrapReceive(fd, (char*)buf, len);
#if TCP_DEBUG
    if( rc != 0 && set_find(tcp_set, fd) ) {
        printf("recv <- %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) netdump(buf, rc);
    }
#endif
    return rc;
}
char* tcp_host(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf);
}
char* tcp_port(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf+512);
}
int tcp_close(int fd) {
    swrapClose(fd);
    return 0;
}
int tcp_debug(int fd) {
#if TCP_DEBUG
    if( set_find(tcp_set, fd) ) {
        set_erase(tcp_set, fd);
        return 0;
    } else {
        set_insert(tcp_set, fd);
        return 1;
    }
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------------

static void network_init() {
    do_once {
        udp_init();
        tcp_init();
    }
}
