#define FWK_IMPLEMENTATION
#include "joint/fwk.h"

/*
A basic node-based UI built with Nuklear.
Builds on the node editor example included in Nuklear v1.00, with the aim of
being used as a prototype for implementing a functioning node editor.

Features:
- Nodes of different types. Currently their implementations are #included in
  the main file, but they could easily be turned into eg. a plugin system.
- Pins/pins of different types -- currently float values and colors.
- Adding and removing nodes.
- Linking nodes, with validation (one link per input, only link similar pins).
- Detaching and moving links.
- Evaluation of output values of connected nodes.
- Memory management based on fixed size arrays for links and node pointers
- Multiple node types
- Multiple pin types
- Linking between pins of the same type
- Detaching and reattaching links
- Getting value from linked node if pin is connected

Todo:
- Complete pin types.
- Allow dragging from output to input pin.
- Cut link by CTRL+clicking input pin.
- Cut link by drawing intersect line on a link.
- Group elemnts together with mouse, or LSHIFT+clicking.
- Drag groups.
- DEL elements.
- DEL groups.
- CTRL-C/CTRL-V/CTRL-X elements.
- CTRL-C/CTRL-V/CTRL-X groups.
- CTRL-Z,CTRL-Y.
- CTRL-N.
- CTRL-L,CTRL-S.
- CTRL-F.
- CTRL-Wheel Zooming.
- Allow to extend node types from Lua.

Extra todo:
- Execution Flow (see: nk_stroke_triangle, nk_fill_triangle)
- Complete missing nodes (see: nk_draw_image, nk_draw_text)
- Right-click could visualize node/board diagram as Lua script.
- Once that done, copy/pasting scripts should work within editor.

Sources:
- https://github.com/Immediate-Mode-UI/Nuklear/pull/561
- https://github.com/vurtun/nuklear/blob/master/demo/node_editor.c
*/

typedef enum pin_type_t {
    type_flow,
    type_int,type_float,
    type_block,type_texture,type_image,
    type_color,
    /*
    type_bool,
    type_char, type_string,
    type_int2, type_int3, type_int4,
    type_float2, type_float3, type_float4,
    type_array, type_map,
    */

    type_total
} pin_type_t;

struct node_pin {
    pin_type_t pin_type;
    nk_bool is_connected;
    struct node* connected_node;
    int connected_pin;
};

struct node {
    int ID;
    char name[32];
    struct nk_rect bounds;
    int input_count;
    int output_count;
    struct node_pin *inputs;
    struct node_pin *outputs;
    struct {
        float in_padding_x;
        float in_padding_y;
        float in_spacing_y;
        float out_padding_x;
        float out_padding_y;
        float out_spacing_y;
    } pin_spacing; /* Maybe this should be called "node_layout" and include the bounds? */
    struct node *next; /* Z ordering only */
    struct node *prev; /* Z ordering only */

    void* (*eval_func)(struct node*, int oIndex);
    void (*display_func)(struct nk_context*, struct node*);
};

struct node_link {
    struct node* input_node;
    int input_pin;
    struct node* output_node;
    int output_pin;
    nk_bool is_active;
};

struct node_linking {
    int active;
    struct node *node;
    int input_id;
    int input_pin;
};

struct node_editor {
    int initialized;
    struct node *node_buf[32];
    struct node_link links[64];
    struct node *output_node;
    struct node *begin;
    struct node *end;
    int node_count;
    int link_count;
    struct nk_rect bounds;
    struct node *selected;
    int show_grid;
    struct nk_vec2 scrolling;
    struct node_linking linking;
};

/* === PROTOTYPES === */
/* The node implementations need these two functions. */
/* These could/should go in a header file along with the node and node_pin structs and be #included in the node implementations */

struct node* node_editor_add(struct node_editor *editor, size_t nodeSize, const char *name, struct nk_rect bounds, int in_count, int out_count);
void* node_editor_eval_connected(struct node *node, int input_pin_number);
/* ================== */

/* === NODE TYPE IMPLEMENTATIONS === */

#define NODE_DEFAULT_ROW_HEIGHT 25


// ----------------------------------------------------------------------------------------------------
// #include "node_output.h"

struct node_type_output {
    struct node node;
    struct nk_colorf input_val;
};

