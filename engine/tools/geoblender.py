#!/usr/bin/python

import math
import os
import sys
import hashlib
import heapq
import struct

import bpy
import bpy.props as bprop
import bpy_extras.io_utils as bpxio
import mathutils


bl_info = {
	"name":        "Export Orion Core geometry(.geo)",
	"author":      "ycwn",
	"version":     (0, 1),
	"blender":     (2, 74, 0),
	"api":         36079,
	"location":    "File > Export > Orion Core geometry (.geo)",
	"description": "Export Orion Core geometry",
	"warning":     "",
	"wiki_url":    "",
	"tracker_url": "",
	"category":    "Import-Export"
}


GEOMETRY_MAGIC = 0x006f6567  # 'geo\0'

CHUNK_ANIMATION = 0x6d696e61  # 'anim'
CHUNK_BOUNDBOX  = 0x786f6262  # 'bbox'
CHUNK_BONES     = 0x656e6f62  # 'bone'
CHUNK_FRAGMENT  = 0x67617266  # 'frag'
CHUNK_GRAFT     = 0x74667267  # 'grft'
CHUNK_LOD       = 0x646f6c68  # 'hlod'
CHUNK_INDICES   = 0x78646e69  # 'indx'
CHUNK_STRINGS   = 0x73727473  # 'strs'
CHUNK_TEXTURE   = 0x72747874  # 'txtr'
CHUNK_TEXT      = 0x74786574  # 'text'
CHUNK_VERTICES  = 0x74726576  # 'vert'

TEXTURE_MODE_COLORMAP  = 0x61626772  # 'rgba'
TEXTURE_MODE_NORMALMAP = 0x6d726f6e  # 'norm'
TEXTURE_MODE_GLOSSMAP  = 0x736f6c67  # 'glos'

VF_V2  = 0x0000
VF_V3  = 0x0001
VF_T2  = 0x0002
VF_T3  = 0x0006
VF_C4  = 0x0008
VF_N3  = 0x0010
VF_T4  = 0x0020
VF_X1  = 0x0040
VF_X2  = 0x00c0
VF_X3  = 0x01c0
VF_X4  = 0x03c0
VF_B4  = 0x0400
VF_B8  = 0x0c00
VF_B12 = 0x1c00
VF_B16 = 0x3c00

GL_LINES     = 0x01
GL_TRIANGLES = 0x04

GL_UNSIGNED_SHORT = 0x1403
GL_UNSIGNED_INT   = 0x1405


#
# Process a mesh object in the scene
#
def geo_process_mesh(obj, do_apply_mods, do_swap_yz, do_modelview, do_texcoords, do_colors, do_normals, do_tangents, do_bones):

	try:
		mesh = obj.to_mesh(bpy.context.scene, do_apply_mods, 'PREVIEW')

	except:
		return None

	if do_modelview:
		mesh.transform(obj.matrix_world)

	if do_normals:
		mesh.calc_normals_split() # This has to be done before everything else, or weird shit happens

	if do_tangents and mesh.uv_layers.active:
		mesh.calc_tangents() # This has to be done before everything else, or weird shit happens

	geo = {}
	geo['fragments'] = []
	geo['vertices']  = []
	geo['indices']   = []
	geo['bones']     = []
	geo['bonemap']   = {}
	geo['boxes']     = []
	geo['grafts']    = []
	geo['materials'] = {}
	geo['strings']   = {}
	geo['vformat']   = None
	geo['vlen']      = 0

	matgroups = {}
	loops     = mesh.loops
	verts     = mesh.vertices
	texcoords = mesh.uv_layers.active.data     if len(mesh.uv_layers)     > 0 else None
	colors    = mesh.vertex_colors.active.data if len(mesh.vertex_colors) > 0 else None
	bones = {}

	if not texcoords: do_texcoords = False
	if not colors:    do_colors    = False
	if not texcoords: do_tangents  = False

	if not obj.vertex_groups or len(obj.vertex_groups) < 1:
		do_bones = False

	v_form  = VF_V3
	v_len   = 3
	w_count = 0

	if do_texcoords:
		v_form |= VF_T2
		v_len  += 2

	if do_colors:
		v_form |= VF_C4
		v_len  += 4

	if do_normals:
		v_form |= VF_N3
		v_len  += 3

	if do_tangents:
		v_form |= VF_T4
		v_len  += 4

	if do_bones:

		for grp in obj.vertex_groups:
			bones[grp.index] = [ len(bones), grp.name ]

		w_max = 0

		for v in verts:
			if v.groups and len(v.groups) > w_max:
				w_max = len(v.groups)

		w_count = 2     if w_max <= 2 else 4
		v_form |= VF_B4 if w_max <= 2 else VF_B8
		v_len  += 2 * w_count

		for name, index in bones:
			bone = {}
			bone['name']   = name
			bone['parent'] = -1
			bone['rx']     = 0.0
			bone['ry']     = 0.0
			bone['rz']     = 0.0
			bone['rw']     = 1.0
			bone['tx']     = 0.0
			bone['ty']     = 0.0
			bone['tz']     = 0.0
			geo['bones'].append(bone)

