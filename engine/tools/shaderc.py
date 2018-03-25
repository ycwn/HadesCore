#!/usr/bin/python

import argparse
import io
import os
import re
import subprocess
import sys
import tempfile


GR_V2 = 0x0000
GR_V3 = 0x0001
GR_T2 = 0x0002
GR_T3 = 0x0006
GR_C3 = 0x0008
GR_Q4 = 0x0010
GR_X2 = 0x0020
GR_X4 = 0x0060
GR_B2 = 0x0080
GR_B4 = 0x0180
GR_B6 = 0x0380
GR_B8 = 0x0780

GR_DISABLED = -1  # Generic
GR_ENABLED  =  0

GR_POINTS             = 0  # Vertex Topology
GR_LINES              = 1
GR_LINES_ADJ          = 2
GR_LINE_STRIP         = 3
GR_LINE_STRIP_ADJ     = 4
GR_TRIANGLES          = 5
GR_TRIANGLES_ADJ      = 6
GR_TRIANGLE_STRIP     = 7
GR_TRIANGLE_STRIP_ADJ = 8
GR_TRIANGLE_FAN       = 9
GR_PATCHES            = 10

GR_NONE           = 0  # Rasterizer face rendering mode
GR_FRONT          = 1
GR_BACK           = 2
GR_FRONT_AND_BACK = 3

GR_POINT = 0  # Rasterizer polygon fill mode
GR_LINE  = 1
GR_FILL  = 2

GR_NEVER    = 0  # Comparator functions
GR_LESS     = 1
GR_EQUAL    = 2
GR_LEQUAL   = 3
GR_GREATER  = 4
GR_NOTEQUAL = 7
GR_GEQUAL   = 8
GR_ALWAYS   = 9

GR_KEEP      = 0  # Stencil Ops
GR_RESET     = 1
GR_REPLACE   = 2
GR_INCR      = 3
GR_INCR_WRAP = 4
GR_DECR      = 5
GR_DECR_WRAP = 6
GR_INVERT    = 7

GR_RGB       = 0  # Color channels
GR_ALPHA     = 1
GR_RGB_ALPHA = 2

GR_R = 1 << 0  # Color components
GR_G = 1 << 1
GR_B = 1 << 2
GR_A = 1 << 3

GR_ADD     = 0  # Blending equations
GR_SUB     = 1
GR_SUB_REV = 2
GR_MIN     = 3
GR_MAX     = 4

GR_ZERO                     = 0  # Blending functions
GR_ONE                      = 1
GR_SRC_COLOR                = 2
GR_ONE_MINUS_SRC_COLOR      = 3
GR_DST_COLOR                = 4
GR_ONE_MINUS_DST_COLOR      = 5
GR_SRC_ALPHA                = 6
GR_ONE_MINUS_SRC_ALPHA      = 7
GR_DST_ALPHA                = 8
GR_ONE_MINUS_DST_ALPHA      = 9
GR_CONSTANT_COLOR           = 10
GR_ONE_MINUS_CONSTANT_COLOR = 11
GR_CONSTANT_ALPHA           = 12
GR_ONE_MINUS_CONSTANT_ALPHA = 13
GR_SRC_ALPHA_SATURATE       = 14
GR_SRC1_COLOR               = 15
GR_ONE_MINUS_SRC1_COLOR     = 16
GR_SRC1_ALPHA               = 17
GR_ONE_MINUS_SRC1_ALPHA     = 18

GR_CLEAR    = 0  # Logic Ops
GR_AND      = 1
GR_AND_REV  = 2
GR_COPY     = 3
GR_AND_INV  = 4
GR_NOP      = 5
GR_XOR      = 6
GR_OR       = 7
GR_NOR      = 8
GR_EQV      = 9
GR_INV      = 10
GR_OR_REV   = 11
GR_COPY_INV = 12
GR_OR_INV   = 13
GR_NAND     = 14
GR_SET      = 15

GR_SHADER_VERTEX           = 0  # Shader stages
GR_SHADER_TESSELATION_CTRL = 1
GR_SHADER_TESSELATION_EVAL = 2
GR_SHADER_GEOMETRY         = 3
GR_SHADER_FRAGMENT         = 4
GR_SHADER_COMPUTE          = 5
GR_SHADER_MAX              = 6



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
	_ = {   K_DISABLED:         GR_DISABLED,
		K_ADD:              GR_ADD,
		K_SUBTRACT:         GR_SUB,
		K_REVERSE_SUBTRACT: GR_SUB_REV,
		K_MIN:              GR_MIN,
		K_MAX:              GR_MAX
	}
	return _[x] if x in _ else -1