struct nk_colorf *node_output_get(struct node* node) {
    struct node_type_output *output_node = (struct node_type_output*)node;
    if (!node->inputs[0].is_connected) {
        struct nk_colorf black = {0.0f, 0.0f, 0.0f, 0.0f};
        output_node->input_val = black;
    }
    return &output_node->input_val;
}

static void node_output_display(struct nk_context *ctx, struct node *node) {
    if (node->inputs[0].is_connected) {
        struct node_type_output *output_node = (struct node_type_output*)node;
        output_node->input_val = *(struct nk_colorf*)node_editor_eval_connected(node, 0);
        nk_layout_row_dynamic(ctx, 60, 1);
        nk_button_color(ctx, nk_rgba_cf(output_node->input_val));
    }
}

struct node* node_output_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_output *output_node = (struct node_type_output*)node_editor_add(editor, sizeof(struct node_type_output), "Output", nk_rect(position.x, position.y, 100, 100), 1, 0);
    if (output_node){
        output_node->node.inputs[0].pin_type = type_color;
        output_node->node.display_func = node_output_display;
    }
    return (struct node*)output_node;
}

// ----------------------------------------------------------------------------------------------------
// #include "node_float.h"

struct node_type_float {
    struct node node;
    float output_val;
};

static float *node_float_eval(struct node* node, int oIndex) {
    struct node_type_float *float_node = (struct node_type_float*)node;
    NK_ASSERT(oIndex == 0);
    return &float_node->output_val;
}

static void node_float_draw(struct nk_context *ctx, struct node *node) {
    struct node_type_float *float_node = (struct node_type_float*)node;
    nk_layout_row_dynamic(ctx, NODE_DEFAULT_ROW_HEIGHT, 1);
    float_node->output_val = nk_propertyf(ctx, "#Value:", 0.0f, float_node->output_val, 1.0f, 0.01f, 0.01f);
}

void node_float_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_float *float_node = (struct node_type_float*)node_editor_add(editor, sizeof(struct node_type_float), "Float", nk_rect(position.x, position.y, 180, 75), 0, 1);
    if (float_node)
    {
        float_node->output_val = 1.0f;
        float_node->node.display_func = node_float_draw;
        float_node->node.eval_func = (void*(*)(struct node*, int)) node_float_eval;
    }
}

// ----------------------------------------------------------------------------------------------------
// #include "node_color.h"

struct node_type_color {
    struct node node;
    float input_val[4];
    struct nk_colorf output_val;
};

static struct nk_colorf *node_color_eval(struct node* node, int oIndex)
{
    struct node_type_color *color_node = (struct node_type_color*)node;
    NK_ASSERT(oIndex == 0); /* only one output connector */

    return &color_node->output_val;
}


static void node_color_draw(struct nk_context *ctx, struct node *node)
{
    struct node_type_color *color_node = (struct node_type_color*)node;
    float eval_result; /* Get the values from connected nodes into this so the inputs revert on disconnect */
    const char* labels[4] = {"#R:","#G:","#B:","#A:"};
    float color_val[4]; /* Because we can't just loop through the struct... */
    nk_layout_row_dynamic(ctx, NODE_DEFAULT_ROW_HEIGHT, 1);
    nk_button_color(ctx, nk_rgba_cf(color_node->output_val));

    for (int i = 0; i < 4; i++)
    {
        if (color_node->node.inputs[i].is_connected) {
            eval_result = *(float*)node_editor_eval_connected(node, i);
            eval_result = nk_propertyf(ctx, labels[i], eval_result, eval_result, eval_result, 0.01f, 0.01f);
            color_val[i] = eval_result;
        }
        else {
            color_node->input_val[i] = nk_propertyf(ctx, labels[i], 0.0f, color_node->input_val[i], 1.0f, 0.01f, 0.01f);
            color_val[i] = color_node->input_val[i];
        }
    }

    color_node->output_val.r = color_val[0];
    color_node->output_val.g = color_val[1];
    color_node->output_val.b = color_val[2];
    color_node->output_val.a = color_val[3];
}

