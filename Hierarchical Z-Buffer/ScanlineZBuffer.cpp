#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// 线框选色
const GLubyte colorR = 255, colorG = 0, colorB = 255;

double Sfps;

/*struct Triangle;	// 提前声明

struct ClassifiedEdge {
	//	分类边
	//	x_up：边上端点的x坐标
	//	y_up：边上端点的y坐标
	//	y_down：边下端点的y坐标
	//	dx：相邻两条扫描线交点的x坐标差dx(-1 / k)
	//	z_up：边上端点的z坐标
	//	trianglePointer：指向所在的三角形

	float x_up, y_up, x_down, y_down, dx, z_up;
	Triangle* trianglePointer;
};

struct ActiveEdge {
	//	活化边（边对）
	//	xl：左交点的x坐标
	//	dxl：(左交点边上)两相邻扫描线交点的x坐标之差
	//	xr、dxr：右边的交点的两个对应分量；其含义与左边交点一样
	//	zl：左交点处多边形所在平面的深度值；
	//	dzx：沿扫描线向右走过一个像素时，多边形所在平面的深度增量。对于平面方程， dzx＝-a/c(c≠0)；
	//	dzy：沿y方向向下移过一根扫描线时，多边形所在平面的深度增量。对于平面方程， dzy＝b/c(c≠0)；
	//	yl_down、yr_down：左右边下端点的y坐标
	//	trianglePointer：指向所在的三角形
	//	ce1、ce2：指向所包含的两条分类边
	float xl, dxl, xr, dxr, zl, dzx, dzy;
	float yl_down, yr_down;
	Triangle* trianglePointer;
	ClassifiedEdge* ce1, * ce2;
};

struct Triangle {
	//	活化/分类三角形
	//	a, b, c, d：多边形所在平面的方程系数，ax + by + cz + d = 0
	//	edgePointers：指向包含的边
	//	y_down：三角形下端点的y坐标
	vector<ClassifiedEdge*> edgePointers;
	float a, b, c, d;
	float y_down;
};

// 建立边表和三角形表
vector<ActiveEdge*> activeEdgeTable;
vector<Triangle*> classifiedTriangleTable[WINDOW_HEIGHT];
vector<Triangle*> activeTriangleTable;*/
/*// 生成分类边
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

// 生成分类三角形
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

// 生成分类三角形表
void genTables()
{
	// 清空原有的表
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

// 生成活化边（边对）
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

// 模拟RGB帧缓冲区
GLubyte Spixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// 模拟z缓冲区
//float Szbuffer[WINDOW_WIDTH];
float Szbuffer[WINDOW_WIDTH][WINDOW_WIDTH];

struct Edge {
	/*	边
	*	x_up：边上端点的x坐标
	*	y_up：边上端点的y坐标
	*	y_down：边下端点的y坐标
	*	dx：相邻两条扫描线交点的x坐标差dx(-1 / k)
	*	z_up：边上端点的z坐标
	*	trianglePointer：指向所在的三角形
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

// 光栅化三角形（核心）
void SrasterizeTriangle()
{
	/*// 生成新的分类三角形表
	genTables();*/

	// 遍历屏幕上的每一行
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
	{
		// 把帧缓冲区的相应行置成底色，把z缓冲区的各个单元置成最小值
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Spixels[y][x * 3 + 0] = 255;
			Spixels[y][x * 3 + 1] = 255;
			Spixels[y][x * 3 + 2] = 255;
			Szbuffer[x][y] = -1e5;
		}

		/*// 检查分类的三角形表，如果有新的三角形涉及该扫描线，则把它放入活化的三角形表中
		for (Triangle* triangle : classifiedTriangleTable[y])
		{
			activeTriangleTable.push_back(triangle);
			// 把该三角形在oxy平面上的投影和扫描线相交的边加入到活化边表中
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
			// 从活化边表中删除已经完成的边
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
		// 没有被删除的活化边和重建的活化边可以进入后续更新环节
			else
			{
				// 依据活化边表更新深度
				int xl = max(0, int(activeEdgeTable[i]->xl));
				int xr = min(WINDOW_WIDTH - 1, int(activeEdgeTable[i]->xr));
				float zx = activeEdgeTable[i]->zl;
				for (int x = xl; x <= xr; x++)
				{
					if (zx > Szbuffer[x])
					{
						Szbuffer[x] = zx;
						// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
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
				// 活化边推进到下一条扫描线
				activeEdgeTable[i]->xl += activeEdgeTable[i]->dxl;
				activeEdgeTable[i]->ce1->x_up = activeEdgeTable[i]->xl;
				activeEdgeTable[i]->xr += activeEdgeTable[i]->dxr;
				activeEdgeTable[i]->ce2->x_up = activeEdgeTable[i]->xr;
				activeEdgeTable[i]->zl += activeEdgeTable[i]->dzx * activeEdgeTable[i]->dxl + activeEdgeTable[i]->dzy;
				activeEdgeTable[i]->ce1->z_up = activeEdgeTable[i]->zl;
				activeEdgeTable[i]->ce2->z_up = activeEdgeTable[i]->zl + activeEdgeTable[i]->dzx * (activeEdgeTable[i]->ce2->x_up - activeEdgeTable[i]->ce1->x_up);
			}

		// 从活化三角形表中删除已经完成的三角形
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
		// 初始化三角形，按y从大到小排列顶点
		Vertex* v1 = &vertices[faces[i].v1];
		Vertex* v2 = &vertices[faces[i].v2];
		Vertex* v3 = &vertices[faces[i].v3];
		if (v1->y < v2->y)
			swap(v1, v2);
		if (v2->y < v3->y)
			swap(v2, v3);
		if (v1->y < v2->y)
			swap(v1, v2);
		
		// 建立边
		Edge* e1 = SbuildEdge(v1, v2);
		Edge* e2 = SbuildEdge(v1, v3);
		Edge* e3 = SbuildEdge(v2, v3);
		
		// 计算三角形方程
		float a = (v2->y - v1->y) * (v3->z - v1->z) - (v2->z - v1->z) * (v3->y - v1->y);
		float b = (v2->z - v1->z) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->z - v1->z);
		float c = (v2->x - v1->x) * (v3->y - v1->y) - (v2->y - v1->y) * (v3->x - v1->x);
		float d = -(a * v1->x + b * v1->y + c * v1->z);

		// 计算三角形分割点
		int yu = int(v1->y);
		int ym = int(v2->y);
		int yd = int(v3->y);

		// e1在左，e2在右，画上半三角
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
					// 更新zbuffer的深度
					Szbuffer[x][y] = z;

					// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
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
			// 活化边推进到下一条扫描线
			xl += e1->dx;
			xr += e2->dx;
			zl += dzx * e1->dx + dzy;
		}

		// e2在左，e3在右，画下半三角
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
					// 更新zbuffer的深度
					Szbuffer[x][y] = z;

					// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
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
			// 活化边推进到下一条扫描线
			xl += e2->dx;
			xr += e3->dx;
			zl += dzx * e2->dx + dzy;
		}

		// 释放边指针空间
		delete(e1);
		delete(e2);
		delete(e3);
	}

}

// 这是一个纹理
GLuint Stex;

void SZBDisplay()
{
	// 记录开始时间
	auto t1 = std::chrono::high_resolution_clock::now();

	// 旋转被绘制物体
	rotate();

	// 光栅化
	SrasterizeTriangle();

	// 把像素添加到纹理中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Spixels);

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
	Sfps = 1000000.0 / duration;
	printf("SFPS: %f\n", Sfps);

	// 请求重绘
	glutPostRedisplay();
}

float ScanlineZBuffer(int argc, char** argv)
{
	glutCreateWindow("Scanline Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// 创建、绑定、设置纹理
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