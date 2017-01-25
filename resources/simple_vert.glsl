#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lightPos;  // light position
uniform vec3 lightDir;  // light direction
uniform float lightInt; // light intensity
uniform vec3 lightClr;  // light color
uniform vec3 MatAmb;    // ambient
uniform vec3 MatDif;    // diffuse
uniform vec3 MatSpe;    // specular
uniform float MatShi;   // shininess
uniform vec3 cPos;
out vec4 fvertPos;
out vec3 fvertNor;
out mat4 fP;
out mat4 fV;
out mat4 fM;
out vec4 flightPos;  // light position
out vec3 flightDir;  // light direction
out float flightInt; // light intensity
out vec3 flightClr;  // light color
out vec3 fMatAmb;    // ambient
out vec3 fMatDif;    // diffuse
out vec3 fMatSpe;    // specular
out float fMatShi;   // shininess
out vec3 fcPos;

void main()
{
   gl_Position = P * V * M * vertPos;

   fvertPos = M * vertPos;
   fvertNor = normalize(vec3(M * vec4(vertNor, 0)));
   fP = P;
   fV = V;
   fM = M;
   flightPos = lightPos;
   flightDir = lightDir;
   flightInt = lightInt;
   flightClr = lightClr;
   fMatAmb = MatAmb;
   fMatDif = MatDif;
   fMatSpe = MatSpe;
   fMatShi = MatShi;
   fcPos = vec3(M * vec4(cPos, 1));
}
