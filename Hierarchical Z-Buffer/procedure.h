#pragma once
#include <string>
#define MAX_VERTICES 100000
#define MAX_FACES 200000
#define MAX_NORMALS 100000
#define MAX_TEXTURES 100000
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 720

using namespace std;

extern int numVertices;
extern int numFaces;
extern int numNormals;
extern int numTextures;

extern int frameN;
extern int frameS;
extern int frameB;
extern int frameH;

extern float timeN;
extern float timeS;
extern float timeB;
extern float timeH;

struct Vertex
{
    float x, y, z;
};

struct Face
{
    int v1, v2, v3;
    int vn1, vn2, vn3;
    int vt1, vt2, vt3;
};

struct Normal
{
    float nx, ny, nz;
};

struct Texture
{
    float u, v;
};

extern Vertex vertices[];
extern Face faces[];
extern Normal normals[];
extern Texture textures[];

void readOBJ(string filename);
void adjustOBJ();
void rotate();
void NoneZBuffer(int argc, char** argv);
void ScanlineZBuffer(int argc, char** argv);
void BaselineHierarchialZBuffer(int argc, char** argv);
void HierarchialZBuffer(int argc, char** argv);
void summary();