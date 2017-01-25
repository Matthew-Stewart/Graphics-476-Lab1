/* Lab 5 base code - transforms using local matrix functions
   to be written by students -
   CPE 471 Cal Poly Z. Wood + S. Sueda
   */
#include <iostream>
#include <cmath>
#include <math.h>
#include <time.h>
#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "cube.h"
#include "doge.h"
#include "robot.h"

using namespace std;
using namespace std::chrono;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog;
shared_ptr<Program> prog2;
shared_ptr<Shape> sphere;
shared_ptr<Doge> doge;
shared_ptr<Cube> cube;
//Cube *cube;

float objectPos[40];

vector<shared_ptr<Robot>> robots;
vector<shared_ptr<Doge>> doges;

int g_width, g_height;
float sTheta;
float wave = 0;
int wave_left = 0;
int wave_right = 0;
int floor_mat = rand() % 6;
float floor_length = 20.0;
float phi;
float theta;
float pi = M_PI;
float forwardl = 0;
float rightl = 0;
int forwardc = 0;
int rightc = 0;
int up = 0;
int gMat = 0;
int normal = 0;
int lighting = 0;
int lightF = 0;
int lightB = 0;
int lightL = 0;
int lightR = 0;
float lightFor = 0;
float lightSid = 0;
float sunRadius = 6.0f;
float sunAngle = 0.0f;

Eigen::Vector3f curPos = Eigen::Vector3f(0.0f, 0.5f, 0.0f);

double t = 0.0;
double dt = 1 / 60.0 * 1000;

double currentTime = duration_cast< milliseconds >(
         system_clock::now().time_since_epoch()
         ).count();

double timeLastPlaced = 0.0;
int totalObjects = 0;
int objectsHit = 0;

/* types:
   0 = vector  (d, i)
   1 = point   (p, i)
   2 = spot    (p, d, i)
   3 = ambient (i) */
struct light_t {
   int type;
   std::vector<float> pos;
   std::vector<float> dir;
   float itn;
   std::vector<float> clr;
} Light;

void initProg (shared_ptr<Program> *p, int shader) {
   string sha = "";
   if (shader == 2) {
      sha = "2";
   } else if (shader == 3) {
      sha = "3";
   }
   (*p) = make_shared<Program>();
   (*p)->setVerbose(true);
   (*p)->setShaderNames(RESOURCE_DIR + "simple_vert" + sha + ".glsl", RESOURCE_DIR + "simple_frag" + sha + ".glsl");
   (*p)->init();
   (*p)->addUniform("P");
   (*p)->addUniform("V");
   (*p)->addUniform("M");
   (*p)->addAttribute("vertPos");
   (*p)->addAttribute("vertNor");
   if (shader != 2) {
      (*p)->addUniform("MatAmb");
      (*p)->addUniform("MatDif");
      (*p)->addUniform("MatSpe");
      (*p)->addUniform("MatShi");
      (*p)->addUniform("lightPos");
      (*p)->addUniform("lightDir");
      (*p)->addUniform("lightInt");
      (*p)->addUniform("lightClr");
      (*p)->addUniform("cPos");
   }
}

/*int V1;
  int V2;
  int V3;*/

static void error_callback(int error, const char *description)
{
   cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
   }
   if (key == GLFW_KEY_M && action == GLFW_PRESS) {
      gMat = (gMat+1)%6;
   }
   if(key == GLFW_KEY_W && action == GLFW_PRESS) {
      forwardc += 1;
   }
   if(key == GLFW_KEY_A && action == GLFW_PRESS) {
      rightc -= 1;
   }
   if(key == GLFW_KEY_S && action == GLFW_PRESS) {
      forwardc -= 1;
   }
   if(key == GLFW_KEY_D && action == GLFW_PRESS) {
      rightc += 1;
   }
   if(key == GLFW_KEY_W && action == GLFW_RELEASE) {
      forwardc -= 1;
   }
   if(key == GLFW_KEY_A && action == GLFW_RELEASE) {
      rightc += 1;
   }
   if(key == GLFW_KEY_S && action == GLFW_RELEASE) {
      forwardc += 1;
   }
   if(key == GLFW_KEY_D && action == GLFW_RELEASE) {
      rightc -= 1;
   }
}