void node_color_create(struct node_editor *editor, struct nk_vec2 position)
{
    struct node_type_color *color_node = (struct node_type_color*)node_editor_add(editor, sizeof(struct node_type_color), "Color", nk_rect(position.x, position.y, 180, 190), 4, 1);
    if (color_node)
    {
        const struct nk_colorf black = {0.0f, 0.0f, 0.0f, 1.0f};

        for (int i = 0; i < color_node->node.input_count; i++)
            color_node->node.inputs[i].pin_type = type_float;
        color_node->node.outputs[0].pin_type = type_color;

        color_node->node.pin_spacing.in_padding_y += NODE_DEFAULT_ROW_HEIGHT;

        color_node->input_val[0] = 0.0f;
        color_node->input_val[1] = 0.0f;
        color_node->input_val[2] = 0.0f;
        color_node->input_val[3] = 1.0f;

        color_node->output_val = black;

        color_node->node.display_func = node_color_draw;
        color_node->node.eval_func = (void*(*)(struct node*, int)) node_color_eval;
    }
}

// ----------------------------------------------------------------------------------------------------
// #include "node_blend.h"

struct node_type_blend {
    struct node node;
    struct nk_colorf input_val[2];
    struct nk_colorf output_val;
    float blend_val;
};

static struct nk_colorf *node_blend_eval(struct node *node, int oIndex) {
    struct node_type_blend* blend_node = (struct node_type_blend*)node;
    return &blend_node->output_val;
}

static void node_blend_display(struct nk_context *ctx, struct node *node) {
    struct node_type_blend *blend_node = (struct node_type_blend*)node;
    const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
    float blend_amnt;

    nk_layout_row_dynamic(ctx, NODE_DEFAULT_ROW_HEIGHT, 1);
    for (int i = 0; i < 2; i++){
        if(node->inputs[i].is_connected) {
            blend_node->input_val[i] = *(struct nk_colorf*)node_editor_eval_connected(node, i);
        }
        else {
            blend_node->input_val[i] = blank;
        }
        nk_button_color(ctx, nk_rgba_cf(blend_node->input_val[i]));
    }

        if (node->inputs[2].is_connected) {
            blend_amnt = *(float*)node_editor_eval_connected(node, 2);
            blend_amnt = nk_propertyf(ctx, "#Blend", blend_amnt, blend_amnt, blend_amnt, 0.01f, 0.01f);
        }
        else {
            blend_node->blend_val = nk_propertyf(ctx, "#Blend", 0.0f, blend_node->blend_val, 1.0f, 0.01f, 0.01f);
            blend_amnt = blend_node->blend_val;
        }


    if(node->inputs[0].is_connected && node->inputs[1].is_connected) {
        blend_node->output_val.r = blend_node->input_val[0].r * (1.0f-blend_amnt) + blend_node->input_val[1].r * blend_amnt;
        blend_node->output_val.g = blend_node->input_val[0].g * (1.0f-blend_amnt) + blend_node->input_val[1].g * blend_amnt;
        blend_node->output_val.b = blend_node->input_val[0].b * (1.0f-blend_amnt) + blend_node->input_val[1].b * blend_amnt;
        blend_node->output_val.a = blend_node->input_val[0].a * (1.0f-blend_amnt) + blend_node->input_val[1].a * blend_amnt;
    }
    else {
        blend_node->output_val = blank;
    }
}

void node_blend_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_blend* blend_node = (struct node_type_blend*)node_editor_add(editor, sizeof(struct node_type_blend), "Blend", nk_rect(position.x, position.y, 180, 130), 3, 1);
    if (blend_node) {
        const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int i = 0; i < (int)NK_LEN(blend_node->input_val); i++)
            blend_node->node.inputs[i].pin_type = type_color;
        blend_node->node.outputs[0].pin_type = type_color;

        // blend_node->node.pin_spacing.in_padding_y = 42.0f;
        // blend_node->node.pin_spacing.in_spacing_y = 29.0f;

        for (int i = 0; i < (int)NK_LEN(blend_node->input_val); i++)
            blend_node->input_val[i] = blank;
        blend_node->output_val = blank;

        blend_node->blend_val = 0.5f;

        blend_node->node.display_func = node_blend_display;
        blend_node->node.eval_func = (void*(*)(struct node*, int)) node_blend_eval;

    }
}

/* ================================= */

#define NK_RGB3(r,g,b) {r,g,b,255}
#define BG_COLOR ((struct nk_color){60,60,60,192}) // nk_rgba(0,0,0,192)

