#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <memory>
#include "AndroidOut.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <cassert>
#include <vector>
#include <helpers.hpp>
#include <backends/vertex.hpp>


class Renderer {
public:
    void init(android_app* app);
    android_app* app_;
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    EGLContext context_ = EGL_NO_CONTEXT;
    EGLint width_;
    EGLint height_;
    bool shaderNeedsNewProjectionMatrix_ = true;

    void render();

    GLuint vramTex;
    GLuint fbo;
    GLuint vao, vbo;
    GLuint vtxShader, fragShader, shader;
    u8* vramData;
    u16* vramCvt = new u16[1024 * 512];

    const char* vtxShaderSource = R"(#version 300 es
    out vec2 fragUV;

    void main() {
        const vec4 positions[4] = vec4[](
    		vec4(-1.0, 1.0, 1.0, 1.0),   // Top-left
		    vec4(1.0, 1.0, 1.0, 1.0),    // Top-right
	    	vec4(-1.0, -1.0, 1.0, 1.0),  // Bottom-left
    		vec4(1.0, -1.0, 1.0, 1.0)    // Bottom-right
	    );
        const vec2 texcoords[4] = vec2[](
		    vec2(0.0, 0.0),   // Bottom-left
		    vec2(1.0, 0.0),  // Bottom-right
		    vec2(0.0, 1.0),  // Top-left
		    vec2(1.0, 1.0)  // Top-right
	    );
        fragUV = texcoords[gl_VertexID];
        gl_Position = positions[gl_VertexID];
    }
    )";

    const char* fragShaderSource = R"(#version 300 es
    precision mediump float;

    in vec2 fragUV;

    out vec4 col;

    uniform sampler2D tex;

    void main() {
        vec4 colTemp = texture(tex, fragUV);
        uint r = (uint(floor(colTemp.r * 255.0f)) >> 3) & 0x1fu;
        uint g = (uint(floor(colTemp.g * 255.0f)) >> 3) & 0x1fu;
        uint b = (uint(floor(colTemp.b * 255.0f)) >> 3) & 0x1fu;
        uint a = colTemp.a >= 1.0f ? 1u : 0u;
        uint wholeCol = (r << 11) | (g << 6) | (b << 1) | a;
        r = (wholeCol >> 10) & 0x1fu;
        g = (wholeCol >> 5) & 0x1fu;
        b = wholeCol & 0x1fu;
        colTemp.r = float(b) / 31.0f;
        colTemp.g = float(g) / 31.0f;
        colTemp.b = float(r) / 31.0f;

        col = colTemp;
        //col = vec4(1.f, 0.f, 0.f, 1.f);
    }
    )";

    struct ShaderVtx {
        float x, y, z;
        float u, v;
    };
};