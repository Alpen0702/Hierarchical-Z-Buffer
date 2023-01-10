#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <set>
using namespace std;

// 线框选色
const GLubyte colorR = 0, colorG = 255, colorB = 255;

double Bfps;

// 模拟RGB帧缓冲区
GLubyte Bpixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// 四叉树结点
struct Node
{
	Node* father;
	vector<Node*> children;
	int level;
	float depth;
	int l, r, d, u;
	Node(Node* father, int level, float depth, int l, int r, int d, int u) : father(father), level(level), depth(depth), l(l), r(r), d(d), u(u) {};
};

// nodes是非叶结点，leaves是叶子结点
vector<Node*> nodes;
Node* root;
//Node* leaves[WINDOW_WIDTH][WINDOW_HEIGHT];

float Bzbuffer[WINDOW_WIDTH][WINDOW_HEIGHT];
Node* zbufferToNode[WINDOW_WIDTH][WINDOW_HEIGHT];

// 限制的四叉树极限深度
const int MAX_QUAD_LEVEL = 7;

// 建立四叉树
Node* buildQuadTree(Node* father, int level, int l, int r, int d, int u)
{
	// 格子面积为0，不建立
	if (l == r || d == u)
		return nullptr;
	/*// 格子恰好为一个像素，进入leaves成为叶子结点
	if (l == r - 1 && d == u - 1)
	{
		leaves[l][d] = new Node(father, level, -1e5, l, r, d, u);
		//printf("built %d %d\n", l, d);
		return leaves[l][d];
	}*/
	// 格子有面积，允许建立结点
	Node* node = new Node(father, level, -1e5, l, r, d, u);
	nodes.push_back(node);
	
	// 四叉树深度未逾越限制，且本结点不是单个像素点，则继续分割
	if (level < MAX_QUAD_LEVEL && !(l == r - 1 && d == u - 1))
	{
		int lrmid = (l + r) / 2;
		int dumid = (d + u) / 2;
		node->children.push_back(buildQuadTree(node, level + 1, l, lrmid, d, dumid));
		node->children.push_back(buildQuadTree(node, level + 1, l, lrmid, dumid, u));
		node->children.push_back(buildQuadTree(node, level + 1, lrmid, r, d, dumid));
		node->children.push_back(buildQuadTree(node, level + 1, lrmid, r, dumid, u));
	}
	// 该结点是叶子结点，建立像素对结点的指针
	else
		for (int x = l; x < r; x++)
			for (int y = d; y < u; y++)
				zbufferToNode[x][y] = node;
	return node;
}

// 重置四叉树的深度
void resetQuadTree(Node* father)
{
	father->depth = -1e5;
	for (Node* child : father->children)
		if (child)
			resetQuadTree(child);
}

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

Edge* buildEdge(Vertex* v1, Vertex* v2)
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

// 更新结点深度
void update(Node* node)
{
	float minD = 1e5;
	
	// 叶子结点
	if (node->children.empty())
	{
		for (int x = node->l; x < node->r; x++)
			for (int y = node->d; y < node->u; y++)
				if (Bzbuffer[x][y] < minD)
					minD = Bzbuffer[x][y];
	}
	// 非叶子结点
	else
		for (Node* child : node->children)
			if (child && child->depth < minD)
				minD = child->depth;

	if (abs(minD - node->depth) > .000001)
	{
		node->depth = minD;
		if (node->father)
			update(node->father);
	}
}

// 需更新深度的结点队列
Node* updateQueue[100000];

