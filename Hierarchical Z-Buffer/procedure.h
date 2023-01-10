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
void glMain(int argc, char** argv);
float NoneZBuffer(int argc, char** argv);
float ScanlineZBuffer(int argc, char** argv);
float BaselineHierarchialZBuffer(int argc, char** argv);
float HierarchialZBuffer(int argc, char** argv);