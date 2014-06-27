#version 430

layout (local_size_x = 1) in;

layout (binding = 0) buffer DataInBuffer {
	float dataIn[];
};

layout (binding = 1) buffer DataOutBuffer {
	float dataOut[];
};

uniform int pstage;
uniform int ppass;

void main()
{
    int i = int(gl_GlobalInvocationID.x);
	int j = int(mod(i, pstage * 2));
	float compare;
	if (j < mod(ppass, pstage) || j > 2 * pstage - mod(ppass, pstage) - 1)
	{
		compare = 0.0;
	}
	else
	{
		if (mod((j + mod(ppass, pstage)) / ppass, 2.0) < 1.0)
			compare = 1.0;
		else
			compare = -1.0;
	}

	int otherIndex = int(i + compare * ppass);
	float otherKey = dataIn[otherIndex];
	float thisKey = dataIn[i];

	dataOut[i] = thisKey * compare < otherKey * compare ? thisKey : otherKey;
}