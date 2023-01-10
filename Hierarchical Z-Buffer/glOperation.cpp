#include <GL/freeglut.h>
#include "procedure.h"
using namespace std;

float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
const float fovy = 90.0f, aspect = 1.0f, zNear = 0.1f, zFar = 1000.0f;
GLint viewport[4];
GLdouble modelMatrix[16], projMatrix[16];

struct Pixel
{
	int pixelX, pixelY;
};

void InitEnvironment()
{
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1, 0, 1, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
}

// ���任���ģ������ͶӰ��������
/*Pixel getPixel(float x, float y, float z)
{
	Pixel pixel;
	// ƽ��
	float translatedX = x + 0.0f;
	float translatedY = y + 0.0f;
	float translatedZ = z - 5.0f;
	// ��X����ת
	float rotatedX = translatedX;
	float rotatedY = translatedY * cos(rotX) - translatedZ * sin(rotX);
	float rotatedZ = translatedY * sin(rotX) + translatedZ * cos(rotX);
	// ��Y����ת
	rotatedZ = rotatedZ * cos(rotY) - rotatedX * sin(rotY);
	rotatedX = rotatedZ * sin(rotY) + rotatedX * cos(rotY);
	// ��Z����ת
	rotatedX = rotatedX * cos(rotZ) - rotatedY * sin(rotZ);
	rotatedY = rotatedX * sin(rotZ) + rotatedY * cos(rotZ);
	// ͶӰ�任
	float f = 1.0f / tan(fovy * 0.5f);
	float projectionMatrix[16] = {
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
		0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f
	};
	float xPrime = projectionMatrix[0] * rotatedX + projectionMatrix[4] * rotatedY + projectionMatrix[8] * rotatedZ + projectionMatrix[12];
	float yPrime = projectionMatrix[1] * rotatedX + projectionMatrix[5] * rotatedY + projectionMatrix[9] * rotatedZ + projectionMatrix[13];
	float zPrime = projectionMatrix[2] * rotatedX + projectionMatrix[6] * rotatedY + projectionMatrix[10] * rotatedZ + projectionMatrix[14];
	float wPrime = projectionMatrix[3] * rotatedX + projectionMatrix[7] * rotatedY + projectionMatrix[11] * rotatedZ + projectionMatrix[15];
	float normalizedX = xPrime / wPrime;
	float normalizedY = yPrime / wPrime;
	// �ƶ�����Ļ����
	pixel.pixelX = (int)(normalizedX * windowWidth / 2.0 + windowWidth / 2.0);
	pixel.pixelY = (int)(normalizedY * windowHeight / 2.0 + windowHeight / 2.0);
	return pixel;
}*/
Pixel getPixel(float x, float y, float z)
{
	Pixel pixel;
	
	double winx, winy, winz;
	gluProject(x, y, z, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
	//winy = viewport[3] - winy; // ת����������
	pixel.pixelX = winx;
	pixel.pixelY = winy;
	printf("pixel x = %f, y = %f, z = %f\n", winx, winy, winz);
	return pixel;
}

void display()
{
	rotate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -10.0f);			// ������Ļ5����λ
	//glRotatef(rotX, 1.0f, 0.0f, 0.0f);			// ��X����ת
	//glRotatef(rotY, 0.0f, 1.0f, 0.0f);			// ��Y����ת
	//glRotatef(rotZ, 0.0f, 0.0f, 1.0f);			// ��Z����ת
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);		// ģ�;���
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);		// ͶӰ����

    for (int i = 1; i <= numFaces; i++)
    {
		glColorMask(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonOffset(1.1f, 4.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);  //���ö����ƫ��
		glBegin(GL_TRIANGLES);

		//glBindTexture(GL_TEXTURE_2D, texture_id);
		//glNormal3f(normals[faces[i].vn1].nx, normals[faces[i].vn1].ny, normals[faces[i].vn1].nz);
		glVertex3f(vertices[faces[i].v1].x, vertices[faces[i].v1].y, vertices[faces[i].v1].z);
		if (i == 1)
		{
			
			Pixel pixel = getPixel(vertices[faces[i].v1].x, vertices[faces[i].v1].y, vertices[faces[i].v1].z);

		}
				//glNormal3f(normals[faces[i].vn2].nx, normals[faces[i].vn2].ny, normals[faces[i].vn2].nz);
		glVertex3f(vertices[faces[i].v2].x, vertices[faces[i].v2].y, vertices[faces[i].v2].z);
		//glNormal3f(normals[faces[i].vn3].nx, normals[faces[i].vn3].ny, normals[faces[i].vn3].nz);
		glVertex3f(vertices[faces[i].v3].x, vertices[faces[i].v3].y, vertices[faces[i].v3].z);
		glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);

		glColorMask(1, 1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_TRIANGLES);
		//glNormal3f(normals[faces[i].vn1].nx, normals[faces[i].vn1].ny, normals[faces[i].vn1].nz);
		glVertex3f(vertices[faces[i].v1].x, vertices[faces[i].v1].y, vertices[faces[i].v1].z);
		//glNormal3f(normals[faces[i].vn2].nx, normals[faces[i].vn2].ny, normals[faces[i].vn2].nz);
		glVertex3f(vertices[faces[i].v2].x, vertices[faces[i].v2].y, vertices[faces[i].v2].z);
		//glNormal3f(normals[faces[i].vn3].nx, normals[faces[i].vn3].ny, normals[faces[i].vn3].nz);
		glVertex3f(vertices[faces[i].v3].x, vertices[faces[i].v3].y, vertices[faces[i].v3].z);
		glEnd();
		
    }
	rotX += 0.05;        // X����ת
	rotY += 0.30;        // Y����ת
	rotZ += 0.05;        // Z����ת
	glutSwapBuffers();
	glutPostRedisplay();
}

void glMain(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("OBJ Viewer");
	InitEnvironment();
    glutDisplayFunc(display);
    glutMainLoop();
}