#!/usr/bin/python

import argparse
import math
import sys
import struct
import zlib

import ConfigParser as cp
import numpy as np


GEOMETRY_MAGIC = 0x006f6567  # 'geo\0'

CHUNK_DRAWCALL = 0x77617264  # 'draw'
CHUNK_FORMI    = 0x69746D66  # 'fmti'
CHUNK_FORMV    = 0x76746D66  # 'fmtv'
CHUNK_INDICES  = 0x78646E69  # 'indx'
CHUNK_STRINGS  = 0x73727473  # 'strs'
CHUNK_TEXT     = 0x74786574  # 'text'
CHUNK_VERTICES = 0x78747276  # 'vrtx'

VF_V2 = 0x0000
VF_V3 = 0x0001
VF_T2 = 0x0002
VF_T3 = 0x0006
VF_C3 = 0x0008
VF_Q4 = 0x0010
VF_X2 = 0x0020
VF_X4 = 0x0060
VF_B2 = 0x0080
VF_B4 = 0x0180
VF_B6 = 0x0380
VF_B8 = 0x0780

IF_USHORT = 2
IF_UINT   = 4


VM   = 0    # Material
VX   = 1    # Coordinates
VY   = 2
VZ   = 3
VS   = 4    # Texture Coordinates
VT   = 5
VR   = 6    # Vertex Color
VG   = 7
VB   = 8
VIN  = 9    # Normal vector
VJN  = 10
VKN  = 11
VIT  = 12   # Tangent vector
VJT  = 13
VKT  = 14
VIB  = 15   # Binormal vector
VJB  = 16
VKB  = 17
VW0  = 18   # Skeletal animation bones and weights
VB0  = 19
VW1  = 20
VB1  = 21
VW2  = 22
VB2  = 23
VW3  = 24
VB3  = 25
VW4  = 26
VB4  = 27
VW5  = 28
VB5  = 29
VW6  = 30
VB6  = 31
VW7  = 32
VB7  = 33
VNUM = 34   # Total vertex attributes



#
# Type conversions
#
def pack_fp16(v):

	scale_to_inf  = struct.unpack("=f", struct.pack("=I", 0x77800000))[0]
	scale_to_zero = struct.unpack("=f", struct.pack("=I", 0x08800000))[0]
	base          = (abs(v) * scale_to_inf) * scale_to_zero

	w      = struct.unpack("=I", struct.pack("=f", v))[0]
	shl1_w = w + w
	sign   = w      & 0x80000000
	bias   = shl1_w & 0xff000000

	if bias < 0x71000000:
		bias = 0x71000000

	base += struct.unpack("=f", struct.pack("=I", (bias >> 1) + 0x07800000))[0]

	bits          = struct.unpack("=I", struct.pack("=f", base))[0]
	exp_bits      = (bits >> 13) & 0x00007c00
	mantissa_bits = bits & 0x00000fff
	nonsign       = exp_bits + mantissa_bits

	return (sign >> 16) | (0x7e00 if shl1_w > 0xff000000 else nonsign)


def pack_r11g11b10f(r, g, b):
	return ((pack_fp16(r) >> 4) & 0x000007ff) | ((pack_fp16(g) << 7) & 0x003ff800) | ((pack_fp16(b) << 17) & 0xffc00000)


def pack_unorm16(v):
	return int(min(max(v * 65535, 0), 65535))


def pack_snorm16(v):
	return int(min(max(v * 32767.5 * x - 0.5, -32768), 32767))


def pack_uint16(v):
	return int(min(max(v, 0), 65535))


def pack_sint16(v):
	return int(min(max(v, -32768), 32767))


def pack_urange16(v, a, b):
	return int(min(max(65535 * (v - a) / (b - a), 0), 65535))


def pack_srange16(v, a, b):
	return int(min(max(65535 * (v - a) / (b - a) - 32768, -32768), 32767))


def pack_r10g10b10a2_unorm(r, g, b, a):
	r = (int(min(max(r * 1023, 0), 1023)) <<  0) & 0x000003ff
	g = (int(min(max(g * 1023, 0), 1023)) << 10) & 0x000ffc00
	b = (int(min(max(b * 1023, 0), 1023)) << 20) & 0x3ff00000
	a = (int(min(max(a * 3,    0), 3))    << 30) & 0xc0000000
	return r | g | b | a


