#pragma once
#ifndef _DOGE_H_
#define _DOGE_H_

#include <string>
#include <vector>
#include <memory>

class Program;
class Shape;

class Doge
{
public:
   std::shared_ptr<Shape> shape;
	Doge(std::shared_ptr<Shape> shape);
	void draw(std::shared_ptr<Program> prog);
   std::vector<float> velocity;
   std::vector<float> position;
   float orientation;
   int mat;
   bool collected;
   bool placed;

private:

};

#endif
