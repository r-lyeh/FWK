// fast simple ecs
// - rlyeh, public domain
//
// features:
// - mostly heap allocation free
// - 2^64   systems max
// - 2^64   entities max
//
// cons:
// - 64 components max
// - memory requirements may be suboptimal. ie, using unions for the components: all components will equally size like the largest component.
//
// @note:
// - best perf when compiled with `/DNDEBUG /openmp /arch:AVX2 /Os /Ox /Gw /GL /MT`
// - runs 86M ops/sec on my old laptop: updates 50K entities out of 100K in 0.58ms/frame. 1000 frames in 0.57s
//
// @todo:
// - thread-safe
// - world/context management
// - dont use arrays: sequential access is killing performances when doing huge amount of entities (10M). worthy?
//   compromise: use islands. 10K entities per island seems reasonable & best perf. the lower the better.
//   we could parallelize islands as well (openmp? threads?).
// - file format spec:
//   ; ecs data file format (.ini)
//   [entity]
//   components = mesh aabb
//   position = 0 0 0         ; common
//   rotation = 0 0 0         ; common
//   scale    = 2 2 2         ; common
//   aabb.static = 1
//   aabb.size   = 3 3 3
//   aabb.offset = 0 0 0
//   mesh.model = cube
//   mesh.texture = wood_03
//   mesh.texture.tiling = 0.5 0.5
// - file format parser:
//   for each_map_sorted_ptr(ini("ecs.ini"), char *, k, char *, v)
//       printf("'%s'='%s'\n", *k, *v += strspn(*v, " "));
//

/* api */

#ifdef _OPENMP
#   ifdef _MSC_VER
#      define parallel __pragma(omp parallel for)
#      define has_parallel 1
#   else // __GNUC__
#      define parallel _Pragma("omp parallel for") // C99
#      define has_parallel 1
#   endif
#else
#   define parallel
#   define has_parallel 0
#endif

void    ecs_max_components(int max_components);

#define ecs_dump_world(fp, ...)       ecs_dump_world(fp,ecs_mask64(__VA_ARGS__,-1))

#define ecs_add_entity(...)           ecs_add_entity(ecs_mask64(__VA_ARGS__,-1))

union component_t*  ecs_get_component( int eid, int cid );
union component_t** ecs_get_components( int eid );

#define ecs_has_component(eid,...)    ecs_has_component(eid,ecs_mask64(__VA_ARGS__,-1))
#define ecs_add_component(eid,...)    ecs_add_component(eid,ecs_mask64(__VA_ARGS__,-1))
#define ecs_del_component(eid,...)    ecs_del_component(eid,ecs_mask64(__VA_ARGS__,-1))
#define ecs_use_component(eid,...)    ecs_use_component(eid,ecs_mask64(__VA_ARGS__,-1))
#define ecs_off_component(eid,...)    ecs_off_component(eid,ecs_mask64(__VA_ARGS__,-1))
#define each_ecs_component(obj, ...) \
    ( uint64_t sys_ = ecs_mask64(__VA_ARGS__,-1), ent_ = 0; ent_ < world.ne; ++ent_ ) \
        for( component_t **obj = sys_ == (sys_ & world.entities[ent_ * 2 + 0]) ? ecs_get_components(ent_) : 0; obj ; obj = 0 )

#if has_parallel
#undef  each_ecs_component
#define each_ecs_component(obj, ...) \
    ( ent_ = (sys_ = ecs_mask64(__VA_ARGS__,-1), 0); ent_ < world.ne; ++ent_ ) \
        for( component_t **obj = sys_ == (sys_ & world.entities[ent_ * 2 + 0]) ? ecs_get_components(ent_) : 0; obj ; obj = 0 )
static int64_t  ent_;
static uint64_t sys_;
#endif

/* internals */

typedef union component_t {
        struct dummy     { int dummy; };

#ifdef   COMPONENT_HEADER
#include COMPONENT_HEADER
#endif
#ifdef   COMPONENT_DATAS
         COMPONENT_DATAS
#endif
#if defined ECS_DEMO || defined ECS_BENCH
        struct position  { float x,y,z; };    // c1
        struct velocity  { float vx,vy,vz; }; // c2
        struct color     { float r,g,b; };    // c3
        struct health    { float health; };   // c4
        char *name;                           // c5
#endif

} component_t;

