#!/usr/bin/python

import argparse
import io
import os
import subprocess
import sys
import tempfile


VULKAN_H = "/usr/include/vulkan/vulkan.h"

VK_TRUE  = 1
VK_FALSE = 0

SECTION_VERT = 0
SECTION_TESC = 1
SECTION_TESE = 2
SECTION_GEOM = 3
SECTION_FRAG = 4
SECTION_COMP = 5
SECTION_NUM  = 6

GR_V2 = 0x0000
GR_V3 = 0x0001
GR_T2 = 0x0002
GR_T3 = 0x0006
GR_C3 = 0x0008
GR_N3 = 0x0010
GR_X1 = 0x0040
GR_X2 = 0x00c0
GR_X3 = 0x01c0
GR_X4 = 0x03c0
GR_B2 = 0x0400
GR_B4 = 0x0c00
GR_B6 = 0x1c00
GR_B8 = 0x3c00


KEYWORD_ENUM = [

##
# Main directives
#
	'K_BLEND',  'K_DEPTH',   'K_INCLUDE', 'K_PRIMITIVE',
	'K_RASTER', 'K_RENDER',  'K_SECTION', 'K_SHADER',
	'K_STAGE',  'K_STENCIL', 'K_VERTEX',  'K_WRITE',

##
# Shader stages
#
	'K_DOT_VERTEX',   'K_DOT_TESSCTRL', 'K_DOT_TESSEVAL', 'K_DOT_GEOMETRY',
	'K_DOT_FRAGMENT', 'K_DOT_COMPUTE',

##
# Blending functions
#
	'K_CONST',
	'K_COLOR',    'K_ALPHA',
	'K_DISABLED', 'K_ENABLED',

	'K_ADD', 'K_SUBTRACT',  'K_REVERSE_SUBTRACT',
	'K_MIN', 'K_MAX',

	'K_CONST_ALPHA',           'K_CONST_COLOR',         'K_DST_ALPHA',
	'K_DST_COLOR',             'K_ONE',                 'K_ONE_MINUS_CONST_ALPHA',
	'K_ONE_MINUS_CONST_COLOR', 'K_ONE_MINUS_DST_ALPHA', 'K_ONE_MINUS_DST_COLOR',
	'K_ONE_MINUS_SRC_ALPHA',   'K_ONE_MINUS_SRC_COLOR', 'K_SRC_ALPHA',
	'K_SRC_ALPHA_SATURATE',    'K_SRC_COLOR',           'K_ZERO',

	'K_LOGIC_OFF',   'K_LOGIC_CLEAR',  'K_LOGIC_AND',   'K_LOGIC_ANDREV',
	'K_LOGIC_BLT',   'K_LOGIC_ANDINV', 'K_LOGIC_NOP',   'K_LOGIC_XOR',
	'K_LOGIC_OR',    'K_LOGIC_NOR',    'K_LOGIC_EQV',   'K_LOGIC_INV',
	'K_LOGIC_ORREV', 'K_LOGIC_BLTINV', 'K_LOGIC_ORINV', 'K_LOGIC_NAND',
	'K_LOGIC_SET',

##
# Depth testing
#
	'K_NEVER', 'K_LESS',     'K_EQUAL',  'K_LEQUAL',
	'K_GREATER', 'K_NEQUAL', 'K_GEQUAL', 'K_ALWAYS',

	'K_BIAS_ON', 'K_BIAS_OFF',

##
# Primitive topology
#
	'K_POINTS',        'K_LINES',        'K_LINESTRIP', 'K_TRIANGLES',
	'K_TRISTRIP',      'K_TRIFAN',       'K_LINES_ADJ', 'K_LINESTRIP_ADJ',
	'K_TRIANGLES_ADJ', 'K_TRISTRIP_ADJ', 'K_PATCHES',

	'K_RESTART_ON', 'K_RESTART_OFF',

##
# Rasterizer state
#
	'K_DISCARD_ON', 'K_DISCARD_OFF',

	'K_NONE', 'K_FRONT',
	'K_BACK', 'K_BOTH',

##
# Stencil operations
#
	'K_KEEP',   'K_REPLACE', 'K_INCWRAP', 'K_DECWRAP',
	'K_INVERT', 'K_INC',     'K_DEC',

##
# Output write control
#
	'K_DEPTH_ON', 'K_DEPTH_OFF',

	'K_RED_ON',   'K_RED_OFF',
	'K_GREEN_ON', 'K_GREEN_OFF',
	'K_BLUE_ON',  'K_BLUE_OFF',
	'K_ALPHA_ON', 'K_ALPHA_OFF',
	'K_COLOR_ON', 'K_COLOR_OFF'

]


for n, var in enumerate(KEYWORD_ENUM):
	globals()[var] = n


##
# Impoprt symbols from vulkan.h
#
with open(VULKAN_H, 'r') as vk:
	for line in vk:
		if '=' in line:
			s = line.index('=')
			v = line[s + 1:]
			c = v.index(',') if ',' in v else -1
			try:    globals()[line[:s].strip()] = int(v[:c].strip(), 0)
			except: continue


