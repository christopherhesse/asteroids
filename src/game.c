#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#if TARGET_OS_IPHONE
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#endif

#include "game.h"

#if INTERFACE

#define AsteroidCount 128
#define BulletCount 3
#define TextCount 16
#define ExplosionParticleCount 256
#define EntityCount 1 + AsteroidCount + BulletCount + TextCount + ExplosionParticleCount

#define BulletExpiration 1.0
#define ExplosionParticleExpiration 1.0
#define BulletInterval 0.2

#define AsteroidIntensity 0.2
#define BulletIntensity 10.0
#define ShipIntensity 0.2
#define ScoreIntensity 0.2

#define DecayConstant 0.5f // higher = longer trails

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
} Entity;

#define AsteroidSizeSmall 1
#define AsteroidSizeMedium 2
#define AsteroidSizeLarge 3

#endif

extern GLuint ScreenFramebuffer;
extern GLuint ScreenRenderbuffer;
extern int ScreenWidth;
extern int ScreenHeight;

float LineWidthPixels;
int Score;

GLuint CurrentTexture = INVALID;
GLuint CurrentFramebuffer = INVALID;
GLuint PreviousTexture = INVALID;
GLuint PreviousFramebuffer = INVALID;

GLuint ThresholdProgram = INVALID;
GLuint ThresholdVertexArray = INVALID;

GLuint HalfTexture = INVALID;
GLuint HalfFramebuffer = INVALID;
GLuint QuarterTexture = INVALID;
GLuint QuarterFramebuffer = INVALID;

GLuint DecayProgram = INVALID;
GLuint DecayVertexArray = INVALID;
GLuint DecayUniformDecay = INVALID;

GLuint BlurProgram = INVALID;
GLuint BlurHorizontal = INVALID;
GLuint BlurVertexArray = INVALID;
GLuint BlurTextureA = INVALID;
GLuint BlurFramebufferA = INVALID;
GLuint BlurTextureB = INVALID;
GLuint BlurFramebufferB = INVALID;

GLuint BlendProgram = INVALID;
GLuint BlendVertexArray = INVALID;
GLuint BlendTexture = INVALID;
GLuint BlendFramebuffer = INVALID;
GLuint BlendUniformOriginal = INVALID;
GLuint BlendUniformBloom = INVALID;

GLuint LineProgram = INVALID;
GLuint LineUniformTransform = INVALID;
GLuint LineUniformResolution = INVALID;
GLuint LineUniformWidth = INVALID;
GLuint LineUniformIntensity = INVALID;
GLuint LineUniformOffset = INVALID;

GLuint CopyProgram = INVALID;
GLuint CopyVertexArray = INVALID;

GLuint CubeVertexArray = INVALID;

uint8 ImageBuffer[2048 * 2048 * 4];

Entity Entities[EntityCount];
Entity *Asteroids;
Entity *Bullets;
Entity *Ship;
Entity *ExplosionParticles;
Entity *Texts;
Entity *ScoreText;

GLuint VAOBullet;
GLuint VAOShip;
GLuint VAOAsteroid1;
GLuint VAOAsteroid2;
GLuint VAOAsteroid3;
GLuint VAOAsteroid4;

M3 Projection;

#define Scale 0.001

float32 QuadData[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    -1.0f, 1.0f,
    1.0f, -1.0f,
    1.0f, 1.0f,
};

#define KEY_W 13
#define KEY_A 2
#define KEY_D 0
#define KEY_P 35
#define KEY_SPACE 49

bool InputRotateLeft;
bool InputRotateRight;
bool InputFire;
bool InputAccelerate;

bool PauseMode;
float64 Time;
float64 DeltaTime;
float64 LastTimestamp;

float64 LastBulletTime;

void keyboardInput(int code, bool down) {
    if (code == KEY_SPACE) {
        InputFire = down;
    } else if (code == KEY_W) {
        InputAccelerate = down;
    } else if (code == KEY_A) {
        InputRotateLeft = down;
    } else if (code == KEY_D) {
        InputRotateRight = down;
    } else if (code == KEY_P) {
        if (down) {
            PauseMode = !PauseMode;
        }
    } else {
        printf("keyboardInput code=%d\n", code);
    }
}