struct world_t {
    int ne, nc;                    // ne: number of entities, nc: number of components (stride) (cN)
    array(uint64_t) entities;      // vtable entities (2 entries/entity) [ e1(cflags,offset) e2(cflags,offset) .. eN(cflags,offset) ]
    array(component_t) components; // instanced components (nc entries/entity) [ e1(c1,c2..) e2(c1,c2..) .. ]
};

uint64_t ecs_mask64( unsigned id1, ... );
int  (ecs_add_entity)( uint64_t component_mask );
bool (ecs_has_component)( int eid, uint64_t flags );
bool (ecs_add_component)( int eid, uint64_t flags );
bool (ecs_del_component)( int eid, uint64_t flags );
bool (ecs_use_component)( int eid, uint64_t flags );
bool (ecs_off_component)( int eid, uint64_t flags );
void (ecs_dump_world)( FILE *fp, uint64_t component_mask );

extern struct world_t world;

// impl

// static
struct world_t world = {0};

uint64_t ecs_mask64(unsigned id1, ... ) {
    uint64_t flags = 0;

    // update flags and entities of components
    va_list ap;
    va_start(ap, id1);
    for( uint64_t id = id1; id != ((unsigned)-1); id = va_arg(ap, unsigned) ) {
        flags |= 1ull << id;
    }
    va_end(ap);

    return flags;
}

int (ecs_add_entity)( uint64_t component_mask ) {
    component_t c = {0};

    // add mask+offset into entities
    array_push(world.entities, component_mask);
    array_push(world.entities, array_count(world.components));

    // add components into entity
    for (uint64_t cid = 0; cid < world.nc; ++cid) {
        if( (1ull << cid) & component_mask ) {
            array_push( world.components, c );
        }
    }

    int eid = world.ne;
    return world.ne++;
}

void ecs_max_components(int max_comps) {
    world.nc = max_comps;
}

component_t* ecs_get_component( int eid, int target_cid ) {
    uint64_t sys = world.entities[ eid * 2 + 0 ];
    uint64_t off = world.entities[ eid * 2 + 1 ];
    for( uint64_t cid = 0; cid < target_cid; ++cid ) {
        off += !!((1ull << cid) & sys);
    }
    bool has_cid = !!((1ull << target_cid) & sys);
    return (component_t*)(has_cid * (uintptr_t)(&world.components[ off ]));
}

component_t** ecs_get_components( int eid ) {
    static __thread component_t* local[8][64] = {0};
    static __thread int          counter = 0; counter = (counter + 1) % 8;

    uint64_t sys = world.entities[ eid * 2 + 0 ];
    uint64_t off = world.entities[ eid * 2 + 1 ];
    for( uint64_t cid = 0, idx = 0; cid < world.nc; ++cid ) {
        bool has_cid = !!((1ull << cid) & sys);
        local[counter][ cid ] = (component_t*)(has_cid * (uintptr_t)(&world.components[ off ] + idx));
        idx += has_cid;
    }

    return local[counter];
}

bool (ecs_has_component)( int eid, uint64_t flags ) {
    uint64_t sys = world.entities[ eid * 2 + 0 ];
    return flags == (sys & flags);
}
bool (ecs_add_component)( int eid, uint64_t flags ) {
    uint64_t sys  = world.entities[ eid  * 2 + 0 ];
    uint64_t off  = world.entities[ eid  * 2 + 1 ];

    int eid2 = (ecs_add_entity)( sys | flags );
    uint64_t sys2 = world.entities[ eid2 * 2 + 0 ];
    uint64_t off2 = world.entities[ eid2 * 2 + 1 ];

    for( uint64_t cid = 0, idx = 0, idx2 = 0; cid < world.nc; ++cid) {
        if( (1ull << cid) & sys ) {
        if( (1ull << cid) & sys2 ) {
            memcpy( &world.components[ off2 ] + idx2, &world.components[ off ] + idx, sizeof(component_t));
        }
        }
        if( (1ull << cid) & sys ) ++idx;
        if( (1ull << cid) & sys2 ) ++idx2;
    }

    world.entities[ eid * 2 + 0 ] = sys2;
    world.entities[ eid * 2 + 1 ] = off2;

    world.entities[ eid2 * 2 + 0 ] = 0;
    world.entities[ eid2 * 2 + 1 ] = 0;

    return true;
}
bool (ecs_del_component)( int eid, uint64_t flags ) { // clr_component?
    uint64_t sys = world.entities[ eid * 2 + 0 ];
    uint64_t off = world.entities[ eid * 2 + 1 ];
    for( uint64_t cid = 0, idx = 0; cid < world.nc; ++cid) {
        if( (1ull << cid) & flags ) {
            memset( &world.components[ off ] + idx, 0, sizeof(component_t));
        }
        if( (1ull << cid) & sys ) ++idx;
    }
    world.entities[ eid * 2 + 0 ] &= ~flags;
    return true;
}
bool (ecs_use_component)( int eid, uint64_t flags ) {
    world.entities[ eid * 2 + 0 ] |= flags;
    return true;
}
bool (ecs_off_component)( int eid, uint64_t flags ) {
    world.entities[ eid * 2 + 0 ] &= ~flags;
    return true;
}

