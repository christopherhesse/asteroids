/* This file was automatically generated.  Do not edit! */
#define GLSWAP(x, y)  \
    do {              \
        GLuint t = x; \
        x = y;        \
        y = t;        \
    } while (0)
void glPopGroupMarker();
void glPushGroupMarker(char *str);
typedef double float64;
void frame(float64 t);
typedef float float32;
typedef union {
    struct {
        float32 x;
        float32 y;
    };
    float32 m[2];
}V2;
typedef struct {
    int Width;
    int Height;
    int Length;
    V2 *Data;
}Model;
typedef struct {
    float32 m[9];
}M3;
typedef struct {
    bool Active;
    V2 Position;
    V2 Velocity;
    float32 Intensity;
    float32 Rotation;      // radians
    float32 RotationSpeed; // radians per second
    GLuint VBO;
    GLuint VAO;
    Model Model;
    float64 Created;
    M3 Transform;
    int AsteroidSize;
}Entity;
void drawEntities(Entity *entities,int count);
V2 V2Add(V2 vl,V2 vr);
M3 M3Rotate(M3 m,float32 radians);
void update(float64 t,float64 deltaT);
V2 V2MultiplyScalar(V2 v,float32 s);
typedef union {
    struct {
        float32 x;
        float32 y;
        float32 z;
    };
    float32 m[3];
}V3;
typedef struct {
    float32 m[16];
}M4;
V3 M4MultiplyV3(M4 m,V3 v);
extern const M4 M4Identity;
M4 M4Rotate(M4 m,float32 radians,V3 axis);
int findInactiveEntity(Entity *entities,int count);
void destroyAsteroid(float t,Entity *a);
bool collision(V2 center,Entity *e);
M3 M3Scale(M3 m,V2 v);
float32 Rand(float32 min,float32 max);
extern Model ModelAsteroid4;
extern Model ModelAsteroid3;
extern Model ModelAsteroid2;
extern Model ModelAsteroid1;
extern Model ModelShip;
extern Model ModelBullet;
extern const char *const LineFragmentSource;
extern const char *const LineVertexSource;
extern const char *const CopyFragmentSource;
extern const char *const CopyVertexSource;
extern const char *const BlendFragmentSource;
extern const char *const BlendVertexSource;
extern const char *const BlurFragmentSource;
extern const char *const BlurVertexSource;
extern const char *const ThresholdFragmentSource;
extern const char *const ThresholdVertexSource;
extern const char *const DecayFragmentSource;
extern const char *const DecayVertexSource;
GLuint compileProgram(const char *const vertexShaderSource,const char *const fragmentShaderSource,char *name);
void setupCanvas();
void setup();
void setupFullscreenQuad(GLuint program,GLuint *array);
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLuint createModelVAO(GLuint buffer);
GLuint createModelBuffer(Model m);
V2 M3MultiplyV2(M3 ml,V2 vr);
M3 M3Translate(M3 m,V2 v);
extern const M3 M3Identity;
typedef struct {
	int Height;
	int Length;
	Model Characters[];
}ModelFont;
extern ModelFont BattleFont;
Model createTextModel(char *text);
void checkFramebuffer();
void glLabelObject(GLenum type,GLuint object,char *label);
void generateTextureFramebuffer(GLuint *texture,GLuint *framebuffer,int scale,char *name);
void touchInput(float x,float y);
void keyboardInput(int code,bool down);
extern float64 LastBulletTime;
extern float64 LastTimestamp;
extern float64 DeltaTime;
extern float64 Time;
extern bool PauseMode;
extern bool InputAccelerate;
extern bool InputFire;
extern bool InputRotateRight;
extern bool InputRotateLeft;
extern float32 QuadData[];
extern M3 Projection;
extern GLuint VAOAsteroid4;
extern GLuint VAOAsteroid3;
extern GLuint VAOAsteroid2;
extern GLuint VAOAsteroid1;
extern GLuint VAOShip;
extern GLuint VAOBullet;
extern Entity *ScoreText;
extern Entity *Texts;
extern Entity *ExplosionParticles;
extern Entity *Ship;
extern Entity *Bullets;
extern Entity *Asteroids;
#define AsteroidCount 128
#define BulletCount 3
#define TextCount 16
#define ExplosionParticleCount 256
#define EntityCount 1 + AsteroidCount + BulletCount + TextCount + ExplosionParticleCount
extern Entity Entities[EntityCount];
typedef uint8_t uint8;
extern uint8 ImageBuffer[2048 *2048 *4];
extern GLuint CubeVertexArray;
extern GLuint CopyVertexArray;
extern GLuint CopyProgram;
extern GLuint LineUniformOffset;
extern GLuint LineUniformIntensity;
extern GLuint LineUniformWidth;
extern GLuint LineUniformResolution;
extern GLuint LineUniformTransform;
extern GLuint LineProgram;
extern GLuint BlendUniformBloom;
extern GLuint BlendUniformOriginal;
extern GLuint BlendFramebuffer;
extern GLuint BlendTexture;
extern GLuint BlendVertexArray;
extern GLuint BlendProgram;
extern GLuint BlurFramebufferB;
extern GLuint BlurTextureB;
extern GLuint BlurFramebufferA;
extern GLuint BlurTextureA;
extern GLuint BlurVertexArray;
extern GLuint BlurHorizontal;
extern GLuint BlurProgram;
extern GLuint DecayUniformDecay;
extern GLuint DecayVertexArray;
extern GLuint DecayProgram;
extern GLuint QuarterFramebuffer;
extern GLuint QuarterTexture;
extern GLuint HalfFramebuffer;
extern GLuint HalfTexture;
extern GLuint ThresholdVertexArray;
extern GLuint ThresholdProgram;
extern GLuint PreviousFramebuffer;
extern GLuint PreviousTexture;
extern GLuint CurrentFramebuffer;
#define INVALID -2 // invalid value for uniforms/attributes
extern GLuint CurrentTexture;
extern int Score;
extern float LineWidthPixels;
extern int ScreenHeight;
extern int ScreenWidth;
extern GLuint ScreenRenderbuffer;
extern GLuint ScreenFramebuffer;
#define AsteroidSizeLarge 3
#define AsteroidSizeMedium 2
#define AsteroidSizeSmall 1
#define DecayConstant 0.5f // higher = longer trails
#define ScoreIntensity 0.2
#define ShipIntensity 0.2
#define BulletIntensity 10.0
#define AsteroidIntensity 0.2
#define BulletInterval 0.2
#define ExplosionParticleExpiration 1.0
#define BulletExpiration 1.0
#define INTERFACE 0
