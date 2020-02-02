#!/usr/bin/python

import argparse
import math
import os
import sys
import struct

from PIL import Image
import numpy as np

np.set_printoptions(threshold=np.nan)



class model:

	transmittance_width  = 0
	transmittance_height = 0

	irradiance_width  = 0
	irradiance_height = 0

	spectrum_wavelengths = []   # Wavelenths in the spectrum [ 360, 370, 380, ..., 830 ]
	spectrum_irradiance  = []   # Solar irradiace per wavelength
	spectrum_final       = []   # Wavelengths for Red, Green, Blue and Alpha channels

	rayleigh_density_exp      = 0.0
	rayleigh_density_scale    = 0.0
	rayleigh_density_linear   = 0.0
	rayleigh_density_const    = 0.0
	rayleigh_scattering       = []     # Rayleigh scattering coefficients per wavelength

	mie_density_exp      = 0.0
	mie_density_scale    = 0.0
	mie_density_linear   = 0.0
	mie_density_const    = 0.0
	mie_scattering       = []     # Mie scattering coefficients per wavelength
	mie_extinction       = []     # Mie extinction coefficients per wavelength
	mie_phase            = 0.0    # Mie phase parameter

	absorption_low_density_exp    = 0.0
	absorption_low_density_scale  = 0.0
	absorption_low_density_linear = 0.0
	absorption_low_density_const  = 0.0

	absorption_high_density_exp    = 0.0
	absorption_high_density_scale  = 0.0
	absorption_high_density_linear = 0.0
	absorption_high_density_const  = 0.0

	absorption_height     = 0.0   # Altitude at which density layers switch
	absorption_extinction = []    # High-altitude absorption coefficients per wavelength

	ground_albedo = []   # Ground albedo per wavelength

	sun_angular_radius = 0.05  # Angular radius of sun
	sun_max_zenith     = 102.0 # Maximum zenith angle to precompute (102)

	planet_radius     = 0.0     # radius of planet
	atmosphere_radius = 1.0     # radius of atmosphere


#    double length_unit_in_meters, # length unit
#    unsigned int num_precomputed_wavelengths, # 3
#    bool combine_scattering_textures, # TRUE
#    bool half_precision # FALSE, use doubles


def clamp(x, a, b):
	return a if x < a else b if x > b else x



def perror(str):
	sys.stderr.write("atmotool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("atmotool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Precompute atmospheric scattering models")
	argp.add_argument('model', nargs=1)

	argv = argp.parse_args()

	return argv



def model_load(cfg):
	pass



def calculate_rayleigh_scattering(spectrum, coeff):
	return [ coeff * pow(float(el) / 1.0e-4, -4.0) for el in spectrum ]



def calculate_mie_extinction(spectrum, alpha, beta, height):
	return [ height * beta * pow(float(el) / 1.0e-4, -alpha) for el in spectrum ]



def calculate_mie_scattering(spectrum, alpha, beta, height, albedo):
	return [ albedo * height * beta * pow(float(el) / 1.0e-4, -alpha) for el in spectrum ]



def compute_transmittance(x, y, z, atmo, channel):

	INTEGRATION_SAMPLES = 500

	# Distance to top atmosphere boundary for a horizontal ray at ground level.
	H = math.sqrt(atmo.atmosphere_radius * atmo.atmosphere_radius - atmo.planet_radius * atmo.planet_radius)

	# Distance to the horizon, from which we can compute r:
	rho = H * y
	r   = math.sqrt(rho * rho + atmo.planet_radius * atmo.planet_radius)
	mu  = 1.0

	# Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	# and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
	# from which we can recover mu:
	d = (atmo.atmosphere_radius - r) + x * (rho + H - (atmo.atmosphere_radius - r))

	if abs(d) > 1.0e-3:
		mu = clamp((H * H - rho * rho - d * d) / (2.0 * r * d), -1.0, +1.0)


	det         = r * r * (mu * mu - 1.0) + atmo.atmosphere_radius * atmo.atmosphere_radius
	dist_to_top = clamp(-r * mu + math.sqrt(max(det, 0.0)), atmo.planet_radius, atmo.atmosphere_radius)
	dx          = dist_to_top / float(INTEGRATION_SAMPLES)

	dist_r = 0.0  # Total Rayleigh length
	dist_m = 0.0  # Total Mie length
	dist_a = 0.0  # Total absorption length

	for i in xrange(INTEGRATION_SAMPLES + 1):

		d_i = float(i) * dx
		r_i = math.sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r) - atmo.planet_radius  # Altitude
		w_i = 0.5 if (i == 0 or i == INTEGRATION_SAMPLES) else 1.0                    # Sample weight (from trapezoidal rule)

		yr_i = clamp(atmo.rayleigh_density_exp * math.exp(atmo.rayleigh_density_scale * r_i) + atmo.rayleigh_density_linear * r_i + atmo.rayleigh_density_const, 0.0, 1.0)
		ym_i = clamp(atmo.mie_density_exp      * math.exp(atmo.mie_density_scale      * r_i) + atmo.mie_density_linear      * r_i + atmo.mie_density_const,      0.0, 1.0)
		ya_i = 0.0

		if r_i < atmo.absorption_height:
			ya_i = clamp(atmo.absorption_low_density_exp * math.exp(atmo.absorption_low_density_scale * r_i) + atmo.absorption_low_density_linear * r_i + atmo.absorption_low_density_const, 0.0, 1.0)

		else:
			ya_i = clamp(atmo.absorption_high_density_exp * math.exp(atmo.absorption_high_density_scale * r_i) + atmo.absorption_high_density_linear * r_i + atmo.absorption_high_density_const, 0.0, 1.0)

		dist_r += yr_i * w_i * dx
		dist_m += ym_i * w_i * dx
		dist_a += ya_i * w_i * dx

	return math.exp(-(
		dist_r * atmo.rayleigh_scattering[channel] +
		dist_m * atmo.mie_extinction[channel]      +
		dist_a * atmo.absorption_extinction[channel]))



