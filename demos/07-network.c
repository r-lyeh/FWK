// network demo: sockets, downloads, script, @todo: ecs, vm, obj, teal
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\07-network.c` (windows)
// `sh MAKE.bat demos/07-network.c` (linux, osx)

#include "fwk.h"

volatile int client_socket = -1;
volatile int server_socket = -1;

int server_echo(int cli) {
    ui_notify("Server: client connected", va("Address %s:%s", tcp_host(cli), tcp_port(cli)));
    char buf[128];
    int len = tcp_recv(cli, buf, 128);
    if( len > 0 ) tcp_send(cli, buf, len); // echo
    tcp_close(cli);
    return 1;
}
int server_thread(void *userdata) {
    server_socket = tcp_bind("0.0.0.0", "8080", 10); // port "0" for auto
    for(;;) {
        tcp_peek(server_socket, server_echo);
        sleep_ms(100);
    }
}
void client_message(const char *msg) {
    client_socket = tcp_open("127.0.0.1", "8080");
    if( client_socket >= 0 ) {
        tcp_send(client_socket, msg, strlen(msg));
        char buf[128];
        int rlen = tcp_recv(client_socket, buf, 128);
        if(rlen > 0) ui_notify("Client", va("Received '%.*s' from server", rlen, buf));
        // tcp_close(client_socket);
    }
}


int main() {
    window_create(0.85, 0);
    window_title(__FILE__);
    while(window_swap()) {
        static int ui_open; ui_open = 1;
        if( ui_window("Network", &ui_open) ) {

            if( ui_button("Server: listen") ) { do_once thread(server_thread, NULL); }
            ui_label(server_socket >= 0 ? "Listening at 8080 port" : "Not ready");

            ui_separator();

            if( ui_button("Client: send message") ) client_message("Hi");
            ui_label(client_socket >= 0 ? va("Connected to server. My address is %s:%s", tcp_host(client_socket), tcp_port(client_socket)) : "Not connected");

            ui_separator();

            if( ui_button("Download test (https)")) {
                array(char) webfile = download("https://www.google.com/"); // @leak
                ui_notify("Download test", va("%d bytes downloaded from google.com", array_count(webfile)));
            }

            ui_separator();

            if( ui_button("Script test")) {
                script_run( "ui_notify(\"Script test\", \"Hello from \" .. _VERSION)");
                script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );
            }

            ui_window_end();
        }
    }
}

// this demo supersedes following old sources:
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-demo.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-script.c
// https://github.com/r-lyeh/FWK/blob/45e34d7890b2b8fe1f4994f4b76e496280d83cb6/demos/00-socket.c
