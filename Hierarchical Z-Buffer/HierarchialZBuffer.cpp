#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// 线框选色
const GLubyte colorR = 34, colorG = 139, colorB = 34;

double Hfps;

// 模拟RGB帧缓冲区
GLubyte Hpixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// 四叉树结点
struct QuadNode
{
	QuadNode* father;
	vector<QuadNode*> children;
	int level;
	float depth;
	int l, r, d, u;
	QuadNode(QuadNode* father, int level, float depth, int l, int r, int d, int u) : father(father), level(level), depth(depth), l(l), r(r), d(d), u(u) {};
};

// nodes是非叶结点，leaves是叶子结点
vector<QuadNode*> quadNodes;
QuadNode* leaves[WINDOW_WIDTH][WINDOW_HEIGHT];
QuadNode* quadRoot;

// 建立四叉树
QuadNode* buildQuadTree(QuadNode* father, int level, int l, int r, int d, int u)
{
	// 格子面积为0，不建立
	if (l == r || d == u)
		return nullptr;
	// 格子恰好为一个像素，进入leaves成为叶子结点
	if (l == r - 1 && d == u - 1)
	{
		leaves[l][d] = new QuadNode(father, level, -1e5, l, r, d, u);
		//printf("built %d %d\n", l, d);
		return leaves[l][d];
	}
	// 格子非叶子且有面积，继续拆分
	QuadNode* quadNode = new QuadNode(father, level, -1e5, l, r, d, u);
	quadNodes.push_back(quadNode);
	int lrmid = (l + r) / 2;
	int dumid = (d + u) / 2;

	quadNode->children.push_back(buildQuadTree(quadNode, level + 1, l, lrmid, d, dumid));
	quadNode->children.push_back(buildQuadTree(quadNode, level + 1, l, lrmid, dumid, u));
	quadNode->children.push_back(buildQuadTree(quadNode, level + 1, lrmid, r, d, dumid));
	quadNode->children.push_back(buildQuadTree(quadNode, level + 1, lrmid, r, dumid, u));
	return quadNode;
}

// 重置四叉树的深度
void resetQuadTree(QuadNode* father)
{
	father->depth = -1e5;
	for (QuadNode* child : father->children)
		if (child)
			resetQuadTree(child);
}

// 定义三角形面片的边结构
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

// 生成三角形面片的边结构
Edge* HbuildEdge(Vertex* v1, Vertex* v2)
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
void update(QuadNode* quadNode)
{
	float minD = 1e5;
	for (QuadNode* child : quadNode->children)
		if (child && child->depth < minD)
			minD = child->depth;
	if (abs(minD - quadNode->depth) > .000001)
	{
		quadNode->depth = minD;
		if (quadNode->father)
			update(quadNode->father);
	}
}

// 光栅化三角形
void HrasterizeTriangle(Vertex* v1, Vertex* v2, Vertex* v3)
{
	if (v1->y < v2->y)
		swap(v1, v2);
	if (v2->y < v3->y)
		swap(v2, v3);
	if (v1->y < v2->y)
		swap(v1, v2);

	Edge* e1 = HbuildEdge(v1, v2);
	Edge* e2 = HbuildEdge(v1, v3);
	Edge* e3 = HbuildEdge(v2, v3);
	float a = (v2->y - v1->y) * (v3->z - v1->z) - (v2->z - v1->z) * (v3->y - v1->y);
	float b = (v2->z - v1->z) * (v3->x - v1->x) - (v2->x - v1->x) * (v3->z - v1->z);
	float c = (v2->x - v1->x) * (v3->y - v1->y) - (v2->y - v1->y) * (v3->x - v1->x);
	float d = -(a * v1->x + b * v1->y + c * v1->z);

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
			if (z > leaves[x][y]->depth)
			{
				// 递归更新四叉树深度
				leaves[x][y]->depth = z;
				update(leaves[x][y]->father);

				// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
				if (x == intxl || x == intxr)
				{
					Hpixels[y][x * 3 + 0] = colorR;
					Hpixels[y][x * 3 + 1] = colorG;
					Hpixels[y][x * 3 + 2] = colorB;
				}
				else
				{
					Hpixels[y][x * 3 + 0] = 255;
					Hpixels[y][x * 3 + 1] = 255;
					Hpixels[y][x * 3 + 2] = 255;
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
			if (z > leaves[x][y]->depth)
			{
				// 递归更新四叉树深度
				leaves[x][y]->depth = z;
				update(leaves[x][y]->father);

				// 因为演示的是线框图，故给边缘上彩色，给面片上背景色
				if (x == intxl || x == intxr)
				{
					Hpixels[y][x * 3 + 0] = colorR;
					Hpixels[y][x * 3 + 1] = colorG;
					Hpixels[y][x * 3 + 2] = colorB;
				}
				else
				{
					Hpixels[y][x * 3 + 0] = 255;
					Hpixels[y][x * 3 + 1] = 255;
					Hpixels[y][x * 3 + 2] = 255;
				}
			}
			z += dzx;
		}
		// 活化边推进到下一条扫描线
		xl += e2->dx;
		xr += e3->dx;
		zl += dzx * e2->dx + dzy;
	}

	delete(e1);
	delete(e2);
	delete(e3);
}

