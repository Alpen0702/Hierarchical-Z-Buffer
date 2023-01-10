#include "procedure.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

void readOBJ(string filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error opening file" << endl;
        exit(1);
    }

    string line;
    char* token;
    char* temp;

    while (getline(file, line))
    {
        if (line[0] == 'v')
        {
            if (line[1] == 'n')
            {
                // 顶点法向
                numNormals++;
                if (numNormals > MAX_VERTICES)
                {
                    cerr << "Too many normals" << endl;
                    exit(1);
                }
                token = strtok_s((char*)line.c_str(), " ", &temp);
                token = strtok_s(NULL, " ", &temp);
                normals[numNormals].nx = atof(token);
                token = strtok_s(NULL, " ", &temp);
                normals[numNormals].ny = atof(token);
                token = strtok_s(NULL, " ", &temp);
                normals[numNormals].nz = atof(token);
            }
            else if (line[1] == 't')
            {
                // 顶点纹理
                numTextures++;
                if (numTextures > MAX_TEXTURES)
                {
                    cerr << "Too many textures" << endl;
                    exit(1);
                }
                token = strtok_s((char*)line.c_str(), " ", &temp);
                token = strtok_s(NULL, " ", &temp);
                textures[numTextures].u = atof(token);
                token = strtok_s(NULL, " ", &temp);
                textures[numTextures].v = atof(token);
            }
            else
            {
                // 顶点坐标
                numVertices++;
                if (numVertices > MAX_VERTICES)
                {
                    cerr << "Too many vertices" << endl;
                    exit(1);
                }
                token = strtok_s((char*)line.c_str(), " ", &temp);
                token = strtok_s(NULL, " ", &temp);
                vertices[numVertices].x = atof(token);
                token = strtok_s(NULL, " ", &temp);
                vertices[numVertices].y = atof(token);
                token = strtok_s(NULL, " ", &temp);
                vertices[numVertices].z = atof(token);
            }
        }
        else if (line[0] == 'f')
        {
            // 三角面片
            numFaces++;
            if (numFaces > MAX_FACES)
            {
                cerr << "Too many faces" << endl;
                exit(1);
            }
            token = strtok_s((char*)line.c_str(), " ", &temp);
            token = strtok_s(NULL, "/", &temp);
            faces[numFaces].v1 = atoi(token);
            token = strtok_s(NULL, "/", &temp);
            faces[numFaces].vt1 = atoi(token);
            token = strtok_s(NULL, " ", &temp);
            faces[numFaces].vn1 = atoi(token);
            token = strtok_s(NULL, "/", &temp);
            faces[numFaces].v2 = atoi(token);
            token = strtok_s(NULL, "/", &temp);
            faces[numFaces].vt2 = atoi(token);
            token = strtok_s(NULL, " ", &temp);
            faces[numFaces].vn2 = atoi(token);
            token = strtok_s(NULL, "/", &temp);
            faces[numFaces].v3 = atoi(token);
            /*token = strtok_s(NULL, "/", &temp);
            faces[numFaces].vt3 = atoi(token);
            token = strtok_s(NULL, " ", &temp);
            faces[numFaces].vn3 = atoi(token);*/
        }
    }

    file.close();
}