#
# TODO: Process armature hierarchy, process graftpoints
#

	geo['vformat'] = v_form
	geo['vlen']    = v_len

	TRI  = [ 0, 1, 2 ]
	QUAD = [ 0, 1, 2, 0, 2, 3 ]  # NOTE: n-Gons are not supported, calc_tangents() doesnt work with them

	for poly in mesh.polygons:

		if not poly.material_index in matgroups:
			matgroups[poly.material_index] = []

		for index in (TRI if len(poly.loop_indices) < 4 else QUAD):

			lp     = loops[poly.loop_indices[index]]
			vx     = verts[poly.vertices[index]]
			vertex = [ 0.0 for k in range(v_len) ]
			m      = 0


			if do_swap_yz:
				vertex[m + 0] = vx.co.x
				vertex[m + 1] = vx.co.z
				vertex[m + 2] = vx.co.y
				m += 3

			else:
				vertex[m + 0] = vx.co.x
				vertex[m + 1] = vx.co.y
				vertex[m + 2] = vx.co.z
				m += 3

			if texcoords:
				vertex[m + 0] = texcoords[lp.index].uv[0]
				vertex[m + 1] = texcoords[lp.index].uv[1]
				m += 2

			if colors:
				vertex[m + 0] = colors[lp.index].color.r
				vertex[m + 1] = colors[lp.index].color.g
				vertex[m + 2] = colors[lp.index].color.b
				vertex[m + 3] = 1.0
				m += 4

			if do_normals:
				if do_swap_yz:
					vertex[m + 0] = lp.normal.x
					vertex[m + 1] = lp.normal.z
					vertex[m + 2] = lp.normal.y

				else:
					vertex[m + 0] = lp.normal.x
					vertex[m + 1] = lp.normal.y
					vertex[m + 2] = lp.normal.z

				m += 3

			if do_tangents:
				if do_swap_yz:
					vertex[m + 0] = lp.tangent.x
					vertex[m + 1] = lp.tangent.z
					vertex[m + 2] = lp.tangent.y

				else:
					vertex[m + 0] = lp.tangent.x
					vertex[m + 1] = lp.tangent.y
					vertex[m + 2] = lp.tangent.z

				vertex[m + 3] = lp.bitangent_sign
				m += 4

			if do_bones:
				if vx.groups:
					for grp in heapq.nlargest(w_count, vx.groups, key=lambda g: g.weight if bone_index[grp.group] >= 0 else -1.0):
						if bone_index[grp.group] >= 0:
							vertex[m + 0] = bone_index[grp.group]
							vertex[m + 1] = grp.weight
							m += 2

			matgroups[poly.material_index].extend(vertex)

	fragments = []
	vertices  = []

	for id, verts in matgroups.items():

		mat = mesh.materials[id].name if id < len(mesh.materials) else 'default'

		frag = {}
		frag['name']     = mat
		frag['material'] = mat
		frag['mode']     = GL_TRIANGLES
		frag['start']    = len(vertices) // v_len
		frag['count']    = len(verts) // v_len
		fragments.append(frag)

		vertices += verts

	geo['fragments'] = fragments
	geo['vertices']  = vertices

	geo['materials']['default']={
		'name':     'default',
		'ambient':  [ 0.0, 0.0, 0.0 ],
		'diffuse':  [ 1.0, 1.0, 1.0 ],
		'specular': [ 0.0, 0.0, 0.0 ],
		'alpha':    1.0,
		'exponent': 0.0,
		'radiance': 0.0,
		'textures': []
	}

	for mat in mesh.materials:

		textures = []
		shader   = "shader::none"

		for tex in mat.texture_slots:
			if tex and tex.use and tex.name not in textures:
				textures.append(tex.name)

		for key in mat.keys():
			if key.strip()[0:7] == "shader:":
				shader = key[key.index(':')+1:].strip()

		material = {}
		material['name']     = mat.name
		material['shader']   = shader
		material['ambient']  = [ mat.ambient,  mat.ambient,  mat.ambient ] # FIXME: This should modulate the global ambience
		material['diffuse']  = [ mat.diffuse_color[0],  mat.diffuse_color[1],  mat.diffuse_color[2] ]
		material['specular'] = [ mat.specular_color[0], mat.specular_color[1], mat.specular_color[2] ]
		material['alpha']    = mat.alpha
		material['exponent'] = mat.specular_hardness
		material['radiance'] = mat.emit
		material['textures'] = textures

		geo['materials'][mat.name] = material

	bpy.data.meshes.remove(mesh)

	return geo



