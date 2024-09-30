#include "engine.h"

int yes() {
    return puts("yes"),1;
}
int no() {
    return puts("no"),0;
}
int hi() {
    return puts("hi"),1;
}

int run_bt(void *ud) {
    bt_t *b = (bt_t*)ud;
    int rc=bt_run(b);
    printf("rc:%d\n", rc);
    ui_notify("bt done", va("status: %d", rc));
    return 0;
}

int main() {
    window_create(50, WINDOW_SQUARE); // WINDOW_MSAA4);
    window_title(__FILE__);

    bt_addfun("yes", yes);
    bt_addfun("no", no);
    bt_addfun("hi", hi);
    bt_t b = bt("bt1.ini", 0);

    // game loop
    while( window_swap() && !input(KEY_ESC) ) {
        if( ui_panel("AI", 0) )  {
            int choice = ui_buttons(2, "BT Run", "BT Reload");
            if(choice == 1) thread(run_bt, &b);
            if(choice == 2) b = bt("bt1.ini", 0);
            ui_separator();
            ui_bt(&b);
            ui_panel_end();
        }

        font_print(va(FONT_TOP FONT_RIGHT "bt.node: %d bytes", (int)sizeof(bt_t)));
    }
}
