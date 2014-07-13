#version 430

in vec3 position;

uniform mat4 projectionView;
uniform mat4 view;

void main()
{
  gl_PointSize = 10.0;
  gl_Position = projectionView * vec4(position, 1.0);
  gl_Position.z = 0.0;
}