static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
   double posX, posY;
   glfwGetCursorPos(window, &posX, &posY);
   if (action == GLFW_PRESS) {
      cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
   }
}

static void resize_callback(GLFWwindow *window, int width, int height) {
   g_width = width;
   g_height = height;
   glViewport(0, 0, width, height);
}

void drawRobot(shared_ptr<MatrixStack> MV, int i, shared_ptr<Robot> r) {
   float x = r->position[0];
   //float x = objectPos[i];
   float z = r->position[2];
   //float z = objectPos[i+1];
   float y = r->position[1];
   /* draw bottom cube */
   MV->pushMatrix();
   MV->loadIdentity();
   /* body + arms */
   MV->translate(Eigen::Vector3f(x, -0.5 + y, z));
   MV->scale(Eigen::Vector3f(0.75, 1.0, 0.75));
   MV->rotate(r->orientation, Eigen::Vector3f(0, 1, 0));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->scale(Eigen::Vector3f(1.2, 0.9, 1.2));

   /* right arm */
   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(0.75, 0.95, 0));
   MV->rotate(sTheta, Eigen::Vector3f(0, 0, 1));
   MV->translate(Eigen::Vector3f(0.75, 0, 0));
   MV->scale(Eigen::Vector3f(0.75, 0.25, 0.25));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();

   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(0.75, 1.0, 0));
   MV->rotate(sTheta + 30, Eigen::Vector3f(0, 0, 1));
   MV->translate(Eigen::Vector3f(1.3, -0.75, 0));
   MV->rotate(wave, Eigen::Vector3f(0, 0, 1));
   MV->translate(Eigen::Vector3f(0.7, 0, 0));
   MV->scale(Eigen::Vector3f(0.75, 0.25, 0.25));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(1.3, 0.3, 0));
   MV->scale(Eigen::Vector3f(0.33, 1.0, 1.0));
   MV->rotate(20, Eigen::Vector3f(0, 0, 1));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();
   MV->popMatrix();

   /* left arm */
   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(-0.75, 0.95, 0));
   MV->rotate(-1 * sTheta, Eigen::Vector3f(0, 0, 1));
   MV->translate(Eigen::Vector3f(-0.75, 0, 0));
   MV->scale(Eigen::Vector3f(0.75, 0.25, 0.25));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();

   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(-0.75, 1.0, 0));
   MV->rotate(-1 * sTheta - 30, Eigen::Vector3f(0, 0, 1));
   MV->translate(Eigen::Vector3f(-2, -0.75, 0));
   MV->scale(Eigen::Vector3f(0.75, 0.25, 0.25));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->pushMatrix();
   MV->translate(Eigen::Vector3f(-1.3, 0.3, 0));
   MV->scale(Eigen::Vector3f(0.33, 1.0, 1.0));
   MV->rotate(-20, Eigen::Vector3f(0, 0, 1));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();
   MV->popMatrix();
   MV->popMatrix();

   /* head */
   MV->pushMatrix();
   MV->loadIdentity();
   MV->translate(Eigen::Vector3f(x, 0.8 + y, z));
   MV->rotate(45, Eigen::Vector3f(0, 1, 0));
   MV->scale(Eigen::Vector3f(0.5, 0.6, 0.5));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();

   /* right eye */
   MV->pushMatrix();
   MV->loadIdentity();
   MV->translate(Eigen::Vector3f(x + 0.35, 1.0 + y, 0.355 + z));
   MV->rotate(45, Eigen::Vector3f(0, 1, 0));
   MV->scale(Eigen::Vector3f(1.0, 1.0, 0.15));
   MV->rotate((sTheta*sTheta), Eigen::Vector3f(0, 0, 1));
   MV->scale(Eigen::Vector3f(0.1, 0.1, 0.1));
   MV->rotate(45, Eigen::Vector3f(1, 0, 0));
   MV->rotate(45, Eigen::Vector3f(0, 1, 0));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();

   /* left eye */
   MV->pushMatrix();
   MV->loadIdentity();
   MV->translate(Eigen::Vector3f(x - 0.35, 1.0 + y, 0.355 + z));
   MV->rotate(-45, Eigen::Vector3f(0, 1, 0));
   MV->scale(Eigen::Vector3f(1.0, 1.0, 0.15));
   MV->rotate(-1*(sTheta*sTheta), Eigen::Vector3f(0, 0, 1));
   MV->scale(Eigen::Vector3f(0.1, 0.1, 0.1));
   MV->rotate(45, Eigen::Vector3f(1, 0, 0));
   MV->rotate(45, Eigen::Vector3f(0, 1, 0));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, MV->topMatrix().data());
   cube->draw(prog);
   MV->popMatrix();
}

