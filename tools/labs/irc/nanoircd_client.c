// public domain

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define UNUSED
#pragma comment(lib, "ws2_32")
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#define closesocket close
#define Sleep(t) usleep(t*1000)
#define UNUSED __attribute__((unused))
#endif

#define BUF_SIZE 4096

void setnonblock(int sock)
{
#ifdef WIN32
    unsigned long on = 1;
    ioctlsocket(sock, FIONBIO, &on);
#else
    fcntl(sock, F_SETFL, O_NONBLOCK, 1);
#endif
}

typedef struct {
    int sock;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];
    int len;
    char nick[32];
    char chan[32];
    char name[32];
    char pong[64];
    int index;
    int state;
    int timer;
} ircd_t;

int die(int k)
{
    printf("socket error\n");
    exit(k);
}

int CL_APPEND(char *buf, char *sep, char *key)
{
    if (key && strlen(key))
    {
        if (strlen(buf))
        {
            strcat(buf, " ");
            if (sep)
                strcat(buf, sep);
        }
        strcat(buf, key);
    }
    return 0;
}

void servmsg(int sock, char *text, char *cmd, char *opt)
{
    int len;
    char buf[BUF_SIZE] = { 0 };
    CL_APPEND(buf, 0, cmd);
    CL_APPEND(buf, 0, opt);
    CL_APPEND(buf, ":", text);
    strcat(buf, "\n");
    len = strlen(buf);
    printf("[send %d bytes] %s", len, buf);
    send(sock, buf, len, 0);
}

int ircd_parse(ircd_t * cl, char *buf, UNUSED int len)
{
    char opt[BUF_SIZE];
    char from[BUF_SIZE];
    char mask[BUF_SIZE];
    char dest[BUF_SIZE];
    char cmd[BUF_SIZE];
    char text[BUF_SIZE];
    int code;

    int sock = cl->sock;

    if (cl->state == 0)
    {
        servmsg(sock, 0, "NICK", cl->nick);
        sprintf(opt, "%s %s %s", "jpilot", "test", "localhost");
        servmsg(sock, cl->name, "USER", opt);
        cl->state = 1;
    }
    else
    {
        if (sscanf(buf, "PING :%s", text) == 1)
        {
                servmsg(sock, text, "PONG", "");
        }
        else if (sscanf(buf, ":%s %03d %s :%[^\r^\n]", from, &code, dest, text) > 1)
        {
            switch (code)
            {
                case 1:
                    servmsg(sock, 0, "JOIN", cl->chan);
                    break;
            }
        }
        else if (sscanf(buf, ":%[^!]!%s %s %s :%[^\r^\n]", from, mask, cmd, dest, text) >= 3)
        {
            if (!strcmp(cmd, "PRIVMSG") && strcmp(from, cl->nick))
                servmsg(sock, text, "PRIVMSG", from);
        }
    }

    return 0;
}

int ircd_init(ircd_t * cl, char *name, int port)
{
    struct sockaddr_in addr;
    struct hostent *hp;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    (hp = gethostbyname(name)) ? addr.sin_addr.s_addr = *(unsigned int *)hp->h_addr_list[0] : 0;

    (void) (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != -1 || die(0));

    printf("connected to %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    setnonblock(sock);

    cl->sock = sock;
    cl->state = 0;
    cl->len = 0;

    ircd_parse(cl, 0, 0); //a must to start connection

    return 0;
}

int ircd_update(ircd_t * cl)
{
    int len;
    char *p;
    char buf[BUF_SIZE];
    int sock = cl->sock;

    len = recv(sock, buf, BUF_SIZE - 1, 0);

    if (len < 0)
    {
//        printf("DEBUG: server hung up\n");
    }

    if (len > 0)
    {
        buf[len] = 0;
        printf("[recv %d bytes] %s\n", len, buf);

        if (cl->len + len < BUF_SIZE)
        {
            memcpy(cl->buf + cl->len, buf, len);
            cl->len += len;
        }

        while ((p = memchr(cl->buf, '\n', cl->len)))
        {
            len = (p - cl->buf) + 1;
            memmove(buf, cl->buf, len);
            memmove(cl->buf, cl->buf + len, BUF_SIZE - len);
            cl->len -= len;
            buf[len - 1] = 0;
            ircd_parse(cl, buf, len);
        }
    }

    return 0;
}

ircd_t m_ircd;

int main(UNUSED int argc, UNUSED char **argv)
{
    char *host = "localhost";
    int port = 6667;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    strcpy(m_ircd.nick, "Player001");
    strcpy(m_ircd.name, "Player");
    strcpy(m_ircd.chan, "#test");

    ircd_init(&m_ircd, host, port);

    while (1)
    {
        ircd_update(&m_ircd);
        Sleep(100);
    }
}