def compute_direct_irradiance(x, y, z, atmo, transmittance, channel):

	r    = atmo.planet_radius + y * (atmo.atmosphere_radius - atmo.planet_radius)
	mu_s = 2.0 * x - 1.0

	# Approximate average of the cosine factor mu_s over the visible fraction of
	# the Sun disc.
	avg_sun_cos = \
		0.0                     if mu_s < -atmo.sun_angular_radius else \
		atmo.sun_angular_radius if mu_s > +atmo.sun_angular_radius else \
		(mu_s + atmo.sun_angular_radius) * (mu_s + atmo.sun_angular_radius) / (4.0 * atmo.sun_angular_radius)

	# Distance to top atmosphere boundary for a horizontal ray at ground level.
	H = math.sqrt(atmo.atmosphere_radius * atmo.atmosphere_radius - atmo.planet_radius * atmo.planet_radius)

	# Distance to the horizon.
	rho = math.sqrt(r * r - atmo.planet_radius * atmo.planet_radius)

	# Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	# and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
	d = clamp(-r * mu_s + math.sqrt(max(0.0, r * r * (mu_s * mu_s - 1.0) + atmo.atmosphere_radius * atmo.atmosphere_radius)), -atmo.planet_radius, atmo.atmosphere_radius)

	x_mu = (d - (atmo.atmosphere_radius - r)) / ((rho + H) - (atmo.atmosphere_radius - r))
	x_r = rho / H;

	s = clamp(int(x_mu * (atmo.transmittance_width  - 1)), 0, transmittance.shape[0] - 1)
	t = clamp(int(x_r  * (atmo.transmittance_height - 1)), 0, transmittance.shape[1] - 1)

	return atmo.spectrum_irradiance[channel] * avg_sun_cos * transmittance[s,t,0]




vec2 GetTransmittanceTextureUvFromRMu(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu) {
  assert(r >= atmo.planet_radius && r <= atmo.atmosphere_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(atmo.atmosphere_radius * atmo.atmosphere_radius -
      atmo.planet_radius * atmo.planet_radius);
  // Distance to the horizon.
  Length rho =
      SafeSqrt(r * r - atmo.planet_radius * atmo.planet_radius);
  // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
  // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
  Length d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
  Length d_min = atmo.atmosphere_radius - r;
  Length d_max = rho + H;
  Number x_mu = (d - d_min) / (d_max - d_min);
  Number x_r = rho / H;
  return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}



DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu) {
  assert(r >= atmo.planet_radius && r <= atmo.atmosphere_radius);
  vec2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
  return DimensionlessSpectrum(texture(transmittance_texture, uv));
}



