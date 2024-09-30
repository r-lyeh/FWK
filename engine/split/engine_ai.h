// AI framework
// - rlyeh, public domain.
//
// [src] original A-star code by @mmozeiko (PD) - https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275
// [src] original swarm/boids code by @Cultrarius (UNLICENSE) - https://github.com/Cultrarius/Swarmz

// pathfinding -----------------------------------------------------------------

API int pathfind_astar(int width, int height, const unsigned* map, vec2i src, vec2i dst, vec2i* path, size_t maxpath);

// ----------------------------------------------------------------------------
// Behavior trees: decision planning and decision making.
// Supersedes finite state-machines (FSM) and hierarchical finite state-machines (HFSM).

typedef int (*bt_func)();

typedef struct bt_t {
    uint64_t type;
    int (*action)();
    union {
        int argi;
        float argf;
    };
    array(struct bt_t) children;
} bt_t;

API bt_t    bt(const char *ini_file, unsigned flags);
API int     bt_run(bt_t *b);
API void    bt_addfun(const char *name, int(*func)());
API bt_func bt_findfun(const char *name);
API char   *bt_funcname(bt_func fn);

API int ui_bt(bt_t *b);

// boids/swarm -----------------------------------------------------------------

typedef enum SWARM_DISTANCE {
    SWARM_DISTANCE_LINEAR,
    SWARM_DISTANCE_INVERSE_LINEAR,
    SWARM_DISTANCE_QUADRATIC,
    SWARM_DISTANCE_INVERSE_QUADRATIC
} SWARM_DISTANCE;

#define boid(...) C_CAST(boid_t, __VA_ARGS__)

typedef struct boid_t {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 prev_position;
} boid_t;

typedef struct swarm_t {
    array(boid_t) boids;

    float perception_radius; // determines the vision radius of each boid. Only boids within this distance influence each other.

    float separation_weight; // how much boids repel each other
    SWARM_DISTANCE separation_type;

    float alignment_weight; // how much boids want go in the same direction
    float cohesion_weight; // how much boids want to be in the center of the swarm

    float steering_weight;
    array(vec3) steering_targets;
    SWARM_DISTANCE steering_target_type;

    float blindspot_angledeg;
    float max_acceleration; // how fast each boid can change its direction
    float max_velocity; // how fast each boid can move

    // private:
    map(vec3*, array(boid_t*)) voxel_cache_;
    float blindspot_angledeg_compare_value_;
} swarm_t;

API swarm_t swarm();
API void    swarm_update(swarm_t *self, float delta); // acc,vel,pos
API void    swarm_update_acceleration_only(swarm_t *self, float delta); // acc
API void    swarm_update_acceleration_and_velocity_only(swarm_t *self, float delta); // acc,vel

API int     ui_swarm(swarm_t *self);