def BLEND_SRC(x):
	_ = {   K_DISABLED:              GR_DISABLED,
		K_CONST_ALPHA:           GR_CONSTANT_ALPHA,
		K_CONST_COLOR:           GR_CONSTANT_COLOR,
		K_DST_ALPHA:             GR_DST_ALPHA,
		K_DST_COLOR:             GR_DST_COLOR,
		K_ONE:                   GR_ONE,
		K_ONE_MINUS_CONST_ALPHA: GR_ONE_MINUS_CONSTANT_ALPHA,
		K_ONE_MINUS_CONST_COLOR: GR_ONE_MINUS_CONSTANT_COLOR,
		K_ONE_MINUS_DST_ALPHA:   GR_ONE_MINUS_DST_ALPHA,
		K_ONE_MINUS_DST_COLOR:   GR_ONE_MINUS_DST_COLOR,
		K_ONE_MINUS_SRC_ALPHA:   GR_ONE_MINUS_SRC_ALPHA,
		K_ONE_MINUS_SRC_COLOR:   GR_ONE_MINUS_SRC_COLOR,
		K_SRC_ALPHA:             GR_SRC_ALPHA,
		K_SRC_ALPHA_SATURATE:    GR_SRC_ALPHA_SATURATE,
		K_SRC_COLOR:             GR_SRC_COLOR,
		K_ZERO:                  GR_ZERO
	}
	return _[x] if x in _ else -1


def BLEND_DST(x):
	_ = {   K_DISABLED:              GR_DISABLED,
		K_CONST_ALPHA:           GR_CONSTANT_ALPHA,
		K_CONST_COLOR:           GR_CONSTANT_COLOR,
		K_DST_ALPHA:             GR_DST_ALPHA,
		K_DST_COLOR:             GR_DST_COLOR,
		K_ONE:                   GR_ONE,
		K_ONE_MINUS_CONST_ALPHA: GR_ONE_MINUS_CONSTANT_ALPHA,
		K_ONE_MINUS_CONST_COLOR: GR_ONE_MINUS_CONSTANT_COLOR,
		K_ONE_MINUS_DST_ALPHA:   GR_ONE_MINUS_DST_ALPHA,
		K_ONE_MINUS_DST_COLOR:   GR_ONE_MINUS_DST_COLOR,
		K_ONE_MINUS_SRC_ALPHA:   GR_ONE_MINUS_SRC_ALPHA,
		K_ONE_MINUS_SRC_COLOR:   GR_ONE_MINUS_SRC_COLOR,
		K_SRC_ALPHA:             GR_SRC_ALPHA,
		K_SRC_COLOR:             GR_SRC_COLOR,
		K_ZERO:                  GR_ZERO
	}
	return _[x] if x in _ else -1


def BLEND_LOGICOP(x):
	_ = {   K_LOGIC_OFF:    GR_DISABLED,
		K_LOGIC_CLEAR:  GR_CLEAR,
		K_LOGIC_AND:    GR_AND,
		K_LOGIC_ANDREV: GR_AND_REV,
		K_LOGIC_BLT:    GR_COPY,
		K_LOGIC_ANDINV: GR_AND_INV,
		K_LOGIC_NOP:    GR_NOP,
		K_LOGIC_XOR:    GR_XOR,
		K_LOGIC_OR:     GR_OR,
		K_LOGIC_NOR:    GR_NOR,
		K_LOGIC_EQV:    GR_EQV,
		K_LOGIC_INV:    GR_INV,
		K_LOGIC_ORREV:  GR_OR_REV,
		K_LOGIC_BLTINV: GR_COPY_INV,
		K_LOGIC_ORINV:  GR_OR_INV,
		K_LOGIC_NAND:   GR_NAND,
		K_LOGIC_SET:    GR_SET
	}
	return _[x] if x in _ else -1


def DEPTH_MODE(x):
	_ = {   K_DISABLED: GR_DISABLED,
		K_NEVER:    GR_NEVER,
		K_LESS:     GR_LESS,
		K_EQUAL:    GR_EQUAL,
		K_LEQUAL:   GR_LEQUAL,
		K_GREATER:  GR_GREATER,
		K_NEQUAL:   GR_NOTEQUAL,
		K_GEQUAL:   GR_GEQUAL,
		K_ALWAYS:   GR_ALWAYS
	}
	return _[x] if x in _ else -1


def PRIMITIVE_TOPOLOGY(x):
	_ = {   K_POINTS:        GR_POINTS,
		K_LINES:         GR_LINES,
		K_LINESTRIP:     GR_LINE_STRIP,
		K_TRIANGLES:     GR_TRIANGLES,
		K_TRISTRIP:      GR_TRIANGLE_STRIP,
		K_TRIFAN:        GR_TRIANGLE_FAN,
		K_LINES_ADJ:     GR_LINES_ADJ,
		K_LINESTRIP_ADJ: GR_LINE_STRIP_ADJ,
		K_TRIANGLES_ADJ: GR_TRIANGLES_ADJ,
		K_TRISTRIP_ADJ:  GR_TRIANGLE_STRIP_ADJ,
		K_PATCHES:       GR_PATCHES
	}
	return _[x] if x in _ else -1


