/* This file was automatically generated.  Do not edit! */
extern int ScreenHeight;
extern int ScreenWidth;
void renderCanvas();
void drawBox(int x,int y,int width,int height);
void drawText(char *text,int x,int y);
void initCanvas();
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
typedef int32_t int32;
extern const char *const CanvasFragmentSource;
extern const char *const CanvasVertexSource;
GLuint compileProgram(const char *const vertexShaderSource,const char *const fragmentShaderSource,char *name);
typedef uint8_t uint8;
void glLabelObject(GLenum type,GLuint object,char *label);
typedef struct {
    int Width;
    int Height;
    int Chars;
    uint8_t *Data;
}Font;
extern Font Terminus32Medium;
void setupCanvas();
extern int DrawCommandsLength;
typedef float float32;
typedef union {
    struct {
        float32 x;
        float32 y;
        float32 z;
        float32 w;
    };
    float32 m[4];
}V4;
typedef struct {
    GLuint Texture;
    V4 Color;
    size_t Length;
    float32 *Data;
}DrawCommand;
#define DRAW_COMMANDS_COUNT 1024
extern DrawCommand DrawCommands[DRAW_COMMANDS_COUNT];
extern Font DrawFont;
extern V4 DrawColor;
extern GLuint CanvasColor;
extern GLuint CanvasResolution;
extern GLuint CanvasBuffer;
extern GLuint CanvasVertexArray;
extern GLuint CanvasProgram;
extern GLuint WhiteTexture;
#define INVALID -2 // invalid value for uniforms/attributes
extern GLuint FontTexture;
#define INTERFACE 0
