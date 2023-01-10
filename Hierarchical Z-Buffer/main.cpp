#include "procedure.h"
#include <GL/freeglut.h>
using namespace std;

int main(int argc, char** argv)
{
    //string modelPath = "teapot.obj";
    //string modelPath = "cubes.obj";
    string modelPath = "hut_t.obj";
    //string modelPath = "house_t.obj";
    readOBJ(modelPath);
    adjustOBJ();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //printf("********NFPS: %f********\n", NoneZBuffer(argc, argv));
    //printf("********SFPS: %f********\n", ScanlineZBuffer(argc, argv));
    printf("********BFPS: %f********\n", BaselineHierarchialZBuffer(argc, argv));
    //printf("********HFPS: %f********\n", HierarchialZBuffer(argc, argv));
    //glMain(argc, argv);
}