// 八叉树结点
struct OctNode
{
	OctNode* children[8] = { nullptr };
	vector<int> triangleIDs;
	float xmin, ymin, zmin, xmax, ymax, zmax;
	OctNode(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) :xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax), zmin(zmin), zmax(zmax) {}
};

OctNode* octRoot;

// 相互调用，提前声明
void splitOctNode(OctNode* octNode);

// 八叉树结点最大面片数，若超过则应分割该结点
const int MAX_OCT_TRIANGLES = 10;

// 判断顶点是否在八叉树结点中
bool isVertexInOctNode(OctNode* octNode, Vertex* vertex)
{
	return vertex->x >= octNode->xmin &&
		vertex->y >= octNode->ymin &&
		vertex->z >= octNode->zmin &&
		vertex->x <= octNode->xmax &&
		vertex->y <= octNode->ymax &&
		vertex->z <= octNode->zmax;
}

// 判断边是否与正方形有交点
bool isEdgeCrossRectangle(Vertex* v1, Vertex* v2, int Ndir, float dist, float min1, float max1, float min2, float max2)
{
	float deltaX = v2->x - v1->x;
	float deltaY = v2->y - v1->y;
	float deltaZ = v2->z - v1->z;
	float t;
	// 比值t = (dist-(v1-原点向量)·Normal向量)/Normal向量·(v2-v1)向量
	if (Ndir == 0)
	{
		t = (dist - v1->x) / (v2->x - v1->x);
		if (t >= 0.0f && t <= 1.0f)
		{
			float newY = t * deltaY + v1->y;
			float newZ = t * deltaZ + v1->z;
			if (newY >= min1 && newY <= max1 && newZ >= min2 && newZ <= max2)
				return true;
		}
	}
	else if (Ndir == 1)
	{
		t = (dist - v1->y) / (v2->y - v1->y);
		if (t >= 0.0f && t <= 1.0f)
		{
			float newX = t * deltaX + v1->x;
			float newZ = t * deltaZ + v1->z;
			if (newX >= min1 && newX <= max1 && newZ >= min2 && newZ <= max2)
				return true;
		}
	}
	else if (Ndir == 2)
	{
		t = (dist - v1->z) / (v2->z - v1->z);
		if (t >= 0.0f && t <= 1.0f)
		{
			float newX = t * deltaX + v1->x;
			float newY = t * deltaY + v1->y;
			if (newX >= min1 && newX <= max1 && newY >= min2 && newY <= max2)
				return true;
		}
	}
	return false;
}

// 判断边是否与八叉树结点有交点
bool isEdgeCrossOctNode(OctNode* octNode, Vertex* v1, Vertex* v2)
{
	return isEdgeCrossRectangle(v1, v2, 0, octNode->xmin, octNode->ymin, octNode->ymax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 0, octNode->xmax, octNode->ymin, octNode->ymax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 1, octNode->ymin, octNode->xmin, octNode->xmax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 1, octNode->ymax, octNode->xmin, octNode->xmax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 2, octNode->zmin, octNode->xmin, octNode->xmax, octNode->ymin, octNode->ymax) ||
		isEdgeCrossRectangle(v1, v2, 2, octNode->zmax, octNode->xmin, octNode->xmax, octNode->ymin, octNode->ymax);
}