static
struct editor_node_style {
    int pin_type;
    const char *shape;
    struct nk_color color_idle;
    struct nk_color color_hover;
} styles[] = {
    // order matters:
    { type_flow, "triangle_right", NK_RGB3(200,200,200), NK_RGB3(255,255,255) }, // if .num_links == 0
    { type_int, "circle", NK_RGB3(33,227,175), NK_RGB3(135,239,195) },
    { type_float, "circle", NK_RGB3(156,253,65), NK_RGB3(144,225,137) },
    { type_block, "circle", NK_RGB3(6,165,239), NK_RGB3(137,196,247) },
    { type_texture, "circle", NK_RGB3(148,0,0), NK_RGB3(183,137,137) },
    { type_image, "circle", NK_RGB3(200,130,255), NK_RGB3(220,170,255) },
    { type_color, "circle", NK_RGB3(252,200,35), NK_RGB3(255,217,140) },
};

#define COLOR_FLOW_HI styles[type_flow].color_hover
#define COLOR_FLOW_LO styles[type_flow].color_idle

#define GRID_SIZE 64.0f
#define GRID_COLOR ((struct nk_color)NK_RGB3(80,80,120))
#define GRID_THICKNESS 1.0f

// 4 colors: top-left, top-right, bottom-right, bottom-left
#define GRID_BG_COLORS ((struct nk_color){30,30,30,255}), ((struct nk_color){40,20,0,255}), ((struct nk_color){30,30,30,255}), ((struct nk_color){20,30,40,255})

#define LINK_THICKNESS 1.0f
#define LINK_DRAW(POINT_A,POINT_B,COLOR) do { \
    vec2 a = (POINT_A); \
    vec2 b = (POINT_B); \
    nk_stroke_line(canvas, a.x, a.y, b.x, b.y, LINK_THICKNESS, COLOR); \
} while(0)
#undef LINK_DRAW
#define LINK_DRAW(POINT_A,POINT_B,COLOR) do { \
    vec2 a = (POINT_A); \
    vec2 b = (POINT_B); \
    nk_stroke_curve(canvas, a.x, a.y, a.x+50, a.y, b.x-50, b.y, b.x, b.y, LINK_THICKNESS, COLOR); \
} while(0)
#undef LINK_DRAW
#define LINK_DRAW(POINT_A,POINT_B,COLOR) do { \
    vec2 a = (POINT_A); \
    vec2 b = (POINT_B); \
    float dist2 = len2( sub2( ptr2(&b.x), ptr2(&a.x) ) ); \
    vec2 mid_a = mix2( ptr2(&a.x), ptr2(&b.x), 0.25 ); mid_a.y += dist2/2; \
    vec2 mid_b = mix2( ptr2(&a.x), ptr2(&b.x), 0.75 ); mid_b.y += dist2/3; \
    nk_stroke_curve(canvas, a.x, a.y, mid_a.x, mid_a.y, mid_b.x, mid_b.y, b.x, b.y, LINK_THICKNESS, COLOR); \
} while(0)


#define PIN_RADIUS 12
#define PIN_THICKNESS 1.0f
#define PIN_DRAW(PIN_ADDR,POINT,RADIUS) do { \
    circle.x = (POINT).x - (RADIUS) / 2; \
    circle.y = (POINT).y - (RADIUS) / 2; \
    circle.w = circle.h = (RADIUS); \
    struct nk_color color = node_get_type_color((PIN_ADDR).pin_type); \
    if((PIN_ADDR).is_connected) \
    nk_fill_circle(canvas, circle, color); \
    else \
    nk_stroke_circle(canvas, circle, PIN_THICKNESS, color); \
} while(0)


static struct nk_color node_get_type_color(unsigned pin_type) {
    for( int i = 0; i < type_total; ++i )
        if( styles[i].pin_type == pin_type )
            return styles[i].color_idle;
    return ((struct nk_color)NK_RGB3(255,0,255));
}

static void node_editor_push(struct node_editor *editor, struct node *node) {
    if (!editor->begin) {
        node->next = NULL;
        node->prev = NULL;
        editor->begin = node;
        editor->end = node;
    } else {
        node->prev = editor->end;
        if (editor->end)
            editor->end->next = node;
        node->next = NULL;
        editor->end = node;
    }
}

static void node_editor_pop(struct node_editor *editor, struct node *node) {
    if (node->next)
        node->next->prev = node->prev;
    if (node->prev)
        node->prev->next = node->next;
    if (editor->end == node)
        editor->end = node->prev;
    if (editor->begin == node)
        editor->begin = node->next;
    node->next = NULL;
    node->prev = NULL;
}

