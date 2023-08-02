#include "chonk_renderer.h"


//! executes glGetString and outputs the result to logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}
#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

void Renderer::init(android_app* app) {
    app_ = app;
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    glClearColor(1.f, 0.f, 0.f, 1.f);
    //glClear(GL_COLOR_BUFFER_BIT);

    glGenTextures(1, &vramTex);
    glBindTexture(GL_TEXTURE_2D, vramTex);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB5_A1, 1024, 512);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 0);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /*glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, vramTex, 0);*/

    vtxShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vtxShader, 1, &vtxShaderSource, NULL);
    glCompileShader(vtxShader);
    int success;
	char InfoLog[512];
	glGetShaderiv(vtxShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vtxShader, 512, NULL, InfoLog);
		aout << "Vertex shader compilation failed" << std::endl;
        aout << InfoLog << std::endl;
	}

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSource, NULL);
    glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragShader, 512, NULL, InfoLog);
		aout << "Fragment shader compilation failed" << std::endl;
        aout << InfoLog << std::endl;
	}

    shader = glCreateProgram();
    glAttachShader(shader, vtxShader);
    glAttachShader(shader, fragShader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, InfoLog);
        aout << "linking shader program failed" << std::endl;
        aout << InfoLog << std::endl;
	}

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
}

float xDpi, yDpi;

extern "C" JNIEXPORT void JNICALL Java_com_example_chonkystation_MainActivity_00024Companion_setDPI(JNIEnv* env, jobject, jfloat xDpiScale, jfloat yDpiScale) {
    aout << "set xdpi to " << xDpiScale << std::endl;
    xDpi = xDpiScale;
    yDpi = yDpiScale;
};

void Renderer::render() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
        //glViewport(0, 0, width * 96 / xDpi * 6, height * 96 / yDpi * 6 / (width / height));

        // make sure that we lazily recreate the projection matrix before we render
        shaderNeedsNewProjectionMatrix_ = true;
    }

    glClearColor(1.f, 0.f, 0.f, 1.f);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, vramTex);

    aout << "Uploading VRAM ..." << std::endl;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, vramData);
    aout << "Done." << std::endl;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
    glUseProgram(shader);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /*aout << "Binding read fb ..." << std::endl;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    //aout << "Binding draw fb ..." << std::endl;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    aout << "Blitting ..." << std::endl;
    glBlitFramebuffer(0, 512, 1024, 0, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    aout << "Done." << std::endl;*/

    const auto error = glGetError();
    if(error)
        aout << "[FATAL] GL Error: %d\n" << error;

    // Present the rendered image. This is an implicit glFlush.
    if(display_ == NULL || surface_ == NULL) return;
    auto swapResult = eglSwapBuffers(display_, surface_);
    aout << "Swapped buffers" << std::endl;
    if(swapResult != EGL_TRUE) {
        aout << "failed to swap buffers" << std::endl;
        const auto error = glGetError();
        if(error)
            Helpers::panic("[FATAL] GL Error: %d\n", error);
    }
    aout << "Done drawing" << std::endl;
}