void touchInput(float x, float y) {
    //    if (Rotate) {
    //        RotateClockwise = !RotateClockwise;
    //    }
    //    Rotate = !Rotate;
}

void generateTextureFramebuffer(GLuint *texture, GLuint *framebuffer, int scale, char *name) {
    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
    glLabelObject(GL_FRAMEBUFFER, *framebuffer, name);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glLabelObject(GL_TEXTURE, *texture, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ScreenWidth / scale, ScreenHeight / scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);

    checkFramebuffer();
}

Model createTextModel(char *text) {
    int length = 0;
    for (int i = 0; i < strlen(text); i++) {
        Model charModel = BattleFont.Characters[text[i]];
        length += charModel.Length;
    }

    Model textModel = {
        .Height = BattleFont.Height,
    };

    V2 *data = malloc(length * sizeof(V2));
    for (int i = 0; i < strlen(text); i++) {
        Model charModel = BattleFont.Characters[text[i]];
        if (charModel.Data == NULL) {
            continue;
        }
        for (int j = 0; j < charModel.Length; j++) {
            V2 point = charModel.Data[j];
            M3 transform = M3Identity;
            transform = M3Translate(transform, (V2){textModel.Width, textModel.Height});
            data[textModel.Length] = M3MultiplyV2(transform, point);
            textModel.Length++;
        }
        textModel.Width += charModel.Width;
    }
    textModel.Data = data;
    return textModel;
}

GLuint createModelBuffer(Model m) {
    V2 data[m.Length * 6];
    for (int i = 0; i < m.Length; i += 2) {
        for (int j = 0; j < 6; j++) {
            data[i * 6 + j * 2] = m.Data[i];
            data[i * 6 + j * 2 + 1] = m.Data[i + 1];
        }
    }

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, m.Length * 6 * sizeof(V2), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
}

GLuint createModelVAO(GLuint buffer) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    GLuint attribStart = glGetAttribLocation(LineProgram, "start");
    GLuint attribEnd = glGetAttribLocation(LineProgram, "end");
    glEnableVertexAttribArray(attribStart);
    glEnableVertexAttribArray(attribEnd);
    glVertexAttribPointer(attribStart, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), BUFFER_OFFSET(0));
    glVertexAttribPointer(attribEnd, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), BUFFER_OFFSET(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return vao;
}

void setupFullscreenQuad(GLuint program, GLuint *array) {
    glGenVertexArrays(1, array);
    glBindVertexArray(*array);

    {
        GLuint arrayBuffer;
        glGenBuffers(1, &arrayBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QuadData), QuadData, GL_STATIC_DRAW);

        GLuint position = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    }

    glBindVertexArray(0);
}