//helper function to set materials
void SetMaterial(int i, shared_ptr<Program> p) {
   assert(glGetError() == GL_NO_ERROR);
   int j = i % 6;
   switch (j) {
      case 0: // shiny blue plastic
         glUniform3f(p->getUniform("MatAmb"), 0.02, 0.04, 0.2);
         glUniform3f(p->getUniform("MatDif"), 0.0, 0.16, 0.9);
         glUniform3f(p->getUniform("MatSpe"), 0.14, 0.2, 0.8);
         glUniform1f(p->getUniform("MatShi"), 120.0);
         break;
      case 1: // flat grey
         glUniform3f(p->getUniform("MatAmb"), 0.13, 0.13, 0.14);
         glUniform3f(p->getUniform("MatDif"), 0.3, 0.3, 0.4);
         glUniform3f(p->getUniform("MatSpe"), 0.3, 0.3, 0.4);
         glUniform1f(p->getUniform("MatShi"), 4.0);
         break;
      case 2: // brass
         glUniform3f(p->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
         glUniform3f(p->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
         glUniform3f(p->getUniform("MatSpe"), 0.9922, 0.941176, 0.80784);
         glUniform1f(p->getUniform("MatShi"), 27.9);
         break;
      case 3: // copper
         glUniform3f(p->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
         glUniform3f(p->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
         glUniform3f(p->getUniform("MatSpe"), 0.257, 0.1376, 0.08601);
         glUniform1f(p->getUniform("MatShi"), 12.8);
         break;
      case 4: // grass
         glUniform3f(p->getUniform("MatAmb"), 0.1313, 0.2735, 0.0225);
         glUniform3f(p->getUniform("MatDif"), 0.1538, 0.2748, 0.0228);
         glUniform3f(p->getUniform("MatSpe"), 0.157, 0.5376, 0.08601);
         glUniform1f(p->getUniform("MatShi"), 50.0);
         break;
      case 5: // sun
         glUniform3f(p->getUniform("MatAmb"), 0.8313, 0.8735, 0.1225);
         glUniform3f(p->getUniform("MatDif"), 0.8538, 0.8748, 0.1228);
         glUniform3f(p->getUniform("MatSpe"), 0.057, 0.0376, 0.08601);
         glUniform1f(p->getUniform("MatShi"), 10.0);
         break;
   }
}

static void init()
{
   GLSL::checkVersion();

   srand(time(0));

   sTheta = 0;
   // Set background color.
   glClearColor(.12f, .34f, .56f, 1.0f);
   // Enable z-buffer test.
   glEnable(GL_DEPTH_TEST);

   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

   assert(glGetError() == GL_NO_ERROR);
   // Initialize mesh.
   sphere = make_shared<Shape>();
   sphere->loadMesh(RESOURCE_DIR + "smoothsphere.obj");
   sphere->resize();
   sphere->init();
   assert(glGetError() == GL_NO_ERROR);

   /*shared_ptr<Shape> dogeShape = make_shared<Shape>();
   doge = make_shared<Doge>(dogeShape);
   doge->shape->loadMesh(RESOURCE_DIR + "dog.obj");
   doge->shape->resize();
   doge->shape->init();
   assert(glGetError() == GL_NO_ERROR);*/

   shared_ptr<Shape> cubeShape = make_shared<Shape>();
   cube = make_shared<Cube>(cubeShape);
   cube->shape->loadMesh(RESOURCE_DIR + "cube.obj");
   cube->shape->resize();
   cube->shape->init();
   assert(glGetError() == GL_NO_ERROR);

   // objects + floor
   if (prog == nullptr) {
      initProg(&prog, 1);
   }
   totalObjects += 1;

   // light
   if (prog2 == nullptr) {
      initProg(&prog2, 2);
   }
   totalObjects += 1;

   for (int i = 0; i < 10; i++) {
      robots.push_back(make_shared<Robot>());
      robots[i]->mat = rand() % 5;
      robots[i]->orientation = 0;
      totalObjects += 1;
   }

   // robots
   for (int i = 0; i < 10; i++) {
      float x = 15 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 30));
      float y = 1.0;
      float z = 15 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 30));
      robots[i]->position[0] = x;
      robots[i]->position[1] = y;
      robots[i]->position[2] = z;
      float rand360 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360));
      robots[i]->orientation = rand360;
      robots[i]->velocity[0] = sin(rand360);
      robots[i]->velocity[1] = 0;
      robots[i]->velocity[2] = cos(rand360);
   }
}

