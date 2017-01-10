/* This file was automatically generated.  Do not edit! */
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
extern Model ModelTestShape;
extern Model ModelSmallAsteroid2;
extern Model ModelSmallAsteroid1;
extern Model ModelShip;
extern Model ModelCat;
extern Model ModelBullet;
extern Model ModelAsteroid4;
extern Model ModelAsteroid3;
extern Model ModelAsteroid2;
extern Model ModelAsteroid1;
typedef struct {
	int Height;
	int Length;
	Model Characters[];
}ModelFont;
extern ModelFont SeparatorFont;
extern ModelFont BattleFont;
#define INTERFACE 0