#
# Compact the mesh's vertices and generate indices
#
def geo_compact_vertices(geo, tolerance):

	v_len    = geo['vlen']
	vertices = []
	indices  = []

	print("geoblender: Compacting %d vertices..." % (len(geo['vertices']) / v_len))

	for vn in range(0, len(geo['vertices']), v_len):

		vertex = geo['vertices'][vn:vn + v_len]
		index  = -1

		for vm in range(0, len(vertices), v_len):
			match = True
			for k in range(v_len):
				if math.fabs(vertex[k] - vertices[vm + k]) >= tolerance:
					match = False
					break
			if match:
				index = vm // v_len
				break

		if index < 0:
			index = len(vertices) // v_len
			vertices.extend(vertex)

		indices.append(index)

	print("geoblender: Mesh now has %d vertices and %d indices." % (len(vertices) // v_len, len(indices)))

	geo['vertices'] = vertices
	geo['indices']  = indices



#
# Process all the animations in the scene
#
def geo_process_animations(geo, obj):
	print("geoblender: FIXME: Animation processing not implemented!")



#
# Process the boundboxes relevant to this object
#
def geo_process_boundboxes(geo, obj):

	cache = {}
	boxes = []

	cache[obj.name] = -1

	for ob in bpy.context.scene.objects: # Blender seems to have the objects sorted, so only one pass is needed
		if ob.name.startswith('box:') and ob.parent != None:

			if not ob.parent.name in cache:
				continue

			try:
				mesh = ob.to_mesh(bpy.context.scene, False, 'PREVIEW')

			except:
				print("geoblender: Failed to process boundbox %s\n" % ob.name)
				continue

			mesh.transform(ob.matrix_world) # TODO: Maybe keep the matrix to make a better fitting OBB?

			cache[ob.name] = len(boxes)

			minimum = [ +1.0e+6, +1.0e+6, +1.0e+6 ]
			maximum = [ -1.0e+6, -1.0e+6, -1.0e+6 ]

			for vert in mesh.vertices:
				minimum = [ min(minimum[n], x) for n, x in enumerate([ vert.co.x, vert.co.z, -vert.co.y ]) ]
				maximum = [ max(maximum[n], x) for n, x in enumerate([ vert.co.x, vert.co.z, -vert.co.y ]) ]

			box = {}
			box['name']   = ob.name[ob.name.index(':') + 1:]
			box['parent'] = cache[ob.parent.name]
			box['bone']   = -1
			box['radius'] = math.sqrt(sum([ (maximum[n] - minimum[n]) * (maximum[n] - minimum[n]) for n in range(3) ])) / 2.0
			box['cen']    = [ (maximum[n] + minimum[n]) / 2.0 for n in range(3) ]
			box['min']    = minimum
			box['max']    = maximum

			boxes.append(box)

			bpy.data.meshes.remove(mesh)

	geo['boxes'] = boxes



#
# Process all the graftpoints in scene, keep the ones related to the object
#
def geo_process_graftpoints(geo, obj):

	grafts = []

	for ob in bpy.context.scene.objects:
		if ob.name.startswith('graft:') and ob.parent == obj:

			try:
				mesh = ob.to_mesh(bpy.context.scene, False, 'PREVIEW')

			except:
				print("geoblender: Failed to process graftpoint %s\n" % ob.name)
				continue

			mesh.transform(ob.matrix_world)

			minimum = [ +1.0e+6, +1.0e+6, +1.0e+6 ]
			maximum = [ -1.0e+6, -1.0e+6, -1.0e+6 ]
			center  = [ 0.0, 0.0, 0.0 ]
			count   = 0
			normal  = mesh.tessfaces[0].normal

			for vert in [  mesh.vertices[i] for i in mesh.tessfaces[0].vertices ]:
				for n, x in enumerate([ vert.co.x, vert.co.z, -vert.co.y ]):
					center[n]  = center[n] + x
					minimum[n] = min(minimum[n], x)
					maximum[n] = max(maximum[n], x)

				count += 1

			if count > 0:
				graft = {}
				graft['name'] = ob.name[ob.name.index(':') + 1:]
				graft['bone'] = -1
				graft['cen']  = [ center[0] / count, center[1] / count, center[2] / count ]
				graft['dir']  = [ normal.x, normal.z, -normal.y ]
				graft['min']  = minimum
				graft['max']  = maximum

				grafts.append(graft)

			bpy.data.meshes.remove(mesh)

	geo['grafts'] = grafts



#
# Write a representation of geometry to disk
#
def geo_export_binary(geo, file):

	chunks  = []
	strings = b''


	def mkstr(text):
		nonlocal strings
		start    = len(strings)
		strings += bytes(text, 'utf-8') + b'\x00'
		return start


	def generate_strings():
		if len(strings) > 0:
			chunks.insert(0, [ CHUNK_STRINGS, strings ]) #FIXME: Add chunk priority sorting


	def generate_fragments():

		fragments = b''

		for frag in geo['fragments']:

			mat = geo['materials'][frag['material']]

			fragments += struct.pack('@II12f4I',
				mkstr(frag['name']),
				mkstr(mat['shader']),
				mat['ambient'][0],  mat['ambient'][1],  mat['ambient'][2],
				mat['diffuse'][0],  mat['diffuse'][1],  mat['diffuse'][2],
				mat['specular'][0], mat['specular'][1], mat['specular'][2],
				mat['alpha'],       mat['exponent'],    mat['radiance'],
				frag['mode'], frag['start'], frag['count'], 0)

		if len(fragments) > 0:
			chunks.append([ CHUNK_FRAGMENT, fragments ])


	def generate_textures():

		textures = []

		for name, mat in geo['materials'].items():
			for tex in mat['textures']:

				mode = -1

				if tex.startswith('color:'):
					mode = TEXTURE_MODE_COLORMAP

				elif tex.startswith('normal:'):
					mode = TEXTURE_MODE_NORMALMAP

				elif tex.startswith('gloss:'):
					mode = TEXTURE_MODE_GLOSSMAP

				if mode >= 0:
					textures.append([
						CHUNK_TEXTURE,
						struct.pack('@2I', mkstr(tex[tex.index(':') + 1:]), mode)
					])

		if len(textures) > 0:
			chunks.extend(textures)


	def generate_skeleton():

		bones = b''.join([
			struct.pack('@Ii7f',
				mkstr(bone['name']),
				bone['parent'],
				bone['rx'], bone['ry'], bone['rz'], bone['rw'],
				bone['tx'], bone['ty'], bone['tz']) for bone in geo['bones'] ])

		if len(bones) > 0:
			chunks.append([ CHUNK_BONES, bones ])


	def generate_animations():
		#chunks.append([ CHUNK_ANIMATION, b'' ])
		pass


	def generate_boundboxes():

		boxes = b''.join([
			struct.pack('@Iii10f',
				mkstr(box['name']),
				box['parent'],
				box['bone'],
				box['radius'],
				box['cen'][0], box['cen'][1], box['cen'][2],
				box['min'][0], box['min'][1], box['min'][2],
				box['max'][0], box['max'][1], box['max'][2]) for box in geo['boxes'] ])

		if len(boxes) > 0:
			chunks.append([ CHUNK_BOUNDBOX, boxes ])


	def generate_graftpoints():

		grafts = b''.join([
			struct.pack('@Ii12f',
				mkstr(graft['name']),
				graft['bone'],
				graft['cen'][0], graft['cen'][1], graft['cen'][2],
				graft['dir'][0], graft['dir'][1], graft['dir'][2],
				graft['min'][0], graft['min'][1], graft['min'][2],
				graft['max'][0], graft['max'][1], graft['max'][2]) for graft in geo['grafts'] ])

		if len(grafts) > 0:
			chunks.append([ CHUNK_GRAFT, grafts ])


	def generate_lod(): # FIXME: This needs to be a per-fragment thing
		#chunks.append([ CHUNK_LOD, struct.pack('=2If2I', 0, 0, 0.0, 0, len(geo['indices'])) ])
		pass


	def generate_text():
		for key in sorted(geo['strings']):
			chunks.append([ CHUNK_TEXT, struct.pack('@II', mkstr(key), mkstr(geo['strings'][key])) ])


	def generate_vertices():
		chunks.append([ CHUNK_VERTICES, struct.pack('=%df' % len(geo['vertices']), *geo['vertices']) ])


	def generate_indices():

		if len(geo['vertices']) > 65535:
			chunks.append([ CHUNK_INDICES, struct.pack('=%dI' % len(geo['indices']), *geo['indices']) ])

		else:
			chunks.append([ CHUNK_INDICES, struct.pack('=%dH' % len(geo['indices']), *geo['indices']) ])


	try:
		out = open(file, 'wb')

	except:
		return

	generate_fragments()
	generate_lod()
	generate_textures()
	generate_skeleton()
	generate_animations()
	generate_boundboxes()
	generate_graftpoints()
	generate_vertices()
	generate_indices()
	generate_text()
	generate_strings()

	iformat = GL_UNSIGNED_INT if len(geo['vertices']) > 65535 else GL_UNSIGNED_SHORT
	vformat = geo['vformat']

	header = struct.pack('@4I', GEOMETRY_MAGIC, len(chunks), vformat, iformat)
	offset = 16 + 16 * len(chunks)

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

	out.close()



def io_export_main(ctx, opts):

	objects =\
		ctx.selected_objects if opts['do_objects'] == 'sel' else\
		ctx.scene.objects    if opts['do_objects'] == 'all' else\
		[ bpy.context.scene.objects.active ]

	for obj in objects:

		print("geoblender: Processing '%s'..." % obj.name)

		out = opts['filepath'].replace('%', obj.name)
		geo = geo_process_mesh(
			obj,
			opts['do_apply_mods'],
			opts['do_swap_yz'],
			opts['do_modelview'],
			opts['do_texcoords'],
			opts['do_colors'],
			opts['do_normals'],
			opts['do_tangents'],
			opts['do_armatures'])

		if not geo:
			print("geoblender: Unable to process '%s', skipping..." % obj.name)
			continue

		geo_compact_vertices(geo, 0.001)

		if opts['do_animation']:
			geo_process_animations(geo, obj)

		if opts['do_boundboxes']:
			geo_process_boundboxes(geo, obj)

		if opts['do_grafts']:
			geo_process_graftpoints(geo, obj)

		if opts['do_comment'] != "":
			geo['strings']['comment'] = opts['do_comment']

		print("geoblender: Writing to '%s'..." % out)

		geo_export_binary(geo, out)

		print("geoblender: Done.")



#
# Blender UI Stuff
#
class io_export_orion_geometry(bpy.types.Operator, bpxio.ExportHelper):

	bl_idname  = "export_scene.geo"
	bl_label   = "Export Geometry"
	bl_options = { 'PRESET' }

	filename_ext = ".geo"
	filter_glob  = bprop.StringProperty(default="*.geo", options={'HIDDEN'})
	path_mode    = bpxio.path_reference_mode

	do_objects  = bprop.EnumProperty(name="",
		items=(
			('one',  "Single export, active object",      ""),
			('sel',  "Separate export, selected oblects", ""),
			('all',  "Separate export, all objects",      "")),
		default='one')
	do_apply_mods = bprop.BoolProperty(name="Apply Modifiers",          default=True)
	do_swap_yz    = bprop.BoolProperty(name="Swap Y/Z axes",            default=True)
	do_modelview  = bprop.BoolProperty(name="Apply ModelView",          default=True)
	do_texcoords  = bprop.BoolProperty(name="Export Texcoords",         default=True)
	do_colors     = bprop.BoolProperty(name="Export Vertex Colors",     default=True)
	do_normals    = bprop.BoolProperty(name="Export Normals",           default=True)
	do_tangents   = bprop.BoolProperty(name="Export Tangent Space",     default=True)
	do_animation  = bprop.BoolProperty(name="FIXME: Export Animations", default=True)
	do_armatures  = bprop.BoolProperty(name="FIXME: Export Armatures",  default=True)
	do_boundboxes = bprop.BoolProperty(name="Export Bounding Boxes",    default=True)
	do_grafts     = bprop.BoolProperty(name="Export Grafting Points",   default=True)
	do_comment    = bprop.StringProperty(name="Comment", default="Created with Blender v2.74")

	def execute(self, context):
		io_export_main(context, self.as_keywords())
		return { 'FINISHED' }



def menu_export_geo(self, context):
	self.layout.operator(io_export_orion_geometry.bl_idname, text = "Orion Core geometry (.geo)")



def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_export_geo)



def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_export_geo)



if __name__ == "__main__":
	register()

