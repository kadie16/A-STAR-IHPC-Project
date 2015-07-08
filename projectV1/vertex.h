#ifndef VERTEX_H
#define VERTEX_H
#include <string>
#include <sstream>
#include<iostream>
#include <cmath>
#include <vector>
class Vertex
{
private:
  float x,y,z;
  std::vector<float> normal;

public:
    Vertex();
    Vertex(std::string input, char type);
    void print();
    void parseCoordinates(std::string input);
    void parseFace(std::string input);
    float getX();
    float getY();
    float getZ();
    float * toArray();
    void assignNormal(float v1, float v2, float v3);
    std::vector<float> findNormal(Vertex v2, Vertex v3);
    void inheritNormal(Vertex parent);
    std::vector<float> getNormal();


};

#endif // VERTEX_H
