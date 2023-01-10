#include <GL/freeglut.h>
#include "procedure.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

// �߿�ѡɫ
const GLubyte colorR = 34, colorG = 139, colorB = 34;

double Hfps;

// ģ��RGB֡������
GLubyte Hpixels[WINDOW_HEIGHT][WINDOW_WIDTH * 3];

// �Ĳ������
struct QuadNode
{
	QuadNode* father;
	vector<QuadNode*> children;
	int level;
	float depth;
	int l, r, d, u;
	QuadNode(QuadNode* father, int level, float depth, int l, int r, int d, int u) : father(father), level(level), depth(depth), l(l), r(r), d(d), u(u) {};
};

// nodes�Ƿ�Ҷ��㣬leaves��Ҷ�ӽ��
vector<QuadNode*> quadNodes;
QuadNode* leaves[WINDOW_WIDTH][WINDOW_HEIGHT];
QuadNode* quadRoot;

// �����Ĳ���
QuadNode* buildQuadTree(QuadNode* father, int level, int l, int r, int d, int u)
{
	// �������Ϊ0��������
	if (l == r || d == u)
		return nullptr;
	// ����ǡ��Ϊһ�����أ�����leaves��ΪҶ�ӽ��
	if (l == r - 1 && d == u - 1)
	{
		leaves[l][d] = new QuadNode(father, level, -1e5, l, r, d, u);
		//printf("built %d %d\n", l, d);
		return leaves[l][d];
	}
	// ���ӷ�Ҷ������������������
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

// �����Ĳ��������
void resetQuadTree(QuadNode* father)
{
	father->depth = -1e5;
	for (QuadNode* child : father->children)
		if (child)
			resetQuadTree(child);
}

// ������������Ƭ�ı߽ṹ
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

// ������������Ƭ�ı߽ṹ
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

// ���½�����
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

// ��դ��������
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
			if (z > leaves[x][y]->depth)
			{
				// �ݹ�����Ĳ������
				leaves[x][y]->depth = z;
				update(leaves[x][y]->father);

				// ��Ϊ��ʾ�����߿�ͼ���ʸ���Ե�ϲ�ɫ������Ƭ�ϱ���ɫ
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
			if (z > leaves[x][y]->depth)
			{
				// �ݹ�����Ĳ������
				leaves[x][y]->depth = z;
				update(leaves[x][y]->father);

				// ��Ϊ��ʾ�����߿�ͼ���ʸ���Ե�ϲ�ɫ������Ƭ�ϱ���ɫ
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
		// ����ƽ�����һ��ɨ����
		xl += e2->dx;
		xr += e3->dx;
		zl += dzx * e2->dx + dzy;
	}

	delete(e1);
	delete(e2);
	delete(e3);
}

// �˲������
struct OctNode
{
	OctNode* children[8] = { nullptr };
	vector<int> triangleIDs;
	float xmin, ymin, zmin, xmax, ymax, zmax;
	OctNode(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) :xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax), zmin(zmin), zmax(zmax) {}
};

OctNode* octRoot;

// �໥���ã���ǰ����
void splitOctNode(OctNode* octNode);

// �˲�����������Ƭ������������Ӧ�ָ�ý��
const int MAX_OCT_TRIANGLES = 10;

// �ж϶����Ƿ��ڰ˲��������
bool isVertexInOctNode(OctNode* octNode, Vertex* vertex)
{
	return vertex->x >= octNode->xmin &&
		vertex->y >= octNode->ymin &&
		vertex->z >= octNode->zmin &&
		vertex->x <= octNode->xmax &&
		vertex->y <= octNode->ymax &&
		vertex->z <= octNode->zmax;
}

