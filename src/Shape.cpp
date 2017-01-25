#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
   eleBufID(0),
   posBufID(0),
   norBufID(0),
   texBufID(0),
   vaoID(0)
{
}

Shape::~Shape()
{
}

void Shape::ComputeNormals() {
   for (int i = 0; i < eleBuf.size() / 3; i++) {
      int v0idx = 3*eleBuf[3*i+0];
      int v1idx = 3*eleBuf[3*i+1];
      int v2idx = 3*eleBuf[3*i+2];

      Eigen::Vector3f v0(posBuf[v0idx+0],posBuf[v0idx+1],posBuf[v0idx+2]);
      Eigen::Vector3f v1(posBuf[v1idx+0],posBuf[v1idx+1],posBuf[v1idx+2]);
      Eigen::Vector3f v2(posBuf[v2idx+0],posBuf[v2idx+1],posBuf[v2idx+2]);

      Eigen::Vector3f e1 = v1 - v0;
      Eigen::Vector3f e2 = v2 - v0;

      Eigen::Vector3f ne = e1.cross(e2);

      norBuf[v0idx+0] += ne(0);
      norBuf[v0idx+1] += ne(1);
      norBuf[v0idx+2] += ne(2);
      norBuf[v1idx+0] += ne(0);
      norBuf[v1idx+1] += ne(1);
      norBuf[v1idx+2] += ne(2);
      norBuf[v2idx+0] += ne(0);
      norBuf[v2idx+1] += ne(1);
      norBuf[v2idx+2] += ne(2);

   }
   for (int i = 0; i < norBuf.size() / 3; i++) {
      float len = sqrt(norBuf[3*i+0]*norBuf[3*i+0] + norBuf[3*i+1]*norBuf[3*i+1] + norBuf[3*i+2]*norBuf[3*i+2]);

      norBuf[3*i+0] /= len;
      norBuf[3*i+1] /= len;
      norBuf[3*i+2] /= len;
   }
}

void Shape::loadMesh(const string &meshName)
{
   assert(glGetError() == GL_NO_ERROR);
   // Load geometry
   // Some obj files contain material information.
   // We'll ignore them for this assignment.
   vector<tinyobj::shape_t> shapes;
   vector<tinyobj::material_t> objMaterials;
   string errStr;
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
   if(!rc) {
      cerr << errStr << endl;
   } else {
      posBuf = shapes[0].mesh.positions;
      //norBuf = shapes[0].mesh.normals;
      texBuf = shapes[0].mesh.texcoords;
      eleBuf = shapes[0].mesh.indices;

      for (size_t v = 0; v < posBuf.size(); v++) {
         norBuf.push_back(0);
      }

      ComputeNormals();

   }
   assert(glGetError() == GL_NO_ERROR);
}

void Shape::resize() {
   assert(glGetError() == GL_NO_ERROR);
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
      if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
      if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

      if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
      if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

      if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
      if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
   }

   //From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
      posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
      assert(posBuf[3*v+0] >= -1.0 - epsilon);
      assert(posBuf[3*v+0] <= 1.0 + epsilon);
      posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
      assert(posBuf[3*v+1] >= -1.0 - epsilon);
      assert(posBuf[3*v+1] <= 1.0 + epsilon);
      posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
      assert(posBuf[3*v+2] >= -1.0 - epsilon);
      assert(posBuf[3*v+2] <= 1.0 + epsilon);
   }
   assert(glGetError() == GL_NO_ERROR);
}

void Shape::init()
{
   assert(glGetError() == GL_NO_ERROR);
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

   // Send the position array to the GPU
   glGenBuffers(1, &posBufID);
   glBindBuffer(GL_ARRAY_BUFFER, posBufID);
   glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

   // Send the normal array to the GPU
   if(norBuf.empty()) {
      norBufID = 0;
   } else {
      glGenBuffers(1, &norBufID);
      glBindBuffer(GL_ARRAY_BUFFER, norBufID);
      glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
   }

   // Send the texture array to the GPU
   if(texBuf.empty()) {
      texBufID = 0;
   } else {
      glGenBuffers(1, &texBufID);
      glBindBuffer(GL_ARRAY_BUFFER, texBufID);
      glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
   }

   // Send the element array to the GPU
   glGenBuffers(1, &eleBufID);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);

   // Unbind the arrays
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   // check OpenGL error
   GLenum err;
   while ((err = glGetError()) != GL_NO_ERROR) {
      string error;
      switch(err) {
         case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
         case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
         case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
         case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
         case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
      }
      cerr << "OpenGL error: " << error.c_str();
      cerr << "OpenGL error: " << err << endl;
   }

   //cout << "OpenGL Error: " << glGetError() << endl;
   assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
   assert(glGetError() == GL_NO_ERROR);
   int h_pos, h_nor, h_tex;
   h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
   // Bind position buffer
   h_pos = prog->getAttribute("vertPos");
   GLSL::enableVertexAttribArray(h_pos);
   glBindBuffer(GL_ARRAY_BUFFER, posBufID);
   glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

   // Bind normal buffer
   h_nor = prog->getAttribute("vertNor");
   if(h_nor != -1 && norBufID != 0) {
      GLSL::enableVertexAttribArray(h_nor);
      glBindBuffer(GL_ARRAY_BUFFER, norBufID);
      glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
   }
   int dotexture = 0;
   if (dotexture == 1 && texBufID != 0) {
      // Bind texcoords buffer
      h_tex = prog->getAttribute("vertTex");
      if(h_tex != -1 && texBufID != 0) {
         GLSL::enableVertexAttribArray(h_tex);
         glBindBuffer(GL_ARRAY_BUFFER, texBufID);
         glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
      }
   }

   // Bind element buffer
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);

   // Draw
   glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);

   // Disable and unbind
   if(h_tex != -1) {
      GLSL::disableVertexAttribArray(h_tex);
   }
   if(h_nor != -1) {
      GLSL::disableVertexAttribArray(h_nor);
   }
   GLSL::disableVertexAttribArray(h_pos);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   assert(glGetError() == GL_NO_ERROR);
}
