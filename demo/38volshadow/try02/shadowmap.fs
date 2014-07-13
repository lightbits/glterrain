#version 430

out vec4 outColor;

void main()
{
  vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
  float r2 = dot(xy, xy);
  float density = exp2(-r2 * 5.0) * 0.05;
  outColor = vec4(density);
}