def RASTER_MODE(x):
	_ = {   K_NONE:  GR_NONE,
		K_FRONT: GR_FRONT,
		K_BACK:  GR_BACK,
		K_BOTH:  GR_FRONT_AND_BACK
	}
	return _[x] if x in _ else -1


def SECTION_MASK(x):
	_ = {   K_DOT_VERTEX:   GR_SHADER_VERTEX,
		K_DOT_TESSCTRL: GR_SHADER_TESSELATION_CTRL,
		K_DOT_TESSEVAL: GR_SHADER_TESSELATION_EVAL,
		K_DOT_GEOMETRY: GR_SHADER_GEOMETRY,
		K_DOT_FRAGMENT: GR_SHADER_FRAGMENT,
		K_DOT_COMPUTE:  GR_SHADER_COMPUTE
	}
	return _[x] if x in _ else -1


def STENCIL_MODE(x): return DEPTH_MODE(x)
def STENCIL_OP(x):
	_ = {   K_KEEP:    GR_KEEP,
		K_ZERO:    GR_RESET,
		K_REPLACE: GR_REPLACE,
		K_INCWRAP: GR_INCR,
		K_DECWRAP: GR_DECR,
		K_INVERT:  GR_INVERT,
		K_INC:     GR_INCR_WRAP,
		K_DEC:     GR_DECR_WRAP
	}
	return _[x] if x in _ else -1


def WRITE_CHANNEL(x):
	_ = {   K_RED_ON:    GR_R,
		K_RED_OFF:   GR_R,
		K_GREEN_ON:  GR_G,
		K_GREEN_OFF: GR_G,
		K_BLUE_ON:   GR_B,
		K_BLUE_OFF:  GR_B,
		K_ALPHA_ON:  GR_A,
		K_ALPHA_OFF: GR_A,
		K_COLOR_ON:  GR_R | GR_G | GR_B,
		K_COLOR_OFF: GR_R | GR_G | GR_B

	}
	return _[x] if x in _ else -1


def WRITE_MODE(x):
	_ = {   K_RED_ON:   1, K_RED_OFF:   0,
		K_GREEN_ON: 1, K_GREEN_OFF: 0,
		K_BLUE_ON:  1, K_BLUE_OFF:  0,
		K_ALPHA_ON: 1, K_ALPHA_OFF: 0,
		K_COLOR_ON: 1, K_COLOR_OFF: 0
	}
	return _[x] if x in _ else -1



class shader_data:

	sections = [ ]
	module   = [ ]
	active   = [ ]
	metadata = ""

	shader_name   = None
	shader_stage  = 500
	render_target = "framebuffer"

	# Input assembler control
	input_vertex_format      = -1
	input_primitive_topology = -1
	input_primitive_restart  = 0

	# Rasterizer control
	raster_discard          = 0
	raster_cullmode         = RASTER_MODE(K_BOTH)
	raster_depth_bias       = 0
	raster_depth_bias_const = 0.0
	raster_depth_bias_slope = 0.0
	raster_depth_bias_clamp = 0.0

	# Depth testing control
	depth_write = 1
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
		self.sections = [ ""   for n in xrange(GR_SHADER_MAX) ]
		self.module   = [ ""   for n in xrange(GR_SHADER_MAX) ]
		self.active   = [ True for n in xrange(GR_SHADER_MAX) ]



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
			shader.active = [ True for n in xrange(GR_SHADER_MAX) ]

		else:
			shader.active = [ False for n in xrange(GR_SHADER_MAX) ]

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
			shader.depth_write = 1

		elif state[0] == K_DEPTH_OFF:
			shader.depth_write = 0

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


	#vertex V2|V3[T2|T3][C3][Q4][X2|X4][B2|B4|B6|B8]
	def process_vertex(token, state, count):

		n    = 0
		mode = 0

		if   token[0][n:n+2] == 'V2': mode |= GR_V2; n += 2
		elif token[0][n:n+2] == 'V3': mode |= GR_V3; n += 2

		if   token[0][n:n+2] == 'T2': mode |= GR_T2; n += 2
		elif token[0][n:n+2] == 'T3': mode |= GR_T3; n += 2

		if token[0][n:n+2] == 'C3': mode |= GR_C3; n += 2
		if token[0][n:n+2] == 'Q4': mode |= GR_Q4; n += 2

		if   token[0][n:n+2] == 'X2': mode |= GR_X2; n += 2
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
			if shader.active[GR_SHADER_VERTEX]:           shader.sections[GR_SHADER_VERTEX]           += text
			if shader.active[GR_SHADER_TESSELATION_CTRL]: shader.sections[GR_SHADER_TESSELATION_CTRL] += text
			if shader.active[GR_SHADER_TESSELATION_EVAL]: shader.sections[GR_SHADER_TESSELATION_EVAL] += text
			if shader.active[GR_SHADER_GEOMETRY]:         shader.sections[GR_SHADER_GEOMETRY]         += text
			if shader.active[GR_SHADER_FRAGMENT]:         shader.sections[GR_SHADER_FRAGMENT]         += text
			if shader.active[GR_SHADER_COMPUTE]:          shader.sections[GR_SHADER_COMPUTE]          += text

		shader.sections[GR_SHADER_VERTEX]           += "\n"
		shader.sections[GR_SHADER_TESSELATION_CTRL] += "\n"
		shader.sections[GR_SHADER_TESSELATION_EVAL] += "\n"
		shader.sections[GR_SHADER_GEOMETRY]         += "\n"
		shader.sections[GR_SHADER_FRAGMENT]         += "\n"
		shader.sections[GR_SHADER_COMPUTE]          += "\n"

		text = ""

	return shader