DimensionlessSpectrum GetTransmittance(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) {
  assert(r >= atmo.planet_radius && r <= atmo.atmosphere_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(d >= 0.0 * m);

  Length r_d  = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_d = ClampCosine((r * mu + d) / r_d);

  if (ray_r_mu_intersects_ground) {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r_d, -mu_d) /
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r, -mu),
        DimensionlessSpectrum(1.0));
  } else {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r, mu) /
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r_d, mu_d),
        DimensionlessSpectrum(1.0));
  }
}







def compute_single_scattering(x, y, z, atmo, transmittance, channel):

	# layout(location = 0) out vec3 delta_rayleigh;
	# layout(location = 1) out vec3 delta_mie;
	# layout(location = 2) out vec4 scattering;
	# layout(location = 3) out vec3 single_mie_scattering;

	# uniform mat3 luminance_from_radiance;
	# uniform sampler2D transmittance_texture;

	const vec4 SCATTERING_TEXTURE_SIZE = vec4(SCATTERING_TEXTURE_NU_SIZE - 1, SCATTERING_TEXTURE_MU_S_SIZE, SCATTERING_TEXTURE_MU_SIZE, SCATTERING_TEXTURE_R_SIZE);

	frag_coord_nu   = floor(x / SCATTERING_TEXTURE_MU_S_SIZE);
	frag_coord_mu_s = mod(x, SCATTERING_TEXTURE_MU_S_SIZE);

	# Distance to top atmosphere boundary for a horizontal ray at ground level.
	H = sqrt(atmo.atmosphere_radius * atmo.atmosphere_radius - atmo.planet_radius * atmo.planet_radius);

	# Distance to the horizon.
	rho = H * z / SCATTERING_TEXTURE_R_SIZE;
	r   = sqrt(rho * rho + atmo.planet_radius * atmo.planet_radius);

	if y / SCATTERING_TEXTURE_SIZE.z < 0.5:
		# Distance to the ground for the ray (r,mu), and its minimum and maximum
		# values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
		# we can recover mu:
		Length d_min = r - atmo.planet_radius;
		Length d_max = rho;
		Length d = d_min + (d_max - d_min) * (1.0 - 2.0 * y / SCATTERING_TEXTURE_SIZE.z);
		mu = d == 0.0 * m ? -1.0: ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = true;

	else:
		# Distance to the top atmosphere boundary for the ray (r,mu), and its
		# minimum and maximum values over all mu - obtained for (r,1) and
		# (r,mu_horizon) - from which we can recover mu:
		Length d_min = atmo.atmosphere_radius - r;
		Length d_max = rho + H;
		Length d = d_min + (d_max - d_min) * (2.0 * y / SCATTERING_TEXTURE_SIZE.z - 1.0);
		mu = d == 0.0 * m ? Number(1.0) : ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = false;

	Number x_mu_s = frag_coord_mu_s / SCATTERING_TEXTURE_SIZE.y;
	Length d_min  = atmo.atmosphere_radius - atmo.planet_radius;
	Length d_max  = H;
	Number A      = -2.0 * atmosphere.mu_s_min * atmo.planet_radius / (d_max - d_min);
	Number a      = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
	Length d      = d_min + min(a, A) * (d_max - d_min);

	mu_s = d == 0.0 * m ? Number(1.0) : ClampCosine((H * H - d * d) / (2.0 * atmo.planet_radius * d));
	nu   = ClampCosine((frag_coord_nu / SCATTERING_TEXTURE_SIZE.x) * 2.0 - 1.0);


	# Clamp nu to its valid range of values, given mu and mu_s.
	nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)), mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));

	const int SAMPLE_COUNT = 50;
	# The integration step, i.e. the length of each integration interval.
	Length dx = DistanceToNearestAtmosphereBoundary(atmosphere, r, mu, ray_r_mu_intersects_ground) / Number(SAMPLE_COUNT);


