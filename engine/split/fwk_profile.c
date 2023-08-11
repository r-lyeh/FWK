#if ENABLE_PROFILER
profiler_t profiler;
int profiler_enabled = 1;

void (profile_init)() { map_init(profiler, less_str, hash_str); profiler_enabled &= !!profiler; }
int  (profile_enable)(bool on) { return profiler_enabled = on; }
void (profile_render)() { 
    if(!profiler_enabled) return;

    int has_menu = ui_has_menubar();
    if( !has_menu ) {
        static int cook_on_demand; do_once cook_on_demand = COOK_ON_DEMAND;
        if( !cook_on_demand ) {
            // render profiler, unless we are in the cook progress screen
            static unsigned frames = 0; if(frames <= 0) frames += cook_progress() >= 100;
            if( frames <= 0 ) return;
        }
    }

    if( has_menu ? ui_window("Profiler", 0) : ui_panel("Profiler", 0) ) {

        double fps = window_fps();
        profile_setstat("Render.num_fps", fps);

        if(1) { // @todo: ui_plot()
            // draw fps-meter: 300 samples, [0..70] range each, 70px height plot.
            nk_layout_row_dynamic(ui_ctx, 70, 1);

            enum { COUNT = 300 };

            static float values[COUNT] = {0}; static int offset = 0;
            values[offset=(offset+1)%COUNT] = fps;

            int index = -1;
            if( nk_chart_begin(ui_ctx, NK_CHART_LINES, COUNT, 0.f, 70.f) ) {
                for( int i = 0; i < COUNT; ++i ) {
                    nk_flags res = nk_chart_push(ui_ctx, (float)values[i]);
                    if( res & NK_CHART_HOVERING ) index = i;
                    if( res & NK_CHART_CLICKED ) index = i;
                }
                nk_chart_end(ui_ctx);
            }

            //  hightlight 60fps, 36fps and 12fps
            struct nk_rect space; nk_layout_peek(&space, ui_ctx);
            struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
            nk_stroke_line(canvas, space.x+0,space.y-60,space.x+space.w,space.y-60, 1.0, nk_rgba(0,255,0,128));
            nk_stroke_line(canvas, space.x+0,space.y-36,space.x+space.w,space.y-36, 1.0, nk_rgba(255,255,0,128));
            nk_stroke_line(canvas, space.x+0,space.y-12,space.x+space.w,space.y-12, 1.0, nk_rgba(255,0,0,128));

            if( index >= 0 ) {
                nk_tooltipf(ui_ctx, "%.2f fps", (float)values[index]);
            }
        }

        for each_map_ptr_sorted(profiler, const char *, key, struct profile_t, val ) {
            if( isnan(val->stat) ) {
                float v = val->avg/1000.0;
                ui_slider2(*key, &v, va("%.2f ms", val->avg/1000.0));
            } else {
                float v = val->stat;
                ui_slider2(*key, &v, va("%.2f", val->stat));
                val->stat = 0;
            }
        }

        (has_menu ? ui_window_end : ui_panel_end)();
    }
}
#endif
