#include "Doge.h"
#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

//#include "tiny_obj_loader.h"

using namespace std;

Doge::Doge(std::shared_ptr<Shape> shape) {
   position = std::vector<float>(3);
	velocity = std::vector<float>(3);
   this->shape = shape;
}

void Doge::draw(std::shared_ptr<Program> prog) {
   this->shape->draw(prog);
}

