#include "fwk.h"

enum { MAX_NPCS = 5, MAX_CLIENTS = 5 };
struct player_t {
    uint64_t seen_until;
    float x,y,z,angle;
    uint32_t color;
};
struct npc_t {
    float x,y,z;
    uint32_t color;
};
struct world_t {
    struct player_t player[MAX_CLIENTS];
    struct npc_t npc[MAX_NPCS];
} world = {0};

void show_notification(char *msg) {
    printf("notif %s\n", msg);
    ui_notify("server", msg);
}

void bind_netbuffers(int64_t self_id) {
    uint32_t colors[] = { ORANGE,GREEN,RED,CYAN,PURPLE,YELLOW,GRAY,PINK,AQUA };
    for (int64_t i=0; i<MAX_NPCS; ++i) {
        // as an example, let only server to set initial pos
        if (self_id==0) {
            world.npc[i].x = i*3.f + 4.f;
        }
        world.npc[i].color = colors[i%(sizeof colors / sizeof colors[0])];
        network_buffer(&world.npc[i], sizeof(struct npc_t), NETWORK_RECV, 0);
    }

    for (int64_t i=0; i<MAX_CLIENTS; ++i) {
        world.player[i].color = colors[i%(sizeof colors / sizeof colors[0])];
        network_buffer(&world.player[i], sizeof(struct player_t), (i!=self_id ? NETWORK_RECV : NETWORK_SEND) | NETWORK_UNRELIABLE, i /* each client owns exactly 1 buffer */);
    };

    // register server->client rpc
    if (self_id > 0) {
        network_rpc("void show_notification(char*)", show_notification);
    }
}

int main() {
    // ifdef(win32, FreeConsole()); // tty_detach()

    // network setup
    network_create(MAX_CLIENTS, "127.0.0.1", 0, flag("--client") ? NETWORK_CONNECT : 0);
    int64_t self_id = network_get(NETWORK_RANK);
    bind_netbuffers(self_id);

    // game setup
    camera_t cam = camera();
    window_create( 0.35f, WINDOW_MSAA8|WINDOW_SQUARE );
    struct player_t *self = &world.player[self_id];

    network_put(NETWORK_SEND_MS, 33); // 0.033 s

    // game loop
    while( window_swap() && !input(KEY_ESC) ) {
        // network sync
        char **event = network_sync(0); // timeout_ms:0
        while(*event) {
            int code;
            char *msg;
            int ev = network_event(*event++, &code, &msg);
            printf( "network event id: %d err: %d msg: %s\n", ev, code, msg );
        }

        self_id = network_get(NETWORK_RANK);
        if (network_get(NETWORK_LIVE) == 0) {
            network_create(MAX_CLIENTS, "127.0.0.1", 0, flag("--client") ? NETWORK_CONNECT|NETWORK_NOFAIL : 0);
            self_id = network_get(NETWORK_RANK);
            if (self_id != -1) {
                bind_netbuffers(self_id);
            }
            continue;
        }

        /* quick hack to broadcast notif from host */
        if (self_id == 0 && input_down(KEY_F3)) {
            printf("rpc %s\n", "show_notification \"hi, sailor!\"");
            network_rpc_send(rand()%4, "show_notification \"hi, sailor!\"");
        }

        // camera tracking
        cam.position = vec3(self->x,100,self->z);
        camera_lookat(&cam, vec3(self->x,0,self->z));

        // input - move player
        float iy = input(KEY_UP) - input(KEY_DOWN);
        float ix = input(KEY_RIGHT) - input(KEY_LEFT);
        if( iy || ix ) {
            self->x += iy*window_delta()*15;
            self->z += ix*window_delta()*15;
        }
        self->seen_until = date_epoch() + 4;

        // npc - update npc movement on server-side
        if (self_id == 0) {
            for (int i = 0; i < MAX_NPCS; ++i) {
                struct npc_t *n = &world.npc[i];
                n->z = sinf(window_time())*4.f;
            }
        }

        // background - draw grid
        ddraw_ground(0);

        // foreground - draw all players
        for( int id = 0; id < MAX_CLIENTS; ++id ) {
            struct player_t *p = &world.player[id];
            if (p->seen_until < date_epoch()) continue; /* skip inactive players */
            ddraw_color( p->color );
            ddraw_capsule(vec3(p->x,0,p->z), vec3(p->x,2,p->z), 1);
            ddraw_text(vec3(p->x,4,p->z), 0.01, va("player #%d", id));
        }
        for( int id = 0; id < MAX_NPCS; ++id ) {
            struct npc_t *p = &world.npc[id];
            ddraw_color( p->color );
            ddraw_capsule(vec3(p->x,0,p->z), vec3(p->x,2,p->z), 1);
            ddraw_text(vec3(p->x,4,p->z), 0.01, va("npc #%d", id));
        }

        // stats
        window_title(va("player #%lld", self_id));
    }
}