void setup() {
    srand((unsigned)time(NULL));

    LineWidthPixels = ScreenHeight * 0.010;

    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glActiveTexture(GL_TEXTURE0);

    setupCanvas();

    generateTextureFramebuffer(&CurrentTexture, &CurrentFramebuffer, 1, "FrameA");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    generateTextureFramebuffer(&PreviousTexture, &PreviousFramebuffer, 1, "FrameB");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    generateTextureFramebuffer(&HalfTexture, &HalfFramebuffer, 2, "Half");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    generateTextureFramebuffer(&QuarterTexture, &QuarterFramebuffer, 4, "Quarter");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    generateTextureFramebuffer(&BlurTextureA, &BlurFramebufferA, 8, "BlurA");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    generateTextureFramebuffer(&BlurTextureB, &BlurFramebufferB, 8, "BlurB");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    generateTextureFramebuffer(&BlendTexture, &BlendFramebuffer, 1, "Blend");

    glClearColor(1.0f, 0.078f, 0.58f, 1.0f);

    DecayProgram = compileProgram(DecayVertexSource, DecayFragmentSource, "Decay");
    DecayUniformDecay = glGetUniformLocation(DecayProgram, "decay");
    setupFullscreenQuad(DecayProgram, &DecayVertexArray);

    ThresholdProgram = compileProgram(ThresholdVertexSource, ThresholdFragmentSource, "Threshold");
    setupFullscreenQuad(ThresholdProgram, &ThresholdVertexArray);

    BlurProgram = compileProgram(BlurVertexSource, BlurFragmentSource, "Blur");
    BlurHorizontal = glGetUniformLocation(BlurProgram, "horizontal");
    setupFullscreenQuad(BlurProgram, &BlurVertexArray);

    BlendProgram = compileProgram(BlendVertexSource, BlendFragmentSource, "Blend");
    setupFullscreenQuad(BlendProgram, &BlendVertexArray);
    BlendUniformOriginal = glGetUniformLocation(BlendProgram, "original");
    BlendUniformBloom = glGetUniformLocation(BlendProgram, "bloom");

    CopyProgram = compileProgram(CopyVertexSource, CopyFragmentSource, "Copy");
    setupFullscreenQuad(CopyProgram, &CopyVertexArray);

    LineProgram = compileProgram(LineVertexSource, LineFragmentSource, "Line");
    LineUniformTransform = glGetUniformLocation(LineProgram, "transform");
    LineUniformResolution = glGetUniformLocation(LineProgram, "resolution");
    LineUniformTransform = glGetUniformLocation(LineProgram, "transform");
    LineUniformWidth = glGetUniformLocation(LineProgram, "width");
    LineUniformIntensity = glGetUniformLocation(LineProgram, "intensity");
    LineUniformOffset = glGetUniformLocation(LineProgram, "offset");

    VAOBullet = createModelVAO(createModelBuffer(ModelBullet));
    VAOShip = createModelVAO(createModelBuffer(ModelShip));
    VAOAsteroid1 = createModelVAO(createModelBuffer(ModelAsteroid1));
    VAOAsteroid2 = createModelVAO(createModelBuffer(ModelAsteroid2));
    VAOAsteroid3 = createModelVAO(createModelBuffer(ModelAsteroid3));
    VAOAsteroid4 = createModelVAO(createModelBuffer(ModelAsteroid4));

    int offset = 0;
    Asteroids = &Entities[offset];
    offset += AsteroidCount;
    Bullets = &Entities[offset];
    offset += BulletCount;
    ExplosionParticles = &Entities[offset];
    offset += ExplosionParticleCount;
    Ship = &Entities[offset];
    offset += 1;
    Texts = &Entities[offset];
    offset += TextCount;

    ScoreText = &Texts[0];

    for (int i = 0; i < 1; i++) {
        Entity *e = &Asteroids[i];
        *e = (Entity){
            .Active = true,
            .Created = 0.0,
            .Intensity = AsteroidIntensity,
            .Position = (V2){Rand(-1.0, 1.0), Rand(-1.0, 1.0)},
            .Velocity = (V2){Rand(-0.1, 0.1), Rand(-0.1, 0.1)},
            .Model = ModelAsteroid4,
            .VAO = VAOAsteroid4,
            .AsteroidSize = AsteroidSizeLarge,
        };
    }

    *Ship = (Entity){
        .Active = true,
        .Intensity = ShipIntensity,
        .Model = ModelShip,
        .VAO = VAOShip,
    };

    *ScoreText = (Entity){
        .Active = true,
        .Intensity = ScoreIntensity,
        .Position = (V2){0.8, 0.8},
    };

    Projection = M3Scale(M3Identity, (V2){(float)ScreenHeight / (float)ScreenWidth, 1.0});
}

bool collision(V2 center, Entity *e) {
    V2 points[e->Model.Length];
    for (int k = 0; k < e->Model.Length; k++) {
        points[k] = M3MultiplyV2(e->Transform, e->Model.Data[k]);
    }

    bool inside = false;
    for (int k = 0; k < e->Model.Length; k += 2) {
        V2 p1 = points[k];
        V2 p2 = points[k + 1];
        if ((p1.y > center.y) != (p2.y > center.y) && (center.x < (p2.x - p1.x) * (center.y - p1.y) / (p2.y - p1.y) + p1.x)) {
            inside = !inside;
        }
    }
    return inside;
}

