#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// �߿�ѡɫ
const GLubyte colorR = 255, colorG = 0, colorB = 255;

double Sfps;

/*struct Triangle;	// ��ǰ����

struct ClassifiedEdge {
	//	�����
	//	x_up�����϶˵��x����
	//	y_up�����϶˵��y����
	//	y_down�����¶˵��y����
	//	dx����������ɨ���߽����x�����dx(-1 / k)
	//	z_up�����϶˵��z����
	//	trianglePointer��ָ�����ڵ�������

	float x_up, y_up, x_down, y_down, dx, z_up;
	Triangle* trianglePointer;
};

struct ActiveEdge {
	//	��ߣ��߶ԣ�
	//	xl���󽻵��x����
	//	dxl��(�󽻵����)������ɨ���߽����x����֮��
	//	xr��dxr���ұߵĽ����������Ӧ�������京������߽���һ��
	//	zl���󽻵㴦���������ƽ������ֵ��
	//	dzx����ɨ���������߹�һ������ʱ�����������ƽ����������������ƽ�淽�̣� dzx��-a/c(c��0)��
	//	dzy����y���������ƹ�һ��ɨ����ʱ�����������ƽ����������������ƽ�淽�̣� dzy��b/c(c��0)��
	//	yl_down��yr_down�����ұ��¶˵��y����
	//	trianglePointer��ָ�����ڵ�������
	//	ce1��ce2��ָ�������������������
	float xl, dxl, xr, dxr, zl, dzx, dzy;
	float yl_down, yr_down;
	Triangle* trianglePointer;
	ClassifiedEdge* ce1, * ce2;
};

struct Triangle {
	//	�/����������
	//	a, b, c, d�����������ƽ��ķ���ϵ����ax + by + cz + d = 0
	//	edgePointers��ָ������ı�
	//	y_down���������¶˵��y����
	vector<ClassifiedEdge*> edgePointers;
	float a, b, c, d;
	float y_down;
};

// �����߱�������α�
vector<ActiveEdge*> activeEdgeTable;
vector<Triangle*> classifiedTriangleTable[WINDOW_HEIGHT];
vector<Triangle*> activeTriangleTable;*/
/*// ���ɷ����
ClassifiedEdge* genClassifiedEdge(Vertex* v1, Vertex* v2)
{
	ClassifiedEdge* classifiedEdge = new ClassifiedEdge;
	classifiedEdge->x_up = v1->x;
	classifiedEdge->y_up = v1->y;
	classifiedEdge->y_down = v2->y;
	if (abs(v1->y - v2->y) >= .000001)
		classifiedEdge->dx = -1.0f / ((v1->y - v2->y) / (v1->x - v2->x));
	classifiedEdge->z_up = v1->z;
	return classifiedEdge;
}

// ���ɷ���������
Triangle* genClassifiedTriangle(Vertex* v1, Vertex* v2, Vertex* v3)
{
	Triangle* triangle = new Triangle;
	triangle->a = (v2->y - v1->y) * (v3->z - v1->z) - (v2->z - v1->z) * (v3->y - v1->y);
	triangle->b = (v2->z - v1->z) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->z - v1->z);
	triangle->c = (v2->x - v1->x) * (v3->y - v1->y) - (v2->y - v1->y) * (v3->x - v1->x);
	if (abs(triangle->a) + abs(triangle->b) + abs(triangle->c) < .000001)
		return nullptr;
	triangle->d = -(triangle->a * v1->x + triangle->b * v1->y + triangle->c * v1->z);
	triangle->y_down = min(min(v1->y, v2->y), v3->y);
	return triangle;
}

// ���ɷ��������α�
void genTables()
{
	// ���ԭ�еı�
	for (int i = 0; i < WINDOW_HEIGHT; i++)
		classifiedTriangleTable[i].clear();
	activeEdgeTable.clear();
	activeTriangleTable.clear();

	for (int i = 1; i <= numFaces; i++)
	{
		Vertex* v1 = &vertices[faces[i].v1];
		Vertex* v2 = &vertices[faces[i].v2];
		Vertex* v3 = &vertices[faces[i].v3];
		ClassifiedEdge* ce1, * ce2, * ce3;
		if (v1->y > v2->y)
			ce1 = genClassifiedEdge(v1, v2);
		else
			ce1 = genClassifiedEdge(v2, v1);
		if (v2->y > v3->y)
			ce2 = genClassifiedEdge(v2, v3);
		else
			ce2 = genClassifiedEdge(v3, v2);
		if (v3->y > v1->y)
			ce3 = genClassifiedEdge(v3, v1);
		else
			ce3 = genClassifiedEdge(v1, v3);
		Triangle* ct = genClassifiedTriangle(v1, v2, v3);
		if (ct != nullptr)
		{
			ce1->trianglePointer = ct;
			ce2->trianglePointer = ct;
			ce3->trianglePointer = ct;
			ct->edgePointers.push_back(ce1);
			ct->edgePointers.push_back(ce2);
			ct->edgePointers.push_back(ce3);
			classifiedTriangleTable[int(max(max(v1->y, v2->y), v3->y))].push_back(ct);
		}
	}
}

// ���ɻ�ߣ��߶ԣ�
ActiveEdge* genEdgePair(ClassifiedEdge* edge1, ClassifiedEdge* edge2)
{
	ActiveEdge* activeEdge = new ActiveEdge;
	if (edge1 == nullptr)
		return nullptr;
	if (edge2 == nullptr)
	{
		activeEdge->trianglePointer = edge1->trianglePointer;
		activeEdge->xl = edge1->x_up;
		activeEdge->dxl = edge1->dx;
		activeEdge->yl_down = edge1->y_down;
		activeEdge->xr = edge1->x_down;
		activeEdge->dxr = activeEdge->dxl;
		activeEdge->yr_down = activeEdge->yl_down;
		if (abs(activeEdge->trianglePointer->c) > .000001)
		{
			activeEdge->dzx = -activeEdge->trianglePointer->a / activeEdge->trianglePointer->c;
			activeEdge->dzy = activeEdge->trianglePointer->b / activeEdge->trianglePointer->c;
		}
		else
		{
			activeEdge->dzx = 0;
			activeEdge->dzy = 0;
		}
		activeEdge->zl = edge1->z_up + activeEdge->dzy * (edge1->y_up - int(edge1->y_up)) + activeEdge->dzx * edge1->dx * (edge1->y_up - int(edge1->y_up));
		activeEdge->ce1 = edge1;
		activeEdge->ce2 = edge1;
		return activeEdge;
	}

	float midx1 = (edge1->y_up - edge1->y_down) * edge1->dx / 2 + edge1->x_up;
	float midx2 = (edge2->y_up - edge2->y_down) * edge2->dx / 2 + edge2->x_up;
	if (midx2 < midx1)
		swap(edge1, edge2);
	activeEdge->trianglePointer = edge1->trianglePointer;
	activeEdge->xl = edge1->x_up + edge1->dx * (edge1->y_up - int(edge1->y_up));
	activeEdge->dxl = edge1->dx;
	activeEdge->yl_down = edge1->y_down;
	activeEdge->xr = edge2->x_up + edge2->dx * (edge2->y_up - int(edge2->y_up));
	activeEdge->dxr = edge2->dx;
	activeEdge->yr_down = edge2->y_down;
	if (abs(activeEdge->trianglePointer->c) > .000001)
	{
		activeEdge->dzx = -activeEdge->trianglePointer->a / activeEdge->trianglePointer->c;
		activeEdge->dzy = activeEdge->trianglePointer->b / activeEdge->trianglePointer->c;
	}
	else
	{
		activeEdge->dzx = 0;
		activeEdge->dzy = 0;
	}
	activeEdge->zl = edge1->z_up + activeEdge->dzy * (edge1->y_up - int(edge1->y_up)) + activeEdge->dzx * edge1->dx * (edge1->y_up - int(edge1->y_up));
	activeEdge->ce1 = edge1;
	activeEdge->ce2 = edge2;
	return activeEdge;
}*/

