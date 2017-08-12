
#section
#ifndef _ORION_PLASMA_GLSL_H
#define _ORION_PLASMA_GLSL_H


#section .vert, .frag

float plasma(
		vec3  point,
		float time,
		vec4  wave_Ax, vec4 wave_Ay, vec4 wave_Az,
		vec4  wave_Bx, vec4 wave_By, vec4 wave_Bz,
		vec4  wave_C,
		vec4  wave_D)
{

	float v = 0.0;

	v += wave_Ax.x * cos(wave_Ax.y * point.x + wave_Ax.z * time + wave_Ax.w);
	v += wave_Ay.x * cos(wave_Ay.y * point.y + wave_Ay.z * time + wave_Ay.w);
	v += wave_Az.x * cos(wave_Az.y * point.z + wave_Az.z * time + wave_Az.w);

	vec4 phase = vec4(
		wave_Bx.x * point.x + wave_Bx.y * (0.5 * cos(wave_Bx.z * time + wave_Bx.w) - 0.5),
		wave_By.x * point.y + wave_By.y * (0.5 * cos(wave_By.z * time + wave_By.w) - 0.5),
		wave_Bz.x * point.z + wave_Bz.y * (0.5 * cos(wave_Bz.z * time + wave_Bz.w) - 0.5),
		1.0);

	v += wave_C.x * cos(wave_C.y * length(phase) + wave_C.z * time + wave_C.w);

	return cos(wave_D.y * v + wave_D.w);

}


#section
#endif

