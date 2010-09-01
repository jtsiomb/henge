varying vec3 ldir, vdir;

uniform sampler2D tex_dif, tex_norm;

void main()
{
	vec4 tdiff = texture2D(tex_dif, gl_TexCoord[0].st);
	vec3 tnorm = texture2D(tex_norm, gl_TexCoord[0].st).xyz;

	/* move normal back to [-1, 1] */
	tnorm = tnorm * 2.0 - 1.0;
	
	vec3 l = normalize(ldir);
	vec3 v = normalize(vdir);
	vec3 n = normalize(tnorm);
	vec3 h = normalize(l + v);

	vec4 diff = tdiff * max(dot(n, l), 0.0) * gl_FrontMaterial.diffuse;
	vec4 spec = pow(max(dot(n, h), 0.0), gl_FrontMaterial.shininess) * gl_FrontMaterial.specular;

	gl_FragColor = diff + spec;
}
