// https://github.com/BareRose/swrap/blob/master/swrap.h

/*
swrap - Portable, protocol-agnostic TCP and UDP socket wrapper, primarily designed for client-server models in applications such as games

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software.
If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
swrap supports the following three configurations:
#define SWRAP_EXTERN
    Default, should be used when using swrap in multiple compilation units within the same project.
#define SWRAP_IMPLEMENTATION
    Must be defined in exactly one source file within a project for swrap to be found by the linker.
#define SWRAP_STATIC
    Defines all swrap functions as static, useful if swrap is only used in a single compilation unit.
*/

//include only once
#ifndef SWRAP_H
#define SWRAP_H

//process configuration
#ifdef SWRAP_STATIC
    #define SWRAP_IMPLEMENTATION
    #define SWDEF static
#else //SWRAP_EXTERN
    #define SWDEF extern
#endif

//constants
#define SWRAP_TCP 0
#define SWRAP_UDP 1
#define SWRAP_BIND 0
#define SWRAP_CONNECT 1
#define SWRAP_DEFAULT 0x00
#define SWRAP_NOBLOCK 0x01
#define SWRAP_NODELAY 0x02

//structs
struct swrap_addr {
    char data[128]; //enough space to hold any kind of address
};

//function declarations
SWDEF int swrapInit();
SWDEF int swrapSocket(int, int, char, const char*, const char*);
SWDEF void swrapClose(int);
SWDEF void swrapTerminate();
SWDEF int swrapListen(int, int);
SWDEF int swrapAccept(int, struct swrap_addr*);
SWDEF int swrapAddress(int, struct swrap_addr*);
SWDEF int swrapAddressInfo(struct swrap_addr*, char*, size_t, char*, size_t);
SWDEF int swrapSend(int, const char*, size_t);
SWDEF int swrapReceive(int, char*, size_t);
SWDEF int swrapSendTo(int, struct swrap_addr*, const char*, size_t);
SWDEF int swrapReceiveFrom(int, struct swrap_addr*, char*, size_t);
SWDEF int swrapSelect(int, double);
SWDEF int swrapMultiSelect(int*, size_t, double);

//implementation section
#ifdef SWRAP_IMPLEMENTATION

//includes
#ifdef _WIN32 //windows
    #include <ws2tcpip.h>
#else //unix
    #include <sys/socket.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <netinet/tcp.h>
#endif
#include <stddef.h> //NULL
#include <limits.h> //INT_MAX on emscripten //< @r-lyeh: added