static void render()
{
   Light.type = 1;
   Light.pos = { sunRadius * cos(sunAngle), 3.0f, sunRadius * sin(sunAngle), 0.0f };
   Light.dir = { 0,0,0 };
   Light.itn = 1;
   Light.clr = { 1,1,1,1 };

   // Get current frame buffer size.
   int width, height;
   glfwGetFramebufferSize(window, &width, &height);

   // Clear framebuffer.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Use the matrix stack for Lab 6
   float aspect = width/(float)height;

   // Create the matrix stacks - please leave these alone for now
   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();
   auto M = make_shared<MatrixStack>();
   // Apply perspective projection.
   P->pushMatrix();
   P->perspective(45.0f, aspect, 0.01f, 100.0f);

   float LAx = 1 * cos(phi) * cos(theta);
   float LAy = 1 * sin(phi);
   float LAz = 1 * cos(phi) * cos(pi / 2 - theta);

   // Eigen::Vector3f eye = Vector3f(rightl*LAx, 0.0f, forwardl*LAz);
   Eigen::Vector3f eye = curPos;
   LAx = LAx + eye(0);
   LAy = LAy + eye(1);
   LAz = LAz + eye(2);
   Eigen::Vector3f LA = Eigen::Vector3f(LAx, LAy, LAz);
   Eigen::Vector3f upp = Eigen::Vector3f(0, 1, 0);

   Eigen::Vector3f w = (eye - LA).normalized();
   Eigen::Vector3f u = upp.cross(w);
   //Eigen::Vector3f v = w.cross(u);

   eye = eye + w + u;
   LA = LA + w + u;

   V->pushMatrix();
   V->lookAt(eye, LA, upp);

   prog->bind();
   glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   SetMaterial(floor_mat, prog);

   // floor
   M->pushMatrix();
   M->loadIdentity();
   M->translate(Eigen::Vector3f(0, -0.8, 0));
   M->scale(Eigen::Vector3f(floor_length, 0.5, floor_length));
   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   cube->draw(prog);
   M->popMatrix();

   // robots
   for (int i = 0; i < robots.size(); i++) {
      SetMaterial(robots[i]->mat, prog);
      //drawRobot(M, 2*i, robots[i]);
      drawRobot(M, i, robots[i]);
   }
   assert(glGetError() == GL_NO_ERROR);

   // doges
   for (int i = 0; i < doges.size(); i++) {
      SetMaterial(doges[i]->mat, prog);
      //float x = objectPos[2*i+20] - curPos(0);
      //float y = objectPos[2*i+21] - curPos(2);
      float x = doges[i]->position[0];
      float z = doges[i]->position[2];
      float angle = atan(z / x) + pi / 2.0;
      M->pushMatrix();
      M->loadIdentity();
      M->translate(Eigen::Vector3f(x, 0.8, z));
      M->rotate(-1 * angle * 180 / pi, Eigen::Vector3f(0, 1, 0));
      M->scale(Eigen::Vector3f(1.5, 1.5, 1.5));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
      doge->draw(prog);
      M->popMatrix();
   }
   assert(glGetError() == GL_NO_ERROR);


   glUniform4f(prog->getUniform("lightPos"), Light.pos[0], Light.pos[1], Light.pos[2], Light.pos[3]);
   glUniform3f(prog->getUniform("lightDir"), Light.dir[0], Light.dir[1], Light.dir[2]);
   glUniform1f(prog->getUniform("lightInt"), Light.itn);
   glUniform3f(prog->getUniform("lightClr"), Light.clr[0], Light.clr[1], Light.clr[2]);

   glUniform3f(prog->getUniform("cPos"), curPos(0), curPos(1), curPos(2));
   assert(glGetError() == GL_NO_ERROR);

   prog->unbind();

   prog2->bind();
   assert(glGetError() == GL_NO_ERROR);
   glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
   assert(glGetError() == GL_NO_ERROR);
   glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, V->topMatrix().data());
   assert(glGetError() == GL_NO_ERROR);
   //SetMaterial(gMat++, prog2);
   assert(glGetError() == GL_NO_ERROR);

   M->pushMatrix();
   M->loadIdentity();
   M->translate(Eigen::Vector3f(sunRadius * cos(sunAngle), 3.0f, sunRadius * sin(sunAngle)));
   M->scale(Eigen::Vector3f(0.3, 0.3, 0.3));
   glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, M->topMatrix().data());
   sphere->draw(prog2);
   M->popMatrix();
   assert(glGetError() == GL_NO_ERROR);

   prog2->unbind();

   // Pop matrix stacks.
   V->popMatrix();
   P->popMatrix();
   assert(glGetError() == GL_NO_ERROR);
}

