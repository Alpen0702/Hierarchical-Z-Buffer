#include "procedure.h"
#include <GL/freeglut.h>
using namespace std;

int main(int argc, char** argv)
{
    // ѡ��ģ��
    //string modelPath = "cubes.obj";   // �˸�ȱ��������壬��64�����㣬80����Ƭ
    //string modelPath = "teapot.obj";  // �����������4658�����㣬9216����Ƭ
    string modelPath = "hut_t.obj";     // һ��С���ӣ���43788�����㣬85644����Ƭ
    //string modelPath = "house_t.obj"; // һ��ͥԺ����58226�����㣬110560����Ƭ
    
    // ���벢����ģ�������ڴ�С
    readOBJ(modelPath);
    adjustOBJ();

    // gl����ǰ׼��
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // ���ֻ�ͼ����ʹ��zbuffer��ʹ��ɨ����zbuffer��ʹ�ü򵥵Ĳ��zbuffer��ʹ�ô��а˲����Ĳ��zbuffer
    NoneZBuffer(argc, argv);
    ScanlineZBuffer(argc, argv);
    BaselineHierarchialZBuffer(argc, argv);
    HierarchialZBuffer(argc, argv); 
    
    // �����ܽᱨ��
    summary();
    system("pause");
}