//general functions
SWDEF int swrapInit () {
    //initializes socket functionality, returns 0 on success
    #ifdef _WIN32
        WSADATA WsaData;
        return (WSAStartup(MAKEWORD(2,2), &WsaData) != NO_ERROR);
    #else
        return 0;
    #endif
}
SWDEF int swrapSocket (int prot, int mode, char flags, const char* host, const char* serv) {
    //protocol-agnostically creates a new socket configured according to the given parameters
    //sockets have to be created and bound/connected all at once to allow for protocol-agnosticity
    //int: Protocol of the socket, either SWRAP_TCP or SWRAP_UDP for TCP or UDP respectively
    //  SWRAP_TCP: TCP protocol connection-oriented reliable delivery, see swrapListen/Accept
    //  SWRAP_UDP: UDP protocol connectionless unreliable, SWRAP_CONNECT just assigns correspondent
    //int: Mode of the socket
    //  SWRAP_BIND: Bind to given address (or all interfaces if NULL) and port, e.g. for a server
    //  SWRAP_CONNECT: Connect to given address (localhost if NULL) and port, e.g. for a client
    //char: Configuration flags, either SWRAP_DEFAULT or a bitwise combination of flags
    //  SWRAP_NOBLOCK: Sets the socket to be non-blocking, default is blocking
    //  SWRAP_NODELAY: Disables Nagle's for TCP sockets, default is enabled
    //char*: Host/address as a string, can be IPv4, IPv6, etc...
    //char*: Service/port as a string, e.g. "1728" or "http"
    //returns socket handle, or -1 on failure
    struct addrinfo* result, hint = {
        (mode == SWRAP_BIND) ? AI_PASSIVE : 0, //ai_flags
        AF_UNSPEC, //ai_family
        (prot == SWRAP_TCP) ? SOCK_STREAM : SOCK_DGRAM, //ai_socktype
        0, 0, NULL, NULL, NULL};
    //get address info
    if (getaddrinfo(host, serv, &hint, &result)) return -1;
    //create socket
    #ifdef _WIN32
        SOCKET wsck = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (wsck == INVALID_SOCKET) return -1;
        //reject socket handle outside int range
        if (wsck > INT_MAX) {
            closesocket(wsck);
            return -1;
        }
        //convert to int
        int sock = wsck;
    #else
        int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock == -1) return -1;
    #endif
    //make sure IPV6_ONLY is disabled
    if (result->ai_family == AF_INET6) {
        int no = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&no, sizeof(no));
    }
    //set TCP_NODELAY if applicable
    if (prot == SWRAP_TCP) {
        int nodelay = (flags&SWRAP_NODELAY);
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&nodelay, sizeof(nodelay));
    }
    //bind if applicable
    if ((mode == SWRAP_BIND)&&(bind(sock, result->ai_addr, result->ai_addrlen))) {
        swrapClose(sock);
        return -1;
    }
    //set non-blocking if needed
    if (flags&SWRAP_NOBLOCK) {
        #ifdef _WIN32
            DWORD no_block = 1;
            if (ioctlsocket(sock, FIONBIO, &no_block)) {
                swrapClose(sock);
                return -1;
            }
        #else
            if (fcntl(sock, F_SETFL, O_NONBLOCK, 1) == -1) {
                swrapClose(sock);
                return -1;
            }
        #endif
    }
    //connect if applicable (return only relevant if blocking)
    if ((mode == SWRAP_CONNECT)&&(connect(sock, result->ai_addr, result->ai_addrlen))&&(!(flags&SWRAP_NOBLOCK))) {
        swrapClose(sock);
        return -1;
    }
    //free address info
    freeaddrinfo(result);
    //return socket handle
    return sock;
}
SWDEF void swrapClose (int sock) {
    //closes the given socket
    #ifdef _WIN32
        closesocket(sock);
    #else
        close(sock);
    #endif
}
SWDEF void swrapTerminate () {
    //terminates socket functionality
    #ifdef _WIN32
        WSACleanup();
    #endif
}

//connection functions
SWDEF int swrapListen (int sock, int blog) {
    //configures the given socket (must be SWRAP_TCP + SWRAP_BIND) to listen for new connections with given maximum backlog
    //returns 0 on success, non-zero on failure
    return listen(sock, blog);
}
SWDEF int swrapAccept (int sock, struct swrap_addr* addr) {
    //uses the given socket (must be swrapListen) to accept a new incoming connection, optionally returning its address
    //returns a socket handle for the new connection, or -1 on failure (e.g. if there are no new connections)
    #ifdef _WIN32
        int addr_size = sizeof(struct swrap_addr);
        SOCKET wsck = accept(sock, (struct sockaddr*)addr, (addr) ? &addr_size : NULL);
        if (wsck == INVALID_SOCKET) return -1;
        //reject socket handle outside int range
        if (wsck > INT_MAX) {
            closesocket(wsck);
            return -1;
        }
        //return new socket
        return wsck;
    #else
        socklen_t addr_size = sizeof(struct swrap_addr);
        return accept(sock, (struct sockaddr*)addr, (addr) ? &addr_size : NULL);
    #endif
}

//address functions
SWDEF int swrapAddress (int sock, struct swrap_addr* addr) {
    //writes the address the given socket is bound to into given address pointer, useful when automatically assigning port
    //returns 0 on success, non-zero on failure
    #ifdef _WIN32
        int addr_size = sizeof(struct swrap_addr);
    #else
        socklen_t addr_size = sizeof(struct swrap_addr);
    #endif
    return getsockname(sock, (struct sockaddr*)addr, &addr_size);
}
SWDEF int swrapAddressInfo (struct swrap_addr* addr, char* host, size_t host_size, char* serv, size_t serv_size) {
    //writes the host/address and service/port of given address into given buffers (pointer + size), either buffer may be NULL
    //returns 0 on success, non-zero on failure
    return getnameinfo((struct sockaddr*)addr, sizeof(struct swrap_addr), host, host_size, serv, serv_size, 0);
}

