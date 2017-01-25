#pragma once
#ifndef _ROBOT_H_
#define _ROBOT_H_

#include <string>
#include <vector>
#include <memory>

class Program;
class Shape;

class Robot
{
public:
   std::shared_ptr<Shape> shape;
	Robot();
	void draw(std::shared_ptr<Program> prog);
   std::vector<float> velocity;
   std::vector<float> position;
   float orientation;
   int mat;

private:

};

#endif
