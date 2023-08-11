// simple osc server. designed for easy integration with a gameloop / immediate mode style.
// - rlyeh, public domain. forked from original code by @mmalex (public domain).
// 
// @todo: add support for // wildcard
// @todo: add support for [ array parameters ] 

API int osc_listen( const char *mask, const char *port );   // creates a listening socket
API int osc_update(int s, int timeout_ms);                  // call every frame. reads the udp port and parse all messages found there
API int osc_list(const struct osc_message **first);         // returns number of received messages, also set arg pointer to first item
API int osc_debug( FILE *out, const char *inmsg, int len ); // debugs raw osc buffer to stream
API const struct osc_message *osc_find(const char *addr);   // finds most recent message matching 'addr'

// OSC types from the spec.

enum {
    OSC_INT = 'i',
    OSC_FLOAT = 'f',
    OSC_STRING = 's',
    OSC_SYMBOL = 'S',
    OSC_BLOB = 'b',
    OSC_INT64 = 'h',
    OSC_TIME = 't',
    OSC_DOUBLE = 'd',
    OSC_CHAR = 'c',
    OSC_RGBA = 'r',
    OSC_MIDI = 'm',
    OSC_TRUE = 'T',
    OSC_FALSE = 'F',
    OSC_NIL = 'N',
    OSC_INFINITY = 'I',
    // OSC_ARRAY = '[',  // @todo
};

// OSC message

typedef struct osc_message {
    const char *pattern;// address in osc message
    const char *types;  // string of characters taken from the OSC types enum
    const char *data;   // pointer to raw osc data. for debugging purposes only (?)
#if 0
    int64_t i[8];       // integer interpretation of first 8 params (for blobs & strings, is length)
    const char *s[8];   // for blobs and strings
    float f[8];         // floating point interpretation of first 8 params
#else
    union variant {
        int64_t i;
        const char *s;
        double f;
        uintptr_t up;
    } v[8];
#endif
} osc_message;


#pragma once

enum { OSC_MAX_BUF = 8*1024*1024 }; // was: 65536
enum { OSC_MAX_MESSAGES = 65536 }; // was: 1024

static int osc_match_(const char *pat, const char *addr) {
    for (int n=0;*pat;addr++,pat++) switch (*pat) {
    default:  if (*pat!=*addr) return 0; break;
    case '?': break;
    case '[': n=(*++pat=='!'); for (pat+=n; *pat!=']' && *pat;++pat) { if (pat[1]=='-') { if (*addr>=*pat && *addr<=pat[2]) { n=!n; break; } pat+=pat[2] ? 2 : 1; } else if (*pat==*addr) { n=!n; break; } }
              if (!n) return 0; while (*pat && *pat!=']') pat++; break;
    case '{': n=0; for (const char *p=++pat; *p && *p!='}' && *p!='/'; pat=++p) { while (*p && *p!='}' && *p!='/' && *p!=',') p++; if (!strncmp(pat,addr,p-pat)) { addr+=p-pat; n=1; break; } }
              while (*pat && *pat!='}') pat++; if (!n) return 0; break;
    case '*': while (pat[1]=='*') pat++; n=0; if (pat[1]=='/' || pat[1]==0) { while (*addr && *addr!='/') ++addr; n=1; } else for (;*addr!='/' && *addr; ++addr) if (osc_match_(pat + 1, addr)) { n=1; break; }
              if (!n) return 0; addr--; break;
    // @todo: add // wildcard support
    }
    return *addr== 0;
}

static float  osc_asfloat_(uint32_t x)  { union { float f;  uint32_t i; } u; u.i=x; return u.f; }
static double osc_asdouble_(uint64_t x) { union { double f; uint64_t i; } u; u.i=x; return u.f; }

