// public domain

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <memory.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define UNUSED
#pragma comment(lib, "ws2_32")
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#define closesocket close
#define Sleep(t) usleep(t*1000)
#define UNUSED __attribute__((unused))
#endif

#define MAX_CLIENTS 32
#define BUF_SIZE 4096
#define printf _log

void _log(char *fmt, ...)
{
    int i;
    char buf[BUF_SIZE];
    char timestr[32];
    time_t t = time(0);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUF_SIZE - 1, fmt, ap);
    va_end(ap);
    for (i = 0; i < (int)strlen(buf) - 1; i++)
        buf[i] = (unsigned char)buf[i] < 32 ? '.' : buf[i];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S ", localtime(&t));
    fprintf(stdout, "%s", timestr);
    fprintf(stdout, "%s", buf);
    fflush(stdout);
}

typedef struct {
    int sock;
    struct sockaddr_in addr;
    int len;
    char buf[BUF_SIZE];
    int state;
    char nickname[16];
    char username[32];
    char hostname[32];
    char realname[32];
    char servername[32];
    char chanlist[BUF_SIZE];
} client_t;

char *serv = "localhost";
client_t clients[MAX_CLIENTS];
int num_clients;

int str_split(char **arr, char *str, char *d)
{
    int n = 0;
    char *pch = strtok(str, d);
    while (pch != NULL)
    {
        arr[n++] = pch;
        pch = strtok(NULL, d);
    }
    arr[n] = 0;
    return n;
}

int CL_COUNT(char *list)
{
    int i = 1;
    char *p = list;
    while ((p = strchr(p, ' ')))
    {
        p++;
        i++;
    }
    return i;
}

char *strchr_n(char *str, int n)
{
    char *p = str;
    while ((p = strchr(p, ' ')) && n--)
        if (p)
            p++;
    return p;
}

char *CL_HEAD(char *list, int n)
{
    if (list)
    {
        static char buf[BUF_SIZE];
        char *p = strchr_n(list, n - 1);
        int len = p ? p - list : n ? (int) strlen(list) : 0;
        memcpy(buf, list, len);
        buf[len] = 0;
        return buf;
    }
    return 0;
}

char *CL_TAIL(char *list, int n)
{
    if (list)
    {
        static char buf[BUF_SIZE];
        char *p = strchr_n(list, n - 1);
        int len = p ? p - list + 1 : n ? (int) strlen(list) : 0;
        memcpy(buf, list + len, strlen(list) - len);
        buf[strlen(list) - len] = 0;
        return buf;
    }
    return 0;
}

char *CL_ITEM(char *list, int n)
{
    return CL_HEAD(CL_TAIL(list, n), 1);
}

int CL_ISSET(char *list, char *key)
{
    int i;
    char *arr[BUF_SIZE];
    char buf[BUF_SIZE];
    int count;
    strcpy(buf, list);
    count = str_split(arr, buf, " ");
    for (i = 0; i < count; i++)
    {
        if (strcmp(arr[i], key) == 0)
            return 1;
    }
    return 0;
}

int CL_SET(char *list, char *key)
{
    if (strlen(list) + strlen(key) + 1 >= BUF_SIZE)
        return 0;

    if (!CL_ISSET(list, key))
    {
        if (strlen(list) != 0)
            strcat(list, " ");

        strcat(list, key);
        return 1;
    }
    return 0;
}

int CL_CLR(char *list, char *key)
{
    int i;
    char *arr[1024];
    char buf[1024];
    int count;
    strcpy(buf, list);
    count = str_split(arr, buf, " ");
    list[0] = 0;
    for (i = 0; i < count; i++)
        if (strcmp(arr[i], key) != 0)
            CL_SET(list, arr[i]);
    return 0;
}

int CL_MATCH(char *c1, char *c2)
{
    int i, j, v1, v2;
    char *arr1[BUF_SIZE];
    char *arr2[BUF_SIZE];
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
    strcpy(buf1, c1);
    strcpy(buf2, c2);
    v1 = str_split(arr1, buf1, " ");
    v2 = str_split(arr2, buf2, " ");
    for (j = 0; j < v2; j++)
        for (i = 0; i < v1; i++)
            if (strcmp(arr1[i], arr2[j]) == 0)
                return 1;
    return 0;
}

client_t *get_user_by_nickname(char *nickname)
{
    int i;
    for (i = 0; i < num_clients; i++)
    {
        if (strcmp(nickname, clients[i].nickname) == 0)
            return &clients[i];
    }
    return 0;
}

int get_userlist_by_chan(char *userlist, char *chan)
{
    int i;
    userlist[0] = 0;
    for (i = 0; i < num_clients; i++)
    {
        if (chan == 0 || CL_ISSET(clients[i].chanlist, chan))
            CL_SET(userlist, clients[i].nickname);
    }
    return 0;
}

char *unquote(char *str)
{
    int len = strlen(str);
    if (str[0] == '\"' && str[len - 1] == '\"')
    {
        memmove(str, str + 1, len - 1);
        str[len - 2] = 0;
    }
    return str;
}

