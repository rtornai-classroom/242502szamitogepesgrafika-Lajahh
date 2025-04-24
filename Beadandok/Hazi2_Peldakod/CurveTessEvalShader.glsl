#version 400 core
#define	HERMITE_GMT			1
#define	BEZIER_GMT			2
#define	BEZIER_BERNSTEIN	3

layout (isolines, equal_spacing, ccw) in;

uniform mat4	matModelView;
uniform mat4	matProjection;
uniform int		curveType;
uniform int		controlPointsNumber;

const mat4x4	hermite	= mat4x4( 2, -2,  1,  1,
								 -3,  3, -2, -1,
								  0,  0,  1,  0,
								  1,  0,  0,  0);

const mat4x4	bezier	= mat4x4(-1,  3, -3,  1,
								  3, -6,  3,  0,
								 -3,  3,  0,  0,
								  1,  0,  0,  0);

vec3 GMT(const mat4x3 G, const mat4x4 M, const float t) {
	mat4x3	C = G * M;
	vec4	T = vec4(t * t * t, t * t, t, 1.0f);
	vec3	P = C * T;

	return P;
}

float NCR(int n, int r) {
	if (r == 0) return 1;
	double result = 1.0f;

	for (int k = 1; k <= r; ++k) {
		result *= n - k + 1;
		result /= k;
	}

	return float(result);
}

float blending(int n, int i, float t) {
	return NCR(n, i) * pow(t, i) * pow(1.0f - t, n - i);
}

vec3 BezierCurve(float t) {
	vec3	nextPoint = vec3(0.0f, 0.0f, 0.0f);
		
	for (int i = 0; i < controlPointsNumber; i++)
		nextPoint += blending(controlPointsNumber - 1, i, t) * vec3(gl_in[i].gl_Position);

	return nextPoint;
}

void main() {

if (controlPointsNumber < 2) {
        gl_Position = matProjection * matModelView * vec4(0, 0, 0, 1);
        return;
    }
    
    float t = gl_TessCoord.x;
    vec3 result = vec3(0.0);
    int n = controlPointsNumber - 1;

	mat4x3	G = mat4x3(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), vec3(gl_in[2].gl_Position), vec3(gl_in[3].gl_Position));
	switch (curveType) {
	case HERMITE_GMT:
		gl_Position = matProjection * matModelView * vec4(GMT(G, hermite, gl_TessCoord.x), 1.0);
		break;
	case BEZIER_GMT:
		gl_Position = matProjection * matModelView * vec4(GMT(G, bezier, gl_TessCoord.x), 1.0);
		break;
	case BEZIER_BERNSTEIN:
		gl_Position = matProjection * matModelView * vec4(BezierCurve(gl_TessCoord.x), 1.0);
		break;
	}
}