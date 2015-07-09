#ifndef FACE_H
#define FACE_H
#include "vertex.h"

class Face {
private:
        float in1[3] ;
        float in2[3] ;
        float in3[3] ;
        float normal[3];

public:
    Face();
    Face(float *v1, float *v2, float *v3);
    float * getNormal();

};

#endif // FACE_H