def pack_r10g10b10a2_snorm(r, g, b, a):
	r = (int(min(max(r * 511.5 - 0.5, -512), 511)) <<  0) & 0x000003ff
	g = (int(min(max(g * 511.5 - 0.5, -512), 511)) << 10) & 0x000ffc00
	b = (int(min(max(b * 511.5 - 0.5, -512), 511)) << 20) & 0x3ff00000
	a = (int(min(max(a * 1.5   - 0.5, -2),   1))   << 30) & 0xc0000000
	return r | g | b | a


def pack_r10g10b10a2_uint(r, g, b, a):
	r = (int(min(max(r, 0), 1023)) <<  0) & 0x000003ff
	g = (int(min(max(g, 0), 1023)) << 10) & 0x000ffc00
	b = (int(min(max(b, 0), 1023)) << 20) & 0x3ff00000
	a = (int(min(max(a, 0), 3))    << 30) & 0xc0000000
	return r | g | b | a


def pack_r10g10b10a2_sint(r, g, b, a):
	r = (int(min(max(r, -512), 511)) <<  0) & 0x000003ff
	g = (int(min(max(g, -512), 511)) << 10) & 0x000ffc00
	b = (int(min(max(b, -512), 511)) << 20) & 0x3ff00000
	a = (int(min(max(a, -2), 1))     << 30) & 0xc0000000
	return r | g | b | a



def mat4_to_q(m00, m01, m02, m10, m11, m12, m20, m21, m22):

	qx   = 0.0
	qy   = 0.0
	qz   = 0.0
	qw   = 1.0
	sign = 1.0

	hand = (m00 * (m11 * m22 - m12 * m21) -
		m01 * (m10 * m22 - m12 * m20) +
		m02 * (m10 * m21 - m11 * m20))

	if hand < 0.0:
		m20  = -m20; m21 = -m21; m22 = -m22
		hand = -1.0

	trace = m00 + m11 + m22

	if trace > 0.0:
		sigma = math.sqrt(trace + 1.0) * 2.0
		qx = (m21 - m12) / sigma
		qy = (m02 - m20) / sigma
		qz = (m10 - m01) / sigma
		qw = sigma / 4.0

	elif (m00 > m11) and (m00 > m22):
		sigma = math.sqrt(1.0 + m00 - m11 - m22) * 2.0
		qx = sigma / 4.0
		qy = (m01 + m10) / sigma
		qz = (m02 + m20) / sigma
		qw = (m21 - m12) / sigma

	elif m11 > m22:
		sigma = math.sqrt(1.0 + m11 - m00 - m22) * 2.0
		qx = (m01 + m10) / sigma
		qy = sigma / 4.0
		qz = (m12 + m21) / sigma
		qw = (m02 - m20) / sigma
	else:
		sigma = math.sqrt(1.0 + m22 - m00 - m11) * 2.0
		qx = (m02 + m20) / sigma
		qy = (m12 + m21) / sigma
		qz = sigma / 4.0
		qw = (m10 - m01) / sigma

	if qw < 0.0:
		qx = -qx; qy = -qy; qz = -qz; qw = -qw

	return qx, qy, qz, qw, hand



