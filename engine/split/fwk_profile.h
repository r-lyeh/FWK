// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if !ENABLE_PROFILER
#   define profile(section)             for(int macro(i) = 1; macro(i); macro(i) = 0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile_setstat(name, value) do {} while(0)
#   define profile_init()               do {} while(0)
#   define profile_render()             do {} while(0)
#   define profile_enable(x)            0
#else
#   define profile(section) for( \
        struct profile_t *found = profiler_enabled ? \
            map_find_or_add(profiler, section "@" FILELINE, (struct profile_t){NAN} ) : NULL, \
        *doit = found + ( found ? found->cost = -time_us(), 1 : 1 ); doit; \
        doit = found ? found->cost += time_us(), found->avg = found->cost * 0.25 + found->avg * 0.75, NULL : NULL)  ///+
#   define profile_incstat(name, accum) for( \
        struct profile_t *found = profiler_enabled ? map_find_or_add(profiler, name, (struct profile_t){0}) : NULL; \
        found; found->stat += accum, found = NULL) ///+
#   define profile_setstat(name, value) for( \
        struct profile_t *found = profiler_enabled ? map_find_or_add(profiler, name, (struct profile_t){0}) : NULL; \
        found; found->stat = value, found = NULL) ///+
API int profile_enable(bool on);

struct profile_t { double stat; int32_t cost, avg; }; ///-
typedef map(char *, struct profile_t) profiler_t; ///-
extern API profiler_t profiler; ///-
extern API int profiler_enabled; ///-
#endif
