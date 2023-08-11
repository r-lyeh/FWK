#include "fwk.h" // Minimal C sample

int main() {
    window_create(75.0, 0); // 75% size, no extra flags

    while( window_swap() && !input(KEY_ESC) ) { // game loop
        puts("hello FWK from C!");
    }
}