KEYWORD_LIST = {

# Main directives
	'blend':  K_BLEND,  'depth':   K_DEPTH,   'include': K_INCLUDE, 'primitive': K_PRIMITIVE,
	'raster': K_RASTER, 'render':  K_RENDER,  'section': K_SECTION, 'shader':    K_SHADER,
	'stage':  K_STAGE,  'stencil': K_STENCIL, 'vertex':  K_VERTEX,  'write':     K_WRITE,

# Shader stages
	'.vertex':   K_DOT_VERTEX,   '.vert': K_DOT_VERTEX,
	'.tessctrl': K_DOT_TESSCTRL, '.tesc': K_DOT_TESSCTRL,
	'.tesseval': K_DOT_TESSEVAL, '.tese': K_DOT_TESSEVAL,
	'.geometry': K_DOT_GEOMETRY, '.geom': K_DOT_GEOMETRY,
	'.fragment': K_DOT_FRAGMENT, '.frag': K_DOT_FRAGMENT,
	'.compute':  K_DOT_COMPUTE,  '.comp': K_DOT_COMPUTE,

# Blending functions
	'const':    K_CONST,
	'color':    K_COLOR,    'alpha':   K_ALPHA,
	'disabled': K_DISABLED, 'enabled': K_ENABLED,

	'add': K_ADD, 'subtract': K_SUBTRACT, 'reverse-subtract': K_REVERSE_SUBTRACT,
	'min': K_MIN, 'max':      K_MAX,

	'const-alpha':           K_CONST_ALPHA,
	'const-color':           K_CONST_COLOR,
	'dst-alpha':             K_DST_ALPHA,
	'dst-color':             K_DST_COLOR,
	'one':                   K_ONE,
	'one-minus-const-alpha': K_ONE_MINUS_CONST_ALPHA,
	'one-minus-const-color': K_ONE_MINUS_CONST_COLOR,
	'one-minus-dst-alpha':   K_ONE_MINUS_DST_ALPHA,
	'one-minus-dst-color':   K_ONE_MINUS_DST_COLOR,
	'one-minus-src-alpha':   K_ONE_MINUS_SRC_ALPHA,
	'one-minus-src-color':   K_ONE_MINUS_SRC_COLOR,
	'src-alpha':             K_SRC_ALPHA,
	'src-alpha-saturate':    K_SRC_ALPHA_SATURATE,
	'src-color':             K_SRC_COLOR,
	'zero':                  K_ZERO,

	'logic:off':   K_LOGIC_OFF,   'logic:clear':  K_LOGIC_CLEAR,
	'logic:and':   K_LOGIC_AND,   'logic:andrev': K_LOGIC_ANDREV,
	'logic:blt':   K_LOGIC_BLT,   'logic:andinv': K_LOGIC_ANDINV,
	'logic:nop':   K_LOGIC_NOP,   'logic:xor':    K_LOGIC_XOR,
	'logic:or':    K_LOGIC_OR,    'logic:nor':    K_LOGIC_NOR,
	'logic:eqv':   K_LOGIC_EQV,   'logic:inv':    K_LOGIC_INV,
	'logic:orrev': K_LOGIC_ORREV, 'logic:bltinv': K_LOGIC_BLTINV,
	'logic:orinv': K_LOGIC_ORINV, 'logic:nand':   K_LOGIC_NAND,
	'logic:set':   K_LOGIC_SET,

# Depth testing
	'never':   K_NEVER,   'less':   K_LESS,   'equal':  K_EQUAL,  'lequal': K_LEQUAL,
	'greater': K_GREATER, 'nequal': K_NEQUAL, 'gequal': K_GEQUAL, 'always': K_ALWAYS,

	'bias:on': K_BIAS_ON, 'bias:off': K_BIAS_OFF,

# Primitive topology
	'points':       K_POINTS,       'lines':         K_LINES,         'linestrip':     K_LINESTRIP,
	'triangles':    K_TRIANGLES,    'tristrip':      K_TRISTRIP,      'trifan':        K_TRIFAN,
	'lines-adj':    K_LINES_ADJ,    'linestrip-adj': K_LINESTRIP_ADJ, 'triangles-adj': K_TRIANGLES_ADJ,
	'tristrip-adj': K_TRISTRIP_ADJ, 'patches':       K_PATCHES,

	'restart:on': K_RESTART_ON, 'restart:off': K_RESTART_OFF,

# Rasterizer state
	'discard:on': K_DISCARD_ON, 'discard:off': K_DISCARD_OFF,

	'none': K_NONE, 'front': K_FRONT,
	'back': K_BACK, 'both':  K_BOTH,

# Stencil operations
	'keep':   K_KEEP,   'replace': K_REPLACE, 'inc-wrap': K_INCWRAP, 'dec-wrap': K_DECWRAP,
	'invert': K_INVERT, 'inc':     K_INC,     'dec':      K_DEC,

# Output write control
	'depth:on': K_DEPTH_ON, 'depth:off': K_DEPTH_OFF,

	'red:on':   K_RED_ON,   'red:off':   K_RED_OFF,
	'green:on': K_GREEN_ON, 'green:off': K_GREEN_OFF,
	'blue:on':  K_BLUE_ON,  'blue:off':  K_BLUE_OFF,
	'alpha:on': K_ALPHA_ON, 'alpha:off': K_ALPHA_OFF,
	'color:on': K_COLOR_ON, 'color:off': K_COLOR_OFF

}


