#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#if TARGET_OS_IPHONE
#include <OpenGLES/ES3/gl.h>
#else
#include <OpenGL/gl3.h>
#endif

#include "Canvas.h"

#if INTERFACE

#define DRAW_COMMANDS_COUNT 1024

typedef struct {
    GLuint Texture;
    V4 Color;
    size_t Length;
    float32 *Data;
} DrawCommand;

#endif

//    drawBox(300, 300, 300, 300);
//    DrawColor = (V4){0, 0, 0, 1};
//    glPushGroupMarker("Canvas");
//    renderCanvas();
//    glPopGroupMarker();

GLuint FontTexture = INVALID;
GLuint WhiteTexture = INVALID;

GLuint CanvasProgram = INVALID;
GLuint CanvasVertexArray = INVALID;
GLuint CanvasBuffer = INVALID;
GLuint CanvasResolution = INVALID;
GLuint CanvasColor = INVALID;

V4 DrawColor;
Font DrawFont;

DrawCommand DrawCommands[DRAW_COMMANDS_COUNT];
int DrawCommandsLength = 0;

void setupCanvas() {
    DrawFont = Terminus32Medium;

    glGenTextures(1, &FontTexture);
    glBindTexture(GL_TEXTURE_2D, FontTexture);
    glLabelObject(GL_TEXTURE, FontTexture, "Font");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DrawFont.Width * DrawFont.Chars, DrawFont.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, DrawFont.Data);

    glGenTextures(1, &WhiteTexture);
    glBindTexture(GL_TEXTURE_2D, WhiteTexture);
    glLabelObject(GL_TEXTURE, WhiteTexture, "White");
    uint8 whiteData[] = {
        255, 255, 255, 255,
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData);

    CanvasProgram = compileProgram(CanvasVertexSource, CanvasFragmentSource, "Canvas");
    CanvasColor = glGetUniformLocation(CanvasProgram, "color");
    CanvasResolution = glGetUniformLocation(CanvasProgram, "resolution");

    glGenVertexArrays(1, &CanvasVertexArray);
    glBindVertexArray(CanvasVertexArray);

    glGenBuffers(1, &CanvasBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, CanvasBuffer);

    int32 stride = sizeof(float32) * 4;

    GLuint xy = glGetAttribLocation(CanvasProgram, "xy");
    glEnableVertexAttribArray(xy);
    glVertexAttribPointer(xy, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0));

    GLuint uv = glGetAttribLocation(CanvasProgram, "uv");
    glEnableVertexAttribArray(uv);
    glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(2 * sizeof(float)));
}

void initCanvas() {
    for (int i = 0; i < DrawCommandsLength; i++) {
        DrawCommand cmd = DrawCommands[i];
        free(cmd.Data);
        DrawCommands[i] = (DrawCommand){};
    }
    DrawCommandsLength = 0;
    DrawColor = (V4){1, 1, 1, 1};
}

void drawText(char *text, int x, int y) {
    size_t textLength = strlen(text);
    size_t charWidthFloats = 2 * 3 * 4; // 2 triangles * 3 vertices * 4 floats per vertex
    size_t length = textLength * charWidthFloats;

    DrawCommand cmd = {
        .Texture = FontTexture,
        .Color = DrawColor,
        .Length = length,
        .Data = malloc(length * sizeof(float32)),
    };

    float32 vbottom = (0.0 + 0.5) / (float32)DrawFont.Height;
    float32 vtop = (DrawFont.Height - 1 + 0.5) / (float32)DrawFont.Height;

    float32 ybottom = y;
    float32 ytop = y + DrawFont.Height;

    for (int i = 0; i < textLength; i++) {
        float32 xleft = x;
        float32 xright = x + DrawFont.Width;

        float32 uoffset = text[i] * DrawFont.Width;
        float32 uleft = (uoffset + 0.5) / (float32)(DrawFont.Width * DrawFont.Chars);
        float32 uright = (uoffset + DrawFont.Width - 1 + 0.5) / (float32)(DrawFont.Width * DrawFont.Chars);

        float32 data[] = {
            // bottom left
            xleft, ybottom,
            uleft, vbottom,
            // top left
            xleft, ytop,
            uleft, vtop,
            // bottom right
            xright, ybottom,
            uright, vbottom,
            // bottom right
            xright, ybottom,
            uright, vbottom,
            // top left
            xleft, ytop,
            uleft, vtop,
            // top right
            xright, ytop,
            uright, vtop,
        };
        memcpy(cmd.Data + i * charWidthFloats, data, charWidthFloats * sizeof(float32));
        x += DrawFont.Width;
    }
    DrawCommands[DrawCommandsLength] = cmd;
    DrawCommandsLength++;
}

void drawBox(int x, int y, int width, int height) {
    size_t length = 2 * 3 * 4; // 2 triangles * 3 vertices * 4 floats per vertex

    DrawCommand cmd = {
        .Texture = WhiteTexture,
        .Color = DrawColor,
        .Length = length,
        .Data = malloc(length * sizeof(float32)),
    };

    float32 ybottom = y;
    float32 ytop = y + height;

    float32 xleft = x;
    float32 xright = x + width;

    float32 data[] = {
        // bottom left
        xleft, ybottom,
        0, 0,
        // top left
        xleft, ytop,
        0, 0,
        // bottom right
        xright, ybottom,
        0, 0,
        // bottom right
        xright, ybottom,
        0, 0,
        // top left
        xleft, ytop,
        0, 0,
        // top right
        xright, ytop,
        0, 0,
    };
    memcpy(cmd.Data, data, length * sizeof(float32));
    DrawCommands[DrawCommandsLength] = cmd;
    DrawCommandsLength++;
}

void renderCanvas() {
    glUseProgram(CanvasProgram);
    glBindVertexArray(CanvasVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, CanvasBuffer);
    glUniform2f(CanvasResolution, ScreenWidth, ScreenHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < DrawCommandsLength; i++) {
        DrawCommand cmd = DrawCommands[i];

        glUniform4fv(CanvasColor, 1, cmd.Color.m);
        glBufferData(GL_ARRAY_BUFFER, cmd.Length * sizeof(float32), cmd.Data, GL_DYNAMIC_DRAW);
        glBindTexture(GL_TEXTURE_2D, cmd.Texture);
        glDrawArrays(GL_TRIANGLES, 0, (int)cmd.Length / 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}