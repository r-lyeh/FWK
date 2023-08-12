/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
struct tcp_keepalive {
  u_long onoff;
  u_long keepalivetime;
  u_long keepaliveinterval;
};

#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
#define SIO_RCVALL_MCAST _WSAIOW(IOC_VENDOR,2)
#define SIO_RCVALL_IGMPMCAST _WSAIOW(IOC_VENDOR,3)
#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)
#define SIO_ABSORB_RTRALERT _WSAIOW(IOC_VENDOR,5)
#define SIO_UCAST_IF _WSAIOW(IOC_VENDOR,6)
#define SIO_LIMIT_BROADCASTS _WSAIOW(IOC_VENDOR,7)
#define SIO_INDEX_BIND _WSAIOW(IOC_VENDOR,8)
#define SIO_INDEX_MCASTIF _WSAIOW(IOC_VENDOR,9)
#define SIO_INDEX_ADD_MCAST _WSAIOW(IOC_VENDOR,10)
#define SIO_INDEX_DEL_MCAST _WSAIOW(IOC_VENDOR,11)

#define RCVALL_OFF 0
#define RCVALL_ON 1
#define RCVALL_SOCKETLEVELONLY 2