Length DistanceToNearestAtmosphereBoundary(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu, bool ray_r_mu_intersects_ground) {
  if (ray_r_mu_intersects_ground) {
    return ClampDistance(-r * mu - SafeSqrt(r * r * (mu * mu - 1.0) + atmo.planet_radius * atmo.planet_radius));
  } else {
    return ClampDistance(-r * mu + SafeSqrt(r * r * (mu * mu - 1.0) + atmo.atmosphere_radius * atmo.atmosphere_radius))
  }
}






	# Integration loop.
	DimensionlessSpectrum rayleigh_sum = DimensionlessSpectrum(0.0);
	DimensionlessSpectrum mie_sum = DimensionlessSpectrum(0.0);

	for (int i = 0; i <= SAMPLE_COUNT; ++i)
		Length d_i = Number(i) * dx;
		// The Rayleigh and Mie single scattering at the current sample point.
		DimensionlessSpectrum rayleigh_i;
		DimensionlessSpectrum mie_i;

		Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
		Number mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);

		DimensionlessSpectrum transmittance =
			GetTransmittance(     atmosphere, transmittance_texture, r, mu, d, ray_r_mu_intersects_ground) *
			GetTransmittanceToSun(atmosphere, transmittance_texture, r_d, mu_s_d);

		rayleigh_i = transmittance * GetProfileDensity(atmosphere.rayleigh_density, r_d - atmo.planet_radius);
		mie_i      = transmittance * GetProfileDensity(atmosphere.mie_density, r_d - atmo.planet_radius);

		# Sample weight (from the trapezoidal rule).
		Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_sum += rayleigh_i * weight_i;
		mie_sum += mie_i * weight_i;

	delta_rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance * atmosphere.rayleigh_scattering;
	delta_mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;

	scattering            = vec4(luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r);
	single_mie_scattering = luminance_from_radiance * delta_mie;

	return 0.0



def precompute(func, array, *args):

	z_coeff = 1.0 / float(array.shape[2] - 1) if array.shape[2] > 1 else 1.0
	y_coeff = 1.0 / float(array.shape[1] - 1) if array.shape[1] > 1 else 1.0
	x_coeff = 1.0 / float(array.shape[0] - 1) if array.shape[0] > 1 else 1.0

	for z in xrange(array.shape[2]):
		for y in xrange(array.shape[1]):
			for x in xrange(array.shape[0]):
				array[x,y,z] += func(float(x) * x_coeff, float(y) * y_coeff, float(z) * z_coeff, *args)

	return array;



atmo = model()

atmo.transmittance_width  = 256
atmo.transmittance_height = 64

atmo.irradiance_width  = 64
atmo.irradiance_height = 16

atmo.spectrum_wavelengths = [   # Wavelenths in the spectrum [ 360, 370, 380, ..., 830 ]
	360, 370, 380, 390, 400, 410, 420, 430,
	440, 450, 460, 470, 480, 490, 500, 510,
	520, 530, 540, 550, 560, 570, 580, 590,
	600, 610, 620, 630, 640, 650, 660, 670,
	680, 690, 700, 710, 720, 730, 740, 750,
	760, 770, 780, 790, 800, 810, 820, 830
]

atmo.spectrum_irradiance = [   # Solar irradiace per wavelength
	1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887,  1.61253,
	1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
	1.8685,  1.8931,  1.85149, 1.8504,  1.8341,  1.8345,  1.8147,  1.78158,
	1.7533,  1.6965,  1.68194, 1.64654, 1.6048,  1.52143, 1.55622, 1.5113,
	1.474,   1.4482,  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758,
	1.2367,  1.2082,  1.18737, 1.14683, 1.12362, 1.1058,  1.07124, 1.04992
]

atmo.spectrum_final = [   # Wavelengths for Red, Green, Blue and Alpha channels
	680, 550, 440, 370
]

atmo.rayleigh_density_exp    = +1.0
atmo.rayleigh_density_scale  = -1.0 / 8000.0
atmo.rayleigh_density_linear =  0.0
atmo.rayleigh_density_const  =  0.0
atmo.rayleigh_scattering     = calculate_rayleigh_scattering(atmo.spectrum_wavelengths, 1.24062e-6)    # Rayleigh scattering coefficients per wavelength

