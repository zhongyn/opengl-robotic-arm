attribute vec3 v_coord;
varying vec4 texCoords;
uniform mat4 m, v, p;

void main()
{
	mat4 mvp = p*v*m;
	vec4 pos = mvp*vec4(v_coord, 1.0);
	gl_Position = pos;
	texCoords = vec4(v_coord, 1.0);
}