static int osc_parse_i32_(const char **s, const char *e) {
    if (*s+4>e) { *s=e; return 0; }
    int rv=htonl(*(uint32_t*)*s);
    *s+=4;
    return rv;
}
static int64_t osc_parse_i64_(const char **s, const char *e) {
    if (*s+8>e) { *s=e; return 0; }
    int64_t rv=htonll(*(uint64_t*)*s);
    *s+=8;
    return rv;
}
static const char *osc_parse_str_(const char **s, const char *e) {
    int len=(int)strlen(*s);
    const char *rv=*s;
    *s=rv+((len+4)&~3); 
    if (*s>e) *s=e;
    return rv;
}
static const char *osc_parse_bin_(const char **s, const char *e, int *len) {
    *len=osc_parse_i32_(s,e); 
    int maxlen=(int)(e-*s);
    if (*len>maxlen) *len=maxlen;
    if (*len<0) *len=0;
    const char *rv=*s;
    *s=rv+((*len+3)&~3);
    if (*s>e) *s=e;
    return rv;
}
static int osc_parse_(osc_message *out, int maxmsg, const char *s, const char *e) {
    if (maxmsg<=0 || s>=e) return 0;
    if (*(uint64_t*)s==*(uint64_t*)"#bundle\0") { // bundle is #bundle, uint64_t time, uint32_t length, <osc packet>        
        osc_parse_i64_(&s,e); // @todo: skipped time for now
        int msgcount=0;
        while (s<e && msgcount<maxmsg) {
            int len=osc_parse_i32_(&s,e);
            int maxlen=(int)(e-s);
            if (len>maxlen) len=maxlen;
            if (len<0) len=0;
            int n=osc_parse_(out+msgcount, maxmsg-msgcount, s, s+len);
            msgcount+=n;
            s+=((len+3)&~3);
        }
        return msgcount;
    }
    // single message
    memset(out,0,sizeof(osc_message));
    out->pattern=osc_parse_str_(&s,e);
    if (!out->pattern)
        return 0;
    out->types=(*s==',')?osc_parse_str_(&s,e):",f";
    if (!out->types) 
        return 0;
    out->types++;
    out->data=s;
    for (int param=0;param<8;++param) {
        switch (out->types[param]) {
            default: return 1; // done!
            case OSC_CHAR: case OSC_RGBA:       // all int32...
            case OSC_MIDI: case OSC_INT:        out->v[param].i=osc_parse_i32_(&s,e); break;
            case OSC_TIME: case OSC_INT64:      out->v[param].i=osc_parse_i64_(&s,e); break;
            case OSC_STRING: case OSC_SYMBOL:   out->v[param].s=osc_parse_str_(&s,e); /*out->v[param].i=strlen(out->v[param].s);*/ break;
            case OSC_FLOAT:                     out->v[param].f=osc_asfloat_(osc_parse_i32_(&s,e)); break;
            case OSC_DOUBLE:                    out->v[param].f=osc_asdouble_(osc_parse_i32_(&s,e)); break;
            case OSC_BLOB:                      {int len=0; out->v[param].s=osc_parse_bin_(&s,e,&len); /*out->v[param].i=len;*/ break; } // @todo: important to signal len in variant somewhere
            case OSC_INFINITY:                  out->v[param].f=INFINITY; break;
            case OSC_TRUE:                      out->v[param].i=1; break;
            case OSC_FALSE: case OSC_NIL:       out->v[param].i=0; break;
//          case OSC_ARRAY:                     @todo
        }
    }
    return 1;
}

static struct osc_message *msg = 0; //[OSC_MAX_MESSAGES];
static int msgpos;

int osc_listen( const char *mask, const char *port ) {
    do_once udp_init();

    int fd = swrapSocket(SWRAP_UDP,SWRAP_BIND,SWRAP_NOBLOCK,mask,port);
    return fd;
}

int osc_update(int fd, int timeout_ms /*= -1*/) {
    do_once udp_init();

    static char *buf = 0; // [OSC_MAX_BUF];
    static int bufpos;

    if( !buf ) {
        buf = CALLOC( 1, OSC_MAX_BUF );
        msg = CALLOC( 1, OSC_MAX_MESSAGES * sizeof(struct osc_message) );
    }
    if(fd<0) return 0;

    /* check if something is available */
    msgpos=0;
    if( timeout_ms >= 0 ) {
        int ret = swrapSelect(fd, timeout_ms / 1000.0);
        if (ret <= 0) { // error, or timeout
            return 0;
        }
    }

    for (msgpos=0,bufpos=0;msgpos<OSC_MAX_MESSAGES && bufpos<OSC_MAX_BUF-8;) {
        int n = swrapReceive(fd, buf+bufpos, OSC_MAX_BUF-bufpos-1);

        if( n <= 0 ) {
#ifdef _WIN32
            //if (n == -1 && WSAGetLastError() == WSAEINTR) continue;
            if( WSAGetLastError() != WSAEINTR && /*WSAGetLastError() != WSAEWOULDBLOCK &&*/
                WSAGetLastError() != WSAECONNRESET && WSAGetLastError() != WSAECONNREFUSED ) {
                // error: %d, WSAGetLastError();
                return 0;
            }
#else
            //if (n == -1 && errno == EINTR) continue;
            if( errno != EAGAIN && errno != EINTR && /*errno != EWOULDBLOCK &&*/
                errno != ECONNRESET && errno != ECONNREFUSED ) {
                // error: %s, strerror(errno));
                return 0;
            }
#endif
            continue;
        }
        char *s=buf+bufpos;
        s[n]=0; // null terminate packet always, for easier c handling
        msgpos+=osc_parse_(msg+msgpos,OSC_MAX_MESSAGES-msgpos,s,s+n);    
        bufpos+=n+1;
    }
    return 1;
}

int osc_list(const osc_message **first) {
    *first = msg;
    return msgpos;
}

const osc_message *osc_find(const char *addr) { // search in reverse order, so newest wins
    for (int i=msgpos;i-->0;) if (osc_match_(msg[i].pattern, addr)) return &msg[i];
    return 0;
}

