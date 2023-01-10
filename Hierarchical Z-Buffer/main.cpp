#include "procedure.h"
#include <GL/freeglut.h>
using namespace std;

int main(int argc, char** argv)
{
    // 选择模型
    //string modelPath = "cubes.obj";   // 八个缺面的立方体，含64个顶点，80个面片
    //string modelPath = "teapot.obj";  // 犹他茶壶，含4658个顶点，9216个面片
    string modelPath = "hut_t.obj";     // 一所小房子，含43788个顶点，85644个面片
    //string modelPath = "house_t.obj"; // 一个庭院，含58226个顶点，110560个面片
    
    // 读入并调整模型至窗口大小
    readOBJ(modelPath);
    adjustOBJ();

    // gl运行前准备
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // 四种绘图：不使用zbuffer；使用扫描线zbuffer；使用简单的层次zbuffer；使用带有八叉树的层次zbuffer
    NoneZBuffer(argc, argv);
    ScanlineZBuffer(argc, argv);
    BaselineHierarchialZBuffer(argc, argv);
    HierarchialZBuffer(argc, argv); 
    
    // 生成总结报告
    summary();
    system("pause");
}