def BLEND_MODE(x):
	_ = {   K_DISABLED:         VK_BLEND_OP_MAX_ENUM,
		K_ADD:              VK_BLEND_OP_ADD,
		K_SUBTRACT:         VK_BLEND_OP_SUBTRACT,
		K_REVERSE_SUBTRACT: VK_BLEND_OP_REVERSE_SUBTRACT,
		K_MIN:              VK_BLEND_OP_MIN,
		K_MAX:              VK_BLEND_OP_MAX
	}
	return _[x] if x in _ else -1


def BLEND_SRC(x):
	_ = {   K_DISABLED:              VK_BLEND_FACTOR_MAX_ENUM,
		K_CONST_ALPHA:           VK_BLEND_FACTOR_CONSTANT_ALPHA,
		K_CONST_COLOR:           VK_BLEND_FACTOR_CONSTANT_COLOR,
		K_DST_ALPHA:             VK_BLEND_FACTOR_DST_ALPHA,
		K_DST_COLOR:             VK_BLEND_FACTOR_DST_COLOR,
		K_ONE:                   VK_BLEND_FACTOR_ONE,
		K_ONE_MINUS_CONST_ALPHA: VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
		K_ONE_MINUS_CONST_COLOR: VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
		K_ONE_MINUS_DST_ALPHA:   VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		K_ONE_MINUS_DST_COLOR:   VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		K_ONE_MINUS_SRC_ALPHA:   VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		K_ONE_MINUS_SRC_COLOR:   VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		K_SRC_ALPHA:             VK_BLEND_FACTOR_SRC_ALPHA,
		K_SRC_ALPHA_SATURATE:    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
		K_SRC_COLOR:             VK_BLEND_FACTOR_SRC_COLOR,
		K_ZERO:                  VK_BLEND_FACTOR_ZERO
	}
	return _[x] if x in _ else -1


def BLEND_DST(x):
	_ = {   K_DISABLED:              VK_BLEND_FACTOR_MAX_ENUM,
		K_CONST_ALPHA:           VK_BLEND_FACTOR_CONSTANT_ALPHA,
		K_CONST_COLOR:           VK_BLEND_FACTOR_CONSTANT_COLOR,
		K_DST_ALPHA:             VK_BLEND_FACTOR_DST_ALPHA,
		K_DST_COLOR:             VK_BLEND_FACTOR_DST_COLOR,
		K_ONE:                   VK_BLEND_FACTOR_ONE,
		K_ONE_MINUS_CONST_ALPHA: VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
		K_ONE_MINUS_CONST_COLOR: VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
		K_ONE_MINUS_DST_ALPHA:   VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		K_ONE_MINUS_DST_COLOR:   VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		K_ONE_MINUS_SRC_ALPHA:   VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		K_ONE_MINUS_SRC_COLOR:   VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		K_SRC_ALPHA:             VK_BLEND_FACTOR_SRC_ALPHA,
		K_SRC_COLOR:             VK_BLEND_FACTOR_SRC_COLOR,
		K_ZERO:                  VK_BLEND_FACTOR_ZERO
	}
	return _[x] if x in _ else -1


def BLEND_LOGICOP(x):
	_ = {   K_LOGIC_OFF:    VK_LOGIC_OP_MAX_ENUM,
		K_LOGIC_CLEAR:  VK_LOGIC_OP_CLEAR,
		K_LOGIC_AND:    VK_LOGIC_OP_AND,
		K_LOGIC_ANDREV: VK_LOGIC_OP_AND_REVERSE,
		K_LOGIC_BLT:    VK_LOGIC_OP_COPY,
		K_LOGIC_ANDINV: VK_LOGIC_OP_AND_INVERTED,
		K_LOGIC_NOP:    VK_LOGIC_OP_NO_OP,
		K_LOGIC_XOR:    VK_LOGIC_OP_XOR,
		K_LOGIC_OR:     VK_LOGIC_OP_OR,
		K_LOGIC_NOR:    VK_LOGIC_OP_NOR,
		K_LOGIC_EQV:    VK_LOGIC_OP_EQUIVALENT,
		K_LOGIC_INV:    VK_LOGIC_OP_INVERT,
		K_LOGIC_ORREV:  VK_LOGIC_OP_OR_REVERSE,
		K_LOGIC_BLTINV: VK_LOGIC_OP_COPY_INVERTED,
		K_LOGIC_ORINV:  VK_LOGIC_OP_OR_INVERTED,
		K_LOGIC_NAND:   VK_LOGIC_OP_NAND,
		K_LOGIC_SET:    VK_LOGIC_OP_SET
	}
	return _[x] if x in _ else -1


