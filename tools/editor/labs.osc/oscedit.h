//  public api

int  osc_edit(const char *hierarchy_descriptor, char type, void *ptr); // descriptor format: [ifsTF]/path/name
void osc_edit_sync(int server_fd, int client_fd, unsigned timeout_ms);

int  osc_edit_load(const char *mask);
int  osc_edit_save(const char *mask);
int  osc_edit_reset(const char *mask);

#define OSC_EDIT_PORT "2023" // maybe use portname("OSC_EDITOR_V1", 000) ?

// --- impl

#pragma once
#include "oscpack.h"
#include "oscsend.h"
#include "oscrecv.h"

#ifndef OSC_EDIT_INI
#define OSC_EDIT_INI "oscedit.ini"
#endif

typedef struct osc_variant {
    char *key;          // key address
    char type;          // variant type 
    union osc_variant_ {
        int64_t i;
        char *s;
        double f;
        uintptr_t up;
    } live[1], offline[1];
    bool edited;
} osc_variant;

map(char*, void*) client_vars;
array(char*) client_outgoing;

map(char*, osc_variant) server_vars;
array(char*) server_outgoing;

int osc_edit(const char *hierarchy_descriptor, char type, void *ptr) {
    *((void**)map_find_or_add(client_vars, (char*)hierarchy_descriptor, ptr)) = ptr;
    array_push(client_outgoing, stringf("%c%s", type, hierarchy_descriptor)); // @leak
    return 0;
}

int osc_edit_load(const char *mask) {
    ini_t map = ini(OSC_EDIT_INI);
    if( !map ) return 0;

    map_clear(server_vars); // @todo: @leak: iterate variant strings and free() them

    for each_map_ptr_sorted(map, char *, key, char *, val) {
        if( strmatchi(*key + 2, mask ) ) { // skip initial char_type+dot
            printf("%s=%s\n", *key, *val);

            osc_variant variant = {0};
            variant.type = 0[*key];

            /**/ if( variant.type == 'i' ) variant.live[0].i = atoi(*val);
            else if( variant.type == 'f' ) variant.live[0].f = atof(*val);
            else if( variant.type == 's' ) variant.live[0].s = STRDUP(*val);
            else if( strchr("bTF",variant.type) ) variant.live[0].i = (*val)[3] == 't';

            variant.key = STRDUP(*key + 2);
            variant.offline[0] = variant.live[0];

            map_insert(server_vars, STRDUP(*key + 2), variant); // @todo: no need to STRDUP() here. we got variant.key already allocated.
        }
    }

    map_free(map);

    ui_notify("Loaded.", NULL);
    return 1;
}

int osc_edit_save(const char *mask) {
    unlink( OSC_EDIT_INI );

    for each_map_ptr(server_vars, char *, title, osc_variant, msg) {
        if( strmatchi(*title, mask) ) {
            msg->edited = false;

            if( msg->type == 's' ) msg->offline[0].s = STRDUP(msg->live[0].s); // @leak
            else memcpy( &msg->offline[0], &msg->live[0], sizeof(msg->offline[0]) );

            if( msg->type == 'i' )        ini_write(OSC_EDIT_INI, "i", msg->key, va("%d",   (int)msg->live[0].i ));
            if( msg->type == 'f' )        ini_write(OSC_EDIT_INI, "f", msg->key, va("%f", (float)msg->live[0].f ));
            if( msg->type == 's' )        ini_write(OSC_EDIT_INI, "s", msg->key, va("%s",        msg->live[0].s ));
            if( strchr("bTF",msg->type) ) ini_write(OSC_EDIT_INI, "b", msg->key, va("%s", msg->live[0].i ? "true":"false" ));
        }
    }

    ui_notify("Saved.", NULL);
    return 1;
}

int osc_edit_reset(const char *mask) {
    for each_map_ptr(server_vars, char *, title, osc_variant, msg) {
        if( strmatchi(*title, mask) ) {
            msg->edited = false;

            if( msg->type == 's' ) msg->live[0].s = STRDUP(msg->offline[0].s); // @leak
            else memcpy( &msg->live[0], &msg->offline[0], sizeof(msg->live[0]) );
        }
    }
    return 1;
}