static struct node* node_editor_find_by_id(struct node_editor *editor, int ID) {
    struct node *iter = editor->begin;
    while (iter) {
        if (iter->ID == ID)
            return iter;
        iter = iter->next;
    }
    return NULL;
}

static struct node_link* node_editor_find_link_by_output(struct node_editor *editor, struct node *output_node, int node_input_connector) {
    for( int i = 0; i < editor->link_count; i++ ) {
        if (editor->links[i].output_node == output_node &&
            editor->links[i].output_pin == node_input_connector &&
            editor->links[i].is_active == nk_true) {
            return &editor->links[i];
        }
    }
    return NULL;
}

static struct node_link* node_editor_find_link_by_input(struct node_editor *editor, struct node *input_node, int node_output_connector) {
    for( int i = 0; i < editor->link_count; i++ ) {
        if (editor->links[i].input_node == input_node &&
            editor->links[i].input_pin == node_output_connector &&
            editor->links[i].is_active == nk_true) {
            return &editor->links[i];
        }
    }
    return NULL;
}

static void node_editor_delete_link(struct node_link *link) {
    link->is_active = nk_false;
    link->input_node->outputs[link->input_pin].is_connected = nk_false;
    link->output_node->inputs[link->output_pin].is_connected = nk_false;
}

struct node* node_editor_add(struct node_editor *editor, size_t nodeSize, const char *name, struct nk_rect bounds, int in_count, int out_count) {
    static int IDs = 0;
    struct node *node = NULL;

    if ((nk_size)editor->node_count < NK_LEN(editor->node_buf)) {
        /* node_buf has unused pins */
        node = MALLOC(nodeSize);
        editor->node_buf[editor->node_count++] = node;
        node->ID = IDs++;
    } else {
        /* check for freed up pins in node_buf */
        for (int i = 0; i < editor->node_count; i++) {
            if (editor->node_buf[i] == NULL) {
                node = MALLOC(nodeSize);
                editor->node_buf[i] = node;
                node->ID = i;
                break;
            }
        }
    }
    if (node == NULL) {
        puts("Node creation failed");
        return NULL;
    }

    node->bounds = bounds;

    node->input_count = in_count;
    node->output_count = out_count;

    node->inputs = MALLOC(node->input_count * sizeof(struct node_pin));
    node->outputs = MALLOC(node->output_count * sizeof(struct node_pin));

    for (int  i = 0; i < node->input_count; i++) {
        node->inputs[i].is_connected = nk_false;
        node->inputs[i].pin_type = type_float; /* default pin type */
    }
    for (int  i = 0; i < node->output_count; i++) {
        node->outputs[i].is_connected = nk_false;
        node->outputs[i].pin_type = type_float; /* default pin type */
    }

    /* default pin spacing */
    node->pin_spacing.in_padding_x = 2;
    node->pin_spacing.in_padding_y = 32 + 25/2 + 6; // titlebar height + next half row + adjust
    node->pin_spacing.in_spacing_y = 25; // row height+border
    node->pin_spacing.out_padding_x = 3;
    node->pin_spacing.out_padding_y = 32 + 25/2 + 6; // titlebar height + next half row + adjust
    node->pin_spacing.out_spacing_y = 25; // row height+border

    strcpy(node->name, name);
    node_editor_push(editor, node);

    return node;
}

void *node_editor_eval_connected(struct node* node, int input_pin_number) {
    NK_ASSERT(node->inputs[input_pin_number].is_connected);
    return node->inputs[input_pin_number].connected_node->eval_func(node->inputs[input_pin_number].connected_node, node->inputs[input_pin_number].connected_pin);
}

static void node_editor_link(struct node_editor *editor, struct node *in_node, int in_pin, struct node *out_node, int out_pin) {
    /* Confusingly, in and out nodes/pins here refer to the inputs and outputs OF THE LINK ITSELF, not the nodes */
    struct node_link *link = NULL;

    if ((nk_size)editor->link_count < NK_LEN(editor->links)) {
        link = &editor->links[editor->link_count++];
    } else {
        for (int i = 0; i < (int)NK_LEN(editor->links); i++)
        {
            if (editor->links[i].is_active == nk_false) {
                link = &editor->links[i];
                break;
            }
        }
    }
    if (link) {
        out_node->inputs[out_pin].is_connected = nk_true;
        in_node->outputs[in_pin].is_connected = nk_true;
        out_node->inputs[out_pin].connected_node = in_node;
        out_node->inputs[out_pin].connected_pin = in_pin;

        link->input_node = in_node;
        link->input_pin = in_pin;
        link->output_node = out_node;
        link->output_pin = out_pin;
        link->is_active = nk_true;
    } else {
        puts("Too many links");
    }
}