int osc_debug(FILE *fp, const char *buf, int len) {
    osc_message m[16];
    int nn = osc_parse_(m, 16, buf, buf+len);

    for( int n = 0; n < nn; ++n ) {
        fprintf(fp, "%s [%s]", m[n].pattern, m[n].types);
        // @todo #bundle @%lld

        for(int i = 0; m[n].types[i]; ++i) {
            char f = m[n].types[i];
            /**/ if (f == 'T' || f == 'F' || f == 'N') fprintf( fp, ",%s", (f=='T'?"True":f=='F'?"False":"Null"));
            else if (f == 'h' || f == 't') fprintf( fp, ",%lld", m[n].v[i].i);
            else if (f == 'f' || f == 'd') fprintf( fp, ",%f", m[n].v[i].f);
            else if (f == 'i' || f == 'c' || f == 'r' || f == 'm' ) fprintf( fp, ",%d", (int)m[n].v[i].i);
            else if (f == 's' || f == 'S' ) fprintf( fp, ",%s", m[n].v[i].s);
            else if (f == 'b') fprintf( fp, ",%d bytes", (int)m[n].v[i].i);
            else fprintf(fp, ",%s", "?");
        }

        fprintf(fp, "%s\n", "");
    }
    return 1;
}


#ifdef OSCRECV_DEMO

int main() {
    // @mmalex's tests
    assert(  osc_match_("/[a-c]/?/[abc]/*/fish*/*food/f*/{foo,bar,baz}","/a/b/c//fishfood/monkeyfood/f/bar") );
    assert( !osc_match_("/[a-c]/?/[abc]/*/fish*/*food/f*/{fog,bar,baz}","/a/b/c//fishfood/monkeyfood/f/foo") );
    assert( !osc_match_("/[a-c]/?/[abc]/*/fith*/*food/f*/{foo,bar,baz}","/a/b/c//fishfood/monkeyfood/f/foo") );
    assert( !osc_match_("/[a-c]/?/[abc]/*/fish*/*good/f*/{foo,bar,baz}","/a/b/c//fishfood/monkeyfood/f/foo") );
    assert( !osc_match_("/[a-c]/?/[abc]/*/fish*/*food/g/{foo,bar,baz}","/a/b/c//fishfood/monkeyfood/f/foo") );
    assert(  osc_match_("/[fa-cd]/?/[abc]/*/fish*/*food/f*/{foo,bar,baz}","/d/b/c//fishfood/monkeyfood/f/bar") );

    // Julien Pommier's oscpkt tests
    assert( !osc_match_("//bar", "bar"));
//  assert(  osc_match_("//bar", "/bar"));
//  assert(  osc_match_("//bar", "/foo/plop/bar"));
//  assert(  osc_match_("/foo//", "/foo/plop/df/"));
//  assert(  osc_match_("/foo///////bar", "/foo/plop/baz/bar"));
    assert(  osc_match_("*", "bar"));
    assert(  osc_match_("/foo/*", "/foo/bar"));
//  assert(  osc_match_("/{bar,fo}/b[aA]r", "/fo/bar"));
//  assert( !osc_match_("/{bar,fo}/b[aA]r", "/foo/bar"));
//  assert(  osc_match_("/fo{bar,}/ba[e-t]", "/fo/bar"));
    assert( !osc_match_("/fo{bar,}/ba[t-z]", "/fo/bar"));
//  assert(  osc_match_("/f{,ioio,bar}o/?a[!a]", "/fo/bar"));
    assert(  osc_match_("/foo/bar", "/foo/bar"));
    assert(  osc_match_("/f*o/bar", "/foo/bar"));
    assert(  osc_match_("/fo*o/bar", "/foo/bar"));
//  assert(  osc_match_("/*//bar", "/foo/bar"));
    assert(  osc_match_("/*/bar", "/foo/bar"));
    assert(  osc_match_("/*o/bar", "/foo/bar"));
    assert(  osc_match_("/*/*/*/*a***/*/*/*/*/", "/foo/bar/foo/barrrr/foo/bar/foo/barrrr/"));
    assert( !osc_match_("/*/*/*/**/*/*/*/*/q", "/foo/bar/foo/barrrr/foo/bar/foo/barrrr/p"));
    assert(  osc_match_("[-]", "-"));
//  assert(  osc_match_("[a-]", "a"));
//  assert(  osc_match_("[a-]", "-"));

    int fd = osc_listen( "127.0.0.1", "9000" );
    if( fd ) for(;;) {
        static unsigned char counter = 0;
        printf("\r127.0.0.1:9000 %c", "\\|/-"[ counter = (counter+1) & 3 ]);

        Sleep(100);
        osc_update(fd);

        const osc_message *begin;
        for( int it = 0, end = osc_list(&begin); it < end; ++it ) {
            const osc_message *msg = begin + it;
            printf("> %s [%s]\n", msg->pattern, msg->types);
        }
    }
}

#endif