void destroyAsteroid(float t, Entity *a) {
    for (int k = 0; k < 64; k++) {
        int index = findInactiveEntity(ExplosionParticles, ExplosionParticleCount);
        if (index == -1) {
            break;
        }

        // generate random spherical direction
        M4 rotation = M4Rotate(M4Identity, Rand(-0.5, 0.5) * M_PI, (V3){1, 0, 0});
        rotation = M4Rotate(rotation, Rand(0, 1.0) * 2 * M_PI, (V3){0, 0, 1});
        V3 direction = M4MultiplyV3(rotation, (V3){1, 0, 0});
        V2 velocity = V2MultiplyScalar((V2){direction.x, direction.y}, 0.2);
        ExplosionParticles[index] = (Entity){
            .Active = true,
            .Velocity = velocity,
            .Position = a->Position,
            .Created = t,
            .Intensity = Rand(0.5, 0.8),
            .Model = ModelBullet,
            .VAO = VAOBullet,
        };
    }

    if (a->AsteroidSize > AsteroidSizeSmall) {
        // create two new asteroids
        for (int asteroidNumber = 0; asteroidNumber < 2; asteroidNumber++) {
            int index = findInactiveEntity(Asteroids, AsteroidCount);
            if (index == -1) {
                continue;
            }

            Asteroids[index] = (Entity){
                .Active = true,
                .Created = t,
                .Intensity = AsteroidIntensity,
                .Position = a->Position,
                .Velocity = (V2){Rand(-0.1, 0.1), Rand(-0.1, 0.1)},
                .Model = ModelAsteroid4,
                .VAO = VAOAsteroid4,
                .AsteroidSize = a->AsteroidSize - 1,
            };
        }
    }

    a->Active = false;
    Score++;
}

void update(float64 t, float64 deltaT) {
    if (InputRotateLeft) {
        Ship->Rotation = Ship->Rotation - 3.0 / 180.0 * M_PI;
    }

    if (InputRotateRight) {
        Ship->Rotation = Ship->Rotation + 3.0 / 180.0 * M_PI;
    }

    Ship->Velocity = V2MultiplyScalar(Ship->Velocity, 0.99);
    if (InputAccelerate) {
        // add velocity based on current direction
        M3 transform = M3Rotate(M3Identity, Ship->Rotation);
        V2 deltaV = M3MultiplyV2(transform, (V2){0, 0.02});
        Ship->Velocity = V2Add(Ship->Velocity, deltaV);
    }

    if (InputFire && t - LastBulletTime > BulletInterval) {
        int index = findInactiveEntity(Bullets, BulletCount);
        if (index != -1) {
            LastBulletTime = t;
            M3 transform = M3Rotate(M3Identity, Ship->Rotation);
            V2 velocity = M3MultiplyV2(transform, (V2){0, 3});
            V2 position = V2Add(Ship->Position, M3MultiplyV2(transform, (V2){0, (float32)ModelShip.Height * Scale / 2.0}));
            Bullets[index] = (Entity){
                .Active = true,
                .Created = t,
                .Intensity = BulletIntensity,
                .Position = position,
                .Velocity = velocity,
                .Model = ModelBullet,
                .VAO = VAOBullet,
            };
        }
    }

    for (int i = 0; i < BulletCount; i++) {
        Entity *b = &Bullets[i];

        if (t - b->Created > BulletExpiration) {
            b->Active = false;
        }
    }

    for (int i = 0; i < ExplosionParticleCount; i++) {
        Entity *p = &ExplosionParticles[i];
        if (!p->Active) {
            continue;
        }
        if (t - p->Created > ExplosionParticleExpiration) {
            p->Active = false;
            continue;
        }

        p->Intensity = p->Intensity * 0.98;
    }

    for (int i = 0; i < EntityCount; i++) {
        Entity *e = &Entities[i];
        if (!e->Active) {
            continue;
        }

        e->Rotation = e->Rotation + e->RotationSpeed * deltaT;
        V2 pos = e->Position;
        pos = V2Add(pos, V2MultiplyScalar(e->Velocity, deltaT));

        if (pos.x > 1) {
            pos.x = pos.x - 2;
        } else if (pos.x < -1) {
            pos.x = pos.x + 2;
        }

        if (pos.y > 1) {
            pos.y = pos.y - 2;
        } else if (pos.y < -1) {
            pos.y = pos.y + 2;
        }

        e->Position = pos;

        Model model = e->Model;

        M3 m = Projection;
        m = M3Translate(m, e->Position);
        m = M3Scale(m, (V2){Scale, Scale});
        if (e->AsteroidSize == AsteroidSizeMedium) {
            m = M3Scale(m, (V2){0.5, 0.5});
        } else if (e->AsteroidSize == AsteroidSizeSmall) {
            m = M3Scale(m, (V2){0.25, 0.25});
        }
        m = M3Rotate(m, e->Rotation);
        m = M3Translate(m, (V2){(float)-model.Width / 2.0, (float)-model.Height / 2.0});
        e->Transform = m;
    }

    // check for intersection of bullet and asteroid
    for (int j = 0; j < AsteroidCount; j++) {
        Entity *a = &Asteroids[j];

        if (!a->Active || a->Created == t) {
            continue;
        }

        for (int i = 0; i < BulletCount; i++) {
            Entity *b = &Bullets[i];

            if (!b->Active) {
                continue;
            }

            // check if center of bullet is inside asteroid
            if (!collision(b->Position, a)) {
                continue;
            }

            destroyAsteroid(t, a);
            b->Active = false;
        }

        if (collision(Ship->Position, a)) {
            destroyAsteroid(t, a);
        }
    }

    // update score
    {
        if (ScoreText->Model.Data != NULL) {
            free(ScoreText->Model.Data);
            glDeleteVertexArrays(1, &ScoreText->VAO);
            glDeleteBuffers(1, &ScoreText->VBO);
        }

        char buf[8];
        snprintf(buf, 8, "%d", Score);
        ScoreText->Model = createTextModel(buf);
        ScoreText->VBO = createModelBuffer(ScoreText->Model);
        ScoreText->VAO = createModelVAO(ScoreText->VBO);
    }
}

