#include <stdio.h>
#include <stdbool.h>

#if TARGET_OS_IPHONE
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#endif

#include "glutil.h"

#if INTERFACE

#ifdef DEBUG
#define GLCHECK()                         \
    do {                                  \
        checkGLError(__FILE__, __LINE__); \
    } while (0)
#else
#define GL_CHECK()
#endif

#define INVALID -2 // invalid value for uniforms/attributes
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define GLSWAP(x, y)  \
    do {              \
        GLuint t = x; \
        x = y;        \
        y = t;        \
    } while (0)

#endif

char *GLErrorString(GLenum error) {
    switch (error) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_COMPLETE:
            return "GL_FRAMEBUFFER_COMPLETE";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        default:
            return NULL;
    }
}

void checkGLError(char *file, int line) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }
    char *str = GLErrorString(err);
    if (str == NULL) {
        printf("gl error %lx %s:%d\n", (unsigned long)err, file, line);
    } else {
        printf("gl error %s %s:%d\n", str, file, line);
    }
}

void checkFramebuffer() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fatal("failed to make complete framebuffer object %lx", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

void glPushGroupMarker(char *str) {
    glPushGroupMarkerEXT(0, str);
}

void glPopGroupMarker() {
    glPopGroupMarkerEXT();
}

void glLabelObject(GLenum type, GLuint object, char *label) {
    glLabelObjectEXT(type, object, 0, label);
}

GLuint compileShader(const char *const source, GLuint type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == false) {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char buf[len];
        glGetShaderInfoLog(shader, len, NULL, buf);
        fatal("%s", buf);
    }
    return shader;
}

GLuint compileProgram(const char *const vertexShaderSource, const char *const fragmentShaderSource, char *name) {
    GLuint program = glCreateProgram();
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == false) {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        char buf[len];
        glGetProgramInfoLog(program, len, NULL, buf);
        fatal("%s", buf);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glLabelObject(GL_PROGRAM_OBJECT_EXT, program, name);
    return program;
}