// �жϱ��Ƿ����������н���
bool isEdgeCrossRectangle(Vertex* v1, Vertex* v2, int Ndir, float dist, float min1, float max1, float min2, float max2)
{
	float deltaX = v2->x - v1->x;
	float deltaY = v2->y - v1->y;
	float deltaZ = v2->z - v1->z;
	float t;
	// ��ֵt = (dist-(v1-ԭ������)��Normal����)/Normal������(v2-v1)����
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

// �жϱ��Ƿ���˲�������н���
bool isEdgeCrossOctNode(OctNode* octNode, Vertex* v1, Vertex* v2)
{
	return isEdgeCrossRectangle(v1, v2, 0, octNode->xmin, octNode->ymin, octNode->ymax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 0, octNode->xmax, octNode->ymin, octNode->ymax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 1, octNode->ymin, octNode->xmin, octNode->xmax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 1, octNode->ymax, octNode->xmin, octNode->xmax, octNode->zmin, octNode->zmax) ||
		isEdgeCrossRectangle(v1, v2, 2, octNode->zmin, octNode->xmin, octNode->xmax, octNode->ymin, octNode->ymax) ||
		isEdgeCrossRectangle(v1, v2, 2, octNode->zmax, octNode->xmin, octNode->xmax, octNode->ymin, octNode->ymax);
}

// �ж���������˲�������Ƿ��н���
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

// ����������Ƭ���뵽ָ���˲��������
void insertTriangleToOctNode(OctNode* octNode, int triangleID)
{
	if (!isTriangleCrossOctNode(octNode, triangleID))
		return;
	if (!octNode->children[0])
	{
		octNode->triangleIDs.push_back(triangleID);
		// ����������������̫�࣬��Ҫ�ָ�ý�㣻����������Ѿ��ֵú�ϸ�ˣ��Ͳ�Ӧ�ü�������ȥ��
		if (octNode->triangleIDs.size() > MAX_OCT_TRIANGLES && octNode->xmax - octNode->xmin > 10)
			splitOctNode(octNode);
	}
	else
	{
		auto t1 = std::chrono::high_resolution_clock::now();

		// �·Ÿ�������
		for (int i = 0; i < 8; i++)
			insertTriangleToOctNode(octNode->children[i], triangleID);

		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		float time = duration / 1000000.0;
		//if (octNode->xmin == 0 && octNode->xmax == 720)
			//printf("insert this time: %f\n", time);
	}
}

// �ָ�˲������
void splitOctNode(OctNode* octNode)
{
	float xmax = octNode->xmax;
	float xmin = octNode->xmin;
	float ymax = octNode->ymax;
	float ymin = octNode->ymin;
	float zmax = octNode->zmax;
	float zmin = octNode->zmin;
	// �ָ���
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
	// ������Ƭȫ���·�
	for (int triangleID : octNode->triangleIDs)
		for (int i = 0; i < 8; i++)
			insertTriangleToOctNode(octNode->children[i], triangleID);
	octNode->triangleIDs.clear();
}

// ���ư˲�������ڵ�������
int drawOctNode(OctNode* octNode, QuadNode* quadNode)
{
	// �ж�quadNode���Ӵ�ƽ�����Ƿ����octNode
	if (quadNode->l <= octNode->xmin && quadNode->r >= octNode->xmax && quadNode->d <= octNode->ymin && quadNode->u >= octNode->ymax)
	{
		// quadNode��octNode��ȿ�ǰ��octNode��ȫ���ڵ�
		if (quadNode->depth >= octNode->zmax)
			return 1;
		// octNodeû�б�ȫ���ڵ����ʷ��ж�quadNode�ӽ��
		for (QuadNode* quadChild : quadNode->children)
			if (quadChild && drawOctNode(octNode, quadChild))
				return 1;
		// quadNode�ӽ�㶼���ܰ���octNode����ǰquadNode���ǰ���octNode����С�Ĳ�����㣬�����octNode
		// ��octNode���ӽ�㣬������ӽ��
		if (octNode->children[0])
			for (int i = 0; i < 8; i++)
				drawOctNode(octNode->children[i], quadNode);
		// ����octNode�������Ҷ�ӽ�㣬����octNode�洢��������
		for (int triangleID : octNode->triangleIDs)
		{
			Vertex* v1 = &vertices[faces[triangleID].v1];
			Vertex* v2 = &vertices[faces[triangleID].v2];
			Vertex* v3 = &vertices[faces[triangleID].v3];
			QuadNode* v1n = leaves[int(v1->x)][int(v1->y)];
			QuadNode* v2n = leaves[int(v2->x)][int(v2->y)];
			QuadNode* v3n = leaves[int(v3->x)][int(v3->y)];
			// �Ե���������С��ͬ����
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
			// �����������zֵ����������ɹ�դ��֮
			float depth = v1n->depth;
			if (v1->z > depth || v2->z > depth || v3->z > depth)
				HrasterizeTriangle(v1, v2, v3);
		}
		return 1;
	}
	// ��quadNode������octNode
	return 0;
}

// ɾ���˲������
void resetOctNode(OctNode* octNode)
{
	if (octNode->children[0])
		for (OctNode* child : octNode->children)
			resetOctNode(child);
	octNode->triangleIDs.clear();
}

void Hmain()
{
	// �����Ĳ��������
	resetQuadTree(quadRoot);

	// ��䱳��ɫ
	for (int y = WINDOW_HEIGHT - 1; y >= 0; y--)
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			Hpixels[y][x * 3 + 0] = 255;
			Hpixels[y][x * 3 + 1] = 255;
			Hpixels[y][x * 3 + 2] = 255;
		}

	auto t1 = std::chrono::high_resolution_clock::now();

	// �������μ��뵽�˲�����
	for (int i = 1; i <= numFaces; i++)
		insertTriangleToOctNode(octRoot, i);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	float time = duration / 1000000.0;
	//printf("insert time: %f\n", time);

	t1 = std::chrono::high_resolution_clock::now();

	// �����˲����ռ�
	drawOctNode(octRoot, quadRoot);
	t2 = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	time = duration / 1000000.0;
	//printf("draw time: %f\n", time);

	// ��հ˲���
	resetOctNode(octRoot);
}

// ����һ������
GLuint Htex;

void HZBDisplay()
{
	// ��¼��ʼʱ��
	auto t1 = std::chrono::high_resolution_clock::now();

	// ��ת����������
	rotate();

	// ������
	Hmain();

	// ��������ӵ�������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, Hpixels);

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
	Hfps = 1000000.0 / duration;
	printf("Hfps: %f\n", Hfps);

	// �����ػ�
	glutPostRedisplay();
}

void HierarchialZBuffer(int argc, char** argv)
{
	glutCreateWindow("Hierarchial Z-Buffer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	// �������󶨡���������
	glGenTextures(1, &Htex);
	glBindTexture(GL_TEXTURE_2D, Htex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	// �����Ĳ���
	quadRoot = buildQuadTree(nullptr, 0, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	// �����˲���
	float maxS = min(WINDOW_HEIGHT, WINDOW_WIDTH);
	octRoot = new OctNode(0, maxS, 0, maxS, 0, maxS);

	glutDisplayFunc(HZBDisplay);

	// ��ʼ����
	auto t1 = std::chrono::high_resolution_clock::now();
	printf("********      Hierarchial Z-Buffer Begins     ********\n");
	while (true)
	{
		glutMainLoopEvent();
		frameH++;

		// �رմ���ʱ��������
		if (glutGetWindow() == 0)
		{
			auto t2 = std::chrono::high_resolution_clock::now();
			timeH = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0;
			printf("********       Hierarchial Z-Buffer Ends      ********\n\n");
			return;
		}
	}
}