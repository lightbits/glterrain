#version 430

in vec2 vTexel;

uniform sampler3D tex;
uniform float depth;

out vec4 outColor;

void main()
{
  outColor = texture(tex, vec3(vTexel, depth));
}
