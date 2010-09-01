varying vec3 ldir, vdir;
attribute vec3 attr_tangent;

void main()
{
	gl_Position = ftransform();
	vec3 vpos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vec3 normal = gl_NormalMatrix * gl_Normal;
	vec3 tangent = gl_NormalMatrix * attr_tangent;

	vec3 t = normalize(tangent);
	vec3 n = normalize(normal);
	vec3 b = cross(n, t);

	vec3 ltmp = normalize(gl_LightSource[0].position.xyz - vpos);
	ldir.x = dot(ltmp, t);
	ldir.y = dot(ltmp, b);
	ldir.z = dot(ltmp, n);
	
	vdir.x = -dot(vpos, t);
	vdir.y = -dot(vpos, b);
	vdir.z = -dot(vpos, n);

	gl_TexCoord[0] = gl_MultiTexCoord0;
}
