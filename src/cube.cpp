#include "Cube.h"
#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

//#include "tiny_obj_loader.h"

using namespace std;

shared_ptr<Shape> shape;
float velocity;
std::vector<float> position;
float orientation;

Cube::Cube(shared_ptr<Shape> shape) {
   this->shape = shape;
}

void Cube::draw(std::shared_ptr<Program> prog) {
   this->shape->draw(prog);
}