def perror(str):
	sys.stderr.write("geotool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str, newline=True):
	sys.stderr.write("geotool: ")
	sys.stderr.write(str)
	if newline: sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Convert binary FBX to Hades Geometry")
	argp.add_argument('-o',  '--output',          type=str,            help='Set output file')
	argp.add_argument('-vt', '--texcoords',       action='store_true', help='Export texture coordinates')
	argp.add_argument('-vc', '--colors',          action='store_true', help='Export vertex colors')
	argp.add_argument('-vn', '--tangents',        action='store_true', help='Export tangent space')
	argp.add_argument('-vs', '--shader',          type=str,            help='Set default shader ("shader::default")')
	argp.add_argument('-mp', '--materials',       type=str,            help='Override materials with this file')
	argp.add_argument('-mx', '--material-export', type=str,            help='Export materials to this file')
	argp.add_argument('model')

	argv = argp.parse_args()

	if not argv.output:
		argv.output = ""

	if not argv.shader:
		argv.shader = "shader::default"

	return argv



def fbx_open(model):

	def packread(f, s):
		return struct.unpack(s, f.read(struct.calcsize(s)))

	try:
		fbx = open(model, 'rb')

	except:
		return None

	magic, reserved, version = packread(fbx, "=20sxHI")

	if magic   != "Kaydara FBX Binary  ": perror("File is not a binary FBX!")
	if version != 7400: pwarn("Unknown version %d, some things might not work" % version)

	return fbx



def fbx_read_node(fbx):

	def packread(s):
		return struct.unpack(s, fbx.read(struct.calcsize(s)))

	end_offset, num_props, props_sz, name_len = packread("=IIIB")

#
# I have no idea what the hell this is
# but it always appears after the last
# chunk and fucks up my shitty parser
#
	if end_offset == 0x9abbcfa and num_props == 0x66d4c8d0:
		return 'EOF', [], -1


	name       = packread("=%ds" % name_len)[0]
	properties = []

	for n in xrange(num_props):

		key = packread("=c")[0]
		val = None

		if   key == 'Y': val = packread("=h")[0]
		elif key == 'C': val = packread("=?")[0]
		elif key == 'I': val = packread("=i")[0]
		elif key == 'F': val = packread("=f")[0]
		elif key == 'D': val = packread("=d")[0]
		elif key == 'L': val = packread("=q")[0]
		elif key == 'S': val = packread("=%ds" % packread("=I")[0])[0]
		elif key == 'R': val = packread("=%dp" % packread("=I")[0])[0]

		elif key == 'f' or key == 'd' or key == 'l' or key == 'i' or key == 'b':

			arr_num, arr_enc, arr_sz = packread("=III")
			arr_buf                  = packread("=%ds" % arr_sz)[0]

			if   arr_enc == 0: pass
			elif arr_enc == 1: arr_buf = zlib.decompress(arr_buf)
			else:
				perror("Unknown array compression!")

			if   key == 'f': val = struct.unpack("=%df" % arr_num, arr_buf)
			elif key == 'd': val = struct.unpack("=%dd" % arr_num, arr_buf)
			elif key == 'l': val = struct.unpack("=%dl" % arr_num, arr_buf)
			elif key == 'i': val = struct.unpack("=%di" % arr_num, arr_buf)
			elif key == 'b': val = struct.unpack("=%d?" % arr_num, arr_buf)
			else:
				perror("HE HAD TO SPLIT")

		else:
			perror("Unknown property key '%s'" % key)

		properties.append(val)

	return name, properties, end_offset



def geometry_begin():

	geo = {
		'header':    {},
		'meshes':    [],
		'materials': [],
		'textures':  [],
		'vertices':  [],
		'indices':   [],
		'drawcalls': [],
		'text':      []
	}

	return geo



def geometry_load_fbx(geo, fbx):


	def skip_node(end):
		while fbx.tell() < end:
			name, props, endptr = fbx_read_node(fbx)


	def process_header(props, end):

		header = {
			'version': 0,
			'year':    0, 'month':  0, 'day':    0,
			'hour':    0, 'minute': 0, 'second': 0, 'millisecond': 0,
			'title':   "", 'author': "", 'comment': "", 'creator': ""
		}

		while fbx.tell() < end:

			name, props, endptr = fbx_read_node(fbx)

			if   name == 'FBXVersion':  header['version']     = props[0]
			elif name == 'Year':        header['year']        = props[0]
			elif name == 'Month':       header['month']       = props[0]
			elif name == 'Day':         header['day']         = props[0]
			elif name == 'Hour':        header['year']        = props[0]
			elif name == 'Minute':      header['minute']      = props[0]
			elif name == 'Second':      header['second']      = props[0]
			elif name == 'Millisecond': header['millisecond'] = props[0]
			elif name == 'Title':       header['title']   = props[0]
			elif name == 'Author':      header['author']  = props[0]
			elif name == 'Comment':     header['comment'] = props[0]
			elif name == 'Creator':     header['creator'] = props[0]

		geo['header'] = header


	def process_geometry(props, end):

		meshname = props[1].split("\x00\x01")[0]

		vertices  = None; indices     = None
		texcoords = None; indices_tex = None
		colors    = None; indices_col = None
		normals   = None; indices_nrm = None
		tangents  = None; indices_tan = None
		binormals = None; indices_bin = None
		bones     = None
		materials = None
		layer     = "default"
		layer_map = {}
		layer_ref = {}

		pwarn("Processing geometry for '%s'..." % meshname)

		while fbx.tell() < end:

			name, props, endptr = fbx_read_node(fbx)

			if   name == 'Vertices':           vertices    = props[0]
			elif name == 'UV':                 texcoords   = props[0]
			elif name == 'Normals':            normals     = props[0]
			elif name == 'Tangents':           tangents    = props[0]
			elif name == 'Binormals':          binormals   = props[0]
			elif name == 'Materials':          materials   = props[0]
			elif name == 'PolygonVertexIndex': indices     = props[0]
			elif name == 'UVIndex':            indices_tex = props[0]
			elif name == 'LayerElementNormal':   layer = 'normal'
			elif name == 'LayerElementBinormal': layer = 'binormal'
			elif name == 'LayerElementTangent':  layer = 'tangent'
			elif name == 'LayerElementUV':       layer = 'texcoord'
			elif name == 'LayerElementMaterial': layer = 'material'
			elif name == 'MappingInformationType':   layer_map[layer] = props[0]
			elif name == 'ReferenceInformationType': layer_ref[layer] = props[0]

		mesh = np.zeros((2 * len(indices), VNUM), dtype=np.float64)

		i    = 0
		k    = 0
		poly = 0

		while i < len(indices): # Decode indices and split polygons into triangles

			num = 0
			while indices[i + num] >= 0:
				num += 1

			if   num == 2: tesselation = [ 0, 2, 1 ]
			elif num == 3: tesselation = [ 0, 2, 1, 0, 3, 2 ]
			else:          pwarn("%d-gon detected, skipping!" % (num + 1))

			for vi in tesselation:

				index = indices[i + vi]
				if index < 0: index = -index - 1

				if materials:
					midx = 0
					if   layer_map['material'] == 'AllSame':         midx = 0
					elif layer_map['material'] == 'ByPolygon':       midx = poly
					elif layer_map['material'] == 'ByVertex':        midx = index
					elif layer_map['material'] == 'ByPolygonVertex': midx = i + vi
					if   layer_ref['material'] == 'Direct':          pass
					elif layer_ref['material'] == 'IndexToDirect':   midx = materials[midx]
					mesh[k,VM] = midx

				mesh[k,VX] = vertices[3 * index + 0]
				mesh[k,VY] = vertices[3 * index + 1]
				mesh[k,VZ] = vertices[3 * index + 2]

				if texcoords:
					uidx = -1
					if   layer_map['texcoord'] == 'AllSame':         uidx = 0
					elif layer_map['texcoord'] == 'ByPolygon':       uidx = poly
					elif layer_map['texcoord'] == 'ByVertex':        uidx = index
					elif layer_map['texcoord'] == 'ByPolygonVertex': uidx = i + vi
					if   layer_ref['texcoord'] == 'Direct':          pass
					elif layer_ref['texcoord'] == 'IndexToDirect':   uidx = indices_tex[uidx]
					if uidx >= 0:
						mesh[k,VS] = texcoords[2 * uidx + 0]
						mesh[k,VT] = texcoords[2 * uidx + 1]


				if normals:
					nidx = -1
					if   layer_map['normal'] == 'AllSame':         nidx = 0
					elif layer_map['normal'] == 'ByPolygon':       nidx = poly
					elif layer_map['normal'] == 'ByVertex':        nidx = index
					elif layer_map['normal'] == 'ByPolygonVertex': nidx = i + vi
					if   layer_ref['normal'] == 'Direct':          pass
					elif layer_ref['normal'] == 'IndexToDirect':   nidx = indices_nrm[nidx]
					if nidx >= 0:
						mesh[k,VIN] = normals[3 * nidx + 0]
						mesh[k,VJN] = normals[3 * nidx + 1]
						mesh[k,VKN] = normals[3 * nidx + 2]

				if tangents:
					tidx = -1
					if   layer_map['tangent'] == 'AllSame':         tidx = 0
					elif layer_map['tangent'] == 'ByPolygon':       tidx = poly
					elif layer_map['tangent'] == 'ByVertex':        tidx = index
					elif layer_map['tangent'] == 'ByPolygonVertex': tidx = i + vi
					if   layer_ref['tangent'] == 'Direct':          pass
					elif layer_ref['tangent'] == 'IndexToDirect':   tidx = indices_tan[tidx]
					if tidx >= 0:
						mesh[k,VIT] = tangents[3 * tidx + 0]
						mesh[k,VJT] = tangents[3 * tidx + 1]
						mesh[k,VKT] = tangents[3 * tidx + 2]

				if binormals:
					bidx = -1
					if   layer_map['binormal'] == 'AllSame':         bidx = 0
					elif layer_map['binormal'] == 'ByPolygon':       bidx = poly
					elif layer_map['binormal'] == 'ByVertex':        bidx = index
					elif layer_map['binormal'] == 'ByPolygonVertex': bidx = i + vi
					if   layer_ref['binormal'] == 'Direct':          pass
					elif layer_ref['binormal'] == 'IndexToDirect':   bidx = indices_bin[bidx]
					if bidx >= 0:
						mesh[k,VIB] = binormals[3 * bidx + 0]
						mesh[k,VJB] = binormals[3 * bidx + 1]
						mesh[k,VKB] = binormals[3 * bidx + 2]

				k += 1

			if poly % 1009 == 0:
				pwarn("Populating mesh... %d\r" % poly, False)

			i    += num + 1
			poly += 1

		pwarn("Populating mesh... Done          ")

		flags = []

		if texcoords: flags.append('has-texcoords')
		if colors:    flags.append('has-colors')
		if normals:   flags.append('has-normals')
		if tangents:  flags.append('has-tangents')
		if binormals: flags.append('has-binormals')

		geo['meshes'].append([ meshname, mesh[:k,], flags ])


	def process_material(props, end):

		mat = {
		# Exported by Blender
			'name':          props[1].split("\x00\x01")[0],
			'shading_model': None,
			'ambient':       [ 0.0, 0.0, 0.0 ],
			'diffuse':       [ 0.0, 0.0, 0.0 ],
			'emissive':      [ 0.0, 0.0, 0.0 ],
			'reflection':    [ 0.0, 0.0, 0.0 ],
			'specular':      [ 0.0, 0.0, 0.0 ],
			'transparency':  [ 0.0, 0.0, 0.0 ],
			'ambient_factor':    0.0,
			'diffuse_factor':    0.0,
			'emissive_factor':   0.0,
			'reflection_factor': 0.0,
			'specular_factor':   0.0,
			'shininess':         0.0,
			'shininess_exp':     0.0,
		# Used by the engine
			'shader':      "default",
			'albedo':      [ 0.0, 0.0, 0.0 ],
			'metallicity': 0.0,
			'roughness':   0.0,
			'refraction':  1.0,
		}

		while fbx.tell() < end:

			name, props, endptr = fbx_read_node(fbx)

			if   name == 'ShadingModel': mat['shading_model'] = props[0]
			elif name == 'P':
				if   props[0] == 'AmbientColor':      mat['ambient']      = props[4:7]
				elif props[0] == 'DiffuseColor':      mat['diffuse']      = props[4:7]
				elif props[0] == 'EmissiveColor':     mat['emissive']     = props[4:7]
				elif props[0] == 'ReflectionColor':   mat['reflection']   = props[4:7]
				elif props[0] == 'SpecularColor':     mat['specular']     = props[4:7]
				elif props[0] == 'TransparentColor':  mat['transparency'] = props[4:7]
				elif props[0] == 'AmbientFactor':     mat['ambient_factor']    = props[4]
				elif props[0] == 'DiffuseFactor':     mat['diffuse_factor']    = props[4]
				elif props[0] == 'EmissiveFactor':    mat['emissive_factor']   = props[4]
				elif props[0] == 'ReflectionFactor':  mat['reflection_factor'] = props[4]
				elif props[0] == 'SpecularFactor':    mat['specular_factor']   = props[4]
				elif props[0] == 'Shininess':         mat['shininess']     = props[4]
				elif props[0] == 'ShininessExponent': mat['shininess_exp'] = props[4]

				elif props[0][0:7] == 'shader:':      mat['shader']          = props[0][7:].strip()
				elif props[0]      == 'metallicity':  mat['metallicity']     = props[4]
				elif props[0]      == 'roughness':    mat['roughness']       = props[4]
				elif props[0]      == 'transparency': mat['transparency'][0] = props[4]
				elif props[0]      == 'refraction':   mat['refraction']      = props[4]


		geo['materials'].append(mat)


	def process_texture(props, end):

		tex = {
			'name': props[1].split("\x00\x01")[0],
			'file': ""
		}

		while fbx.tell() < end:

			name, props, endptr = fbx_read_node(fbx)

			if name == 'FileName':
				tex['file'] = props[0]

		geo['textures'].append(tex)


	def process_objects(props, end):

		while fbx.tell() < end:

			name, props, endptr = fbx_read_node(fbx)

			if   name == 'Geometry':      process_geometry(props, endptr)
			elif name == 'Material':      process_material(props, endptr)
			elif name == 'Texture':       process_texture( props, endptr)
			elif name == 'Video':         skip_node(endptr)
			elif name == 'Model':         skip_node(endptr)
			elif name == 'NodeAttribute': skip_node(endptr)
			elif not name: continue
			else:
				perror("Unknown node %s" % name)


	while True:

		name, props, end = fbx_read_node(fbx)

		if   name == 'FBXHeaderExtension': process_header(props, end)
		elif name == 'Objects':            process_objects(props, end)
		elif name == 'FileId':             skip_node(end)
		elif name == 'CreationTime':       skip_node(end)
		elif name == 'Creator':            skip_node(end)
		elif name == 'GlobalSettings':     skip_node(end)
		elif name == 'Documents':          skip_node(end)
		elif name == 'References':         skip_node(end)
		elif name == 'Definitions':        skip_node(end)
		elif name == 'Connections':        skip_node(end)
		elif name == 'Takes':              skip_node(end)
		elif name == 'EOF':                break
		elif not name: continue
		else:
			perror("Unknown node %s" % name)



def geometry_patch_materials(geo, shader, conf):

	for mat in geo['materials']:

		if mat['shader'] == 'default':
			mat['shader'] = shader

		mat['albedo'] = mat['diffuse']

	if not conf:
		return

	pwarn("Patching materials from %s..." % conf)

	ini = cp.SafeConfigParser()
	ini.read(conf)

	for name in ini.sections():

		mat = next((m for m in geo['materials'] if m['name'] == name), None)

		if not mat:
			pwarn("Unknown material %s, skipping..." % name)
			continue

		for arg, val in ini.items(name):
			if   arg == 'shader':   mat['shader'] = (lambda s: s[1:-1] if s[0] == s[-1] and s.startswith(("'", "\"")) else s)(str(val))
			elif arg == 'albedo_r': mat['albedo'][0] = float(val)
			elif arg == 'albedo_g': mat['albedo'][1] = float(val)
			elif arg == 'albedo_b': mat['albedo'][2] = float(val)
			elif arg == 'emissive_r': mat['emissive'][0] = float(val)
			elif arg == 'emissive_g': mat['emissive'][1] = float(val)
			elif arg == 'emissive_b': mat['emissive'][2] = float(val)
			elif arg == 'metallicity':  mat['metallicity']     = float(val)
			elif arg == 'roughness':    mat['roughness']       = float(val)
			elif arg == 'transparency': mat['transparency'][0] = float(val)
			elif arg == 'refraction':   mat['refraction']      = float(val)



def geometry_export_materials(geo, conf):

	pwarn("Exporting materials to %s..." % conf)

	try:
		with open(conf, 'w') as ini:

			ini.write("\n")
			ini.write("\n")
			ini.write(";\n")
			ini.write("; Material definitions\n")
			ini.write(";\n")

			for mat in geo['materials']:

				ini.write("\n")
				ini.write("\n")
				ini.write("[%s]\n" % mat['name'])
				ini.write("shader = \"%s\"\n" % mat['shader'])
				ini.write("albedo_r = %.3f\n" % mat['albedo'][0])
				ini.write("albedo_g = %.3f\n" % mat['albedo'][1])
				ini.write("albedo_b = %.3f\n" % mat['albedo'][2])
				ini.write("emissive_r = %.3f\n" % mat['emissive'][0])
				ini.write("emissive_g = %.3f\n" % mat['emissive'][1])
				ini.write("emissive_b = %.3f\n" % mat['emissive'][2])
				ini.write("metallicity  = %.3f\n" % mat['metallicity'])
				ini.write("roughness    = %.3f\n" % mat['roughness'])
				ini.write("transparency = %.3f\n" % mat['transparency'][0])
				ini.write("refraction   = %.3f\n" % mat['refraction'])

			ini.write("\n")

	except:
		perror("Failed")

	pwarn("Done")



def geometry_encode(geo, do_texcoords, do_colors, do_tangents, do_bones):

	vertices  = []
	indices   = []
	drawcalls = []
	vform     = VF_V3

	if do_texcoords: vform |= VF_T2
	if do_colors:    vform |= VF_C3
	if do_tangents:  vform |= VF_Q4

	for meshname, mesh, flags in geo['meshes']:

		pwarn("Encoding mesh %s..." % meshname)

		min_x = mesh[:,VX].min(); max_x = mesh[:,VX].max();
		min_y = mesh[:,VY].min(); max_y = mesh[:,VY].max();
		min_z = mesh[:,VZ].min(); max_z = mesh[:,VZ].max();

		if do_texcoords and not 'has-texcoords' in flags: pwarn("warning: Mesh is missing texcoords!")
		if do_colors    and not 'has-colors'    in flags: pwarn("warning: Mesh is missing vertex colors!")
		if do_tangents  and not 'has-normals'   in flags: pwarn("warning: Mesh is missing normal vectors!")
		if do_tangents  and not 'has-tangents'  in flags: pwarn("warning: Mesh is missing tangent vectors!")
		if do_tangents  and not 'has-binormals' in flags: pwarn("warning: Mesh is missing binormal vectors!")

		dcindex = [ [] for n in xrange(len(geo['materials'])) ]
		vdct    = {}

		for k in xrange(mesh.shape[0]):

			vertex = b''

			vertex += struct.pack("=HHHH",
				pack_urange16(mesh[k,VX], min_x, max_x),
				pack_urange16(mesh[k,VY], min_y, max_y),
				pack_urange16(mesh[k,VZ], min_z, max_z),
				65535)

			if do_texcoords:
				vertex += struct.pack("=HH",
					pack_unorm16(mesh[k,VS]),
					pack_unorm16(mesh[k,VT]))

			if do_colors:
				vertex += struct.pack("=I",
					pack_r11g11b10f(mesh[k,VR], mesh[k,VG], mesh[k,VB]))

			if do_tangents:
				qx, qy, qz, qw, sgn = mat4_to_q(
					mesh[k,VIT], mesh[k,VJT], mesh[k,VKT],
					mesh[k,VIB], mesh[k,VJB], mesh[k,VKB],
					mesh[k,VIN], mesh[k,VJN], mesh[k,VKN])
				vertex += struct.pack("=I", pack_r10g10b10a2_snorm(qx, qy, qz, sgn))

			index = vdct.get(vertex, None)

			if index is None:
				index = len(vertices)
				vertices.append(vertex)
				vdct[vertex] = index

			dcindex[int(mesh[k,VM])].append(index)

			if k % 1009 == 0:
				pwarn("Compactifying %d vertices... %5.2f%%\r" % (mesh.shape[0], 100.0 * k / mesh.shape[0]), False)

		pwarn("Compactifying %d vertices... Done        " % mesh.shape[0])
		pwarn("Mesh now has %d vertices" % len(vertices))

		start = 0
		num   = 0

		for dc in dcindex:
			count = len(dc)
			if count > 0:
				indices.extend(dc)
				drawcalls.append([
					num,
					meshname + "/" + geo['materials'][num]['name'],
					start, count,
					[ min_x, max_x ],
					[ min_y, max_y ],
					[ min_z, max_z ] ])
				start += count
			num += 1

		pwarn("Mesh has %d drawcalls with %d total triangles" % (len(drawcalls), len(indices) / 3))

	geo['vertices']  = vertices
	geo['indices']   = indices
	geo['drawcalls'] = drawcalls
	geo['formi']     = IF_UINT if max(indices) > 65535 else IF_USHORT
	geo['formv']     = vform



def geometry_add_text(geo, key, text):
	geo['text'].append([ str(key), str(text) ])



def geometry_export_binary(geo, output):

	chunks = []

	class strtable:
		index   = {}
		strings = ''


	def mkstr(text):
		try:
			return strtable.index[text]
		except:
			index             = len(strtable.strings)
			strtable.strings += text + "\x00"
			strtable.index[text] = index
			return index


	def generate_strings():
		if len(strtable.strings) > 0:
			chunks.insert(0, [ CHUNK_STRINGS, strtable.strings ])


	def generate_formats():
		chunks.append([ CHUNK_FORMI, struct.pack("=I", geo['formi']) ])
		chunks.append([ CHUNK_FORMV, struct.pack("=I", geo['formv']) ])


	def generate_drawcalls():

		drawcalls = ''

		for mat_no, name, start, count, range_x, range_y, range_z in geo['drawcalls']:

			mat = geo['materials'][mat_no]

			drawcalls += struct.pack('@4I40f',
				mkstr(name),                                                      # u32 drawcall_name
				mkstr(mat['shader']),                                             # u32 drawcall_shader
				start, count,
				range_x[0], range_y[0], range_z[0], 1.0,                          # f32 minimum[4]
				range_x[1], range_y[1], range_z[1], 1.0,                          # f32 maximum[4]
				mat['albedo'][0],   mat['albedo'][1],   mat['albedo'][2],   0.0,  # f32 albedo[4]
				mat['emissive'][0], mat['emissive'][1], mat['emissive'][2], 0.0,  # f32 emissive[4]
				mat['transparency'][0],                                           # f32 transparency
				mat['metallicity'],                                               # f32 metallicity
				mat['roughness'],                                                 # f32 roughness
				mat['refraction'],                                                # f32 index_of_refraction
				0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,                           # f32 drawcall_extra_args[20]
				0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0)

		if len(drawcalls) > 0:
			chunks.append([ CHUNK_DRAWCALL, drawcalls ])



	def generate_text():
		for key, val in sorted(geo['text']):
			chunks.append([ CHUNK_TEXT, struct.pack('=II', mkstr(key), mkstr(val)) ])


	def generate_vertices():
		chunks.append([ CHUNK_VERTICES, b''.join(geo['vertices']) ])


	def generate_indices():
		if geo['formi'] == IF_UINT:
			chunks.append([ CHUNK_INDICES, struct.pack('=%dI' % len(geo['indices']), *geo['indices']) ])

		else:
			chunks.append([ CHUNK_INDICES, struct.pack('=%dH' % len(geo['indices']), *geo['indices']) ])


	pwarn("Writing geometry to %s..." % output)

	try:
		with open(output, 'wb') as out:

			generate_drawcalls()
			generate_formats()
			generate_vertices()
			generate_indices()
			generate_text()
			generate_strings()

			header = struct.pack('@2I', GEOMETRY_MAGIC, len(chunks))
			offset = 8 + 16 * len(chunks)

			for magic, chunk in chunks:

				while offset % 16 > 0:
					offset += 1

				length  = len(chunk)
				header += struct.pack('@4I', magic, offset, length, 0)
				offset += length

			out.write(header)

			for magic, chunk in chunks:

				while out.tell() % 16 > 0:
					out.write(b'\x00')

				out.write(chunk)

	except:
		perror("Failed")

	pwarn("Done")



argv   = parse_command_line()
output = argv.output

if not output:
	output = (argv.model[:argv.model.rindex('.')] if '.' in argv.model else argv.model) + ".geo"

with fbx_open(argv.model) as fbx:

	if not fbx:
		perror("Failed to open %s" % argv.model)

	geo = geometry_begin()

	geometry_load_fbx(geo, fbx)
	geometry_encode(geo, argv.texcoords, argv.colors, argv.tangents, False)
	geometry_patch_materials(geo, argv.shader, argv.materials)

	if argv.material_export:
		geometry_export_materials(geo, argv.material_export)

	geometry_add_text(geo, 'creator', geo['header']['creator'])

	geometry_export_binary(geo, output)