void osc_edit_sync(int server_fd, int client_fd, unsigned timeout_ms) {
    // client logic
    if( client_fd >= 0 ) {
        // 1. send outgoing vars (widget requests at server)
        // 2. recv modified vars (from user tweaking widgets), wait till timeout_ms
        //    received vars are removed from outgoing queue, so it's never submitted again until it gets modified.

        // [1]
        for(int i = 0; i < array_count(client_outgoing); ++i) {
            char *out = 0;
            /**/ if( client_outgoing[i][0] == 'i' ) out = osc_pack_va(client_outgoing[i]+1, "i",   *(int*)*((void**)map_find(client_vars, client_outgoing[i]+1)) );
            else if( client_outgoing[i][0] == 'f' ) out = osc_pack_va(client_outgoing[i]+1, "f", *(float*)*((void**)map_find(client_vars, client_outgoing[i]+1)) );
            else if( client_outgoing[i][0] == 's' ) out = osc_pack_va(client_outgoing[i]+1, "s", *(char**)*((void**)map_find(client_vars, client_outgoing[i]+1)) );
            else if( client_outgoing[i][0] == 'b' ) out = osc_pack_va(client_outgoing[i]+1, *(bool*)*((void**)map_find(client_vars, client_outgoing[i]+1)) ? "T":"F" );
            else if( client_outgoing[i][0] == 'T' ) out = osc_pack_va(client_outgoing[i]+1, "T" );
            else if( client_outgoing[i][0] == 'F' ) out = osc_pack_va(client_outgoing[i]+1, "F" );
            else if( client_outgoing[i][0] == '\0') out = osc_pack_va(client_outgoing[i]+1, "" );
            else ASSERT(0, "unsupported osc_edit command `%c`", client_outgoing[i][0]);
            if( out ) osc_send(client_fd, out+4, *(int*)out);
        }        

        for(int i = 0; i < array_count(client_outgoing); ++i) {
            FREE(client_outgoing[i]);
        }

        array_clear(client_outgoing);

        // [2]
        sleep_ms(timeout_ms);
    }

    // server logic
    if( server_fd >= 0 ) {
        // 1. recv vars from client
        // 2. if they do not exist: instantiate UI controls for them
        // 3. if they do     exist: update values from client
        // 4. when UI controls are modified, update our local copies and send its values back to client

        osc_update(server_fd, timeout_ms); // call every frame. reads the udp port and parse all messages found there

        const osc_message *begin;
        for( int it = 0, end = osc_list(&begin); it < end; ++it ) {
            const osc_message *msg = begin + it;

            osc_variant oscv, zero = {0};
            oscv.key = STRDUP(msg->pattern);
            oscv.type = msg->types[0];
            if(oscv.type == 's') oscv.live[0].s = STRDUP(msg->v[0].s), oscv.offline[0].s = STRDUP(msg->v[0].s);
            else memcpy(&oscv.live[0], &msg->v[0], sizeof(oscv.live[0])), memcpy(&oscv.offline[0], &msg->v[0], sizeof(oscv.live[0]));

            *map_find_or_add_allocated_key(server_vars, STRDUP(msg->pattern), zero) = oscv;
        }

        static int on = 1;
        if( ui_window("editor", &on) ) {

            int num_messages = osc_list(&begin);
            int prev_title_len = 0; char *prev_title = 0;

            if( !map_count(server_vars) ) {
                // create section header
                int choice = ui_label2_toolbar("New", ICON_MD_LOOP);
                if( choice == 1 ) osc_edit_load("*");
            }
            else
            for each_map_ptr_sorted(server_vars, char *, title, osc_variant, msg) {

                char *second_slash = strchr(*title + 1, '/');
                int title_len = (int)(second_slash - *title) - 1; // -> /player/title -> player
                int different_size = title_len != prev_title_len;  
                int new_section = different_size || (prev_title && strncmp(prev_title, *title, title_len+1));

                // create section header
                if( new_section ) {
                    if( prev_title ) ui_separator();
                    char *caption = va("*%.*s", title_len + 2, *title);
                    int choice = ui_label2_toolbar(caption, prev_title ? "" : va(">%d" ICON_MD_EMAIL " " ICON_MD_UNDO " " ICON_MD_LOOP " " ICON_MD_SD_CARD, num_messages)); // ICON_MD_UNDO " " ICON_MD_UPLOAD " " ICON_MD_DOWNLOAD
                    if( choice == 3 ) osc_edit_reset("*");
                    if( choice == 2 ) osc_edit_load("*");
                    if( choice == 1 ) osc_edit_save("*");
                    prev_title_len = title_len, prev_title = va("%s", *title);
                } else {
                    prev_title_len = title_len;
                }

                ui_label_icon_highlight = !!msg->edited;
//                vec2 ui_label_icon_clicked_L; // left
//                vec2 ui_label_icon_clicked_R; // right

                char *title_copy = va(ICON_MD_UNDO " " "%s", *title + 1 + title_len + 1); // /player/titlexx -> UNDO_ICON titlexx

                // create gui elements
                /**/ if( msg->type == '\0') { ui_label(title_copy); }
                else if( msg->type == 'i' ) { int i = (int)msg->live[0].i; msg->edited |= ui_int(title_copy, &i); msg->live[0].i = i; }
                else if( msg->type == 'f' ) { float f = msg->live[0].f; msg->edited |= ui_float(title_copy, &f); msg->live[0].f = f; }
                else if( msg->type == 's' ) { char s[128] = {0}; strncpy(s, msg->live[0].s, sizeof(s)); uint64_t old = hash_str(s); ui_buffer(title_copy, s, sizeof(s)); uint64_t mod = hash_str(s); if(mod != old) msg->edited |= 1, FREE(msg->live[0].s), msg->live[0].s = stringf("%s",s); }
                else if( strchr("bTF",msg->type) ) { bool b = !!msg->live[0].i; msg->edited |= ui_bool(title_copy, &b); msg->live[0].i = b; }
                else ASSERT(0, "unsupported osc_edit command `%c`", msg->type);

                if( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 ) { // if clicked on UNDO icon (1st icon)
                    osc_edit_reset(*title);
                }
            }

            ui_window_end();
        }

        ui_demo();
    }
}

