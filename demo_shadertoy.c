// shadertoy viewer
// - rlyeh, public domain

// @todo: multipass
// - https://www.shadertoy.com/view/Mst3Wr - la calanque
// - https://www.shadertoy.com/view/XsyGWV - sirenian dawn
// - https://www.shadertoy.com/view/Xst3zX - wordtoy
// - https://www.shadertoy.com/view/MddGzf - bricks game
// - https://www.shadertoy.com/view/Ms33WB - post process - ssao

#include "fwk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
    iTime,
    iDate,
    iGlobalTime,
    iGlobalFrame,
    iGlobalDelta,
    iChannel0,
    iChannel1,
    iChannel2,
    iChannel3,
    iResolution,
    iMouse,
    iOffset,
    iSampleRate,
    iChannelResolution,
    iChannelTime,
    // iCameraScreen
    // iCameraPosition
    // iCameraActive
};
int uniforms[32];
handle vao, program;
int texture_channels[4] = {0};
const char *shaderfile = "shadertoys/!default.fs"; // default shader

void shadertoy_init(void) {
    glGenVertexArrays(1, &vao);

    // Uses gl_VertexID to draw a fullscreen quad without vbo
    const char *vs = "#version 130\n"
        "uniform vec2 iResolution;           // viewport resolution (in pixels)\n"
        "out vec2 texCoord;\n"
        "void main() {\n"
            "   texCoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
            "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
            "   texCoord = texCoord * iResolution;\n"
        "}\n";

    const char *header = "#version 130\n"
        "#define texture2D texture\n"
        "uniform float      iGlobalTime;           // shader playback time (in seconds)\n"
        "uniform float      iGlobalDelta;          // ??\n"
        "uniform float      iGlobalFrame;          // ??\n"
        "uniform float      iSampleRate;           // ??\n"
        "uniform float      iTime;                 // ??\n"
        "uniform float      iChannelTime[4];       // channel playback time (in seconds)\n"
        "uniform vec2       iResolution;           // viewport resolution (in pixels)\n"
        "uniform vec3       iChannelResolution[4]; // channel resolution (in pixels)\n"
        "uniform vec3       iOffset;               // ?? (0,0,0)\n"
        "uniform vec4       iMouse;                // mouse pixel coords. xy: hover, zw: LMB click)\n"
        "uniform vec4       iDate;                 // (year, month, day, time in seconds)\n"
        "uniform sampler2D  iChannel0;             // input channel 0\n" /*sampler%s*/
        "uniform sampler2D  iChannel1;             // input channel 1\n"
        "uniform sampler2D  iChannel2;             // input channel 2\n"
        "uniform sampler2D  iChannel3;             // input channel 3\n"
        "in  vec2 texCoord;\n"
        "out vec4 fragColor;\n"
        "void mainImage( out vec4 fragColor, in vec2 fragCoord );\n"
        "void main() {\n"
        "   mainImage(fragColor, texCoord.xy);\n"
        "}\n";

    char *file = vfs_read(shaderfile);
    if( !file ) return;

    char *fs = (char*)MALLOC(64*1024);
    snprintf(fs, 64*1024, "%s%s", header, file);
    program = shader(vs, fs, "", "fragColor");
    FREE(fs);

    if( strstr(file, "noise3.jpg"))
    texture_channels[0] = texture("shadertoys/tex12.png", 0).id;
    else
    texture_channels[0] = texture("shadertoys/tex04.jpg", 0).id;

    uniforms[iTime] = glGetUniformLocation(program, "iTime");
    uniforms[iDate] = glGetUniformLocation(program, "iDate");
    uniforms[iGlobalTime] = glGetUniformLocation(program, "iGlobalTime");
    uniforms[iGlobalDelta] = glGetUniformLocation(program, "iGlobalDelta");
    uniforms[iGlobalFrame] = glGetUniformLocation(program, "iGlobalFrame");
    uniforms[iResolution] = glGetUniformLocation(program, "iResolution");

    uniforms[iChannel0] = glGetUniformLocation(program, "iChannel0");
    uniforms[iChannel1] = glGetUniformLocation(program, "iChannel1");
    uniforms[iChannel2] = glGetUniformLocation(program, "iChannel2");
    uniforms[iChannel3] = glGetUniformLocation(program, "iChannel3");

    //uniforms[iMouse] = glGetUniformLocation(program, "iMouse");
    //uniforms[iOffset] = glGetUniformLocation(program, "iOffset");
    //uniforms[iSampleRate] = glGetUniformLocation(program, "iSampleRate");
    //uniforms[iChannelResolution] = glGetUniformLocation(program, "iChannelResolution");
    //uniforms[iChannelTime] = glGetUniformLocation(program, "iChannelTime");
    // iCameraScreen
    // iCameraPosition
    // iCameraActive
}

void shadertoy() {
    if(program && vao)
    {
        static int frame = 0;
        static float clickx = 0, clicky = 0;
        if(input(MOUSE_L)) clickx = input(MOUSE_X), clicky = input(MOUSE_Y);

        time_t tmsec = time(0);
        struct tm *tm = localtime(&tmsec);

        static uint64_t old = 0, t = 0;
        uint64_t now = time_ms();
        uint64_t delta = now - old;
        old = now;
        t += delta;

        glUseProgram(program);
        glUniform1f(uniforms[iGlobalTime], t / 1000.f );
        glUniform1f(uniforms[iGlobalFrame], frame++);
        glUniform1f(uniforms[iGlobalDelta], delta / 1000.f );
        glUniform2f(uniforms[iResolution], window_width(), window_height());
        //glUniform4f(uniforms[iMouse], (float)mouse('X'), (float)mouse('Y'), clickx, clicky );

        glUniform1f(uniforms[iTime], time_ss());

        glUniform4f(uniforms[iDate],
            tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600);

        int unit = 0;
        for(int i=0; i<4; i++) {
            if(texture_channels[i]) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(GL_TEXTURE_2D, texture_channels[i]);
                glUniform1i(uniforms[iChannel0+i], unit);
                unit++;
            }
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

int main(int argc, char **argv) {
    if( argc > 1 ) shaderfile = argv[1];

    window_create(75, 0); // WINDOW_MSAA8);
    window_title("FWK");

    const char **browser = vfs_list("shadertoys/*.fs");
    int browser_count = 0; while(browser[browser_count]) browser_count++;

    while(window_swap()) {
        // selector
        int next = input_down(KEY_UP) || input_down(KEY_LEFT);
        int prev = input_down(KEY_DOWN) || input_down(KEY_RIGHT);
        static int selector = 0;
        static int reload = 1;
        if( next ) if( selector > 0 ) --selector, reload = 1;
        if( prev ) if( selector < browser_count - 1 ) ++selector, reload = 1;
        if( reload ) {
            reload = 0;
            shaderfile = stringf("%s", browser[selector]);
            printf("loading %s ...\n", shaderfile);
            window_title(stringf("FWK - %s", browser[selector]));
            shadertoy_init();
        }

        // draw
        shadertoy();

        // UI
        if( ui_begin("Shadertoy", 0)) {
            if( ui_list(stringf("Using %s", browser[selector]), browser, browser_count, &selector) ) {
                reload = 1;
            }
            ui_end();
        }
    }
}