int findInactiveEntity(Entity *entities, int count) {
    for (int i = 0; i < count; i++) {
        if (!entities[i].Active) {
            return i;
        }
    }
    return -1;
}

void drawEntities(Entity *entities, int count) {
    GLuint lastVAO = 0;
    for (int i = 0; i < count; i++) {
        Entity e = entities[i];
        if (!e.Active) {
            continue;
        }

        if (lastVAO != e.VAO) {
            glBindVertexArray(e.VAO);
            lastVAO = e.VAO;
        }
        glUniform2f(LineUniformOffset, Rand(-1, 1), Rand(-2, 2));
        glUniform1f(LineUniformIntensity, e.Intensity);
        glUniformMatrix3fv(LineUniformTransform, 1, 0, e.Transform.m);
        Model model = e.Model;
        glDrawArrays(GL_TRIANGLES, 0, model.Length / 2 * 6);
    }
}

void frame(float64 t) {
    if (LastTimestamp == 0) {
        LastTimestamp = t;
    }
    float64 deltaT = t - LastTimestamp;
    LastTimestamp = t;

    if (PauseMode) {
        // don't increment time in PauseMode
        // also no need to update the world
    } else {
        Time += deltaT;
        update(Time, deltaT);
    }

    //    initCanvas();

    glBindFramebuffer(GL_FRAMEBUFFER, CurrentFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushGroupMarker("Previous Frame");
    glUseProgram(DecayProgram);
    glBindVertexArray(DecayVertexArray);
    glUniform1f(DecayUniformDecay, DecayConstant);
    glBindTexture(GL_TEXTURE_2D, PreviousTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glPopGroupMarker();

    glPushGroupMarker("Entities");
    glUseProgram(LineProgram);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    glUniform2f(LineUniformResolution, ScreenWidth, ScreenHeight);
    glUniform1f(LineUniformWidth, LineWidthPixels);

    int offset = (ScreenWidth - ScreenHeight) / 2;

    glScissor(offset, 0, ScreenWidth - offset * 2, ScreenHeight);
    glEnable(GL_SCISSOR_TEST);
    drawEntities(Asteroids, AsteroidCount);
    drawEntities(Bullets, BulletCount);
    drawEntities(ExplosionParticles, ExplosionParticleCount);
    drawEntities(Ship, 1);
    glDisable(GL_SCISSOR_TEST);
    drawEntities(Texts, TextCount);

    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glPopGroupMarker();

    // resize in stages so that small version of texture is sampled from all input pixels
    glPushGroupMarker("Threshold");
    glViewport(0, 0, ScreenWidth / 2, ScreenHeight / 2);
    glBindFramebuffer(GL_FRAMEBUFFER, HalfFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(ThresholdProgram);
    glBindVertexArray(ThresholdVertexArray);
    glBindTexture(GL_TEXTURE_2D, CurrentTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glPopGroupMarker();

    glUseProgram(CopyProgram);
    glBindVertexArray(CopyVertexArray);

    glPushGroupMarker("Resize to Quarter");
    glViewport(0, 0, ScreenWidth / 4, ScreenHeight / 4);
    glBindFramebuffer(GL_FRAMEBUFFER, QuarterFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, HalfTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopGroupMarker();

    glPushGroupMarker("Resize to Eighth");
    glViewport(0, 0, ScreenWidth / 8, ScreenHeight / 8);
    glBindFramebuffer(GL_FRAMEBUFFER, BlurFramebufferA);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, QuarterTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopGroupMarker();

    glUseProgram(0);
    glBindVertexArray(0);

    glPushGroupMarker("Blur");
    GLuint BlurReadFramebuffer = BlurFramebufferA;
    GLuint BlurReadTexture = BlurTextureA;
    GLuint BlurWriteFramebuffer = BlurFramebufferB;
    GLuint BlurWriteTexture = BlurTextureB;

    glUseProgram(BlurProgram);
    glBindVertexArray(BlurVertexArray);
    //    int blurFactor = (int)(LineWidthPixels / 8);
    int blurFactor = 1;
    if (blurFactor == 0) {
        blurFactor = 1;
    }

    // only blur once in vertical direction
    glBindFramebuffer(GL_FRAMEBUFFER, BlurWriteFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, BlurReadTexture);
    glUniform1ui(BlurHorizontal, false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    GLSWAP(BlurReadFramebuffer, BlurWriteFramebuffer);
    GLSWAP(BlurReadTexture, BlurWriteTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, BlurWriteFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, BlurReadTexture);
    glUniform1ui(BlurHorizontal, true);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    GLSWAP(BlurReadFramebuffer, BlurWriteFramebuffer);
    GLSWAP(BlurReadTexture, BlurWriteTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, BlurWriteFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, BlurReadTexture);
    glUniform1ui(BlurHorizontal, false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    GLSWAP(BlurReadFramebuffer, BlurWriteFramebuffer);
    GLSWAP(BlurReadTexture, BlurWriteTexture);

    glUseProgram(0);
    glBindVertexArray(0);
    glPopGroupMarker();

    glViewport(0, 0, ScreenWidth, ScreenHeight);

    glPushGroupMarker("Blend");
    glBindFramebuffer(GL_FRAMEBUFFER, ScreenFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(BlendProgram);
    glBindVertexArray(BlendVertexArray);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CurrentTexture);
    glUniform1i(BlendUniformOriginal, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, BlurReadTexture);
    glUniform1i(BlendUniformBloom, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glBindVertexArray(0);
    glPopGroupMarker();

#if TARGET_OS_IPHONE
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ScreenFramebuffer);
        const GLenum discards[] = {GL_DEPTH_ATTACHMENT};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    }
#endif

    // flip textures
    {
        GLuint t = PreviousFramebuffer;
        PreviousFramebuffer = CurrentFramebuffer;
        CurrentFramebuffer = t;
    }

    {
        GLuint t = PreviousTexture;
        PreviousTexture = CurrentTexture;
        CurrentTexture = t;
    }
}