void (ecs_dump_world)( FILE *fp, uint64_t sys_mask ) {
    if( sys_mask ) for( int eid = 0; eid < world.ne; ++eid ) {
        uint64_t sys = world.entities[ eid * 2 + 0 ];
        uint64_t off = world.entities[ eid * 2 + 1 ];
        if( sys_mask != (sys & sys_mask)) continue;
        fprintf(fp, "eid:%d sys:%#x ", eid, (unsigned)sys);
        for( uint64_t cid = 0, idx = 0; cid < world.nc; ++cid ) {
            if( (1ull << cid) & sys ) {
                fprintf(fp, "cid:%d,%p+%d ", (int)cid, &world.components[ off ], (int)idx );
                ++idx;
            }
        }
        fputc('\n', fp);
    }
}


#ifdef ECS_BENCH
#include <stdio.h>
#include <assert.h>

int main(int argc, char **argv) {
    int player;

    /* entities to spawn */
    #ifndef ECS_N
    const int ECS_N = argc > 1 ? atoi(argv[1]) : 100000;
    #endif

    /* frames to benchmark */
    #ifndef ECS_F
    const int ECS_F = argc > 2 ? atoi(argv[2]) : 1000;
    #endif

    // declare components: c1, c2, ...
    enum { POSITION, VELOCITY, COLOR, HEALTH, INPUT, NAME, TOTAL };
    ecs_max_components(TOTAL);

    // spawn entities
    {
        double start = time_ss();

        player = ecs_add_entity(NAME, POSITION, VELOCITY, HEALTH, INPUT);
        ecs_get_component(player, VELOCITY)->vx = 1;
        ecs_get_component(player, VELOCITY)->vy = 2;

        for (int i = 0; i < ECS_N; ++i) {
            switch (i & 3) {
                break; case 0:; /* static enemy */
                int enemy0 = ecs_add_entity(NAME, POSITION, COLOR, HEALTH);
                break; case 1:; /* dynamic enemy */
                int enemy1 = ecs_add_entity(NAME, POSITION, COLOR, HEALTH, VELOCITY);
                break; case 2:; /* static light */
                int light0 = ecs_add_entity(NAME, POSITION, COLOR);
                break; case 3:; /* dynamic light */
                int light1 = ecs_add_entity(NAME, POSITION, COLOR, VELOCITY);
            }
        }

        double end = time_ss();
        double t = (end - start);

        int T = 1 * ECS_N;
        printf("%17s: %d frame(s) * %d num_entities = %d total ops, in %.3fs => %.3fM ops/s, %.2fms/frame\n",
            "spawn benchmark", 1, ECS_N, T, t, (T / 1000000.0) / t, (t * 1000 / 1) );
    }

    // process & benchmark
    {
        double start = time_ss();

        for( int frame = 0; frame < ECS_F; ++frame ) {
            parallel
            for each_ecs_component(obj, POSITION, VELOCITY) {
                component_t *p = obj[POSITION];
                component_t *v = obj[VELOCITY];
                p->x += v->vx;
                p->y += v->vy;
                p->z += v->vz;
            }
        }

        double end = time_ss();
        double t = (end - start);

        // stats
        int num_iterated_entities = ECS_N;
        int num_processed_entities = 0;
        for each_ecs_component(obj, POSITION, VELOCITY) {
            ++num_processed_entities;
        }

        int T = ECS_F * num_processed_entities;
        printf("%17s: %d frame(s) * %d num_entities = %d total ops, in %.3fs => %.3fM ops/s, %.2fms/frame\n",
            "process benchmark", ECS_F, num_processed_entities, T, t, (T / 1000000.0) / t, (t * 1000 / ECS_F) );
    }

    {component_t *p = ecs_get_component(player, POSITION);
     printf("eid:%d (position: %f,%f,%f)\n", player, p->x, p->y, p->z );}

    assert( ecs_get_component(player, POSITION)->x == (ECS_F * 1));
    assert( ecs_get_component(player, POSITION)->y == (ECS_F * 2));

    assert( ~puts("Ok") );
}
#endif

