#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// �߿�ѡɫ
const GLubyte colorR = 0, colorG = 255, colorB = 255;

double Nfps;

// ģ��RGB֡������
GLubyte Npixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// ��դ���߶�
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
	// �����߶ε������յ�������
	float dx = v2->x - v1->x;
	float dy = v2->y - v1->y;

	// ������ĺ���������յ�ĺ����꣬�򽻻�������
	if (dx < 0) 
	{
		swap(v1, v2);
		dx = -dx;
		dy = -dy;
	}

	// ����б��
	float k = dy / dx;

	// ���б�ʵľ���ֵ���� 1�����߶���ת 90 �ȣ�ʹ��б�ʵľ���ֵС�ڵ��� 1
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

// ��դ��������
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

// ����һ������
GLuint Ntex;

void NZBDisplay()
{
	
	// ��¼��ʼʱ��
	auto t1 = std::chrono::high_resolution_clock::now();

	// ��ת����������
	rotate();

	// ��դ��
	NrasterizeTriangle();

	// ��������ӵ�������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Npixels);

	// ��������
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

	// ����ǰ�󻺳���
	glutSwapBuffers();

	// ����FPS
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	Nfps = 1000000.0 / duration;
	printf("NFPS: %f\n", Nfps);

	// �����ػ�
	glutPostRedisplay();
}

float NoneZBuffer(int argc, char** argv)
{
	glutCreateWindow("None Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// �������󶨡���������
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