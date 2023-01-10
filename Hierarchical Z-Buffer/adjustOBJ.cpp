#include "procedure.h"

void adjustOBJ()
{
	float maxD = 0;
	float maxX = 0;
	float maxY = 0;
	float maxZ = 0;
	float minX = 0;
	float minY = 0;
	float minZ = 0;
	for (int i = 1; i <= numVertices; i++)
	{
		maxX = max(maxX, vertices[i].x);
		maxY = max(maxY, vertices[i].y);
		maxZ = max(maxZ, vertices[i].z);
		minX = min(minX, vertices[i].x);
		minY = min(minY, vertices[i].y);
		minZ = min(minZ, vertices[i].z);
	}
	float midX = (maxX + minX) / 2;
	float midY = (maxY + minY) / 2;
	float midZ = (maxZ + minZ) / 2;

	for (int i = 1; i <= numVertices; i++)
	{
		vertices[i].x -= midX;
		vertices[i].y -= midY;
		vertices[i].z -= midZ;
		maxD = max(maxD, abs(vertices[i].x));
		maxD = max(maxD, abs(vertices[i].y));
		maxD = max(maxD, abs(vertices[i].z));
	}

	maxD *= sqrt(3.0f);
	float maxS = min(WINDOW_HEIGHT, WINDOW_WIDTH);
	float trans = maxS / maxD;
	for (int i = 1; i <= numVertices; i++)
	{
		vertices[i].x = (vertices[i].x * trans + maxS) / 2;
		vertices[i].y = (vertices[i].y * trans + maxS) / 2;
		vertices[i].z = (vertices[i].z * trans + maxS) / 2;
	}
}

void rotate()
{
	double rad = 0.01;

	double u = 1, v = 1, w = 1;

	// 旋转轴的单位向量
	double u_hat = u / sqrt(u * u + v * v + w * w);
	double v_hat = v / sqrt(u * u + v * v + w * w);
	double w_hat = w / sqrt(u * u + v * v + w * w);

	// 旋转矩阵
	double R[3][3] = {
		{cos(rad) + u_hat * u_hat * (1 - cos(rad)),
		 u_hat * v_hat * (1 - cos(rad)) - w_hat * sin(rad),
		 u_hat * w_hat * (1 - cos(rad)) + v_hat * sin(rad)},
		{v_hat * u_hat * (1 - cos(rad)) + w_hat * sin(rad),
		 cos(rad) + v_hat * v_hat * (1 - cos(rad)),
		 v_hat * w_hat * (1 - cos(rad)) - u_hat * sin(rad)},
		{w_hat * u_hat * (1 - cos(rad)) - v_hat * sin(rad),
		 w_hat * v_hat * (1 - cos(rad)) + u_hat * sin(rad),
		 cos(rad) + w_hat * w_hat * (1 - cos(rad))}
	};

	double x, y, z;

	for (int i = 1; i <= numVertices; i++)
	{
		x = vertices[i].x;
		y = vertices[i].y;
		z = vertices[i].z;
		vertices[i].x = R[0][0] * x + R[0][1] * y + R[0][2] * z;
		vertices[i].y = R[1][0] * x + R[1][1] * y + R[1][2] * z;
		vertices[i].z = R[2][0] * x + R[2][1] * y + R[2][2] * z;
	}
}