#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(16);
}

void GLWidget::initializeGL(){
    glClearColor(.753, 0, .46, 0);
}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);

    glRotatef(0.5,1,1,1);
    /* load coordinates into triangles through loop */
    /* refer to boxes for examples on interaction */
    glBegin(GL_TRIANGLES);

    /* MANUAL TRIANGLE
        glColor3f(0,1,1);
        glVertex3f(-0.5, -0.5, 0);
        glColor3f(1,0,1);
        glVertex3f( 0.5, -0.5, 0);
        glColor3f(0,0,1);
        glVertex3f( 0.0,  0.5, -0.5); */
    if (objPtr){
        Vertex v;
        Vertex f;
        int v1, v2, v3;
        //glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

            for (unsigned i = 0 ; i < vertices.size() ; i++)
            {
                /* x, y, z in face f are the indices of the vertices that make that face */
                /* need to glVertex3f for each vertex */
                f = faces.at(i);
                v1 = f.getX();
                v2 = f.getY();
                v3 = f.getZ();

                v = vertices.at(v1 - 1);
                glColor3f(0,1,1);
                glVertex3f(v.getX(), v.getY(), v.getZ());
                v = vertices.at(v2 - 1);
                glVertex3f(v.getX(), v.getY(), v.getZ());
                v = vertices.at(v3 - 1);
                glVertex3f(v.getX(), v.getY(), v.getZ());
            }
    }
    glEnd();

}

void GLWidget::grabObj(objLoad objFile){
    objPtr = &objFile;
    vertices = objPtr->getVertices();
    faces = objPtr->getFacets();

    /* load coordinates into triangles through loop */
    /* refer to boxes for examples on interaction */

}


void GLWidget::resizeGL(int w, int h){

}
