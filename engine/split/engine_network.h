// -----------------------------------------------------------------------------
// network framework
// - rlyeh, public domain

API array(char) download( const char *url );
API int         download_file( FILE *out, const char *url );
API int         portname( const char *service_name, unsigned retries );

API bool        network_tests();

// -----------------------------------------------------------------------------
// udp wrapper
// - rlyeh, public domain.

// server
API int   udp_bind(const char *address, const char *port);

// client
API int   udp_open(const char *address, const char *port);

// common
API int   udp_send(int, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_sendto(int, const char *ip, const char *port, const void *buf, int len ); // <0 error, >0 bytes sent ok
API int   udp_recv(int, void *buf, int len ); // <0 error, 0 orderly shutdown, >0 received bytes
API int   udp_peek(int); // <0 error, 0 timeout, >0 data

// -----------------------------------------------------------------------------
// tcp wrapper
// - rlyeh, public domain

// client
API int   tcp_open(const char *address, const char *port);

// server
API int   tcp_bind(const char *interface_, const char *port, int queue);
API int   tcp_peek(int, int(*callback)(int));

// common
API int   tcp_send(int, const void* buf, int len);
API int   tcp_recv(int, void* buf, int len);
API char* tcp_host(int); // info
API char* tcp_port(int); // info

API int   tcp_close(int);

// extras
API int   tcp_debug(int); // toggle traffic monitoring on/off for given socket
//API int   tcp_printf(int, const char *fmt, ...); // printf message in remote end
//API int   tcp_crypt(int,uint64_t);               // set shared secret
