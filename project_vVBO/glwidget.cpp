#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(16);
    frameTimer.start();
}

void GLWidget::initializeGL(){
    glEnable( GL_COLOR_MATERIAL );
    glEnable( GL_DEPTH_TEST );
    glMatrixMode(GL_MODELVIEW);
    glShadeModel( GL_SMOOTH );
    glClearDepth( 1.0f );
    glDepthFunc( GL_LEQUAL );
    /* Background Setting */
    glClearColor( 0, 0, 0, 1);
    /* Light Settings */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glShadeModel( GL_SMOOTH );
    glEnable(GL_NORMALIZE);
    zoomF = 1;
    scale = 1;
    transX = 0;
    transY = 0;
    radius = 0;
    red = 166;
    green = 166;
    blue = 166;
    mag = 0;
    lastClick[0] = 69.0f;
    lastClick[1] = 69.0f;
    mouseHeld = false;
    rotationOK = false;
    translateOK = false;
    needsReset = false;
    cullingOK = false;
    volumeOK = false;
    drawPlaneMode = false;
    w0 = this->width();
    h0 = this->height();
    cam.setAspect(w0,h0);
    axisOfRotation.setX(0);
    axisOfRotation.setY(0);
    axisOfRotation.setZ(0);
    initShaders();
    _shaderProgram.use();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void GLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    //_shaderProgram.use();
    int xNow = x;
    int yNow = y;
    /* If a File is Loaded*/
    if(objPtr)
   {
        glUniform3f(_shaderProgram.getUniformLocation("vertexColor"),red/255,green/255,blue/255);
        dx = (xNow - prevPos[0])/2;
        dy = (yNow - prevPos[1])/2;
        QMatrix4x4 mat;
        if (needsReset)
            this->resetView();
        cam.setZoom(zoomF);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        /* CULLING */
        if (cullingOK)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else
            glDisable(GL_CULL_FACE);
        /* ROTATION */
        if (mouseHeld && rotationOK && !translateOK && (dx!=0 || dy!=0))
        {
            this->drag2Rotate(dx,dy);
        }
        /* TRANSLATION */
        else if (mouseHeld && translateOK && !rotationOK)
        {
            this->drag2Translate(dx,dy);
        }
        /* ZOOM */
        else if (mouseHeld && zoomOK) {
            this->drag2Zoom(dy);
        }
        if (drawPlaneMode){
            if(lastClick[0] != 69.0f || lastClick[1] != 69.0f)
                drawPlane(0, 0);
        }
        /* Apply Current Position */
        glMatrixMode(GL_MODELVIEW);
        m.translate(0,0); // applies current translation
        //cam.moveToCenter();
        rotateAboutModelCenter(m, currQ);
        if (volumeOK)
            drawVolume();
        else
            m.drawMe();
        /* Revert to Original Matrix for Future Transformations */
        glPopMatrix();
        glPushMatrix();
        rotateAboutModelCenter(m, currQ);
        drawAxes();
        glPopMatrix();
        prevPos[0] = xNow;
        prevPos[1] = yNow;
   }
    frameCount++;
    //shaderProgram.unuse();
}

void GLWidget::resetView()
{
    if (needsReset) {
        currQ.setScalar(1);
        currQ.setX(0);
        currQ.setY(0);
        currQ.setZ(0);
        cam.viewModel();
        cam.moveToCenter();
        maxCoords = m.max();
        minCoords = m.min();
        std::cout << maxCoords.at(0) << std::endl;
        std::cout << minCoords.at(0) << std::endl;
        zoomF = cam.fitModel(maxCoords.at(0), minCoords.at(0),
                     maxCoords.at(1), minCoords.at(1),
                     maxCoords.at(2), minCoords.at(2));
        needsReset = false;
   }
}

void GLWidget::drawModel(model mod)
{
    mod.drawMe();
}

void GLWidget::drawVolume()
{
   m.drawVolume();
}

void GLWidget::drawQuad(Polyhedron::Facet_const_handle f)
{
   /* CGAL::Point_3<Kernel> p1,p2,p3,p4;
    p1 = circulator->vertex()->point();
    ++circulator;
    p2 = circulator->vertex()->point();
    ++circulator;
    p3 = circulator->vertex()->point();
    ++circulator;
    p4 = circulator->vertex()->point();
    CGAL::Vector_3<Kernel> normal = CGAL::normal(p2, p1, p3);
    glBegin(GL_QUADS);
    glColor3f(red, green, blue);
    glNormal3f(normal.hx(), normal.hy(), normal.hz());
    glVertex3f(p1.hx(), p1.hy(), p1.hz());
    glVertex3f(p2.hx(), p2.hy(), p2.hz());
    glVertex3f(p3.hx(), p3.hy(), p3.hz());
    glVertex3f(p4.hx(), p4.hy(), p4.hz()); */
}

void GLWidget::drawAxes()
{
    glLineWidth(2);
    //glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    /* Red X Axis */
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(2,0,0);
    /* Green Y Axis */
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,2,0);
    /* Blue Z Axis */
    glColor3f(0,0,2);
    glVertex3f(0,0,0);
    glVertex3f(0,0,2);
    glEnd();
    // glEnable(GL_LIGHTING);
}

