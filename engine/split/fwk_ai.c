// AI framework
// - rlyeh, public domain.
//
// [src] original A-star code by @mmozeiko (PD) - https://gist.github.com/mmozeiko/68f0a8459ef2f98bcd879158011cc275
// [src] original swarm/boids code by @Cultrarius (UNLICENSE) - https://github.com/Cultrarius/Swarmz

// boids/swarm -----------------------------------------------------------------

vec3 rnd3() { // random uniform
    float theta = randf() * C_PI * 2;
    float r = sqrt(randf());
    float z = sqrt(1.0f - r * r) * (randf() > 0.5f ? -1.0f : 1.0f);
    return vec3(r * cos(theta), r * sin(theta), z);
}
int less3(vec3 *lhs, vec3 *rhs) {
    if(lhs->x != rhs->x) return lhs->x - rhs->x;
    if(lhs->y != rhs->y) return lhs->y - rhs->y;
    if(lhs->z != rhs->z) return lhs->z - rhs->z;
    return 0;
}
uint64_t hash3(vec3 *v) {
    uint64_t h1 = hash_flt(v->x);
    uint64_t h2 = hash_flt(v->y);
    uint64_t h3 = hash_flt(v->z);
    return (h1 * 31 + h2) * 31 + h3;
}
vec3 clamplen3(vec3 v, float length) {
    return len3(v) <= length ? v : scale3(norm3(v), length);
}

float transform_distance(float distance, SWARM_DISTANCE type) {
    float quad;
    /**/ if (type == SWARM_DISTANCE_LINEAR)            return distance;
    else if (type == SWARM_DISTANCE_QUADRATIC)         return distance * distance;
    else if (type == SWARM_DISTANCE_INVERSE_LINEAR)    return distance == 0 ? 0 : 1 / distance;
    else if (type == SWARM_DISTANCE_INVERSE_QUADRATIC) return (quad = distance * distance), (quad == 0 ? 0 : 1 / quad);
    return distance; // throw exception instead?
}

typedef struct nearby_boid_t {
    boid_t *boid;
    vec3 direction;
    float distance;
} nearby_boid_t;

static
vec3 get_voxel_for_boid(float perception_radius, const boid_t *b) { // quantize position
    float r = absf(perception_radius);
    return vec3( (int)(b->position.x / r), (int)(b->position.y / r), (int)(b->position.z / r) );
}

static
void check_voxel_for_boids(float perception_radius, float blindspot_angledeg_compare_value, array(boid_t*) voxel_cached, array(nearby_boid_t) *result, const vec3 voxelPos, const boid_t *b) {
    for each_array_ptr(voxel_cached, const boid_t*, test) {
        vec3 p1 = b->position;
        vec3 p2 = (*test)->position;
        vec3 vec = sub3(p2, p1);
        float distance = len3(vec);

        float compare_value = 0;
        float l1 = len3(vec);
        float l2 = len3(b->velocity);
        if (l1 != 0 && l2 != 0) {
            compare_value = dot3(neg3(b->velocity), vec) / (l1 * l2);
        }

        if ((&b) != test && distance <= perception_radius && (blindspot_angledeg_compare_value > compare_value || len3(b->velocity) == 0)) {
            nearby_boid_t nb;
            nb.boid = (boid_t*)*test;
            nb.distance = distance;
            nb.direction = vec;
            array_push(*result, nb);
        }
    }
}

static
array(nearby_boid_t) get_nearby_boids(const swarm_t *self, const boid_t *b) {
    array(nearby_boid_t) result = 0;
    array_reserve(result, array_count(self->boids));

    vec3 voxelPos = get_voxel_for_boid(self->perception_radius, b);
    voxelPos.x -= 1;
    voxelPos.y -= 1;
    voxelPos.z -= 1;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                array(boid_t*) *found = map_find(self->voxel_cache_, &voxelPos);
                if( found ) check_voxel_for_boids(self->perception_radius, self->blindspot_angledeg_compare_value_, *found, &result, voxelPos, b);
                voxelPos.z++;
            }
            voxelPos.z -= 3;
            voxelPos.y++;
        }
        voxelPos.y -= 3;
        voxelPos.x++;
    }
    return result;
}