def DEPTH_MODE(x):
	_ = {   K_DISABLED: VK_COMPARE_OP_MAX_ENUM,
		K_NEVER:    VK_COMPARE_OP_NEVER,
		K_LESS:     VK_COMPARE_OP_LESS,
		K_EQUAL:    VK_COMPARE_OP_EQUAL,
		K_LEQUAL:   VK_COMPARE_OP_LESS_OR_EQUAL,
		K_GREATER:  VK_COMPARE_OP_GREATER,
		K_NEQUAL:   VK_COMPARE_OP_NOT_EQUAL,
		K_GEQUAL:   VK_COMPARE_OP_GREATER_OR_EQUAL,
		K_ALWAYS:   VK_COMPARE_OP_ALWAYS
	}
	return _[x] if x in _ else -1


def PRIMITIVE_TOPOLOGY(x):
	_ = {   K_POINTS:        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		K_LINES:         VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		K_LINESTRIP:     VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		K_TRIANGLES:     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		K_TRISTRIP:      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		K_TRIFAN:        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		K_LINES_ADJ:     VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
		K_LINESTRIP_ADJ: VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
		K_TRIANGLES_ADJ: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
		K_TRISTRIP_ADJ:  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
		K_PATCHES:       VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
	}
	return _[x] if x in _ else -1


def RASTER_MODE(x):
	_ = {   K_NONE:  VK_CULL_MODE_FRONT_AND_BACK,
		K_FRONT: VK_CULL_MODE_BACK_BIT,
		K_BACK:  VK_CULL_MODE_FRONT_BIT,
		K_BOTH:  VK_CULL_MODE_NONE
	}
	return _[x] if x in _ else -1


def SECTION_MASK(x):
	_ = {   K_DOT_VERTEX:   SECTION_VERT,
		K_DOT_TESSCTRL: SECTION_TESC,
		K_DOT_TESSEVAL: SECTION_TESE,
		K_DOT_GEOMETRY: SECTION_GEOM,
		K_DOT_FRAGMENT: SECTION_FRAG,
		K_DOT_COMPUTE:  SECTION_COMP
	}
	return _[x] if x in _ else -1


def STENCIL_MODE(x): return DEPTH_MODE(x)
def STENCIL_OP(x):
	_ = {   K_KEEP:    VK_STENCIL_OP_KEEP,
		K_ZERO:    VK_STENCIL_OP_ZERO,
		K_REPLACE: VK_STENCIL_OP_REPLACE,
		K_INCWRAP: VK_STENCIL_OP_INCREMENT_AND_CLAMP,
		K_DECWRAP: VK_STENCIL_OP_DECREMENT_AND_CLAMP,
		K_INVERT:  VK_STENCIL_OP_INVERT,
		K_INC:     VK_STENCIL_OP_INCREMENT_AND_WRAP,
		K_DEC:     VK_STENCIL_OP_DECREMENT_AND_WRAP
	}
	return _[x] if x in _ else -1


def WRITE_CHANNEL(x):
	_ = {   K_RED_ON:    VK_COLOR_COMPONENT_R_BIT,
		K_RED_OFF:   VK_COLOR_COMPONENT_R_BIT,
		K_GREEN_ON:  VK_COLOR_COMPONENT_G_BIT,
		K_GREEN_OFF: VK_COLOR_COMPONENT_G_BIT,
		K_BLUE_ON:   VK_COLOR_COMPONENT_B_BIT,
		K_BLUE_OFF:  VK_COLOR_COMPONENT_B_BIT,
		K_ALPHA_ON:  VK_COLOR_COMPONENT_A_BIT,
		K_ALPHA_OFF: VK_COLOR_COMPONENT_A_BIT,
		K_COLOR_ON:  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
		K_COLOR_OFF: VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT

	}
	return _[x] if x in _ else -1


def WRITE_MODE(x):
	_ = {   K_RED_ON:   VK_TRUE, K_RED_OFF:   VK_FALSE,
		K_GREEN_ON: VK_TRUE, K_GREEN_OFF: VK_FALSE,
		K_BLUE_ON:  VK_TRUE, K_BLUE_OFF:  VK_FALSE,
		K_ALPHA_ON: VK_TRUE, K_ALPHA_OFF: VK_FALSE,
		K_COLOR_ON: VK_TRUE, K_COLOR_OFF: VK_FALSE
	}
	return _[x] if x in _ else -1