//send/receive functions
SWDEF int swrapSend (int sock, const char* data, size_t data_size) {
    //uses the given socket (either SWRAP_CONNECT or returned by swrapAccept) to send given data (pointer + size)
    //at most INT_MAX bytes of data will be sent, data sizes greater than that are clamped to INT_MAX
    //returns how much data was actually sent (may be less than data size), or -1 on failure
    return send(sock, data, (data_size > INT_MAX) ? INT_MAX : data_size, 0);
}
SWDEF int swrapReceive (int sock, char* data, size_t data_size) {
    //receives data using given socket (either SWRAP_CONNECT or returned by swrapAccept) into given buffer (pointer + size)
    //at most INT_MAX bytes of data will be received, buffer sizes greater than INT_MAX have no additional benefit
    //returns the number of bytes received, 0 on orderly shutdown, or -1 on failure (e.g. no data to receive)
    return recv(sock, data, (data_size > INT_MAX) ? INT_MAX : data_size, 0);
}
SWDEF int swrapSendTo (int sock, struct swrap_addr* addr, const char* data, size_t data_size) {
    //uses the given socket to send given data (pointer + size) to the given swrap_addr (e.g. from swrapReceiveFrom)
    //at most INT_MAX bytes of data will be sent, data sizes greater than that are clamped to INT_MAX
    //returns how much data was actually sent (may be less than data size), or -1 on failure
    return sendto(sock, data, (data_size > INT_MAX) ? INT_MAX : data_size, 0, (struct sockaddr*)addr, sizeof(struct swrap_addr));
}
SWDEF int swrapReceiveFrom (int sock, struct swrap_addr* addr, char* data, size_t data_size) {
    //receives data using given socket into given buffer (pointer + size), optionally returning sender's address
    //at most INT_MAX bytes of data will be received, buffer sizes greater than INT_MAX have no additional benefit
    //returns the number of bytes received, 0 on orderly shutdown, or -1 on failure (e.g. no data to receive)
    #ifdef _WIN32
        int addr_size = sizeof(struct swrap_addr);
    #else
        socklen_t addr_size = sizeof(struct swrap_addr);
    #endif
    return recvfrom(sock, data, (data_size > INT_MAX) ? INT_MAX : data_size, 0, (struct sockaddr*)addr, &addr_size);
}

//select functions
SWDEF int swrapSelect (int sock, double timeout) {
    //waits either until given socket has new data to receive or given time (in seconds) has passed
    //if given socket is -1 an empty select will be performed, which is just a sub-second sleep
    //returns 1 if new data is available, 0 if timeout was reached, and -1 on error
    fd_set set; struct timeval time;
    //fd set
    FD_ZERO(&set);
    if (sock > -1) FD_SET(sock, &set);
    //timeout
    time.tv_sec = timeout;
    time.tv_usec = (timeout - time.tv_sec)*1000000.0;
    //return
    return select(sock+1, &set, NULL, NULL, &time);
}
SWDEF int swrapMultiSelect (int* socks, size_t socks_size, double timeout) {
    //waits either until a socket in given list has new data to receive or given time (in seconds) has passed
    //if the given list length is 0 an empty select will be performed, which is just a sub-second sleep
    //returns 1 or more if new data is available, 0 if timeout was reached, and -1 on error
    fd_set set; struct timeval time; int sock_max = -1;
    //fd set
    FD_ZERO(&set);
    for (size_t i = 0; i < socks_size; i++) {
        if (socks[i] > sock_max) sock_max = socks[i];
        if (socks[i] > -1) FD_SET(socks[i], &set);
    }
    //timeout
    time.tv_sec = timeout;
    time.tv_usec = (timeout - time.tv_sec)*1000000.0;
    //return
    return select(sock_max+1, &set, NULL, NULL, &time);
}

#endif //SWRAP_IMPLEMENTATION
#endif //SWRAP_H
