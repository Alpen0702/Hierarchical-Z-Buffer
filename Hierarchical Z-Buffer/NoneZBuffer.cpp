#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// 线框选色
const GLubyte colorR = 0, colorG = 255, colorB = 255;

double Nfps;

// 模拟RGB帧缓冲区
GLubyte Npixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// 光栅化线段
void NrasterizeLine(Vertex* v1, Vertex* v2) 
{
	if (v1->y < v2->y)
		swap(v1, v2);
	float dx = 0;
	if (abs(v1->y - v2->y) > .000001)
		dx = -1.0f / ((v1->y - v2->y) / (v1->x - v2->x));
	int yu = int(v1->y);
	int yd = int(v2->y);
	float x = v1->x;
	for (int y = yu; y > yd; y--)
	{
		Npixels[int(y)][int(x) * 3 + 0] = colorR;
		Npixels[int(y)][int(x) * 3 + 1] = colorG;
		Npixels[int(y)][int(x) * 3 + 2] = colorB;
		x += dx;
	}
	/*
	// 计算线段的起点和终点的坐标差
	float dx = v2->x - v1->x;
	float dy = v2->y - v1->y;

	// 如果起点的横坐标大于终点的横坐标，则交换两个点
	if (dx < 0) 
	{
		swap(v1, v2);
		dx = -dx;
		dy = -dy;
	}

	// 计算斜率
	float k = dy / dx;

	// 如果斜率的绝对值大于 1，则将线段旋转 90 度，使得斜率的绝对值小于等于 1
	if (abs(k) > 1) 
	{
		swap(v1->x, v1->y);
		swap(v2->x, v2->y);
		k = 1.0 / k;
		float y = v1->y;
		for (int x = v1->x; x <= v2->x; x++) {
			Npixels[int(x)][int(y) * 3 + 0] = colorR;
			Npixels[int(x)][int(y) * 3 + 1] = colorG;
			Npixels[int(x)][int(y) * 3 + 2] = colorB;
			y += k;
		}
	}
	else
	{
		float y = v1->y;
		for (int x = v1->x; x <= v2->x; x++) {
			Npixels[int(y)][x * 3 + 0] = colorR;
			Npixels[int(y)][x * 3 + 1] = colorG;
			Npixels[int(y)][x * 3 + 2] = colorB;
			y += k;
		}
	}*/
	
}

// 光栅化三角形
void NrasterizeTriangle()
{
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Npixels[y][x * 3 + 0] = 255;
			Npixels[y][x * 3 + 1] = 255;
			Npixels[y][x * 3 + 2] = 255;
		}

	for (int i = 1; i <= numFaces; i++)
	{
		Vertex* v1 = &vertices[faces[i].v1];
		Vertex* v2 = &vertices[faces[i].v2];
		Vertex* v3 = &vertices[faces[i].v3];
		NrasterizeLine(v1, v2);
		NrasterizeLine(v1, v3);
		NrasterizeLine(v2, v3);
	}

}

// 这是一个纹理
GLuint Ntex;

void NZBDisplay()
{
	
	// 记录开始时间
	auto t1 = std::chrono::high_resolution_clock::now();

	// 旋转被绘制物体
	rotate();

	// 光栅化
	NrasterizeTriangle();

	// 把像素添加到纹理中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Npixels);

	// 绘制纹理
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(-1, 1);
	glEnd();

	// 交换前后缓冲区
	glutSwapBuffers();

	// 计算FPS
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	Nfps = 1000000.0 / duration;
	printf("NFPS: %f\n", Nfps);

	// 请求重绘
	glutPostRedisplay();
}

float NoneZBuffer(int argc, char** argv)
{
	glutCreateWindow("None Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// 创建、绑定、设置纹理
	glGenTextures(1, &Ntex);
	glBindTexture(GL_TEXTURE_2D, Ntex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	glutDisplayFunc(NZBDisplay);

	while (true) 
	{
		glutMainLoopEvent();
		if (glutGetWindow() == 0) 
			return Nfps;
	}
}