static
void update_boid(swarm_t *self, boid_t *b) {
    vec3 separation_sum = {0};
    vec3 heading_sum = {0};
    vec3 position_sum = {0};
    vec3 po = b->position;

    array(nearby_boid_t) nearby = get_nearby_boids(self, b); // @leak

    for each_array_ptr(nearby, nearby_boid_t, closeboid_t) {
        if (closeboid_t->distance == 0) {
            separation_sum = add3(separation_sum, scale3(rnd3(), 1000)); //addscale3
        }
        else {
            float separation_factor = transform_distance(closeboid_t->distance, self->separation_type);
            separation_sum = add3(separation_sum, scale3(neg3(closeboid_t->direction), separation_factor)); // addscale3
        }
        heading_sum = add3(heading_sum, closeboid_t->boid->velocity); // inc3
        position_sum = add3(position_sum, closeboid_t->boid->position); // inc3
    }

    vec3 steering_target = b->position;
    float target_distance = -1;
    for( int i = 0, end = array_count(self->steering_targets); i < end; ++i ) {
        vec3 *target = &self->steering_targets[i];

        float distance = transform_distance(len3(sub3(*target,b->position)), self->steering_target_type);
        if (target_distance < 0 || distance < target_distance) {
            steering_target = *target;
            target_distance = distance;
        }
    }

    int nearby_size = array_count(nearby);

    // Separation: steer to avoid crowding local flockmates
    vec3 separation = nearby_size > 0 ? scale3(separation_sum, 1.f / nearby_size) : separation_sum;

    // Alignment: steer towards the average heading of local flockmates
    vec3 alignment = nearby_size > 0 ? scale3(heading_sum, 1.f / nearby_size) : heading_sum;

    // Cohesion: steer to move toward the average position of local flockmates
    vec3 avgposition = nearby_size > 0 ? scale3(position_sum, 1.f / nearby_size) : b->position;
    vec3 cohesion = sub3(avgposition, b->position);

    // Steering: steer towards the nearest target location (like a moth to the light)
    vec3 steering = scale3(norm3(sub3(steering_target, b->position)), target_distance);

    // calculate boid acceleration
    vec3 acceleration;
    acceleration = scale3(separation, self->separation_weight);
    acceleration = add3(acceleration, scale3(alignment, self->alignment_weight));
    acceleration = add3(acceleration, scale3(cohesion, self->cohesion_weight));
    acceleration = add3(acceleration, scale3(steering, self->steering_weight));
    b->acceleration = clamplen3(acceleration, self->max_acceleration);
}

swarm_t swarm() {
    swarm_t self = {0};

    self.boids = NULL;

    self.perception_radius = 3; // 30

    self.separation_weight = 0.1; // 1
    self.separation_type = SWARM_DISTANCE_INVERSE_QUADRATIC;

    self.alignment_weight = 0.1; // 1
    self.cohesion_weight = 0.1; // 1

    self.steering_weight = 0.1; // 0.1
    // array_push(self.steering_targets, vec3(0,0,0));
    self.steering_target_type = SWARM_DISTANCE_LINEAR;

    self.blindspot_angledeg = 2; // 20
    self.max_acceleration = 1; // 10;
    self.max_velocity = 2; // 20;

    self.blindspot_angledeg_compare_value_ = 0; // = cos(M_PI * 2 * blindspot_angledeg / 360)
    map_init(self.voxel_cache_, less3, hash3);
    return self;
}

void swarm_update_acceleration_only(swarm_t *self) {
    self->perception_radius += !self->perception_radius; // 0->1

    // build voxel cache
    map_clear(self->voxel_cache_);
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];

        vec3 *key = MALLOC(sizeof(vec3));
        *key = get_voxel_for_boid(self->perception_radius, b);
        array(boid_t*) *found = map_find_or_add_allocated_key( self->voxel_cache_, key, 0 );
        array_push(*found, b);
    }

    // update all boids
    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        update_boid(self, b);
    }
}

void swarm_update_acceleration_and_velocity_only(swarm_t *self, float delta) {
    self->blindspot_angledeg_compare_value_ = cosf(C_PI * 2 * self->blindspot_angledeg / 360.0f);
    swarm_update_acceleration_only(self);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->velocity = clamplen3(add3(b->velocity, scale3(b->acceleration, delta)), self->max_velocity);
    }
}

void swarm_update(swarm_t *self, float delta) {
    swarm_update_acceleration_and_velocity_only(self, delta);

    for( int i = 0, end = array_count(self->boids); i < end; ++i ) {
        boid_t *b = &(self->boids)[i];
        b->prev_position = b->position;
        b->position = add3(b->position, scale3(b->velocity, delta));
    }
}

