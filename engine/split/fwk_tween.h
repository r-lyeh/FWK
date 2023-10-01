// ----------------------------------------------------------------------------
// ease

API float ease_nop(float t);
API float ease_linear(float t);

API float ease_out_sine(float t);
API float ease_out_quad(float t);
API float ease_out_cubic(float t);
API float ease_out_quart(float t);
API float ease_out_quint(float t);
API float ease_out_expo(float t);
API float ease_out_circ(float t);
API float ease_out_back(float t);
API float ease_out_elastic(float t);
API float ease_out_bounce(float t);

API float ease_in_sine(float t);
API float ease_in_quad(float t);
API float ease_in_cubic(float t);
API float ease_in_quart(float t);
API float ease_in_quint(float t);
API float ease_in_expo(float t);
API float ease_in_circ(float t);
API float ease_in_back(float t);
API float ease_in_elastic(float t);
API float ease_in_bounce(float t);

API float ease_inout_sine(float t);
API float ease_inout_quad(float t);
API float ease_inout_cubic(float t);
API float ease_inout_quart(float t);
API float ease_inout_quint(float t);
API float ease_inout_expo(float t);
API float ease_inout_circ(float t);
API float ease_inout_back(float t);
API float ease_inout_elastic(float t);
API float ease_inout_bounce(float t);

API float ease_inout_perlin(float t);

enum EASE_FLAGS {
    EASE_SINE,
    EASE_QUAD,
    EASE_CUBIC,
    EASE_QUART,
    EASE_QUINT,
    EASE_EXPO,
    EASE_CIRC,
    EASE_BACK,
    EASE_ELASTIC,
    EASE_BOUNCE,

    EASE_IN,
    EASE_OUT = 0,
    EASE_INOUT = EASE_IN * 2,

    EASE_NOP = EASE_INOUT | (EASE_BOUNCE + 1),
    EASE_LINEAR,
    EASE_INOUT_PERLIN,

    EASE_NUM
};

API float ease(float t01, unsigned fn); // / 0-to-1
API float ease_pong(float t01, unsigned fn); // \ 1-to-0
API float ease_ping_pong(float t, unsigned fn1, unsigned fn2); // /\ 0-to-1-to-0
API float ease_pong_ping(float t, unsigned fn1, unsigned fn2); // \/ 1-to-0-to-1

API const char *ease_enum(unsigned fn);
API const char**ease_enums();

// ----------------------------------------------------------------------------
// tween

typedef struct tween_keyframe_t {
    float t;
	vec3 v;
    unsigned ease;
} tween_keyframe_t;

typedef struct tween_t {
	array(tween_keyframe_t) keyframes;

	vec3 result;
	float time;
	float duration;
} tween_t;

API tween_t tween();
API void      tween_setkey(tween_t *tw, float t, vec3 v, unsigned easing_mode);
API void        tween_delkey(tween_t *tw, float t);
API float     tween_update(tween_t *tw, float dt);
API void      tween_reset(tween_t *tw);
API void    tween_destroy(tween_t *tw);