// 光栅化三角形
void BrasterizeTriangle(Vertex* v1, Vertex* v2, Vertex* v3)
{
	if (v1->y < v2->y)
		swap(v1, v2);
	if (v2->y < v3->y)
		swap(v2, v3);
	if (v1->y < v2->y)
		swap(v1, v2);

	Edge* e1 = buildEdge(v1, v2);
	Edge* e2 = buildEdge(v1, v3);
	Edge* e3 = buildEdge(v2, v3);
	float a = (v2->y - v1->y) * (v3->z - v1->z) - (v2->z - v1->z) * (v3->y - v1->y);
	float b = (v2->z - v1->z) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->z - v1->z);
	float c = (v2->x - v1->x) * (v3->y - v1->y) - (v2->y - v1->y) * (v3->x - v1->x);
	float d = -(a * v1->x + b * v1->y + c * v1->z);

	int yu = int(v1->y);
	int ym = int(v2->y);
	int yd = int(v3->y);

	int queueSize = 0;

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
			if (z > Bzbuffer[x][y])
			{
				// 若这一格原先的深度就是四叉树结点中最深的，则需要更新四叉树结点的深度
				if (abs(zbufferToNode[x][y]->depth - Bzbuffer[x][y]) < .000001)
					//update(zbufferToNode[x][y]);
					//updateQueue.push_back(zbufferToNode[x][y]);
					updateQueue[queueSize++] = zbufferToNode[x][y];
				Bzbuffer[x][y] = z;

				// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
				if (x == intxl || x == intxr)
				{
					Bpixels[y][x * 3 + 0] = colorR;
					Bpixels[y][x * 3 + 1] = colorG;
					Bpixels[y][x * 3 + 2] = colorB;
				}
				else
				{
					Bpixels[y][x * 3 + 0] = 255;
					Bpixels[y][x * 3 + 1] = 255;
					Bpixels[y][x * 3 + 2] = 255;
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
			if (z > Bzbuffer[x][y])
			{
				// 若这一格原先的深度就是四叉树结点中最深的，则需要更新四叉树结点的深度
				if (abs(zbufferToNode[x][y]->depth - Bzbuffer[x][y]) < .000001)
					//update(zbufferToNode[x][y]);
					//updateQueue.push_back(zbufferToNode[x][y]);
					updateQueue[queueSize++] = zbufferToNode[x][y];
				Bzbuffer[x][y] = z;

				// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
				if (x == intxl || x == intxr)
				{
					Bpixels[y][x * 3 + 0] = colorR;
					Bpixels[y][x * 3 + 1] = colorG;
					Bpixels[y][x * 3 + 2] = colorB;
				}
				else
				{
					Bpixels[y][x * 3 + 0] = 255;
					Bpixels[y][x * 3 + 1] = 255;
					Bpixels[y][x * 3 + 2] = 255;
				}
			}
			z += dzx;
		}
		// 活化边推进到下一条扫描线
		xl += e2->dx;
		xr += e3->dx;
		zl += dzx * e2->dx + dzy;
	}

	// 更新四叉树结点深度
	//for (Node* updateNode : updateSet)
		//update(updateNode);

	for (int i = 0; i < queueSize; i++)
		if (i == 0 || updateQueue[i - 1] != updateQueue[i])
			update(updateQueue[i]);

	delete(e1);
	delete(e2);
	delete(e3);
}

void Bmain()
{
	// 重置四叉树的深度
	resetQuadTree(root);
	
	// 填充背景色，并清空zbuffer
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Bpixels[y][x * 3 + 0] = 255;
			Bpixels[y][x * 3 + 1] = 255;
			Bpixels[y][x * 3 + 2] = 255;
			Bzbuffer[x][y] = -1e5;
		}

	int noRas = 0;

	// 逐个计算三角形
	for (int i = 1; i <= numFaces; i++)
	{
		Vertex* v1 = &vertices[faces[i].v1];
		Vertex* v2 = &vertices[faces[i].v2];
		Vertex* v3 = &vertices[faces[i].v3];
		Node* v1n = zbufferToNode[int(v1->x)][int(v1->y)];
		Node* v2n = zbufferToNode[int(v2->x)][int(v2->y)];
		Node* v3n = zbufferToNode[int(v3->x)][int(v3->y)];
		
		// 自底向上找最小共同祖先
		while (v1n->level > v2n->level)
			v1n = v1n->father;
		while (v2n->level > v1n->level)
			v2n = v2n->father;
		while (v1n->level > v3n->level)
			v1n = v1n->father;
		while (v3n->level > v1n->level)
			v3n = v3n->father;
		while (v2n->level > v3n->level)
			v2n = v2n->father;
		while (v3n->level > v2n->level)
			v3n = v3n->father;
		while (!(v1n == v2n && v2n == v3n))
		{
			v1n = v1n->father;
			v2n = v2n->father;
			v3n = v3n->father;
		}

		// 若三角形最大z值大于祖先结点，则可光栅化之
		float depth = v1n->depth;
		if (v1->z > depth || v2->z > depth || v3->z > depth)
			BrasterizeTriangle(v1, v2, v3);
		else
			noRas++;
	}
	// 探索：有多少比例的面片因为BaselineHierarchialZBuffer算法而免于被绘制？
	printf("No Rasterize Rate: %f\n", float(noRas) / numFaces);
}

// 这是一个纹理
GLuint Btex;

void BHZBDisplay()
{

	// 记录开始时间
	auto t1 = std::chrono::high_resolution_clock::now();

	// 旋转被绘制物体
	rotate();

	// 主程序
	Bmain();

	// 把像素添加到纹理中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Bpixels);

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
	Bfps = 1000000.0 / duration;
	printf("Bfps: %f\n", Bfps);

	// 请求重绘
	glutPostRedisplay();
}

float BaselineHierarchialZBuffer(int argc, char** argv)
{
	glutCreateWindow("Baseline Hierarchial Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// 创建、绑定、设置纹理
	glGenTextures(1, &Btex);
	glBindTexture(GL_TEXTURE_2D, Btex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	// 建立四叉树
	root = buildQuadTree(nullptr, 0, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	glutDisplayFunc(BHZBDisplay);
	while (true)
	{
		glutMainLoopEvent();
		if (glutGetWindow() == 0)
			return Bfps;
	}
}