void reply(client_t * cl, char *dest, char *fmt, ...)
{
    int j, len;
    char buf[BUF_SIZE];

    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");
    len = strlen(buf);

    if (!dest || !strlen(dest))
    {
        printf("[send %d bytes] %s", len, buf);
        send(cl->sock, buf, len, 0);
    }
    else
    {
        for (j = 0; j < num_clients; j++)
        {
            if ((CL_ISSET(dest, clients[j].nickname) || (CL_MATCH(dest, clients[j].chanlist) && cl->sock != clients[j].sock)))
            {
                printf("[send %d bytes] %s", len, buf);
                send(clients[j].sock, buf, len, 0);
            }
        }
    }
}

int string_to_argv(char **argv, char *str, int size)
{
    int i, argc = 0;
    char *p1 = str;
    char *p2 = str;
    argv[argc++] = p2;
    while ((p2 = strchr(p1, ' ')) && argc < size)
    {
        argv[argc++] = p2 + 1;
        p1 = p2 + 1;
        *p2 = 0;
        if (p1[0] == ':')
            break;
    }
    for (i = 0; i < argc; i++)
    {
        unquote(argv[i]);
        if (argv[i][0] == ':')
            argv[i]++;
    }
    return argc;
}

void ircd_parse(client_t * clients, int k, UNUSED int num_clients, char *buf)
{
    char mask[BUF_SIZE];
    int i = 0;
    char *topic = "Welcome!";
    char *cmd;
    int argc;
    char *argv[16];
    char userlist[1024] = {
        0
    };
    client_t *cl = &clients[k];

    for (i = 0; i < (int)strlen(buf); i++)
    {
        unsigned char c = buf[i];
        buf[i] = c == '\n' || c == '\r' ? 0 : c;
    }

    printf("DEBUG: parsing line %s\n", buf);

    argc = string_to_argv(argv, buf, 16);
    cmd = argv[0];

    if (cl->state < 2)
    {
        if (argc == 2 && !strcmp(cmd, "NICK"))
        {
            printf("DEBUG: nick is %s\n", argv[1]);

            get_userlist_by_chan(userlist, 0);

            if (CL_ISSET(userlist, argv[1]))
            {
                reply(cl, 0, ":%s 433 * %s: Username is already in use.", serv, argv[1]);
            }
            else
            {
                cl->state = cl->state == 0 ? 1 : 2;
                strncpy(cl->nickname, argv[1], 15);
            }
        }

        if (argc == 5 && !strcmp(cmd, "USER"))
        {
            strncpy(cl->username, argv[1], 15);
            strncpy(cl->hostname, argv[2], 31);
            strncpy(cl->servername, argv[3], 31);
            strncpy(cl->realname, argv[4], 31);
            cl->state = cl->state == 0 ? 1 : 2;
        }
    }

    if (cl->state == 2)
    {
        reply(cl, 0, ":%s 001 %s :%s", serv, cl->nickname, "Welcome to IRCd!");
        reply(cl, 0, ":%s 005 %s %s :%s", serv, cl->nickname, "NICKLEN=15 NETWORK=local", "are supported by this server");
        reply(cl, 0, ":%s 376 %s :%s", serv, cl->nickname, "End of /MOTD command.");    //forces autorejoin in mIRC
        cl->state = 3;
    }
    else if (cl->state >= 3)
    {
        sprintf(mask, "%s!%s@%s", cl->nickname, cl->username, cl->servername);

        if (!strcmp(cmd, "JOIN"))
        {
            char *chans[BUF_SIZE];
            int count = str_split(chans, argv[1], ",");
            for (i = 0; i < count; i++)
            {
                char *chan = chans[i];

                if (!CL_ISSET(cl->chanlist, chan))
                {
                    CL_SET(cl->chanlist, chan);
                    get_userlist_by_chan(userlist, chan);
                    reply(cl, userlist, ":%s %s %s", mask, cmd, chan);

                    reply(cl, 0, ":%s MODE %s %s", serv, chan, "+nt");
                    reply(cl, 0, ":%s 332 %s %s :%s", serv, cl->nickname, chan, topic);

                    reply(cl, 0, ":%s 353 %s = %s :%s", serv, cl->nickname, chan, userlist);
                    reply(cl, 0, ":%s 366 %s %s :%s", serv, cl->nickname, chan, "End of /NAMES list");
                }
            }
        }
        else if (!strcmp(cmd, "NICK"))
        {
            printf("DEBUG: user %s is about to change username to %s\n", cl->nickname, argv[1]);

            get_userlist_by_chan(userlist, 0);

            if (CL_ISSET(userlist, argv[1]) || !strcmp(cl->nickname, argv[1]))
            {
                reply(cl, 0, ":%s 433 * %s: Username is already in use.", serv, argv[1]);
            }
            else
            {
                strncpy(cl->nickname, argv[1], 15);
                get_userlist_by_chan(userlist, 0);
                reply(cl, 0, ":%s %s %s", mask, cmd, cl->nickname);
            }
        }
        else if (!strcmp(cmd, "USERHOST"))
        {
            reply(cl, 0, ":%s 302 %s :%s=+%s@%s", serv, argv[1], cl->nickname, cl->username, cl->servername);
        }
        else if (!strcmp(cmd, "PART"))
        {
            reply(cl, 0, ":%s %s %s", mask, cmd, argv[1]);
            CL_CLR(cl->chanlist, argv[1]);
        }
        else if (!strcmp(cmd, "QUIT"))
        {
            reply(cl, cl->chanlist, ":%s %s :%s", mask, cmd, argv[1]);
        }
        else if (!strcmp(cmd, "MODE"))
        {
            if (argc == 2)
                reply(cl, 0, ":%s 324 %s %s %s", serv, cl->nickname, argv[1], "+nt");

            if (argc == 3 && strstr(argv[2], "b"))  // a must for getting properties
                reply(cl, 0, ":%s 368 %s %s :End of Channel Ban List", serv, cl->nickname, argv[1]);
        }
        else if (!strcmp(cmd, "WHOIS"))
        {
            client_t *cw;

            if ((cw = get_user_by_nickname(argv[1])))
            {
                reply(cl, 0, ":%s 311 %s %s %s %s * %s", serv, cl->nickname, cw->nickname, cw->username, cw->servername, cw->realname);

                if (strlen(cw->chanlist))
                    reply(cl, 0, ":%s 319 %s %s :%s", serv, cl->nickname, cw->nickname, cw->chanlist);

                reply(cl, 0, ":%s 318 %s %s :%s", serv, cl->nickname, cw->nickname, "End of /WHOIS list");
            }
        }
        else if (!strcmp(cmd, "PRIVMSG") || !strcmp(cmd, "NOTICE"))
        {
            char *dest = argv[1];
            reply(cl, dest, ":%s %s %s :%s", mask, cmd, argv[1], argv[2]);
        }
        else if (!strcmp(cmd, "PING"))
        {
            reply(cl, 0, "PONG %s :%s", serv, cl->nickname);
        }
    }
}