// ģ��RGB֡������
GLubyte Spixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// ģ��z������
//float Szbuffer[WINDOW_WIDTH];
float Szbuffer[WINDOW_WIDTH][WINDOW_WIDTH];

struct Edge {
	/*	��
	*	x_up�����϶˵��x����
	*	y_up�����϶˵��y����
	*	y_down�����¶˵��y����
	*	dx����������ɨ���߽����x�����dx(-1 / k)
	*	z_up�����϶˵��z����
	*	trianglePointer��ָ�����ڵ�������
	*/
	float x_up, y_up, x_down, y_down, dx, z_up, z_down;
};

Edge* SbuildEdge(Vertex* v1, Vertex* v2)
{
	Edge* edge = new Edge;
	edge->x_up = v1->x;
	edge->y_up = v1->y;
	edge->x_down = v2->x;
	edge->y_down = v2->y;
	if (abs(v1->y - v2->y) > .000001)
		edge->dx = -1.0f / ((v1->y - v2->y) / (v1->x - v2->x));
	edge->z_up = v1->z;
	edge->z_down = v2->z;
	return edge;
}

// ��դ�������Σ����ģ�
void SrasterizeTriangle()
{
	/*// �����µķ��������α�
	genTables();*/

	// ������Ļ�ϵ�ÿһ��
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
	{
		// ��֡����������Ӧ���óɵ�ɫ����z�������ĸ�����Ԫ�ó���Сֵ
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Spixels[y][x * 3 + 0] = 255;
			Spixels[y][x * 3 + 1] = 255;
			Spixels[y][x * 3 + 2] = 255;
			Szbuffer[x][y] = -1e5;
		}

		/*// ������������α�������µ��������漰��ɨ���ߣ�����������������α���
		for (Triangle* triangle : classifiedTriangleTable[y])
		{
			activeTriangleTable.push_back(triangle);
			// �Ѹ���������oxyƽ���ϵ�ͶӰ��ɨ�����ཻ�ı߼��뵽��߱���
			ClassifiedEdge* edge1 = nullptr, * edge2 = nullptr;

			for (ClassifiedEdge* edgePointer : triangle->edgePointers)
				if (edgePointer->y_up >= y)
					if (edge1 == nullptr)
						edge1 = edgePointer;
					else
						edge2 = edgePointer;

			ActiveEdge* activeEdge = genEdgePair(edge1, edge2);
			if (activeEdge != nullptr)
				activeEdgeTable.push_back(activeEdge);
		}

		for (int i = 0; i < activeEdgeTable.size(); i++)
			// �ӻ�߱���ɾ���Ѿ���ɵı�
			if (activeEdgeTable[i]->yl_down > y || activeEdgeTable[i]->yr_down > y)
			{
				Triangle* trianglePointer = activeEdgeTable[i]->trianglePointer;
				ClassifiedEdge* edge1 = nullptr, * edge2 = nullptr;

				for (ClassifiedEdge* edgePointer : trianglePointer->edgePointers)
					if (edgePointer->y_down <= y)
						if (edge1 == nullptr)
							edge1 = edgePointer;
						else
							edge2 = edgePointer;

				ActiveEdge* activeEdge = genEdgePair(edge1, edge2);
				if (activeEdge != nullptr)
					activeEdgeTable.push_back(activeEdge);
				delete activeEdgeTable[i];
				activeEdgeTable.erase(activeEdgeTable.begin() + i);
				i--;
			}
		// û�б�ɾ���Ļ�ߺ��ؽ��Ļ�߿��Խ���������»���
			else
			{
				// ���ݻ�߱�������
				int xl = max(0, int(activeEdgeTable[i]->xl));
				int xr = min(WINDOW_WIDTH - 1, int(activeEdgeTable[i]->xr));
				float zx = activeEdgeTable[i]->zl;
				for (int x = xl; x <= xr; x++)
				{
					if (zx > Szbuffer[x])
					{
						Szbuffer[x] = zx;
						// ��Ϊ��ʾ�����߿�ͼ���ʸ���Ե�ϲ�ɫ������Ƭ�ϱ���ɫ
						if (x == xl || x == xr)
						{
							Spixels[y][x * 3 + 0] = colorR;
							Spixels[y][x * 3 + 1] = colorG;
							Spixels[y][x * 3 + 2] = colorB;
						}
						else
						{
							Spixels[y][x * 3 + 0] = 255;
							Spixels[y][x * 3 + 1] = 255;
							Spixels[y][x * 3 + 2] = 255;
						}
					}
					zx += activeEdgeTable[i]->dzx;
				}
				// ����ƽ�����һ��ɨ����
				activeEdgeTable[i]->xl += activeEdgeTable[i]->dxl;
				activeEdgeTable[i]->ce1->x_up = activeEdgeTable[i]->xl;
				activeEdgeTable[i]->xr += activeEdgeTable[i]->dxr;
				activeEdgeTable[i]->ce2->x_up = activeEdgeTable[i]->xr;
				activeEdgeTable[i]->zl += activeEdgeTable[i]->dzx * activeEdgeTable[i]->dxl + activeEdgeTable[i]->dzy;
				activeEdgeTable[i]->ce1->z_up = activeEdgeTable[i]->zl;
				activeEdgeTable[i]->ce2->z_up = activeEdgeTable[i]->zl + activeEdgeTable[i]->dzx * (activeEdgeTable[i]->ce2->x_up - activeEdgeTable[i]->ce1->x_up);
			}

		// �ӻ�����α���ɾ���Ѿ���ɵ�������
		for (int i = 0; i < activeTriangleTable.size(); i++)
			if (int(activeTriangleTable[i]->y_down - 1) > y)
			{
				for (int j = 0; j < activeTriangleTable[i]->edgePointers.size(); j++)
				{
					if (activeTriangleTable[i]->edgePointers.size() == 1)
					{
						delete activeTriangleTable[i]->edgePointers[j];
						delete activeTriangleTable[i];
						break;
					}
					delete activeTriangleTable[i]->edgePointers[j];
				}

				activeTriangleTable.erase(activeTriangleTable.begin() + i);
				i--;
			}
		*/
	}
	
	for (int i = 1; i <= numFaces; i++)
	{
		// ��ʼ�������Σ���y�Ӵ�С���ж���
		Vertex* v1 = &vertices[faces[i].v1];
		Vertex* v2 = &vertices[faces[i].v2];
		Vertex* v3 = &vertices[faces[i].v3];
		if (v1->y < v2->y)
			swap(v1, v2);
		if (v2->y < v3->y)
			swap(v2, v3);
		if (v1->y < v2->y)
			swap(v1, v2);
		
		// ������
		Edge* e1 = SbuildEdge(v1, v2);
		Edge* e2 = SbuildEdge(v1, v3);
		Edge* e3 = SbuildEdge(v2, v3);
		
		// ���������η���
		float a = (v2->y - v1->y) * (v3->z - v1->z) - (v2->z - v1->z) * (v3->y - v1->y);
		float b = (v2->z - v1->z) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->z - v1->z);
		float c = (v2->x - v1->x) * (v3->y - v1->y) - (v2->y - v1->y) * (v3->x - v1->x);
		float d = -(a * v1->x + b * v1->y + c * v1->z);

		// ���������ηָ��
		int yu = int(v1->y);
		int ym = int(v2->y);
		int yd = int(v3->y);

		// e1����e2���ң����ϰ�����
		if (e1->x_down > e2->x_down)
			swap(e1, e2);
		float xl = max(0, int(e1->x_up));
		float xr = min(WINDOW_WIDTH - 1, int(e2->x_up));
		float zl = e1->z_up;
		float dzx = -a / c;
		float dzy = b / c;

		for (int y = yu; y > ym; y--)
		{
			float z = zl;
			int intxl = int(xl);
			int intxr = int(xr);

			for (int x = intxl; x <= intxr; x++)
			{
				if (z > Szbuffer[x][y])
				{
					// ����zbuffer�����
					Szbuffer[x][y] = z;

					// ��Ϊ��ʾ�����߿�ͼ���ʸ���Ե�ϲ�ɫ������Ƭ�ϱ���ɫ
					if (x == intxl || x == intxr)
					{
						Spixels[y][x * 3 + 0] = colorR;
						Spixels[y][x * 3 + 1] = colorG;
						Spixels[y][x * 3 + 2] = colorB;
					}
					else
					{
						Spixels[y][x * 3 + 0] = 255;
						Spixels[y][x * 3 + 1] = 255;
						Spixels[y][x * 3 + 2] = 255;
					}
					z += dzx;
				}
			}
			// ����ƽ�����һ��ɨ����
			xl += e1->dx;
			xr += e2->dx;
			zl += dzx * e1->dx + dzy;
		}

		// e2����e3���ң����°�����
		if (e1->y_down < e2->y_down)
			swap(e1, e2);
		if (e2->x_up > e3->x_up)
			swap(e2, e3);

		xl = e2->x_up + e2->dx * (e2->y_up - ym);
		xr = e3->x_up + e3->dx * (e3->y_up - ym);
		zl = e2->z_up + e2->dx * (e2->y_up - ym) * dzx + dzy * (e2->y_up - ym);

		for (int y = ym; y > yd; y--)
		{
			float z = zl;
			int intxl = int(xl);
			int intxr = int(xr);

			for (int x = intxl; x <= intxr; x++)
			{
				if (z > Szbuffer[x][y])
				{
					// ����zbuffer�����
					Szbuffer[x][y] = z;

					// ��Ϊ��ʾ�����߿�ͼ���ʸ���Ե�ϲ�ɫ������Ƭ�ϱ���ɫ
					if (x == intxl || x == intxr)
					{
						Spixels[y][x * 3 + 0] = colorR;
						Spixels[y][x * 3 + 1] = colorG;
						Spixels[y][x * 3 + 2] = colorB;
					}
					else
					{
						Spixels[y][x * 3 + 0] = 255;
						Spixels[y][x * 3 + 1] = 255;
						Spixels[y][x * 3 + 2] = 255;
					}
				}
				z += dzx;
			}
			// ����ƽ�����һ��ɨ����
			xl += e2->dx;
			xr += e3->dx;
			zl += dzx * e2->dx + dzy;
		}

		// �ͷű�ָ��ռ�
		delete(e1);
		delete(e2);
		delete(e3);
	}

}

// ����һ������
GLuint Stex;

void SZBDisplay()
{
	// ��¼��ʼʱ��
	auto t1 = std::chrono::high_resolution_clock::now();

	// ��ת����������
	rotate();

	// ��դ��
	SrasterizeTriangle();

	// ��������ӵ�������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Spixels);

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
	Sfps = 1000000.0 / duration;
	printf("SFPS: %f\n", Sfps);

	// �����ػ�
	glutPostRedisplay();
}

float ScanlineZBuffer(int argc, char** argv)
{
	glutCreateWindow("Scanline Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// �������󶨡���������
	glGenTextures(1, &Stex);
	glBindTexture(GL_TEXTURE_2D, Stex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	glutDisplayFunc(SZBDisplay);

	while (true)
	{
		glutMainLoopEvent();
		if (glutGetWindow() == 0)
			return Sfps;
	}
}