class shader_data:

	sections = [ ]
	binary   = [ ]
	active   = [ ]
	metadata = ""

	shader_name   = None
	shader_stage  = 500
	render_target = "framebuffer"

	# Input assembler control
	input_vertex_format      = -1
	input_primitive_topology = -1
	input_primitive_restart  = VK_FALSE

	# Rasterizer control
	raster_discard          = VK_FALSE
	raster_cullmode         = RASTER_MODE(K_BOTH)
	raster_depth_bias       = VK_FALSE
	raster_depth_bias_const = 0.0
	raster_depth_bias_slope = 0.0
	raster_depth_bias_clamp = 0.0

	# Depth testing control
	depth_write = VK_TRUE
	depth_func  = DEPTH_MODE(K_DISABLED)

	# Stencil test control
	stencil_fail      = [ STENCIL_OP(K_KEEP), STENCIL_OP(K_KEEP) ] # Front, back
	stencil_pass      = [ STENCIL_OP(K_KEEP), STENCIL_OP(K_KEEP) ]
	stencil_depthfail = [ STENCIL_OP(K_KEEP), STENCIL_OP(K_KEEP) ]
	stencil_func      = [ STENCIL_MODE(K_DISABLED), STENCIL_MODE(K_DISABLED) ]
	stencil_mask      = [ 0xffffffff, 0xffffffff ]
	stencil_write     = [ 0xffffffff, 0xffffffff ]
	stencil_ref       = [ 0,          0 ]

	# Color blend control, per attachment
	color_mask  = {}
	blend_color = {} #[]: func, src, dst
	blend_alpha = {} #[]: func, src, dst

	# Master blend control
	blend_logic_op = BLEND_LOGICOP(K_LOGIC_OFF)
	blend_const    = [ 0.0, 0.0, 0.0, 0.0 ]


	def __init__(self):
		self.sections = [ ""   for n in xrange(SECTION_NUM) ]
		self.binary   = [ ""   for n in xrange(SECTION_NUM) ]
		self.active   = [ True for n in xrange(SECTION_NUM) ]



