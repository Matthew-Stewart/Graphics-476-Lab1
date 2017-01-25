#pragma once
#ifndef _CUBE_H_
#define _CUBE_H_

#include <string>
#include <vector>
#include <memory>

class Program;
class Shape;

class Cube
{
   public:
      std::shared_ptr<Shape> shape;
      Cube(std::shared_ptr<Shape> shape);
      void draw(std::shared_ptr<Program> prog);
      float velocity;
      std::vector<float> position;
      float orientation;

   private:

};

#endif