int die(int code)
{
    perror("socket error");
    exit(code);
}

int main(int argc, char *argv[])
{
    int port = argc > 1 ? atoi(argv[1]) : 6667;
    char buf[BUF_SIZE];
    struct sockaddr_in addr;
    int i, len;
    int listener;
    fd_set read_mask;
    struct timeval timeout;
    int on = 1;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    (void) ((listener = socket(AF_INET, SOCK_STREAM, 0)) > 0 || die(0));

    (void) (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int)) != -1 || die(0));
    (void) (setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int)) != -1 || die(0));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    (void) (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != -1 || die(0));

    (void) (listen(listener, SOMAXCONN) != -1 || die(0));

    printf("DEBUG: server socket (%d) listening at %s:%d\n", listener, inet_ntoa(addr.sin_addr), port);

    for (;;)
    {
        FD_ZERO(&read_mask);
        FD_SET(listener, &read_mask);

        for (i = 0; i < num_clients; i++)
            FD_SET(clients[i].sock, &read_mask);

        if (select(FD_SETSIZE, &read_mask, 0, 0, &timeout) > 0)
        {
            if (FD_ISSET(listener, &read_mask) && num_clients < MAX_CLIENTS)
            {
                int newfd;
                struct sockaddr_in addr;
                unsigned int addr_size = sizeof(addr);
                if ((newfd = accept(listener, (struct sockaddr *)&addr, &addr_size)) != -1)
                {
                    client_t *cl = &clients[num_clients];
                    printf("client %d connected [%s:%d]\n", num_clients, inet_ntoa(addr.sin_addr), addr.sin_port);
                    cl->sock = newfd;
                    cl->state = 0;
                    cl->len = 0;
                    cl->nickname[0] = 0;
                    cl->chanlist[0] = 0;
                    memcpy(&cl->addr, &addr, sizeof(addr));
                    num_clients++;
                }
            }

            for (i = 0; i < num_clients; i++)
            {
                if (FD_ISSET(clients[i].sock, &read_mask))
                {
                    len = recv(clients[i].sock, buf, BUF_SIZE - 1, 0);
                    if (len <= 0)
                    {
                        printf("client %d hung up\n", i);
                        closesocket(clients[i].sock);
                        memmove(&clients[i], &clients[i + 1], sizeof(client_t) * (num_clients - i));
                        num_clients--;
                    }
                    else
                    {
                        char *p;
                        buf[len] = 0;
                        printf("[recv %d bytes] %s\n", len, buf);
                        if (clients[i].len + len < BUF_SIZE)
                        {
                            memcpy(clients[i].buf + clients[i].len, buf, len);
                            clients[i].len += len;
                        }

                        while ((p = (char*)memchr(clients[i].buf, '\n', clients[i].len)))
                        {
                            len = (p - clients[i].buf) + 1;
                            memmove(buf, clients[i].buf, len);
                            memmove(clients[i].buf, clients[i].buf + len, BUF_SIZE - len);
                            clients[i].len -= len;
                            buf[len] = 0;
                            ircd_parse(clients, i, num_clients, buf);
                        }
                    }
                }
            }
        }
        Sleep(1);
    }
    return 0;
}
