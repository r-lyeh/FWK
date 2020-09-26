#include "fwk.h"

#define SCRIPT(...) #__VA_ARGS__

#if 0 // teal
    script_run("local tl=require(\"tl\")\ntl.loader()");
    script_run("addsub = require(\"s2\"); print (addsub.add(10, 20))");
s2.tl:
    local function add(a: number, b: number): number
       return a + b
    end
    local s = add(1,2)
    print(s)
#endif

int main() {
    script_init();

    script_run(SCRIPT(
        window_create(75.0,0);
        while window_swap() do
            ddraw_grid(10);
            ddraw_flush();
            if ui_begin("Hello Lua", 0) then
                ui_end();
            end;
        end;
    ));
}
