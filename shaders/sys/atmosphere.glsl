
#section
#ifndef _ORION_ATMOSPHERE_GLSL_H
#define _ORION_ATMOSPHERE_GLSL_H


#section .vert, .frag

const vec3 v3InvWavelengthR = vec3(5.602, 12.484, 25.731); // 1 / pow(wavelength/1000, 4) for the red(650), green(532), and blue(444) channels
const vec3 v3InvWavelengthM = vec3(1.436, 1.699, 1.978);   // 1 / pow(wavelength/1000, 0.84) for the red(650), green(532), and blue(444) channels

const float fKrESun = 0.0020 * 20.0;                       // Kr * ESun
const float fKmESun = 0.0010 * 20.0;                       // Km * ESun
const float fKr4PI  = 0.0020 * 4.0 * 3.141592653589793;    // Kr * 4 * PI
const float fKm4PI  = 0.0010 * 4.0 * 3.141592653589793;    // Km * 4 * PI

const int SAMPLES = 2;

const float RADIUS_ATMO   = 10.25; // Fiddle with these at your own peril!
const float RADIUS_PLANET = 10.0;  // Maybe calculate this as part of the incoming vertex and scale RADIUS_ATMO as needeed
	                               // The difference of angles might be subtle, but it might have a prominent effect
const float DENSITY       =  0.25;

const float SCALE_ATMO    = 1.0 / (RADIUS_ATMO - RADIUS_PLANET);
const float SCALE_DENSITY = SCALE_ATMO / DENSITY;


float atmo_opticalscale(float fCos)
{
	float x = 1.0 - fCos;
	return DENSITY * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}



void atmo_inscatter(in vec3 eye, in vec3 normal, in vec3 light, out vec3 irradiance, out vec3 extinction)
{

	const vec3 power_spectrum    = v3InvWavelengthR * fKrESun + v3InvWavelengthM * fKmESun;
	const vec3 power_attenuation = v3InvWavelengthR * fKr4PI  + v3InvWavelengthM * fKm4PI;

// Find eye-vertex intersections(near, far) at upper atmosphere
	vec3  ray  = normalize(RADIUS_PLANET * normal - eye);
	float beta = -dot(eye, ray);
	float near = beta - sqrt(max(0.0, RADIUS_ATMO * RADIUS_ATMO + beta * beta - length2(eye)));
	float far  = length(RADIUS_PLANET * normal - eye);

// Calculate optical depth parameters
	float light_scale   = atmo_opticalscale(+dot(normal, light));
	float camera_scale  = atmo_opticalscale(-dot(normal, ray));
	float camera_offset = camera_scale * exp(-1.0 / SCALE_DENSITY);
	float interval      = SCALE_ATMO * (far - near) / float(SAMPLES);

// Cast a ray from the near to the far point
	vec3 delta  = ray * interval / SCALE_ATMO;
	vec3 sample = eye + ray * near + delta * 0.5;

	irradiance = vec3(0.0);
	extinction = vec3(0.0);

	for(int n=0; n < SAMPLES; n++, sample += delta) {

		float height  = length(sample) - RADIUS_PLANET;
		float depth   = exp(-height * SCALE_DENSITY);
		float scatter = depth * (camera_scale + light_scale) - camera_offset;

		extinction  = exp(-scatter * power_attenuation);
		irradiance += extinction * depth * interval;

	}

	irradiance *= power_spectrum;

}


#section
#endif

