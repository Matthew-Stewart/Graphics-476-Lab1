#version  330 core
in vec4 fvertPos;
in vec3 fvertNor;
in mat4 fP;
in mat4 fV;
in mat4 fM;
in vec4 flightPos;  // light position
in vec3 flightDir;  // light direction
in float flightInt; // light intensity
in vec3 flightClr;  // light color
in vec3 fMatAmb;    // ambient
in vec3 fMatDif;    // diffuse
in vec3 fMatSpe;    // specular
in float fMatShi;   // shininess
in vec3 fcPos;
out vec3 color;

void main()
{
   vec4 wPos = fvertPos;
   vec3 L = normalize(vec3(flightPos - fvertPos));
   vec3 V = normalize(fcPos - wPos.xyz);
   vec3 N = normalize(fvertNor);
   vec3 dif = fMatDif * max(dot(N, L), 0) * flightClr;
   // vec3 R = -1 * L + 2 * max((dot(N, L)),0) * N;
   // vec3 spe = fMatSpe * (pow(dot(V, R), fMatShi)) * flightClr;
   vec3 H = normalize((V + L) / 2);
   vec3 spe = fMatSpe * (pow(dot(N, H), fMatShi)) * flightClr;
   vec3 amb = fMatAmb * flightClr;

   float d = distance(wPos, flightPos);

   vec3 shapeClr = min(amb + 1/(0.2 + 0.15*d + 0.02*d*d) * (dif + spe) * flightInt, 1);

   color = shapeClr;
}
