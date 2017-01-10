/* This file was automatically generated.  Do not edit! */
GLuint compileProgram(const char *const vertexShaderSource,const char *const fragmentShaderSource,char *name);
GLuint compileShader(const char *const source,GLuint type);
void glLabelObject(GLenum type,GLuint object,char *label);
void glPopGroupMarker();
void glPushGroupMarker(char *str);
void fatal(const char *fmt,...);
void checkFramebuffer();
char *GLErrorString(GLenum error);
#define GLSWAP(x, y)  \
    do {              \
        GLuint t = x; \
        x = y;        \
        y = t;        \
    } while (0)
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define INVALID -2 // invalid value for uniforms/attributes
#if !(defined(DEBUG))
#define GL_CHECK()
#endif
void checkGLError(char *file,int line);
#if defined(DEBUG)
#define GLCHECK()                         \
    do {                                  \
        checkGLError(__FILE__, __LINE__); \
    } while (0)
#endif
#define INTERFACE 0