def shader_compile(shader, compiler, options, keep):

	def comment_replacer(match):
		s = match.group(0)
		return " " if s.startswith('/') else s

	comment_pattern = re.compile(
		r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
		re.DOTALL | re.MULTILINE)

	section_name = [ "vert",  "tesc",  "tese",  "geom",  "frag",  "comp" ]
	section_ext  = [ ".vert", ".tesc", ".tese", ".geom", ".frag", ".comp" ]

	for n in xrange(GR_SHADER_MAX):

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

				spirv = ""

				with open(spv_file, 'r') as spv:
					 for line in re.sub(comment_pattern, comment_replacer, spv.read()).split("\n"):
						line = line.strip()
						if line:
							spirv += line + "\n"

				shader.module[n] = spirv.strip()

			else:
				perror("compiler failed")

		except Exception:
				perror("failed to execute compiler")

		finally:
			if not keep:
				os.remove(src_file)
				os.remove(spv_file)



def shader_package(shader, output):

	data = [
		".sh %s %d" % (shader.shader_name, shader.shader_stage),

		".vf %d"    % shader.input_vertex_format,
		".vt %d %d" % (shader.input_primitive_topology, shader.input_primitive_restart),

		".rp %s"    % shader.render_target,
		".rm %d %d" % (shader.raster_cullmode, GR_FILL),

		".db %d %f %f %f" % (shader.raster_depth_bias, shader.raster_depth_bias_slope, shader.raster_depth_bias_const, shader.raster_depth_bias_clamp),
		".dw %d"          % shader.depth_write,
		".df %d"          % shader.depth_func,

		".so %d %d %d %d" % (GR_FRONT, shader.stencil_fail[0], shader.stencil_pass[0], shader.stencil_depthfail[0]),
		".sf %d %d %d"    % (GR_FRONT, shader.stencil_func[0], shader.stencil_mask[0]),
		".sw %d %d"       % (GR_FRONT, shader.stencil_write[0]),
		".sr %d %d"       % (GR_FRONT, shader.stencil_ref[0]),

		".so %d %d %d %d" % (GR_BACK,  shader.stencil_fail[1], shader.stencil_pass[1], shader.stencil_depthfail[1]),
		".sf %d %d %d"    % (GR_BACK,  shader.stencil_func[1], shader.stencil_mask[1]),
		".sw %d %d"       % (GR_BACK,  shader.stencil_write[1]),
		".sr %d %d"       % (GR_BACK,  shader.stencil_ref[1])
	] + [
		".cm %s %d" % (a, m) for a, m in shader.color_mask
	] + [
		".bf %s %d %d %d %d" % (a, GR_RGB,   bc[0], bc[1], bc[2]) for a, bc in shader.blend_color.iteritems()
	] + [
		".bf %s %d %d %d %d" % (a, GR_ALPHA, ba[0], ba[1], ba[2]) for a, ba in shader.blend_alpha.iteritems()
	] + [
		".op %d"          % shader.blend_logic_op,
		".cc %f %f %f %f" % (shader.blend_const[0], shader.blend_const[1], shader.blend_const[2], shader.blend_const[3])
	] + [
		".sp %d %d\n%s" % (n, shader.module[n].count(",") + 1, shader.module[n]) for n in xrange(GR_SHADER_MAX) if len(shader.module[n]) > 0
	]

	with open(output, 'w') as ar:
		ar.write("\n".join(data))



argv   = parse_command_line()
shader = shader_preprocess(shader_data(), argv.input[0], argv.include)

shader_compile(shader, argv.compiler, argv.options, argv.keep)
shader_package(shader, argv.output)