void GLWidget::rotateAboutModelCenter(model mod, QQuaternion q) {
    QMatrix4x4 mat;
    mat.rotate(q);
    glTranslatef(mod.center().at(0), mod.center().at(1), mod.center().at(2));
    glMultMatrixf(mat.constData());
    glTranslatef(-mod.center().at(0), -mod.center().at(1), -mod.center().at(2));
}

void GLWidget::grabObj(objLoad<HDS> objFile) {
    objPtr = &objFile;
    /* TO DO , CLEAN UP UNUSED OBJFILES */
    frameTimer.restart();
    frameCount = 0;
    model m2(objFile);
    m = m2;
    cam.findModel(&m);
    cam.adjustAspect(this->width(), this->height());
    needsReset = true;
}

void GLWidget::grabColor(float r, float g, float b)
{
    red = r;
    green = g;
    blue = b;
    m.setColor(r, g, b);
}

void GLWidget::resizeGL(int w, int h){
    cam.adjustAspect(w,h);
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    this->x = e->x();
    this->y = e->y();
    if (drawPlaneMode){
      lastClick[0] = e->x();
      lastClick[1] = e->y();
    }
    else if (e->button() == Qt::LeftButton && !translateOK)
    {
        zoomOK = false;
        rotationOK = true;
    }
    else if (e->button() == Qt::RightButton)
    {
        rotationOK = false;
        translateOK = false;
        zoomOK = true;
    }
    mouseHeld = true;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    this->x = e->x();
    this->y = e->y();
    //drawPlaneMode = false;
    rotationOK = false;
    mouseHeld = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *e){
    this->x = e->x();
    this->y = e->y();
}

QQuaternion GLWidget::drag2Rotate(float dx, float dy)
{
    /* Define Axis of Rotation */
    axisOfRotation.setX(-dy);
    axisOfRotation.setY(-dx);
    axisOfRotation.setZ(0);
    mag = sqrt(dx*dx + dy*dy);
    /* Update Rotation Quaternion */
    QQuaternion newQ = QQuaternion::fromAxisAndAngle(axisOfRotation, mag);
    currQ = newQ * currQ;
    return currQ;
}

void GLWidget::drag2Translate(float dx, float dy)
{
    float xT,yT;
    /* Adjust Magnitude of Translation to Dimensions of Model */
    xT = (maxCoords.at(0) - minCoords.at(0))*dx/(scale*1*this->width());
    yT = -(maxCoords.at(1) - minCoords.at(1))*dy/(scale*1*this->height());
    m.translate(xT, yT);
}

void GLWidget::drag2Zoom(float dy)
{
    zoomF = zoomF + dy/100.0;
}

int GLWidget::giveFPS()
{
    unsigned num = frameCount/(double)(frameTimer.elapsed()/1000);
    if (objPtr)
        return num;
    else
        return 0;
}

bool GLWidget::generateVolumeMesh()
{
    mesh = m.poly();
    m.generateVolumeMesh();
    using namespace CGAL::parameters;
    Mesh_Domain domain(mesh);
    Mesh_Criteria criteria(facet_angle=30, facet_size=0.1, facet_distance=0.025,
                          cell_radius_edge_ratio=2, cell_size=0.1);
    try {
        std::cout << "made domain correctly" << std::endl;
        c3t3 = CGAL::make_mesh_3<C3T3>(domain, criteria);

    } catch (...) {
        return false;
    }
    return true;
}

void GLWidget::initShaders()
{
    _shaderProgram.compileShaders("/Users/Kadie/Documents/kadiesworkspace/A-STAR-IHPC-Project/project_vVBO/shaders/colorShading.vert",
                                  "/Users/Kadie/Documents/kadiesworkspace/A-STAR-IHPC-Project/project_vVBO/shaders/colorShading.frag");
    _shaderProgram.addAttribute("vertexPosition");
    _shaderProgram.addAttribute("vertexColor");
    _shaderProgram.linkShaders();
}


void GLWidget::drawPlane(float startX, float startY)
{
    /*std::cout << "draw draw draw" << std::endl;
    glBegin(GL_TRIANGLES);
    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
    glVertex3f(startX, startY, 0);
    glVertex3f(startX, this->y, 0);
    glVertex3f(this->x, startY, 0);
    glVertex3f(this->x,this->y,0);
    glVertex3f(startX, this->y, 0);
    glVertex3f(this->x, startY, 0);
    glEnd();*/

    //glLineWidth(3);
       glBegin(GL_QUADS);
       glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
       /* Left */
       glVertex3f(startX, startY, 0);
       glVertex3f(startX, y, 0);
       /* Right */
       glVertex3f(x, startY, 0);
       glVertex3f(x, y, 0);
       /* Top */
       glVertex3f(startX, startY, 0);
       glVertex3f(x, startY, 0);
       /* Bottom */
       glVertex3f(startX, y, 0);
       glVertex3f(x, y, 0);
       glEnd();
}


bool GLWidget::toggleRotation() {
    dx = 0; dy = 0;
    rotationOK = !rotationOK;
    return rotationOK;
}

bool GLWidget::toggleCulling() {
    cullingOK = !cullingOK;
    return cullingOK;
}

bool GLWidget::toggleVolume() {
    volumeOK = !volumeOK;
    return volumeOK;
}

bool GLWidget::toggleTranslation()
{
    dx = 0; dy = 0;
    translateOK = !translateOK;
    return translateOK;
}

bool GLWidget::setDrawPlaneMode(bool setting)
{
    drawPlaneMode = setting;
    return drawPlaneMode;
}