def perror(str):
	sys.stderr.write("shaderc: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("shaderc: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Compile Vulkan shaders to SPIR-V modules")
	argp.add_argument('-o', '--output',   action='store',      type=str, help='Set output file name (aout.a)')
	argp.add_argument('-I', '--include',  action='append',     type=str, help='Add path to include search list')
	argp.add_argument('-K', '--keep',     action='store_true',           help='Keep intermediate files')
	argp.add_argument('--compiler',       action='store',      type=str, help='Set default compiler (glslangValidator -V -x -S {stage} -o {output} {flags} {input})')
	argp.add_argument('input',   nargs=1)
	argp.add_argument('options', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.input:
		perror("no input file specified")

	if not argv.output:
		argv.output = "aout.a"

	if not argv.include:
		argv.include = []

	if not argv.compiler:
		argv.compiler = "glslangValidator -V -x -S {stage} -o {output} {flags} {input}"

	return argv



def shader_preprocess(shader, source, includes):

	comment_line  = False
	comment_block = False
	linenum       = 0

	def lookup(text):
		return KEYWORD_LIST[text] if text in KEYWORD_LIST else -1


	#blend attachment function src-mode dst-mode
	#blend attachment color|alpha function src-mode dst-mode
	#blend logic-op
	#blend const r g b a
	def process_blend(token, state, count):

		if count < 2:
			perror("error: %d: #blend requires at least 2 arguments" % linenum);

		logic_op = BLEND_LOGICOP(state[1])

		if logic_op >= 0:
			shader.blend_logic_op = logic_op

		elif state[0] == K_CONST:

			if count < 5:
				perror("error: %d: #blend color requires 4 components" % linenum);

			shader.blend_const = map(float, token[1:5])

		else:
			atch = token[0]
			func = -1
			src  = -1
			dst  = -1

			if state[1] == K_COLOR:

				func = BLEND_MODE(state[2])
				src  = BLEND_SRC( state[3])
				dst  = BLEND_DST( state[4])

				shader.blend_color[atch] = [ func, src, dst ]

			elif state[1] == K_ALPHA:

				func = BLEND_MODE(state[2])
				src  = BLEND_SRC( state[3])
				dst  = BLEND_DST( state[4])

				shader.blend_alpha[atch] = [ func, src, dst ]

			else:
				func = BLEND_MODE(state[1])
				src  = BLEND_SRC( state[2])
				dst  = BLEND_DST( state[3])

				shader.blend_color[atch] = [ func, src, dst ]
				shader.blend_alpha[atch] = [ func, src, dst ]

			if func < 0 or src < 0 or dst < 0:
				perror("error: %d: invalid blending parameters" % linenum);


	#depth func
	#depth bias:on const slope clamp
	#depth bias:off
	def process_depth(token, state, count):

		if state[0] == K_BIAS_ON:

			if count != 4:
				perror("error: %d: #depth bias:on expects 4 arguments" % linenum);

			shader.depth_bias       = VK_TRUE
			shader_depth_bias_const = float(token[1])
			shader_depth_bias_slope = float(token[2])
			shader_depth_bias_clamp = float(token[3])

		elif state[0] == K_BIAS_OFF:

			if count != 4:
				perror("error: %d: #depth bias:off expects 1 argument" % linenum);

			shader.depth_bias = VK_FALSE

		else:
			shader.depth_func = DEPTH_MODE(state[0])

			if shader.depth_func < 0:
				perror("error: %d: invalid depth mode." % linenum)


	#include <file>
	def process_include(token, state, count):

		if count != 1:
			perror("error: %d: #include expects at least one filename." % linenum)

		if not '<' in token[0] or not '>' in token[0]:
			perror("error: %d: #include expects <filename>, got '%s'." % (linenum, token[0]))

		name = token[0][token[0].index('<')+1:token[0].index('>')]

		if name == "":
			perror("error: %d: empty filename in #include." % linenum)

		for path in includes:
			if os.path.isfile(path + '/' + name):
				shader_preprocess(shader, path + '/' + name, includes)
				return

		perror("error: %d: unable to include '%s'." % (linenum, name))


	#primitive topology
	#primitive restart:on
	#primitive restart:off
	def process_primitive(token, state, count):

		if count != 1:
			perror("error: %d: #primitive expects exactly one argument" % linenum)

		if state[0] == K_RESTART_ON:
			shader.input_primitive_restart = VK_TRUE

		elif state[0] == K_RESTART_OFF:
			shader.input_primitive_restart = VK_FALSE

		else:
			shader.input_primitive_topology = PRIMITIVE_TOPOLOGY(state[0])

			if shader.input_primitive_topology < 0:
				perror("error: %d: invalid primitive topology" % linenum)


	#raster discard:on
	#raster discard:off
	#raster front|back|both|none
	def process_raster(token, state, count):

		if count != 1:
			perror("error: %d: #raster expects exactly one argument" % linenum);

		if state[0] == K_DISCARD_ON:
			shader.raster_discard = VK_TRUE

		elif state[0] == K_DISCARD_OFF:
			shader.raster_discard = VK_FALSE

		else:
			shader.raster_cullmode = RASTER_MODE(state[0])

			if shader.input_primitive_topology < 0:
				perror("error: %d: invalid face culling mode" % linenum)


	#render target-name
	def process_render(token, state, count):

		if count != 1:
			perror("error: %d: #render expects exactly one argument" % linenum);

		shader.render_target = token[0]


	#section [section, ...]
	def process_section(token, state, count):

		if count < 1:
			shader.active = [ True for n in xrange(SECTION_NUM) ]

		else:
			shader.active = [ False for n in xrange(SECTION_NUM) ]

			for n in xrange(len(token)):

				section = SECTION_MASK(state[n])

				if section < 0:
					perror("error: %d: invalid section '%s'." % (linenum, token[n]))

				shader.active[section] = True


	#shader shader-name
	def process_shader(token, state, count):

		if count != 1:
			perror("error: %d: #shader expects exactly one argument" % linenum);

		shader.shader_name = token[0]


	#stencil [front|back] func fail pass depth-fail
	#stencil [front|back] mask cmp-mask write-mask
	#stencil [front|back] ref reference
	def process_stencil(token, state, count):

		num = 0
		front = True
		back  = True

		if state[0] == K_FRONT:
			front = True
			back  = False
			num   = 1

		elif state[0] == K_BACK:
			front = False
			back  = True
			num   = 1

		if state[num] == K_MASK:

			if front:
				shader.stencil_mask[0]  = int(token[num + 1], 0)
				shader.stencil_write[0] = int(token[num + 2], 0)

			if back:
				shader.stencil_mask[1]  = int(token[num + 1], 0)
				shader.stencil_write[1] = int(token[num + 2], 0)

		elif state[num] == K_REF:
			if front: shader.stencil_ref[0] = int(token[num + 1], 0)
			if back:  shader.stencil_ref[1] = int(token[num + 1], 0)

		else:
			func     = STENCIL_MODE(state[num])
			op_fail  = STENCIL_OP(state[num + 1])
			op_pass  = STENCIL_OP(state[num + 2])
			op_dfail = STENCIL_OP(state[num + 3])

			if func     < 0: perror("error: %d: invalid stencil comparison operator"  % linenum);
			if op_pass  < 0: perror("error: %d: invalid stencil pass operation"       % linenum);
			if op_fail  < 0: perror("error: %d: invalid stencil fail operation"       % linenum);
			if op_dfail < 0: perror("error: %d: invalid stencil depth-fail operation" % linenum);

			if front:
				shader.stencil_fail[0]      = op_fail
				shader.stencil_pass[0]      = op_pass
				shader.stencil_depthfail[0] = op_dfail
				shader.stencil_func[0]      = func

			if back:
				shader.stencil_fail[1]      = op_fail
				shader.stencil_pass[1]      = op_pass
				shader.stencil_depthfail[1] = op_dfail
				shader.stencil_func[1]      = func


	#stage priority-num
	def process_stage(token, state, count):

		if count != 1:
			perror("error: %d: #stage expects exactly one argument" % linenum);

		shader.shader_stage = int(token[0])


	#write depth:on
	#write depth:off
	#write attachment [red:on|red:off ...]
	def process_write(token, state, count):

		if count < 1:
			perror("error: %d: #write expects at least one argument." % linenum);

		if state[0] == K_DEPTH_ON:
			shader.depth_write = VK_TRUE

		elif state[0] == K_DEPTH_OFF:
			shader.depth_write = VK_FALSE

		else:
			atch = token[0]
			mask = 0

			for n in xrange(1, count):

				write_chan = WRITE_CHANNEL[state[n]];
				write_mode = WRITE_MODE[   state[n]];

				if write_chan < 0:
					perror("error: %d: invalid color mask mode." % linenum);

				if write_mode:
					mask |= write_chan

				else:
					mask &= ~write_chan;

			shader.color_mask[atch] = mask


	#vertex V2|V3[T2|T3][C3][N3][X1|X2|X3|X4][B2|B4|B6|B8]
	def process_vertex(token, state, count):

		n    = 0
		mode = 0

		if   token[0][n:n+2] == 'V2': mode |= GR_V2; n += 2
		elif token[0][n:n+2] == 'V3': mode |= GR_V3; n += 2

		if   token[0][n:n+2] == 'T2': mode |= GR_T2; n += 2
		elif token[0][n:n+2] == 'T3': mode |= GR_T3; n += 2

		if token[0][n:n+2] == 'C3': mode |= GR_C3; n += 2
		if token[0][n:n+2] == 'N3': mode |= GR_N3; n += 2

		if   token[0][n:n+2] == 'X1': mode |= GR_X1; n += 2
		elif token[0][n:n+2] == 'X2': mode |= GR_X2; n += 2
		elif token[0][n:n+2] == 'X3': mode |= GR_X3; n += 2
		elif token[0][n:n+2] == 'X4': mode |= GR_X4; n += 2

		if   token[0][n:n+2] == 'B2': mode |= GR_B2; n += 2
		elif token[0][n:n+2] == 'B4': mode |= GR_B4; n += 2
		elif token[0][n:n+2] == 'B6': mode |= GR_B6; n += 2
		elif token[0][n:n+2] == 'B8': mode |= GR_B8; n += 2

		if n != len(token[0]):
			perror("error: %d: unexpected vertex format definitions" % linenum);

		shader.input_vertex_format = mode;


	def process_directive(text):

		start = 1
		while start < len(text) and text[start] in " \t\r\n\f": start += 1

		end = start + 1
		while end < len(text) and not text[end] in " \t\r\n\f": end += 1

		directive = lookup(text[start:end])

		if directive >= 0:

			argv  = text[end:].strip()
			token = [ s.strip() for s in argv.split(',') ] if argv != '' else []
			state = [ lookup(s) for s in token ]
			count = len(token)

			if directive == K_BLEND:     process_blend(    token, state, count)
			if directive == K_DEPTH:     process_depth(    token, state, count)
			if directive == K_INCLUDE:   process_include(  token, state, count)
			if directive == K_PRIMITIVE: process_primitive(token, state, count)
			if directive == K_RASTER:    process_raster(   token, state, count)
			if directive == K_RENDER:    process_render(   token, state, count)
			if directive == K_SECTION:   process_section(  token, state, count)
			if directive == K_SHADER:    process_shader(   token, state, count)
			if directive == K_STAGE:     process_stage(    token, state, count)
			if directive == K_STENCIL:   process_stencil(  token, state, count)
			if directive == K_VERTEX:    process_vertex(   token, state, count)
			if directive == K_WRITE:     process_write(    token, state, count)

		return directive


	pwarn("preprocessing '%s'" % source)

	with io.open(source, 'r', encoding='utf-8', errors='replace') as f:
		src = f.read()

	src  = " " + src + " "
	text = ""

	n = 0
	while n < len(src) - 2:

		n += 1

		ch  = src[n]
		pch = src[n - 1]
		cch = src[n]
		nch = src[n + 1]

		if ch != "\n" and (comment_line or comment_block):
			ch = ' '

		if comment_line  and cch == '\n':               comment_line = False
		if comment_block and cch == '/' and pch == '*': comment_block = False

		if cch == '/' and nch == '/': comment_line  = True
		if cch == '/' and nch == '*': comment_block = True

		if ch != "\n" and (comment_line or comment_block):
			ch = ' '

		if ch != "\n":
			text += ch
			continue

		linenum   += 1
		text       = text.strip()
		directive  = -1

		if len(text) > 0 and text[0] == '#':
			directive = process_directive(text)

		if directive < 0:
			if shader.active[SECTION_VERT]: shader.sections[SECTION_VERT] += text
			if shader.active[SECTION_TESC]: shader.sections[SECTION_TESC] += text
			if shader.active[SECTION_TESE]: shader.sections[SECTION_TESE] += text
			if shader.active[SECTION_GEOM]: shader.sections[SECTION_GEOM] += text
			if shader.active[SECTION_FRAG]: shader.sections[SECTION_FRAG] += text
			if shader.active[SECTION_COMP]: shader.sections[SECTION_COMP] += text

		shader.sections[SECTION_VERT] += "\n"
		shader.sections[SECTION_TESC] += "\n"
		shader.sections[SECTION_TESE] += "\n"
		shader.sections[SECTION_GEOM] += "\n"
		shader.sections[SECTION_FRAG] += "\n"
		shader.sections[SECTION_COMP] += "\n"

		text = ""

	return shader



def shader_compile(shader, compiler, options, keep):

	section_name = [ "vert",  "tesc",  "tese",  "geom",  "frag",  "comp" ]
	section_ext  = [ ".vert", ".tesc", ".tese", ".geom", ".frag", ".comp" ]

	for n in xrange(SECTION_NUM):

		if not "main" in shader.sections[n]:
			continue

		pwarn("compiling section %s" % section_name[n])

		src_file = tempfile.mktemp(section_ext[n], "shader_")
		spv_file = tempfile.mktemp(".spv",         "shader_")

		try:
			with open(src_file, 'w') as src:
				src.write(shader.sections[n])

			cmd = str(compiler)
			cmd = cmd.replace("{input}",  "\"%s\"" % src_file)
			cmd = cmd.replace("{output}", "\"%s\"" % spv_file)
			cmd = cmd.replace("{stage}",  "%s"     % section_name[n])
			cmd = cmd.replace("{flags}",  "%s"     % " ".join(options))

			pwarn("%s" % cmd)

			if subprocess.call(cmd, shell=True) == 0:

				with open(spv_file, 'r') as spv:
					shader.binary[n] = spv.read()

			else:
				perror("compiler failed")

		except Exception:
				perror("failed to execute compiler")

		finally:
			if not keep:
				os.remove(src_file)
				os.remove(spv_file)



def shader_build_metadata(shader):

	metadata  = ""
	metadata += "\n"
	metadata += ".sh %s\n" % shader.shader_name
	metadata += ".st %s\n" % shader.shader_stage
	metadata += ".rt %s\n" % shader.render_target
	metadata += "\n"
	metadata += ".vf %x\n"    % shader.input_vertex_format
	metadata += ".tp %d %d\n" % (shader.input_primitive_topology, shader.input_primitive_restart)
	metadata += "\n"
	metadata += ".rd %d\n"          % shader.raster_discard
	metadata += ".rc %d\n"          % shader.raster_cullmode
	metadata += ".db %d %f %f %f\n" % (shader.raster_depth_bias, shader.raster_depth_bias_slope, shader.raster_depth_bias_const, shader.raster_depth_bias_clamp)
	metadata += "\n"
	metadata += ".dw %d\n" % shader.depth_write
	metadata += ".df %d\n" % shader.depth_func
	metadata += "\n"
	metadata += ".sf %d %d\n" % (shader.stencil_fail[0],      shader.stencil_fail[1])
	metadata += ".sp %d %d\n" % (shader.stencil_pass[0],      shader.stencil_pass[1])
	metadata += ".sd %d %d\n" % (shader.stencil_depthfail[0], shader.stencil_depthfail[1])
	metadata += ".sc %d %d\n" % (shader.stencil_func[0],      shader.stencil_func[1])
	metadata += ".sm %d %d\n" % (shader.stencil_mask[0],      shader.stencil_mask[1])
	metadata += ".sw %d %d\n" % (shader.stencil_write[0],     shader.stencil_write[1])
	metadata += ".sr %d %d\n" % (shader.stencil_ref[0],       shader.stencil_ref[1])
	metadata += "\n"

	for a, m  in shader.color_mask:  metadata += ".wm %s %d\n" % (a, m)
	for a, bc in shader.blend_color: metadata += ".bc %s %d %d %d\n" % (a, bc[0], bc[1], bc[2])
	for a, ba in shader.blend_alpha: metadata += ".ba %s %d %d %d\n" % (a, ba[0], ba[1], ba[2])

	metadata += "\n"
	metadata += ".bl %d\n"          % shader.blend_logic_op
	metadata += ".cc %f %f %f %f\n" % (shader.blend_const[0], shader.blend_const[1], shader.blend_const[2], shader.blend_const[3])
	metadata += "\n"

	return metadata



def shader_package(shader, output):

	with open(output, 'w') as ar:

		ar.write("!<arch>\n")

		metadata     = shader_build_metadata(shader)
		metadata_len = len(metadata)

		ar.write("%-16.16s%12d%6d%6d%8d%10d`\n" % (".shader/", 0, 1000, 1000, 666, metadata_len))
		ar.write(metadata)

		if metadata_len & 1:
			ar.write("\n")

		for n, x in enumerate([ ".vert/", ".tesc/", ".tese/", ".geom/", ".frag/", ".comp/" ]):

			binary     = shader.binary[n]
			binary_len = len(binary)

			if binary_len > 0:
				ar.write("%-16.16s%12d%6d%6d%8d%10d`\n" % (x, 0, 1000, 1000, 666, binary_len))
				ar.write(binary)

				if binary_len & 1:
					ar.write("\n")


argv   = parse_command_line()
shader = shader_preprocess(shader_data(), argv.input[0], argv.include)

shader_compile(shader, argv.compiler, argv.options, argv.keep)
shader_package(shader, argv.output)