void moveForward(shared_ptr<Robot> r, float deltaTime) {
   // previous values
   float px = r->position[0];
   float py = r->position[1];
   float pz = r->position[2];
   float pvelx = r->velocity[0] * (1.0 + objectsHit/3.0);
   float pvely = r->velocity[1] * (1.0 + objectsHit/3.0);
   float pvelz = r->velocity[2] * (1.0 + objectsHit/3.0);

   float newx = px + pvelx * deltaTime / 1000.0;
   float newy = py + pvely * deltaTime / 1000.0;
   float newz = pz + pvelz * deltaTime / 1000.0;

   r->position[0] = newx;
   r->position[1] = newy;
   r->position[2] = newz;
}

void moveForward(shared_ptr<Doge> r, float deltaTime) {
   // previous values
   float px = r->position[0];
   float py = r->position[1];
   float pz = r->position[2];
   float pvelx = r->velocity[0];
   float pvely = r->velocity[1];
   float pvelz = r->velocity[2];

   float newx = px + pvelx * deltaTime / 1000.0;
   float newy = py + pvely * deltaTime / 1000.0;
   float newz = pz + pvelz * deltaTime / 1000.0;

   r->position[0] = newx;
   r->position[1] = newy;
   r->position[2] = newz;
}

float collisionDistance = 2.0;