// 判断三角形与八叉树结点是否有交点
bool isTriangleCrossOctNode(OctNode* octNode, int triangleID)
{
	Vertex* v1 = &vertices[faces[triangleID].v1];
	Vertex* v2 = &vertices[faces[triangleID].v2];
	Vertex* v3 = &vertices[faces[triangleID].v3];
	if (isVertexInOctNode(octNode, v1) || isVertexInOctNode(octNode, v2) || isVertexInOctNode(octNode, v3))
		return true;
	if (isEdgeCrossOctNode(octNode, v1, v2) || isEdgeCrossOctNode(octNode, v2, v3) || isEdgeCrossOctNode(octNode, v1, v3))
		return true;
	return false;
}

// 将三角形面片加入到指定八叉树结点中
void insertTriangleToOctNode(OctNode* octNode, int triangleID)
{
	if (!isTriangleCrossOctNode(octNode, triangleID))
		return;
	if (!octNode->children[0])
	{
		octNode->triangleIDs.push_back(triangleID);
		// 如果本结点内三角形太多，就要分割该结点；但若本结点已经分得很细了，就不应该继续分下去了
		if (octNode->triangleIDs.size() > MAX_OCT_TRIANGLES && octNode->xmax - octNode->xmin > 10)
			splitOctNode(octNode);
	}
	else
	{
		auto t1 = std::chrono::high_resolution_clock::now();

		// 下放该三角形
		for (int i = 0; i < 8; i++)
			insertTriangleToOctNode(octNode->children[i], triangleID);

		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		float time = duration / 1000000.0;
		//if (octNode->xmin == 0 && octNode->xmax == 720)
			//printf("insert this time: %f\n", time);
	}
}

// 分割八叉树结点
void splitOctNode(OctNode* octNode)
{
	float xmax = octNode->xmax;
	float xmin = octNode->xmin;
	float ymax = octNode->ymax;
	float ymin = octNode->ymin;
	float zmax = octNode->zmax;
	float zmin = octNode->zmin;
	// 分割结点
	float xmid = (xmax + xmin) / 2;
	float ymid = (ymax + ymin) / 2;
	float zmid = (zmax + zmin) / 2;
	OctNode* on1 = new OctNode(xmin, xmid, ymin, ymid, zmin, zmid);
	OctNode* on2 = new OctNode(xmin, xmid, ymin, ymid, zmid, zmax);
	OctNode* on3 = new OctNode(xmin, xmid, ymid, ymax, zmin, zmid);
	OctNode* on4 = new OctNode(xmin, xmid, ymid, ymax, zmid, zmax);
	OctNode* on5 = new OctNode(xmid, xmax, ymin, ymid, zmin, zmid);
	OctNode* on6 = new OctNode(xmid, xmax, ymin, ymid, zmid, zmax);
	OctNode* on7 = new OctNode(xmid, xmax, ymid, ymax, zmin, zmid);
	OctNode* on8 = new OctNode(xmid, xmax, ymid, ymax, zmid, zmax);
	octNode->children[0] = on1;
	octNode->children[1] = on2;
	octNode->children[2] = on3;
	octNode->children[3] = on4;
	octNode->children[4] = on5;
	octNode->children[5] = on6;
	octNode->children[6] = on7;
	octNode->children[7] = on8;
	// 现有面片全部下放
	for (int triangleID : octNode->triangleIDs)
		for (int i = 0; i < 8; i++)
			insertTriangleToOctNode(octNode->children[i], triangleID);
	octNode->triangleIDs.clear();
}