atmo.mie_density_exp      = +1.0
atmo.mie_density_scale    = -1.0 / 1200.0
atmo.mie_density_linear   =  0.0
atmo.mie_density_const    =  0.0
atmo.mie_scattering       = calculate_mie_scattering(atmo.spectrum_wavelengths, 0.0, 5.328e-3, 1200.0, 0.9)  # Mie scattering coefficients per wavelength
atmo.mie_extinction       = calculate_mie_extinction(atmo.spectrum_wavelengths, 0.0, 5.328e-3, 1200.0)       # Mie extinction coefficients per wavelength
atmo.mie_phase            = 0.8    # Mie phase parameter

atmo.absorption_low_density_exp    =  0.0
atmo.absorption_low_density_scale  =  0.0
atmo.absorption_low_density_linear =  1.0 / 15000.0
atmo.absorption_low_density_const  = -2.0 / 3.0

atmo.absorption_high_density_exp    = 0.0
atmo.absorption_high_density_scale  = 0.0
atmo.absorption_high_density_linear = -1.0 / 15000.0
atmo.absorption_high_density_const  = +8.0 / 3.0

atmo.absorption_height     = 25000.0
atmo.absorption_extinction = [ 300.0 * x * 2.687e20 / 15000.0 for x in [ # Create an ozone layer
	1.18e-27,  2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,  8.451e-27,
	1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26, 1.48e-25,  1.602e-25,
	2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25,   4.266e-25, 4.672e-25, 4.398e-25, 4.701e-25,
	5.019e-25, 4.305e-25, 3.74e-25,  3.215e-25, 2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25,
	1.209e-25, 9.423e-26, 7.455e-26, 6.566e-26, 5.105e-26, 4.15e-26,  4.228e-26, 3.237e-26,
	2.451e-26, 2.801e-26, 2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
]]

atmo.ground_albedo = [ 0.1 for el in atmo.spectrum_wavelengths ]   # Ground albedo per wavelength

atmo.sun_angular_radius = 0.265 * math.pi / 180.0  # Angular radius of sun
atmo.sun_max_zenith     = 120.0 * math.pi / 180.0  # Maximum zenith angle to precompute (120deg)

atmo.planet_radius     = 6360000.0     # radius of planet
atmo.atmosphere_radius = 6420000.0     # radius of atmosphere


transmittance_r = precompute(compute_transmittance, np.zeros((atmo.transmittance_width, atmo.transmittance_height, 1), dtype='float32'), atmo, 32)
#transmittance_g = precompute(compute_transmittance, np.zeros((atmo.transmittance_width, atmo.transmittance_height, 1), dtype='float32'), atmo, 19)
#transmittance_b = precompute(compute_transmittance, np.zeros((atmo.transmittance_width, atmo.transmittance_height, 1), dtype='float32'), atmo, 8)
#transmittance_a = precompute(compute_transmittance, np.zeros((atmo.transmittance_width, atmo.transmittance_height, 1), dtype='float32'), atmo, 1)

print np.min(transmittance_r), np.max(transmittance_r)
#print np.min(transmittance_g), np.max(transmittance_g)
#print np.min(transmittance_b), np.max(transmittance_b)
#print np.min(transmittance_a), np.max(transmittance_a)


direct_irradiance_r = precompute(compute_direct_irradiance, np.zeros((atmo.irradiance_width, atmo.irradiance_height, 1), dtype='float32'), atmo, transmittance_r, 32)
#direct_irradiance_g = precompute(compute_direct_irradiance, np.zeros((atmo.irradiance_width, atmo.irradiance_height, 1), dtype='float32'), atmo, transmittance_g, 19)
#direct_irradiance_b = precompute(compute_direct_irradiance, np.zeros((atmo.irradiance_width, atmo.irradiance_height, 1), dtype='float32'), atmo, transmittance_b,  8)
#direct_irradiance_a = precompute(compute_direct_irradiance, np.zeros((atmo.irradiance_width, atmo.irradiance_height, 1), dtype='float32'), atmo, transmittance_a,  1)

print np.min(direct_irradiance_r), np.max(direct_irradiance_r)
#print np.min(direct_irradiance_g), np.max(direct_irradiance_g)
#print np.min(direct_irradiance_b), np.max(direct_irradiance_b)
#print np.min(direct_irradiance_a), np.max(direct_irradiance_a)