static void node_editor_init(struct node_editor *editor) {
    if (editor->initialized) return;

    struct nk_rect total_space = nk_window_get_content_region(ui_ctx);
    struct nk_vec2 output_node_position = { total_space.w*2/3, total_space.h/3 };
    struct nk_vec2 color_node_position = { total_space.w*1/4, total_space.h/3 };

    memset(editor, 0, sizeof(*editor));

    editor->output_node = node_output_create(editor, output_node_position);
    node_color_create(editor, color_node_position);
    editor->show_grid = nk_true;

    editor->initialized = 1;
}

static int node_editor(struct node_editor *editor) {
    int n = 0;
    struct nk_rect total_space;
    const struct nk_input *in = &ui_ctx->input;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    struct node *updated = 0;

    node_editor_init(editor);

    {
        /* allocate complete window space */
        total_space = nk_window_get_content_region(ui_ctx);
        nk_layout_space_begin(ui_ctx, NK_STATIC, total_space.h, editor->node_count);
        {
            struct node *it = editor->begin;
            struct nk_rect size = nk_layout_space_bounds(ui_ctx);
            struct nk_panel *nodePanel = 0;

            //nk_fill_rect(canvas, size, 0/*rounding*/, ((struct nk_color){30,30,30,255})); // 20,30,40,255
            nk_fill_rect_multi_color(canvas, size, GRID_BG_COLORS);

            if (editor->show_grid) {
                /* display grid */
                for (float x = (float)fmod(size.x - editor->scrolling.x, GRID_SIZE); x < size.w; x += GRID_SIZE)
                    nk_stroke_line(canvas, x+size.x, size.y, x+size.x, size.y+size.h, GRID_THICKNESS, GRID_COLOR);
                for (float y = (float)fmod(size.y - editor->scrolling.y, GRID_SIZE); y < size.h; y += GRID_SIZE)
                    nk_stroke_line(canvas, size.x, y+size.y, size.x+size.w, y+size.y, GRID_THICKNESS, GRID_COLOR);
            }

            /* execute each node as a movable group */
            /* loop through nodes */
            while (it) {
                /* Output node window should not have a close button */
                nk_flags nodePanel_flags = NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_TITLE;
                if (it != editor->output_node)
                    nodePanel_flags |= NK_WINDOW_CLOSABLE;

                /* calculate scrolled node window position and size */
                nk_layout_space_push(ui_ctx, nk_rect(it->bounds.x - editor->scrolling.x,
                    it->bounds.y - editor->scrolling.y, it->bounds.w, it->bounds.h));

                /* execute node window */
                char *name = va(" " ICON_MD_MENU " %s",it->name); //< @r-lyeh added some spacing+icon because of our UI customizations

struct nk_color bak = ui_ctx->style.window.fixed_background.data.color;
ui_ctx->style.window.fixed_background.data.color = BG_COLOR;

                if (nk_group_begin(ui_ctx, name, nodePanel_flags))
                {
                    /* always have last selected node on top */

                    nodePanel = nk_window_get_panel(ui_ctx);
                    if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nodePanel->bounds) &&
                        (!(it->prev && nk_input_mouse_clicked(in, NK_BUTTON_LEFT,
                        nk_layout_space_rect_to_screen(ui_ctx, nodePanel->bounds)))) &&
                        editor->end != it)
                    {
                        updated = it;
                    }

                    if ((nodePanel->flags & NK_WINDOW_HIDDEN)) /* Node close button has been clicked */
                    {
                        /* Delete node */
                        struct node_link *link_remove;
                        node_editor_pop(editor, it);
                        for (int n = 0; n < it->input_count; n++) {
                            if ((link_remove = node_editor_find_link_by_output(editor, it, n)))
                            {
                                node_editor_delete_link(link_remove);
                            }
                        }
                        for (int n = 0; n < it -> output_count; n++) {
                            while((link_remove = node_editor_find_link_by_input(editor, it, n)))
                            {
                                node_editor_delete_link(link_remove);
                            }
                        }
                        NK_ASSERT(editor->node_buf[it->ID] == it);
                        editor->node_buf[it->ID] = NULL;
                        FREE(it->inputs);
                        FREE(it->outputs);
                        FREE(it);
                    }
                    else {

                        /* ================= NODE CONTENT ===================== */

                        it->display_func(ui_ctx, it);

                        /* ==================================================== */

                    }
                    nk_group_end(ui_ctx);

                }

ui_ctx->style.window.fixed_background.data.color = bak;

                if (!(nodePanel->flags & NK_WINDOW_HIDDEN))
                {
                    /* node pin and linking */
                    struct nk_rect bounds;
                    bounds = nk_layout_space_rect_to_local(ui_ctx, nodePanel->bounds);
                    bounds.x += editor->scrolling.x;
                    bounds.y += editor->scrolling.y;
                    it->bounds = bounds;

                    /* output pins */
                    for (int n = 0; n < it->output_count; ++n) {
                        struct nk_rect circle;
                        struct nk_vec2 pt = {nodePanel->bounds.x, nodePanel->bounds.y};
                        pt.x += nodePanel->bounds.w - PIN_RADIUS / 2 + it->pin_spacing.out_padding_x;
                        pt.y += it->pin_spacing.out_padding_y + it->pin_spacing.out_spacing_y * (n);
                        PIN_DRAW(it->outputs[n],pt,PIN_RADIUS);

                        /* start linking process */
                        /* set linking active */
                        if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true)) {
                            editor->linking.active = nk_true;
                            editor->linking.node = it;
                            editor->linking.input_id = it->ID;
                            editor->linking.input_pin = n;
                        }

                        /* draw link being dragged (from linked pin to mouse position) */
                        if (editor->linking.active && editor->linking.node == it &&
                            editor->linking.input_pin == n) {
                                LINK_DRAW(vec2(circle.x+3,circle.y+3),ptr2(&in->mouse.pos.x),COLOR_FLOW_HI);
                        }
                    }

                    /* input pins */
                    for (int n = 0; n < it->input_count; ++n) {
                        struct nk_rect circle;
                        struct nk_vec2 pt = {nodePanel->bounds.x, nodePanel->bounds.y};
                        pt.x += it->pin_spacing.in_padding_x;
                        pt.y += it->pin_spacing.in_padding_y + it->pin_spacing.in_spacing_y * (n);
                        PIN_DRAW(it->inputs[n],pt,PIN_RADIUS);

                        /* Detach link */
                        if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true) &&
                        editor->linking.active == nk_false &&
                        it->inputs[n].is_connected == nk_true) {
                            struct node_link *node_relink = node_editor_find_link_by_output(editor, it, n);
                            editor->linking.active = nk_true;
                            editor->linking.node = node_relink->input_node;
                            editor->linking.input_id = node_relink->input_node->ID;
                            editor->linking.input_pin = node_relink->input_pin;
                            node_editor_delete_link(node_relink);
                        }

                        /* Create link */
                        if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
                            nk_input_is_mouse_hovering_rect(in, circle) &&
                            editor->linking.active &&
                            editor->linking.node != it &&
                            it->inputs[n].pin_type == editor->linking.node->outputs[editor->linking.input_pin].pin_type &&
                            it->inputs[n].is_connected != nk_true) {
                            editor->linking.active = nk_false;

                            node_editor_link(editor, editor->linking.node,
                                editor->linking.input_pin, it, n);
                        }
                    }
                }
                it = it->next;
            }

            /* reset (output) linking connection */
            if (editor->linking.active && (!!input(KEY_LCTRL) || !!input(KEY_RCTRL) || nk_input_is_mouse_released(in, NK_BUTTON_LEFT))) {
                editor->linking.active = nk_false;
                editor->linking.node = NULL;
            }

            /* draw each static link */
            for (int n = 0; n < editor->link_count; ++n) {
                struct node_link *link = &editor->links[n];
                if (link->is_active == nk_true){
                    struct node *ni = link->input_node;
                    struct node *no = link->output_node;
                    struct nk_vec2 l0 = nk_layout_space_to_screen(ui_ctx, nk_vec2(ni->bounds.x + ni->bounds.w + ni->pin_spacing.out_padding_x, 3.0f + ni->bounds.y + ni->pin_spacing.out_padding_y + ni->pin_spacing.out_spacing_y * (link->input_pin)));
                    struct nk_vec2 l1 = nk_layout_space_to_screen(ui_ctx, nk_vec2(no->bounds.x + no->pin_spacing.in_padding_x, 3.0f + no->bounds.y + no->pin_spacing.in_padding_y + no->pin_spacing.in_spacing_y * (link->output_pin)));

                    l0.x -= editor->scrolling.x;
                    l0.y -= editor->scrolling.y;
                    l1.x -= editor->scrolling.x;
                    l1.y -= editor->scrolling.y;

                    struct nk_color color = node_get_type_color(no->inputs[link->output_pin].pin_type);
                    LINK_DRAW(ptr2(&l0.x), ptr2(&l1.x), color);
                }
            }

            if (updated) {
                /* reshuffle nodes to have least recently selected node on top */
                node_editor_pop(editor, updated);
                node_editor_push(editor, updated);
            }

            /* node selection */
            if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nk_layout_space_bounds(ui_ctx))) {
                it = editor->begin;
                editor->selected = NULL;
                editor->bounds = nk_rect(in->mouse.pos.x, in->mouse.pos.y, 100, 200);
                while (it) {
                    struct nk_rect b = nk_layout_space_rect_to_screen(ui_ctx, it->bounds);
                    b.x -= editor->scrolling.x;
                    b.y -= editor->scrolling.y;
                    if (nk_input_is_mouse_hovering_rect(in, b))
                        editor->selected = it;
                    it = it->next;
                }
            }

            /* contextual menu */
            if (nk_contextual_begin(ui_ctx, 0, nk_vec2(150, 220), nk_window_get_bounds(ui_ctx))) {
                struct nk_vec2 wincoords = { in->mouse.pos.x-total_space.x-50, in->mouse.pos.y-total_space.y-32 };

#if 1
            static char *filter = 0;
                static int do_filter = 0;
                if( input_down(KEY_F) ) if( input(KEY_LCTRL) || input(KEY_RCTRL) ) do_filter ^= 1;
                int choice = ui_toolbar(ICON_MD_SEARCH ";");
                if( choice == 1 ) do_filter = 1;
                if( do_filter ) {
                    ui_string(ICON_MD_CLOSE " Filter " ICON_MD_SEARCH, &filter);
                    if( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 ) { // if clicked on CANCEL icon (1st icon)
                        do_filter = 0;
                    }
                } else {
                    if( filter ) filter[0] = '\0';
                }
            char *filter_mask = filter && filter[0] ? va("*%s*", filter) : "*";
#endif

                #define ui_label_filtered(lbl) (strmatchi(lbl,filter_mask) && ui_label(lbl))

                int close = 0;
                if (ui_label_filtered("=Add Color node")) close=1,node_color_create(editor, wincoords);
                if (ui_label_filtered("=Add Float node")) close=1,node_float_create(editor, wincoords);
                if (ui_label_filtered("=Add Blend Node")) close=1,node_blend_create(editor, wincoords);
                if (ui_label_filtered(editor->show_grid ? "=Hide Grid" : "=Show Grid"))
                    close=1,editor->show_grid = !editor->show_grid;
                if(close) do_filter = 0, (filter ? filter[0] = '\0' : '\0'), nk_contextual_close(ui_ctx);
                nk_contextual_end(ui_ctx);
            }
        }
        nk_layout_space_end(ui_ctx);

        /* window content scrolling */
        if (nk_input_is_mouse_hovering_rect(in, nk_window_get_bounds(ui_ctx)) &&
            nk_input_is_mouse_down(in, NK_BUTTON_MIDDLE)) {
            editor->scrolling.x += in->mouse.delta.x;
            editor->scrolling.y += in->mouse.delta.y;
        }
    }

    return !nk_window_is_closed(ui_ctx, "NodeEdit");
}




int main() {
    static int open = 1;
    window_create(0.75, 0);
	while(window_swap() && open) {
		if( ui_window("Node editor",&open) ) {
            static struct node_editor nodeEditor = {0};
            node_editor(&nodeEditor);
            ui_window_end();
		}
	}
}