// 绘制八叉树结点内的三角形
int drawOctNode(OctNode* octNode, QuadNode* quadNode)
{
	// 判断quadNode在视窗平面内是否包含octNode
	if (quadNode->l <= octNode->xmin && quadNode->r >= octNode->xmax && quadNode->d <= octNode->ymin && quadNode->u >= octNode->ymax)
	{
		// quadNode比octNode深度靠前，octNode被全部遮挡
		if (quadNode->depth >= octNode->zmax)
			return 1;
		// octNode没有被全部遮挡，剖分判断quadNode子结点
		for (QuadNode* quadChild : quadNode->children)
			if (quadChild && drawOctNode(octNode, quadChild))
				return 1;
		// quadNode子结点都不能包含octNode，当前quadNode就是包含octNode的最小四叉树结点，则绘制octNode
		// 若octNode有子结点，则绘制子结点
		if (octNode->children[0])
			for (int i = 0; i < 8; i++)
				drawOctNode(octNode->children[i], quadNode);
		// 否则octNode本身就是叶子结点，绘制octNode存储的三角形
		for (int triangleID : octNode->triangleIDs)
		{
			Vertex* v1 = &vertices[faces[triangleID].v1];
			Vertex* v2 = &vertices[faces[triangleID].v2];
			Vertex* v3 = &vertices[faces[triangleID].v3];
			QuadNode* v1n = leaves[int(v1->x)][int(v1->y)];
			QuadNode* v2n = leaves[int(v2->x)][int(v2->y)];
			QuadNode* v3n = leaves[int(v3->x)][int(v3->y)];
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
			// 若三角形最大z值大于祖先则可光栅化之
			float depth = v1n->depth;
			if (v1->z > depth || v2->z > depth || v3->z > depth)
				HrasterizeTriangle(v1, v2, v3);
		}
		return 1;
	}
	// 该quadNode不包含octNode
	return 0;
}

// 删除八叉树结点
void resetOctNode(OctNode* octNode)
{
	if (octNode->children[0])
		for (OctNode* child : octNode->children)
			resetOctNode(child);
	octNode->triangleIDs.clear();
}

void Hmain()
{
	// 重置四叉树的深度
	resetQuadTree(quadRoot);

	// 填充背景色
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Hpixels[y][x * 3 + 0] = 255;
			Hpixels[y][x * 3 + 1] = 255;
			Hpixels[y][x * 3 + 2] = 255;
		}

	auto t1 = std::chrono::high_resolution_clock::now();

	// 将三角形加入到八叉树中
	for (int i = 1; i <= numFaces; i++)
		insertTriangleToOctNode(octRoot, i);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	float time = duration / 1000000.0;
	//printf("insert time: %f\n", time);

	t1 = std::chrono::high_resolution_clock::now();

	// 遍历八叉树空间
	drawOctNode(octRoot, quadRoot);
	t2 = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	time = duration / 1000000.0;
	//printf("draw time: %f\n", time);

	// 清空八叉树
	resetOctNode(octRoot);
}

// 这是一个纹理
GLuint Htex;

void HZBDisplay()
{
	// 记录开始时间
	auto t1 = std::chrono::high_resolution_clock::now();

	// 旋转被绘制物体
	rotate();

	// 主程序
	Hmain();

	// 把像素添加到纹理中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Hpixels);

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
	Hfps = 1000000.0 / duration;
	printf("Hfps: %f\n", Hfps);

	// 请求重绘
	glutPostRedisplay();
}

void HierarchialZBuffer(int argc, char** argv)
{
	glutCreateWindow("Hierarchial Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// 创建、绑定、设置纹理
	glGenTextures(1, &Htex);
	glBindTexture(GL_TEXTURE_2D, Htex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	// 建立四叉树
	quadRoot = buildQuadTree(nullptr, 0, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	// 建立八叉树
	float maxS = min(WINDOW_HEIGHT, WINDOW_WIDTH);
	octRoot = new OctNode(0, maxS, 0, maxS, 0, maxS);

	glutDisplayFunc(HZBDisplay);

	// 开始绘制
	auto t1 = std::chrono::high_resolution_clock::now();
	printf("********      Hierarchial Z-Buffer Begins     ********\n");
	while (true)
	{
		glutMainLoopEvent();
		frameH++;

		// 关闭窗口时结束绘制
		if (glutGetWindow() == 0)
		{
			auto t2 = std::chrono::high_resolution_clock::now();
			timeH = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0;
			printf("********       Hierarchial Z-Buffer Ends      ********\n\n");
			return;
		}
	}
}