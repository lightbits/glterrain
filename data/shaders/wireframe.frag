#version 140

in vec4 vertColor;
in vec3 vBC;

out vec4 outColor;

// http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
// http://stackoverflow.com/questions/6800544/what-is-the-fwidth-glsl-function-actually-used-for
// http://http.developer.nvidia.com/Cg/fwidth.html
// Returns a smoothed measure of how close we are to an edge
float edgeFactor()
{
	vec3 d = fwidth(vBC);
	float thickness = 1.5;
	vec3 a3 = smoothstep(vec3(0.0), d * thickness, vBC);
	return min(min(a3.x, a3.y), a3.z);
}

void main()
{
	outColor.rgb = mix(vec3(0.0), vertColor.rgb, edgeFactor());
	// outColor = vec4(0.0, 0.0, 0.0, (1.0 - edgeFactor()) * 0.95);
}