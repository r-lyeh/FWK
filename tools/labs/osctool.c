#include "fwk.h"
#include "oscsend.h"
#include "oscedit.h"
int main(int argc, char **argv) {
    if( argc > 1 ) {
        char *address = argv[1];
        char *arg = 0; strcatf(&arg, "%s", ""); for(int i = 2; i < argc; ++i) strcatf(&arg, "%s", argv[i]);

        char *message = 0;
             if( !arg[0] )                                                                         message = osc_pack_va(address, "");
        else if( strchr("\'\"`", arg[0]) )                                                         message = osc_pack_va(address, "s", (arg[strlen(arg)-1] = '\0', arg+1) );
        else if( !strcmp(arg, "true") || !strcmp(arg, "false") )                                   message = osc_pack_va(address, arg[0] == 't' ? "T" : "F"); 
        else if( strchr(".+-0123456789", arg[0]) && strpbrk(arg, ".fe") )                          message = osc_pack_va(address, "f", atof(arg));
        else if( strchr( "+-0123456789", arg[0]) && strspn(arg+1, "0123456789") == strlen(arg)-1 ) message = osc_pack_va(address, "i", atoi(arg));
        else                                                                                       message = osc_pack_va(address, "s", arg );
        hexdump( message+4, *(int*)message );

        int socket = osc_open("127.0.0.1", OSC_EDIT_PORT);
        printf("%d bytes sent\n", osc_send( socket, message+4, *(int*)message ));
    } else {
        printf("%s /osc/address/ [argtype]\n", argv[0]);
        printf("argtypes: [false][true][int][float][string]\n\n","\t");
        printf("example: %s /player/time 5.5\n", argv[0]);
        printf("example: %s /player/health 100\n", argv[0]);
        printf("example: %s /player/active true\n", argv[0]);
        printf("example: %s /player/fullname \"john doe\"\n", argv[0]);
    }
}
