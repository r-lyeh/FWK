#include "fwk.h"

#define OSCPACK_C
#define OSCRECV_C
#define OSCSEND_C
#include "oscpack.h"
#include "oscrecv.h"
#include "oscsend.h"

// networked gui, public api
void ui_netconfig(unsigned port);
void ui_netupdate();

// networked gui, private api
static int ui_netsocket = -1, ui_netclient, ui_netserver;
static map(char*,int) ui_netvalues_i;

void ui_netconfig(unsigned port) {
    do_once map_init(ui_netvalues_i, less_str, hash_str);

    if( ui_netsocket < 0 ) {
        ui_netclient = !(tcp_bind("0.0.0.0",va("%d",port+1), 1) >= 0);
        ui_netserver = !ui_netclient;
        ui_netsocket = ui_netclient ? osc_open("127.0.0.1", va("%d",port)) : osc_listen("0.0.0.0", va("%d",port));
    if( ui_netsocket >= 0 ) ui_notify("UI Network config", va("Connected (%s).", ui_netclient ? "client (editor)" : "server (game)"));
    if( ui_netsocket >= 0 ) window_title(ui_netclient ? "UI client (editor)" : "UI server (game)");
    if( ui_netsocket < 0 ) ui_netclient = ui_netserver = 0;
    }
}
void ui_netupdate() {
    if( ui_netserver ) {
        // map_clear(ui_netvalues_i);
        osc_update(ui_netsocket, 16);
    }
    if( ui_netclient ) {
        for each_map_ptr(ui_netvalues_i, char *, name, int, value) {
            char msg[4096];
            int msglen = osc_pack(msg, va("/%s", *name), "i", *value);
            osc_send(ui_netsocket, msg + 4, msglen - 4);
        }
        // map_clear(ui_netvalues_i);
    }
}

int ui_netint(const char *name, int *value) {
    const osc_message *found = osc_find(va("/%s", name)); // search in reverse order, so newest wins
    if( found ) *value = (int)found->v[0].i;

    int changed = ui_int(name, value);
    if( changed ) *map_find_or_add(ui_netvalues_i, (char*)name, *value) = *value;

    return changed; 
}

// demo
int main() {
    window_create(0.66, WINDOW_SQUARE);

    ui_netconfig(1234);

    while( window_swap() ) {

        ui_netupdate();

        static int r = 0, g = 0, b = 0; 
        if( ui_panel("test1", 0) ) {
            ui_netint("Color R", &r); r = clampi(r, 0, 255);
            ui_netint("Color G", &g); g = clampi(g, 0, 255);
            ui_netint("Color B", &b); b = clampi(b, 0, 255);
            ui_panel_end();
        }
        viewport_color3(vec3(r/255.0,g/255.0,b/255.0));
    }
}