int ui_swarm(swarm_t *self) {
    const char *distances[] = {
    "Linear",
    "Inverse Linear",
    "Quadratic",
    "Inverse Quadratic"
    };

    int rc = 0;
    rc |= ui_float( "Perception Radius", &self->perception_radius);
    ui_separator();

    rc |= ui_float( "Separation Weight", &self->separation_weight);
    rc |= ui_radio( "Separation Type", distances, countof(distances), (int*)&self->separation_type);
    ui_separator();

    rc |= ui_float( "Alignment Weight", &self->alignment_weight);
    rc |= ui_float( "Cohesion Weight", &self->cohesion_weight);
    ui_separator();

    rc |= ui_float( "Steering Weight", &self->steering_weight);
    //array(vec3) steering_targets;
    rc |= ui_radio( "Steering Target Type", distances, countof(distances), (int*)&self->steering_target_type);
    ui_separator();

    rc |= ui_float( "Blindspot Angle", &self->blindspot_angledeg);
    rc |= ui_float( "Max Acceleration", &self->max_acceleration);
    rc |= ui_float( "Max Velocity", &self->max_velocity);

    return rc;
}

// pathfinding -----------------------------------------------------------------

static
int pathfind_astar(int width, int height, const unsigned* map, vec2i src, vec2i dst, vec2i* path, size_t maxpath) {
#define ALLOW_DIAGONAL_MOVEMENT 1

#if ALLOW_DIAGONAL_MOVEMENT
    #define ASTAR_DIR_COUNT 8
#else
    #define ASTAR_DIR_COUNT 4
#endif

    static const vec2i dir[ASTAR_DIR_COUNT] =
    {
        {  1,  0 },
        {  0,  1 },
        { -1,  0 },
        {  0, -1 },
#if ALLOW_DIAGONAL_MOVEMENT
        {  1,  1 },
        {  1, -1 },
        { -1,  1 },
        { -1, -1 },
#endif
    };

    #define ASTAR_POS_TYPE vec2i
    #define ASTAR_POS_START src
    #define ASTAR_POS_FINISH dst
    #define ASTAR_POS_INDEX(p) ((p).y * width + (p).x)
    #define ASTAR_MAX_INDEX (width * height)
    #define ASTAR_INDEX_POS(p, i) \
        do {                      \
            (p).x = (i) % width;  \
            (p).y = (i) / width;  \
        } while (0)
    #define ASTAR_POS_EQUAL(a, b) ((a).x == (b).x && (a).y == (b).y)
    #define ASTAR_MAP_IS_FREE(p) ((p).y >= 0 && (p).y < height && (p).x >= 0 && (p).x < width && (char)map[(p).y * width + (p).x] == 0)
    #define ASTAR_NEXT_POS(p, i) \
        do {                     \
            (p).x += dir[i].x;   \
            (p).y += dir[i].y;   \
        } while (0)
    #define ASTAR_PREV_POS(p, i) \
        do {                     \
            (p).x -= dir[i].x;   \
            (p).y -= dir[i].y;   \
        } while (0)
    #define ASTAR_GET_COST(a, b) (abs((a).x - (b).x) + abs((a).y - (b).y))

#if ALLOW_DIAGONAL_MOVEMENT
    #define ASTAR_EXTRA_COST(i) (i < 4 ? 5 : 7) // 7/5 is approx sqrt(2)
    #define ASTAR_COST_MUL 5
#endif

    size_t path_count = 0;
    #define ASTAR_PATH(p) if (path_count < maxpath) path[path_count++] = p

    // tempwork memory, not thread-safe.
    #define ASTAR_TEMP_SIZE (ASTAR_MAX_INDEX * (sizeof(unsigned)*2) + sizeof(unsigned)*4) // (16<<20)
    #define ASTAR_TEMP temp
    static array(char) ASTAR_TEMP; do_once array_resize(ASTAR_TEMP, ASTAR_TEMP_SIZE);

    // #if 1  "astar.h"
    {
        // generic A* pathfinding

        //
        // INTERFACE
        //

        // mandatory macros

        #ifndef ASTAR_POS_TYPE
        #error ASTAR_POS_TYPE should specify position type
        #endif

        #ifndef ASTAR_POS_START
        #error ASTAR_POS_START should specify start position
        #endif

        #ifndef ASTAR_POS_FINISH
        #error ASTAR_POS_FINISH should specify finish position
        #endif

        #ifndef ASTAR_POS_INDEX
        #error ASTAR_POS_INDEX(p) should specify macro to map position to index
        #endif

        #ifndef ASTAR_MAX_INDEX 
        #error ASTAR_MAX_INDEX should specify max count of indices the position can map to
        #endif

        #ifndef ASTAR_INDEX_POS
        #error ASTAR_INDEX_POS(i) should specify macro to map index to position
        #endif

        #ifndef ASTAR_POS_EQUAL
        #error ASTAR_POS_EQUAL(a, b) should specify macro to compare if two positions are the same
        #endif

        #ifndef ASTAR_MAP_IS_FREE
        #error ASTAR_MAP_IS_FREE(p) should specify macro to check if map at position p is free
        #endif

        #ifndef ASTAR_NEXT_POS
        #error ASTAR_NEXT_POS(p, i) should specify macro to get next position in specific direction
        #endif

        #ifndef ASTAR_PREV_POS
        #error ASTAR_PREV_POS(p, i) should specify macro to get previous position from specific direction
        #endif

        #ifndef ASTAR_DIR_COUNT
        #error ASTAR_DIR_COUNT should specify possible direction count
        #endif

        #ifndef ASTAR_GET_COST
        #error ASTAR_GET_COST(a, b) should specify macro to get get cost between two positions
        #endif

        #ifndef ASTAR_PATH
        #error ASTAR_PATH(p) should specify macro that will be invoked on each position for path (in reverse order), including start/finish positions
        #endif

        #if !defined(ASTAR_TEMP) || !defined(ASTAR_TEMP_SIZE)
        #error ASTAR_TEMP and ASTAR_TEMP_SIZE should specify variable & size for temporary memory (should be at least ASTAR_MAX_INDEX * 4 + extra)
        #endif

        // optional macros

        // adds extra cost for specific direction (useful for increasing cost for diagonal movements)
        #ifndef ASTAR_EXTRA_COST
        #define ASTAR_EXTRA_COST(i) 1
        #endif

        // multiplier for adding cost values (current_cost + mul * new_cost) - useful when using extra cost for diagonal movements
        #ifndef ASTAR_COST_MUL
        #define ASTAR_COST_MUL 1
        #endif

        //
        // IMPLEMENTATION
        //

        #if ASTAR_DIR_COUNT <= 4
        #define ASTAR_DIR_BITS 2
        #elif ASTAR_DIR_COUNT <= 8
        #define ASTAR_DIR_BITS 3
        #elif ASTAR_DIR_COUNT <= 16
        #define ASTAR_DIR_BITS 4
        #elif ASTAR_DIR_COUNT <= 32
        #define ASTAR_DIR_BITS 5
        #elif ASTAR_DIR_COUNT <= 64
        #define ASTAR_DIR_BITS 6
        #else
        #error Too many elements for ASTAR_DIR_COUNT, 64 is max
        #endif

        #define ASTAR_COST_BITS (32 - 1 - ASTAR_DIR_BITS)

        #define ASTAR_HEAP_SWAP(a, b) \
        do {                          \
            heapnode t = heap[a];     \
            heap[a] = heap[b];        \
            heap[b] = t;              \
        } while (0)                   \

        #define ASTAR_HEAP_PUSH(idx, c)                   \
        do {                                              \
            heap[heap_count].index = idx;                 \
            heap[heap_count].cost = c;                    \
            int i = heap_count++;                         \
            int p = (i - 1) / 2;                          \
            while (i != 0 && heap[p].cost > heap[i].cost) \
            {                                             \
                ASTAR_HEAP_SWAP(i, p);                    \
                i = p;                                    \
                p = (i - 1) / 2;                          \
            }                                             \
        } while (0) 

        #define ASTAR_HEAP_POP()                                          \
        do {                                                              \
            heap[0] = heap[--heap_count];                                 \
            int i = 0;                                                    \
            for (;;)                                                      \
            {                                                             \
                int l = 2 * i + 1;                                        \
                int r = 2 * i + 2;                                        \
                int s = i;                                                \
                if (l < heap_count && heap[l].cost < heap[i].cost) s = l; \
                if (r < heap_count && heap[r].cost < heap[s].cost) s = r; \
                if (s == i) break;                                        \
                ASTAR_HEAP_SWAP(i, s);                                    \
                i = s;                                                    \
          }                                                               \
        } while (0)

        typedef union {
            struct {
                unsigned int cost    : ASTAR_COST_BITS;
                unsigned int dir     : ASTAR_DIR_BITS;
                unsigned int visited : 1;
            };
            unsigned int all;
        } node;

        typedef struct {
            unsigned int index;
            unsigned int cost;
        } heapnode;

        if (ASTAR_TEMP_SIZE >= sizeof(node) * ASTAR_MAX_INDEX + sizeof(heapnode))
        {
            node* nodes = (node*)ASTAR_TEMP;
            for (unsigned int i = 0; i < ASTAR_MAX_INDEX; i++)
            {
                nodes[i].all = 0;
            }

            heapnode* heap = (heapnode*)((char*)ASTAR_TEMP + sizeof(node) * ASTAR_MAX_INDEX);
            unsigned int heap_max = (ASTAR_TEMP_SIZE - sizeof(node) * ASTAR_MAX_INDEX) / sizeof(heapnode);
            int heap_count = 0;

            ASTAR_POS_TYPE p = ASTAR_POS_START;
            unsigned int nindex = ASTAR_POS_INDEX(p);
            node* n = nodes + nindex;
            n->cost = 0;
            n->visited = 1;
            ASTAR_HEAP_PUSH(nindex, ASTAR_GET_COST(p, ASTAR_POS_FINISH));

            int found = 0;
            while (heap_count != 0)
            {
                nindex = heap[0].index;
                n = nodes + nindex;
                ASTAR_HEAP_POP();

                ASTAR_INDEX_POS(p, nindex);
                if (ASTAR_POS_EQUAL(p, ASTAR_POS_FINISH))
                {
                    found = 1;
                    break;
                }
                n->visited = 1;

                for (unsigned int i = 0; i < ASTAR_DIR_COUNT; i++)
                {
                    ASTAR_POS_TYPE next = p;
                    ASTAR_NEXT_POS(next, i);
                    if (ASTAR_MAP_IS_FREE(next))
                    {
                        unsigned int nnext_index = ASTAR_POS_INDEX(next);
                        node* nnext = nodes + nnext_index;
                        unsigned int cost = n->cost + ASTAR_EXTRA_COST(i);
                        if (nnext->visited == 0 || cost < nnext->cost)
                        {
                            nnext->cost = cost;
                            nnext->dir = i;
                            nnext->visited = 1;
                            if (heap_count == heap_max)
                            {
                                // out of memory
                                goto bail;
                            }
                            unsigned int new_cost = cost + ASTAR_COST_MUL * ASTAR_GET_COST(next, ASTAR_POS_FINISH);
                            ASTAR_HEAP_PUSH(nnext_index, new_cost);
                        }
                    }
                }
            }
        bail:

            if (found)
            {
                ASTAR_PATH(p);
                while (!ASTAR_POS_EQUAL(p, ASTAR_POS_START))
                {
                    ASTAR_PREV_POS(p, n->dir);
                    n = nodes + ASTAR_POS_INDEX(p);
                    ASTAR_PATH(p);
                }
            }
        }
        else
        {
            // not enough temp memory
        }

        #undef ASTAR_POS_TYPE
        #undef ASTAR_POS_START
        #undef ASTAR_POS_FINISH
        #undef ASTAR_POS_INDEX
        #undef ASTAR_MAX_INDEX
        #undef ASTAR_INDEX_POS
        #undef ASTAR_POS_EQUAL
        #undef ASTAR_MAP_IS_FREE
        #undef ASTAR_NEXT_POS
        #undef ASTAR_PREV_POS
        #undef ASTAR_DIR_COUNT
        #undef ASTAR_GET_COST
        #undef ASTAR_EXTRA_COST
        #undef ASTAR_COST_MUL
        #undef ASTAR_PATH
        #undef ASTAR_TEMP
        #undef ASTAR_TEMP_SIZE
        #undef ASTAR_COST_BITS
        #undef ASTAR_DIR_BITS
        #undef ASTAR_HEAP_SWAP
        #undef ASTAR_HEAP_PUSH
        #undef ASTAR_HEAP_POP
    }
    // #endif "astar.h"

    return path_count;
}
