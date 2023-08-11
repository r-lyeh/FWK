#include "fwk.h"
#include "oscedit.h"

// game

struct player {
    char *name;
    int health;
    bool visible;
    float time;
};

void render_game(struct player *p) {
    printf("%*.s\rgame: %s health=%d time=%f visible=%d", 30, "", p->name, p->health, p->time, p->visible);
}

// demo
int main() {
    do_once map_init(client_vars, less_str, hash_str);
    do_once map_init(server_vars, less_str, hash_str);

    int client_socket = osc_open("127.0.0.1", OSC_EDIT_PORT);

    if( client_socket >= 0 ) {
        struct player P1 = { STRDUP("Player 1"), 100, true };

        while( GetAsyncKeyState(VK_ESCAPE) & 0x8000 ^ 0x8000 ) {
            P1.time = time_ss(); // << this is the "game", it just advances "time"

            osc_edit("/player/name", 's', &P1.name);
            osc_edit("/player/health", 'i', &P1.health);
            osc_edit("/player/time", 'f', &P1.time);
            osc_edit("/player/visible", 'b', &P1.visible);
            osc_edit_sync(-1, client_socket, 4);

            render_game(&P1);
        }
    }
}