#ifdef ECS_DEMO
#include <stdio.h>
#include <assert.h>

int main(int argc, char **argv) {
    // declare components: c1, c2, ...
    enum { POSITION, VELOCITY, COLOR, HEALTH, INPUT, TOTAL };
    ecs_max_components(TOTAL);

    // spawn entities
        int player = ecs_add_entity(POSITION, VELOCITY, HEALTH, INPUT);
        ecs_get_component(player, VELOCITY)->vx = 1;
        ecs_get_component(player, VELOCITY)->vy = 2;

        // some more
        int enemy0, enemy1, light0, light1;
        for (int i = 0; i < 10; ++i) {
            if( 0 == (i&3)) /* static enemy  */ enemy0 = ecs_add_entity(POSITION, COLOR, HEALTH);
            if( 1 == (i&3)) /* dynamic enemy */ enemy1 = ecs_add_entity(POSITION, COLOR, HEALTH, VELOCITY);
            if( 2 == (i&3)) /* static light  */ light0 = ecs_add_entity(POSITION, COLOR);
            if( 3 == (i&3)) /* dynamic light */ light1 = ecs_add_entity(POSITION, COLOR, VELOCITY), ecs_get_component(light1, VELOCITY)->vy = 2;
        }

    {
        component_t *p = ecs_get_component(player, POSITION);
        printf("eid:%d (position: %f,%f,%f)\n", player, p->x, p->y, p->z );
        p = ecs_get_component(player, VELOCITY);
        printf("eid:%d (velocity: %f,%f)\n", player, p->vx, p->vy );

        p = ecs_get_component(light1, POSITION);
        printf("eid:%d (position: %f,%f,%f)\n", light1, p->x, p->y, p->z );
        p = ecs_get_component(light1, VELOCITY);
        printf("eid:%d (velocity: %f,%f)\n", light1, p->vx, p->vy );
    }


    // simulate system processing
    int frames = 1000;
        for( int frame = 0; frame < frames; ++frame ) {
            for each_ecs_component(obj, POSITION, VELOCITY) {
                component_t *p = obj[POSITION];
                component_t *v = obj[VELOCITY];
                p->x += v->vx;
                p->y += v->vy;
                p->z += v->vz;
            }
        }

    // verify (should display only player entity)
    ecs_dump_world(stdout, INPUT);

    {
        component_t *p = ecs_get_component(player, POSITION);
        printf("eid:%d (position: %f,%f,%f)\n", player, p->x, p->y, p->z );
        p = ecs_get_component(light1, POSITION);
        printf("eid:%d (position: %f,%f,%f)\n", light1, p->x, p->y, p->z );
    }

    assert( ecs_get_component(player, POSITION)->x == (frames * 1));
    assert( ecs_get_component(player, POSITION)->y == (frames * 2));

    assert( ecs_has_component(player, POSITION) );
    assert(!ecs_has_component(player, COLOR) );

    assert( ecs_add_component(player, COLOR) );
    assert( ecs_has_component(player, COLOR) );

    assert( ecs_off_component(player, POSITION) );
    assert(!ecs_has_component(player, POSITION) );
    assert( ecs_get_component(player, POSITION) == NULL);
    assert( ecs_use_component(player, POSITION) );
    assert( ecs_has_component(player, POSITION) );
    assert( ecs_get_component(player, POSITION)->x == (frames * 1));

    assert( ecs_del_component(player, POSITION) );
    assert(!ecs_has_component(player, POSITION) );
    assert( ecs_get_component(player, POSITION) == NULL );

    assert( ~puts("Ok") );
}
#endif
