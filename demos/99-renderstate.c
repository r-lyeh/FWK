#include <fwk.h>

int main() {
    window_create(40, WINDOW_SQUARE | WINDOW_MSAA8);
    window_title(__FILE__);

    // set up pipeline
    renderstate_t state = renderstate();
    state.clear_color[0] = 0.2f;
    state.clear_color[1] = 0.2f;
    state.clear_color[2] = 0.2f;
    state.clear_color[3] = 1.0f;
    state.depth_test_enabled = GL_TRUE;
    state.depth_func = GL_LEQUAL;
    state.blend_enabled = GL_FALSE;
    state.cull_face_enabled = GL_TRUE;

    // prepare triangle buffer
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // basic shader
    const char *vertex_shader_src = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";

    const char *fragment_shader_src = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

    handle program = shader(vertex_shader_src, fragment_shader_src, "aPos", "FragColor", "");

    // app loop
    while( window_swap() ) {
        // input controls
        if( input(KEY_ESC) ) break;

        // bind pipeline
        renderstate_apply(&state);

        // clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw triangle
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