void collide(shared_ptr<Robot> r1, shared_ptr<Robot> r2) {
   float x1 = r1->position[0];
   float y1 = r1->position[1];
   float z1 = r1->position[2];
   float x2 = r2->position[0];
   float y2 = r2->position[1];
   float z2 = r2->position[2];
   float velx1 = r1->velocity[0];
   float vely1 = r1->velocity[1];
   float velz1 = r1->velocity[2];
   float velx2 = r2->velocity[0];
   float vely2 = r2->velocity[1];
   float velz2 = r2->velocity[2];

   float distance = sqrt((x2 - x1)*(x2 - x1) + (z2 - z1)*(z2 - z1));

   if (distance < collisionDistance) {
      Eigen::Vector3f pos1 = Eigen::Vector3f(x1,y1,z1);
      Eigen::Vector3f pos2 = Eigen::Vector3f(x2,y2,z2);
      Eigen::Vector3f vel1 = Eigen::Vector3f(velx1,vely1,velz1);
      Eigen::Vector3f vel2 = Eigen::Vector3f(velx2,vely2,velz2);
      float from1to2 = vel1.dot(pos2 - pos1);
      float from2to1 = vel2.dot(pos1 - pos2);

      if (from1to2 > 0.0) {
         r1->velocity[0] = velx1 * -1;
         r1->velocity[2] = velz1 * -1;
      }
      if (from2to1 > 0.0) {
         r2->velocity[0] = velx2 * -1;
         r2->velocity[2] = velz2 * -1;
      }
      //printf("Robot with Robot at %f,%f\n", pos1.x(), pos1.z());
   }
}

// didn't have time to make a specific object class
// that both robot and doge would inherit from
void collide(shared_ptr<Robot> r1, shared_ptr<Doge> r2) {
   float x1 = r1->position[0];
   float y1 = r1->position[1];
   float z1 = r1->position[2];
   float x2 = r2->position[0];
   float y2 = r2->position[1];
   float z2 = r2->position[2];
   float velx1 = r1->velocity[0];
   float vely1 = r1->velocity[1];
   float velz1 = r1->velocity[2];
   float velx2 = r2->velocity[0];
   float vely2 = r2->velocity[1];
   float velz2 = r2->velocity[2];

   float distance = sqrt((x2 - x1)*(x2 - x1) + (z2 - z1)*(z2 - z1));

   if (distance < collisionDistance) {
      Eigen::Vector3f pos1 = Eigen::Vector3f(x1,y1,z1);
      Eigen::Vector3f pos2 = Eigen::Vector3f(x2,y2,z2);
      Eigen::Vector3f vel1 = Eigen::Vector3f(velx1,vely1,velz1);
      Eigen::Vector3f vel2 = Eigen::Vector3f(velx2,vely2,velz2);
      float from1to2 = vel1.dot(pos2 - pos1);
      float from2to1 = vel2.dot(pos1 - pos2);

      if (from1to2 > 0.0) {
         r1->velocity[0] = velx1 * -1;
         r1->velocity[2] = velz1 * -1;
      }
      if (from2to1 > 0.0) {
         r2->velocity[0] = velx2 * -1;
         r2->velocity[2] = velz2 * -1;
      }
      //printf("Robot with Doge  at %f,%f\n", pos1.x(), pos1.z());
   }
}

void collide(shared_ptr<Doge> r1, shared_ptr<Doge> r2) {
   float x1 = r1->position[0];
   float y1 = r1->position[1];
   float z1 = r1->position[2];
   float x2 = r2->position[0];
   float y2 = r2->position[1];
   float z2 = r2->position[2];
   float velx1 = r1->velocity[0];
   float vely1 = r1->velocity[1];
   float velz1 = r1->velocity[2];
   float velx2 = r2->velocity[0];
   float vely2 = r2->velocity[1];
   float velz2 = r2->velocity[2];

   float distance = sqrt((x2 - x1)*(x2 - x1) + (z2 - z1)*(z2 - z1));

   if (distance < collisionDistance) {
      Eigen::Vector3f pos1 = Eigen::Vector3f(x1,y1,z1);
      Eigen::Vector3f pos2 = Eigen::Vector3f(x2,y2,z2);
      Eigen::Vector3f vel1 = Eigen::Vector3f(velx1,vely1,velz1);
      Eigen::Vector3f vel2 = Eigen::Vector3f(velx2,vely2,velz2);
      float from1to2 = vel1.dot(pos2 - pos1);
      float from2to1 = vel2.dot(pos1 - pos2);

      if (from1to2 > 0.0) {
         r1->velocity[0] = velx1 * -1;
         r1->velocity[2] = velz1 * -1;
      }
      if (from2to1 > 0.0) {
         r2->velocity[0] = velx2 * -1;
         r2->velocity[2] = velz2 * -1;
      }
      //printf("Doge  with Doge  at %f,%f\n", pos1.x(), pos1.z());
   }
}

