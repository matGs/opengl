#version 330

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;

out vec4 vColor;

void main()
{
   gl_Position = ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
   vColor = vec4(1.0, 1.0, 1.0, 1.0);
}
