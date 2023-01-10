#pragma once
#include "procedure.h"

int numVertices = 0;
int numFaces = 0;
int numNormals = 0;
int numTextures = 0;

int frameN = 0;
int frameS = 0;
int frameB = 0;
int frameH = 0;

float timeN;
float timeS;
float timeB;
float timeH;

Vertex vertices[MAX_VERTICES];
Face faces[MAX_FACES];
Normal normals[MAX_NORMALS];
Texture textures[MAX_TEXTURES];