void collide(Eigen::Vector3f cameraPos, shared_ptr<Robot> r, Eigen::Vector3f oldPos) {
   // you die
   float cx = cameraPos(0);
   float cz = cameraPos(2);
   float rx = r->position[0];
   float rz = r->position[2];

   float distance = sqrt((rx - cx)*(rx - cx) + (rz - cz)*(rz - cz));

   if (distance < collisionDistance) {
      // reset game
      objectsHit = 0;
      totalObjects = 0;
      timeLastPlaced = 0.0;
      doges.clear();
      robots.clear();
      init();
   }
}

void collide(Eigen::Vector3f cameraPos, shared_ptr<Doge> r, Eigen::Vector3f oldPos) {
   float cx = cameraPos(0);
   float cz = cameraPos(2);
   float rx = r->position[0];
   float rz = r->position[2];

   float distance = sqrt((rx - cx)*(rx - cx) + (rz - cz)*(rz - cz));

   if (distance < collisionDistance) {
      if (r->collected == false) {
         r->velocity[0] = 0.0;
         r->velocity[1] = 0.0;
         r->velocity[2] = 0.0;

         r->mat = 5;

         r->collected = true;

         objectsHit += 1;
      }
      curPos = oldPos;
   }
}

void makeDoge() {
   int i = doges.size();

   shared_ptr<Shape> dogeShape = make_shared<Shape>();
   doge = make_shared<Doge>(dogeShape);
   doge->shape->loadMesh(RESOURCE_DIR + "dog.obj");
   doge->shape->resize();
   doge->shape->init();
   assert(glGetError() == GL_NO_ERROR);

      doges.push_back(make_shared<Doge>(dogeShape));
      doges[i]->mat = rand() % 5;
      doges[i]->orientation = 0;
      doges[i]->placed = true;
      doges[i]->collected = false;
      totalObjects += 1;

   // doges
      float x = 15 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 30));
      float y = 1.0;
      float z = 15 - static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 30));
      doges[i]->position[0] = x;
      doges[i]->position[1] = y;
      doges[i]->position[2] = z;
      float rand360 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360));
      doges[i]->velocity[0] = sin(rand360) * 0.8;
      doges[i]->velocity[1] = 0;
      doges[i]->velocity[2] = cos(rand360) * 0.8;

}

void integrate (float deltaTime) {

   if (currentTime - 3000 > timeLastPlaced && doges.size() < 10) {
      makeDoge();
      timeLastPlaced = currentTime;
   }

   // Get current frame buffer size.
   int width, height;
   glfwGetFramebufferSize(window, &width, &height);
   glViewport(0, 0, width, height);

   double x, y;
   glfwGetCursorPos(window, &x, &y);

   phi = -0.4 * pi + 1.6 * pi * y / (double)height;
   theta = pi / 2.0 - 4.0 * pi * x / (double)width;

   //Use the matrix stack for Lab 6
   float speed = 0.0048 * deltaTime;

   float LAx = 1 * cos(phi) * cos(theta);
   float LAy = 1 * sin(phi);
   float LAz = 1 * cos(phi) * cos(pi / 2 - theta);

   Eigen::Vector3f oldCurPos = curPos;

   curPos = Eigen::Vector3f(curPos(0) - speed * rightc * LAz + speed * forwardc * LAx,
         curPos(1), curPos(2) + speed * forwardc * LAz + speed * rightc * LAx);

   if (up == 1) {
      sTheta += deltaTime * 0.040;
   } else {
      sTheta -= deltaTime * 0.040;
   }

   if (sTheta < -70) {
      up = 1;
   }
   if (sTheta > 0) {
      up = 0;
   }

   if (wave_right && wave > -30) {
      wave -= deltaTime * 0.080;
   }
   if (wave_left && wave < 60) {
      wave += deltaTime * 0.080;
   }

   sunAngle += deltaTime * 0.0008;

   for (int i = 0; i < robots.size(); i++) {
      moveForward(robots[i], deltaTime);
   }
   for (int i = 0; i < doges.size(); i++) {
      moveForward(doges[i], deltaTime);
   }
   for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 10; j++) {
         if (i != j) {
            collide(robots[i], robots[j]);
            if (i < doges.size() && j < doges.size()) {
               collide(doges[i], doges[j]);
            }
         }
         if (j < doges.size()) {
            collide(robots[i], doges[j]);
         }
         if (i < doges.size()) {
            collide(robots[j], doges[i]);
         }
      }
      // check collide with camera
      collide(curPos, robots[i], oldCurPos);
      if (i < doges.size()) {
         collide(curPos, doges[i], oldCurPos);
      }
      // check collide with grid edges
      std::vector<float> rpos = robots[i]->position;
      std::vector<float> rvel = robots[i]->velocity;
      if ((rpos[0] < floor_length * -1 && rvel[0] < 0.0) ||
          (rpos[0] > floor_length      && rvel[0] > 0.0)) {
         robots[i]->velocity[0] = rvel[0] * -1;
      }
      if ((rpos[2] < floor_length * -1 && rvel[2] < 0.0) ||
          (rpos[2] > floor_length      && rvel[2] > 0.0)) {
         robots[i]->velocity[2] = rvel[2] * -1;
      }
      if (i < doges.size()) {
         std::vector<float> dpos = doges[i]->position;
         std::vector<float> dvel = doges[i]->velocity;
         if ((dpos[0] < floor_length * -1 && dvel[0] < 0.0) ||
             (dpos[0] > floor_length      && dvel[0] > 0.0)) {
            doges[i]->velocity[0] = dvel[0] * -1;
         }
         if ((dpos[2] < floor_length * -1 && dvel[2] < 0.0) ||
             (dpos[2] > floor_length      && dvel[2] > 0.0)) {
            doges[i]->velocity[2] = dvel[2] * -1;
         }
      }
   }

}

int main(int argc, char **argv) {
   if(argc < 2) {
      cout << "Please specify the resource directory." << endl;
      return 0;
   }
   RESOURCE_DIR = argv[1] + string("/");

   // Set error callback.
   glfwSetErrorCallback(error_callback);
   // Initialize the library.
   if(!glfwInit()) {
      return -1;
   }
   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

   // Create a windowed mode window and its OpenGL context.
   window = glfwCreateWindow(1920, 1080, "Matthew Stewart", NULL, NULL);
   if(!window) {
      glfwTerminate();
      return -1;
   }
   // Make the window's context current.
   glfwMakeContextCurrent(window);
   // Initialize GLEW.
   glewExperimental = true;
   glewExperimental = GL_TRUE;
   if(glewInit() != GLEW_OK) {
      cerr << "Failed to initialize GLEW" << endl;
      return -1;
   }
   //weird bootstrap of glGetError
   glGetError();
   cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

   // Set vsync.
   glfwSwapInterval(1);
   // Set keyboard callback.
   glfwSetKeyCallback(window, key_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

   //glfwSetCursorPosCallback(window, cursor_position_callback);

   // Initialize scene. Note geometry initialized in init now
   init();

   // Loop until the user closes the window.
   while(!glfwWindowShouldClose(window)) {

      assert(glGetError() == GL_NO_ERROR);
      double newTime = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
            ).count();
      double frameTime = newTime - currentTime;
      currentTime = newTime;

      printf("fps: %f,  object count: %d,  objects encountered: %d\n",
         1000.0/frameTime, totalObjects, objectsHit);

      while ( frameTime > 0.0 )
      {
         float deltaTime = min( frameTime, dt );
         integrate( deltaTime );
         frameTime -= deltaTime;
         t += deltaTime;
      }

      // Render scene.
      render();
      assert(glGetError() == GL_NO_ERROR);
      // Swap front and back buffers.
      glfwSwapBuffers(window);
      assert(glGetError() == GL_NO_ERROR);
      // Poll for and process events.
      glfwPollEvents();
      assert(glGetError() == GL_NO_ERROR);
   }
   // Quit program.
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}
