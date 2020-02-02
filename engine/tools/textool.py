#!/usr/bin/python

import argparse
import os
import sys
import struct

from PIL import Image
import numpy as np



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


def pack_unorm8(v): return int(min(max(v * 255, 0), 255))
def pack_snorm8(v): return int(min(max(v * 127.5 * x - 0.5, -128), 127))
def pack_uint8(v):  return int(min(max(v, 0), 255))
def pack_sint8(v):  return int(min(max(v, -128), 127))

def pack_unorm16(v): return int(min(max(v * 65535, 0), 65535))
def pack_snorm16(v): return int(min(max(v * 32767.5 * x - 0.5, -32768), 32767))
def pack_uint16(v):  return int(min(max(v, 0), 65535))
def pack_sint16(v):  return int(min(max(v, -32768), 32767))

def pack_unorm32(v): return int(min(max(v * 4294967295, 0), 4294967295))
def pack_snorm32(v): return int(min(max(v * 2147483647.5 * x - 0.5, -2147483648), 2147483647))
def pack_uint32(v):  return int(min(max(v, 0), 4294967295))
def pack_sint32(v):  return int(min(max(v, -2147483648), 2147483647))

def pack_unorm64(v): return int(min(max(v * 18446744073709551615, 0), 18446744073709551615))
def pack_snorm64(v): return int(min(max(v * 9223372036854775807.5 * x - 0.5, -9223372036854775808), 9223372036854775807))
def pack_uint64(v):  return int(min(max(v, 0), 18446744073709551615))
def pack_sint64(v):  return int(min(max(v, -9223372036854775808), 9223372036854775807))


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



PF_VULKAN  = 0
PF_DXGI    = 1
PF_PACKING = 2
PF_BPP     = 3
PF_BLKW    = 4
PF_BLKH    = 5
PF_FUNC    = 6

PIXELFORMATS = {
	'r4g4-unorm':           [   1,   0, 'L',   8, 1, 1 ],
	'r4g4b4a4-unorm':       [   2,   0, 'L',  16, 1, 1 ],
	'b4g4r4a4-unorm':       [   3, 115, 'L',  16, 1, 1 ],
	'r5g6b5-unorm':         [   4,   0, 'L',  16, 1, 1 ],
	'b5g6r5-unorm':         [   5,  85, 'L',  16, 1, 1 ],
	'r5g5b5a1-unorm':       [   6,   0, 'L',  16, 1, 1 ],
	'b5g5r5a1-unorm':       [   7,  86, 'L',  16, 1, 1 ],
	'a1r5g5b5-unorm':       [   8,   0, 'L',  16, 1, 1 ],
	'r8-unorm':             [   9,  61, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_unorm8(x[0][0][0])) ],
	'r8-snorm':             [  10,  63, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_snorm8(x[0][0][0])) ],
	'r8-uscaled':           [  11,   0, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_uint8(x[0][0][0]))  ],
	'r8-sscaled':           [  12,   0, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_sint8(x[0][0][0]))  ],
	'r8-uint':              [  13,  62, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_uint8(x[0][0][0]))  ],
	'r8-sint':              [  14,  64, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_sint8(x[0][0][0]))  ],
	'r8-srgb':              [  15,   0, 'L',   8, 1, 1, lambda x: struct.pack("=B",    pack_unorm8(x[0][0][0])) ],
	'r8g8-unorm':           [  16,  49, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1])) ],
	'r8g8-snorm':           [  17,  51, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_snorm8(x[0][0][0]), pack_snorm8(x[0][0][1])) ],
	'r8g8-uscaled':         [  16,   0, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][1]))  ],
	'r8g8-sscaled':         [  19,   0, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]))  ],
	'r8g8-uint':            [  20,  50, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_uint8(x[0][0][0]),  pack_sint8(x[0][0][1]))  ],
	'r8g8-sint':            [  21,  52, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]))  ],
	'r8g8-srgb':            [  22,   0, 'L',  16, 1, 1, lambda x: struct.pack("=BB",   pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1])) ],
	'r8g8b8-unorm':         [  23,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][2])) ],
	'r8g8b8-snorm':         [  24,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_snorm8(x[0][0][0]), pack_snorm8(x[0][0][1]), pack_snorm8(x[0][0][2])) ],
	'r8g8b8-uscaled':       [  25,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][2]))  ],
	'r8g8b8-sscaled':       [  26,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][2]))  ],
	'r8g8b8-uint':          [  27,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][2]))  ],
	'r8g8b8-sint':          [  28,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][2]))  ],
	'r8g8b8-srgb':          [  29,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][2])) ],
	'b8g8r8-unorm':         [  30,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0])) ],
	'b8g8r8-snorm':         [  31,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_snorm8(x[0][0][2]), pack_snorm8(x[0][0][1]), pack_snorm8(x[0][0][0])) ],
	'b8g8r8-uscaled':       [  32,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]))  ],
	'b8g8r8-sscaled':       [  33,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]))  ],
	'b8g8r8-uint':          [  34,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]))  ],
	'b8g8r8-sint':          [  35,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]))  ],
	'b8g8r8-srgb':          [  36,   0, 'L',  24, 1, 1, lambda x: struct.pack("=BBB",  pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0])) ],
	'r8g8b8a8-unorm':       [  37,  28, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][3])) ],
	'r8g8b8a8-snorm':       [  38,  31, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_snorm8(x[0][0][0]), pack_snorm8(x[0][0][1]), pack_snorm8(x[0][0][2]), pack_snorm8(x[0][0][3])) ],
	'r8g8b8a8-uscaled':     [  39,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][3]))  ],
	'r8g8b8a8-sscaled':     [  40,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][3]))  ],
	'r8g8b8a8-uint':        [  41,  30, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][3]))  ],
	'r8g8b8a8-sint':        [  42,  32, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][3]))  ],
	'r8g8b8a8-srgb':        [  43,  29, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][3])) ],
	'b8g8r8a8-unorm':       [  44,  87, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][3])) ],
	'b8g8r8a8-snorm':       [  45,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_snorm8(x[0][0][2]), pack_snorm8(x[0][0][1]), pack_snorm8(x[0][0][0]), pack_snorm8(x[0][0][3])) ],
	'b8g8r8a8-uscaled':     [  46,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][3]))  ],
	'b8g8r8a8-sscaled':     [  47,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][3]))  ],
	'b8g8r8a8-uint':        [  48,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]),  pack_uint8(x[0][0][3]))  ],
	'b8g8r8a8-sint':        [  49,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]),  pack_sint8(x[0][0][3]))  ],
	'b8g8r8a8-srgb':        [  50,  91, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0]), pack_unorm8(x[0][0][3])) ],
	'a8b8g8r8-unorm':       [  51,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][3]), pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0])) ],
	'a8b8g8r8-snorm':       [  52,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_snorm8(x[0][0][3]), pack_snorm8(x[0][0][2]), pack_snorm8(x[0][0][1]), pack_snorm8(x[0][0][0])) ],
	'a8b8g8r8-uscaled':     [  53,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][3]),  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]))  ],
	'a8b8g8r8-sscaled':     [  54,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][3]),  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]))  ],
	'a8b8g8r8-uint':        [  55,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_uint8(x[0][0][3]),  pack_uint8(x[0][0][2]),  pack_uint8(x[0][0][1]),  pack_uint8(x[0][0][0]))  ],
	'a8b8g8r8-sint':        [  56,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_sint8(x[0][0][3]),  pack_sint8(x[0][0][2]),  pack_sint8(x[0][0][1]),  pack_sint8(x[0][0][0]))  ],
	'a8b8g8r8-srgb':        [  57,   0, 'L',  32, 1, 1, lambda x: struct.pack("=BBBB", pack_unorm8(x[0][0][3]), pack_unorm8(x[0][0][2]), pack_unorm8(x[0][0][1]), pack_unorm8(x[0][0][0])) ],
	'a2r10g10b10-unorm':    [  58,   0, 'L',  32, 1, 1 ],
	'a2r10g10b10-snorm':    [  59,   0, 'L',  32, 1, 1 ],
	'a2r10g10b10-uscaled':  [  60,   0, 'L',  32, 1, 1 ],
	'a2r10g10b10-sscaled':  [  61,   0, 'L',  32, 1, 1 ],
	'a2r10g10b10-uint':     [  62,   0, 'L',  32, 1, 1 ],
	'a2r10g10b10-sint':     [  63,   0, 'L',  32, 1, 1 ],
	'a2b10g10r10-unorm':    [  64,  24, 'L',  32, 1, 1 ],
	'a2b10g10r10-snorm':    [  65,   0, 'L',  32, 1, 1 ],
	'a2b10g10r10-uscaled':  [  66,   0, 'L',  32, 1, 1 ],
	'a2b10g10r10-sscaled':  [  67,   0, 'L',  32, 1, 1 ],
	'a2b10g10r10-uint':     [  68,  25, 'L',  32, 1, 1 ],
	'a2b10g10r10-sint':     [  69,   0, 'L',  32, 1, 1 ],
	'r16-unorm':            [  70,  56, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_unorm16(x[0][0][0])) ],
	'r16-snorm':            [  71,  58, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_snorm16(x[0][0][0])) ],
	'r16-uscaled':          [  72,   0, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_uint16(x[0][0][0]))  ],
	'r16-sscaled':          [  73,   0, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_sint16(x[0][0][0]))  ],
	'r16-uint':             [  74,  57, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_uint16(x[0][0][0]))  ],
	'r16-sint':             [  75,  59, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_sint16(x[0][0][0]))  ],
	'r16-sfloat':           [  76,  54, 'L',  16, 1, 1, lambda x: struct.pack("=H",    pack_fp16(x[0][0][0]))    ],
	'r16g16-unorm':         [  77,  35, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_unorm16(x[0][0][0]), pack_unorm16(x[0][0][1])) ],
	'r16g16-snorm':         [  78,  37, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_snorm16(x[0][0][0]), pack_snorm16(x[0][0][1])) ],
	'r16g16-uscaled':       [  79,   0, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]))  ],
	'r16g16-sscaled':       [  80,   0, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]))  ],
	'r16g16-uint':          [  81,  36, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]))  ],
	'r16g16-sint':          [  82,  38, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]))  ],
	'r16g16-sfloat':        [  83,  34, 'L',  32, 1, 1, lambda x: struct.pack("=HH",   pack_fp16(x[0][0][0]),    pack_fp16(x[0][0][1]))    ],
	'r16g16b16-unorm':      [  84,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_unorm16(x[0][0][0]), pack_unorm16(x[0][0][1]), pack_unorm16(x[0][0][2])) ],
	'r16g16b16-snorm':      [  85,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_snorm16(x[0][0][0]), pack_snorm16(x[0][0][1]), pack_snorm16(x[0][0][2])) ],
	'r16g16b16-uscaled':    [  86,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]),  pack_uint16(x[0][0][2]))  ],
	'r16g16b16-sscaled':    [  87,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]),  pack_sint16(x[0][0][2]))  ],
	'r16g16b16-uint':       [  88,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]),  pack_uint16(x[0][0][2]))  ],
	'r16g16b16-sint':       [  89,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]),  pack_sint16(x[0][0][2]))  ],
	'r16g16b16-sfloat':     [  90,   0, 'L',  48, 1, 1, lambda x: struct.pack("=HHH",  pack_fp16(x[0][0][0]),    pack_fp16(x[0][0][1]),    pack_fp16(x[0][0][2]))    ],
	'r16g16b16a16-unorm':   [  91,  11, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_unorm16(x[0][0][0]), pack_unorm16(x[0][0][1]), pack_unorm16(x[0][0][2]), pack_unorm16(x[0][0][3])) ],
	'r16g16b16a16-snorm':   [  92,  13, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_snorm16(x[0][0][0]), pack_snorm16(x[0][0][1]), pack_snorm16(x[0][0][2]), pack_snorm16(x[0][0][3])) ],
	'r16g16b16a16-uscaled': [  93,   0, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]),  pack_uint16(x[0][0][2]),  pack_uint16(x[0][0][3]))  ],
	'r16g16b16a16-sscaled': [  94,   0, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]),  pack_sint16(x[0][0][2]),  pack_sint16(x[0][0][3]))  ],
	'r16g16b16a16-uint':    [  95,  12, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_uint16(x[0][0][0]),  pack_uint16(x[0][0][1]),  pack_uint16(x[0][0][2]),  pack_uint16(x[0][0][3]))  ],
	'r16g16b16a16-sint':    [  96,  14, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_sint16(x[0][0][0]),  pack_sint16(x[0][0][1]),  pack_sint16(x[0][0][2]),  pack_sint16(x[0][0][3]))  ],
	'r16g16b16a16-sfloat':  [  97,  10, 'L',  64, 1, 1, lambda x: struct.pack("=HHHH", pack_fp16(x[0][0][0]),    pack_fp16(x[0][0][1]),    pack_fp16(x[0][0][2]),    pack_fp16(x[0][0][3]))     ],
	'r32-uint':             [  98,  42, 'L',  32, 1, 1, lambda x: struct.pack("=I",    pack_uint32(x[0][0][0])) ],
	'r32-sint':             [  99,  43, 'L',  32, 1, 1, lambda x: struct.pack("=I",    pack_sint32(x[0][0][0])) ],
	'r32-sfloat':           [ 100,  41, 'L',  32, 1, 1, lambda x: struct.pack("=f",    x[0][0][0]) ],
	'r32g32-uint':          [ 101,  17, 'L',  64, 1, 1, lambda x: struct.pack("=II",   pack_uint32(x[0][0][0]),  pack_uint32(x[0][0][1])) ],
	'r32g32-sint':          [ 102,  18, 'L',  64, 1, 1, lambda x: struct.pack("=II",   pack_sint32(x[0][0][0]),  pack_sint323(x[0][0][1])) ],
	'r32g32-sfloat':        [ 103,  16, 'L',  64, 1, 1, lambda x: struct.pack("=ff",   x[0][0][0], x[0][0][1]) ],
	'r32g32b32-uint':       [ 104,   7, 'L',  96, 1, 1, lambda x: struct.pack("=III",  pack_uint32(x[0][0][0]),  pack_uint32(x[0][0][1]), pack_uint32(x[0][0][2])) ],
	'r32g32b32-sint':       [ 105,   8, 'L',  96, 1, 1, lambda x: struct.pack("=III",  pack_sint32(x[0][0][0]),  pack_sint32(x[0][0][1]), pack_sint32(x[0][0][2])) ],
	'r32g32b32-sfloat':     [ 106,   6, 'L',  96, 1, 1, lambda x: struct.pack("=fff",  x[0][0][0], x[0][0][1], x[0][0][2]) ],
	'r32g32b32a32-uint':    [ 106,   3, 'L', 128, 1, 1, lambda x: struct.pack("=IIII", pack_uint32(x[0][0][0]),  pack_uint32(x[0][0][1]), pack_uint32(x[0][0][2]), pack_uint32(x[0][0][3])) ],
	'r32g32b32a32-sint':    [ 108,   4, 'L', 128, 1, 1, lambda x: struct.pack("=IIII", pack_sint32(x[0][0][0]),  pack_sint32(x[0][0][1]), pack_sint32(x[0][0][2]), pack_sint32(x[0][0][3])) ],
	'r32g32b32a32-sfloat':  [ 109,   2, 'L', 128, 1, 1, lambda x: struct.pack("=ffff", x[0][0][0], x[0][0][1], x[0][0][2], x[0][0][3]) ],
	'r64-uint':             [ 110,   0, 'L',  64, 1, 1, lambda x: struct.pack("=Q",    pack_uint64(x[0][0][0])) ],
	'r64-sint':             [ 111,   0, 'L',  64, 1, 1, lambda x: struct.pack("=Q",    pack_sint64(x[0][0][0])) ],
	'r64-sfloat':           [ 112,   0, 'L',  64, 1, 1, lambda x: struct.pack("=d",    x[0][0][0]) ],
	'r64g64-uint':          [ 113,   0, 'L', 128, 1, 1, lambda x: struct.pack("=QQ",   pack_uint64(x[0][0][0]),  pack_uint64(x[0][0][1])) ],
	'r64g64-sint':          [ 114,   0, 'L', 128, 1, 1, lambda x: struct.pack("=QQ",   pack_sint64(x[0][0][0]),  pack_sint64(x[0][0][1])) ],
	'r64g64-sfloat':        [ 115,   0, 'L', 128, 1, 1, lambda x: struct.pack("=dd",   x[0][0][0], x[0][0][1]) ],
	'r64g64b64-uint':       [ 116,   0, 'L', 192, 1, 1, lambda x: struct.pack("=QQQ",  pack_uint64(x[0][0][0]),  pack_uint64(x[0][0][1]), pack_uint64(x[0][0][2])) ],
	'r64g64b64-sint':       [ 117,   0, 'L', 192, 1, 1, lambda x: struct.pack("=QQQ",  pack_sint64(x[0][0][0]),  pack_sint64(x[0][0][1]), pack_sint64(x[0][0][2])) ],
	'r64g64b64-sfloat':     [ 118,   0, 'L', 192, 1, 1, lambda x: struct.pack("=ddd",  x[0][0][0], x[0][0][1], x[0][0][2]) ],
	'r64g64b64a64-uint':    [ 119,   0, 'L', 256, 1, 1, lambda x: struct.pack("=QQQQ", pack_uint64(x[0][0][0]),  pack_uint64(x[0][0][1]), pack_uint64(x[0][0][2]), pack_uint64(x[0][0][3])) ],
	'r64g64b64a64-sint':    [ 120,   0, 'L', 256, 1, 1, lambda x: struct.pack("=QQQQ", pack_sint64(x[0][0][0]),  pack_sint64(x[0][0][1]), pack_sint64(x[0][0][2]), pack_sint64(x[0][0][3])) ],
	'r64g64b64a64-sfloat':  [ 121,   0, 'L', 256, 1, 1, lambda x: struct.pack("=dddd", x[0][0][0], x[0][0][1], x[0][0][2], x[0][0][3]) ],
	'b10g11r11-ufloat':     [ 122,  26, 'L',  32, 1, 1 ],
#	'e5b9g9r9-ufloat':
#	'bc1-rgb-unorm':
#	'bc1-rgb-srgb':
#	'bc1-rgba-unorm':
#	'bc1-rgba-srgb':
#	'bc2-unorm':
#	'bc2-srgb':
#	'bc3-unorm':
#	'bc3-srgb':
#	'bc4-unorm':
#	'bc4-snorm':
#	'bc5-unorm':
#	'bc5-snorm':
#	'bc6h-ufloat':
#	'bc6h-sfloat':
#	'bc7-unorm':
#	'bc7-srgb':
#	'etc2-r8g8b8-unorm':
#	'etc2-r8g8b8-srgb':
#	'etc2-r8g8b8a1-unorm':
#	'etc2-r8g8b8a1-srgb':
#	'etc2-r8g8b8a8-unorm':
#	'etc2-r8g8b8a8-srgb':
#	'eac-r11-unorm':
#	'eac-r11-snorm':
#	'eac-r11g11-unorm':
#	'eac-r11g11-snorm':
#	'astc-4x4-unorm':
#	'astc-4x4-srgb':
#	'astc-5x4-unorm':
#	'astc-5x4-srgb':
#	'astc-5x5-unorm':
#	'astc-5x5-srgb':
#	'astc-6x5-unorm':
#	'astc-6x5-srgb':
#	'astc-6x6-unorm':
#	'astc-6x6-srgb':
#	'astc-8x5-unorm':
#	'astc-8x5-srgb':
#	'astc-8x6-unorm':
#	'astc-8x6-srgb':
#	'astc-8x8-unorm':
#	'astc-8x8-srgb':
#	'astc-10x5-unorm':
#	'astc-10x5-srgb':
#	'astc-10x6-unorm':
#	'astc-10x6-srgb':
#	'astc-10x8-unorm':
#	'astc-10x8-srgb':
#	'astc-10x10-unorm':
#	'astc-10x10-srgb':
#	'astc-12x10-unorm':
#	'astc-12x10-srgb':
#	'astc-12x12-unorm':
#	'astc-12x12-srgb':
}



def perror(str):
	sys.stderr.write("textool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("textool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Process and compress textures")
	argp.add_argument('-m',  '--merge',            action='store_true', help='Combine textures into one single file')
	argp.add_argument('-c',  '--cubemap',          action='store_true', help='Create cubemap, implies merge')
	argp.add_argument('-g',  '--generate-mipmaps', action='store_true', help='Generate mipmaps')
	argp.add_argument('-l',  '--list-formats',     action='store_true', help='List supported formats, then exit')
	argp.add_argument('-f',  '--format' ,          action='store',      type=str, help='Select output format (r16g16b16a16-sfloat)')
	argp.add_argument('-S',  '--reshape',          action='store',      type=str, help='Reshape image to w:h:d:l')
	argp.add_argument('-s',  '--scale',            action='store',      type=str, help='Multiply all pixels by r:g:b:a')
	argp.add_argument('-b',  '--bias',             action='store',      type=str, help='Add r:g:b:a to all pixels')
	argp.add_argument('-o',  '--output',           action='store',      type=str, help='Output file ({input}.dds)')
	argp.add_argument('-C',  '--directory',        action='store',      type=str, help='Set working directory')
	argp.add_argument('images', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.format:
		argv.format = 'r16g16b16a16-sfloat'

	if not argv.directory:
		argv.directory = ""

	if not argv.output:
		argv.output = "{input}.dds"

	if not argv.list_formats and len(argv.images) < 1:
		perror("No input files specified")

	return argv



def image_load(images):

	for image in images:

		pwarn("Processing %s..." % image)

		img = Image.open(image)
		img.load()

		pixels   = np.asarray(img, dtype='float32')
		channels = pixels.shape[2] if len(pixels.shape) > 2 else 1
		pixels   = pixels.reshape((pixels.shape[0], pixels.shape[1], 1, 1, channels))

		if channels < 4:
			pixels = np.pad(pixels, ((0,0),(0,0),(0,0),(0,0),(0, 4 - channels)), mode='constant', constant_values=0)

		if img.mode != 'F':
			pixels /= 255.0

		yield image, pixels



def image_load_combined(images):
	return []



def image_resize_half(pixels):
	return pixels



def image_convert(pixels, fmt, mipmaps):

	data = []

	for l in xrange(pixels.shape[3]):

		slice = ""

		for z in xrange(pixels.shape[2]):
			for y in xrange(0, pixels.shape[1], fmt[PF_BLKH]):
				for x in xrange(0, pixels.shape[0], fmt[PF_BLKW]):
					slice += fmt[PF_FUNC](pixels[y:y+fmt[PF_BLKH],x:x+fmt[PF_BLKW],z,l,:])

		data.append([ slice ])

	return data



def dds_write(output, pixels, shape, fmt, cubemap):

	pwarn("Writing to '%s'..." % output)

	try:
		out = open(output, 'wb')

	except:
		perror("Failed to open output file!")


	flags   = 0x1007      # DDS_CAPS, DDS_WIDTH, DDS_HEIGHT, DDS_PF
	caps0   = 0x00001008  # DDSCAPS_COMPLEX, DDSCAPS_TEXTURE
	caps1   = 0
	flags10 = 0
	dims    = 2
	mipmaps = len(pixels[0])

	if fmt[PF_PACKING] == 'L': pitch = int((shape[1] * fmt[PF_BPP] + 7) / 8)
	if fmt[PF_PACKING] == 'B': pitch = max(int((shape[1] + 3) / 4), 1) * fmt[PF_BPP]

	if pitch    > 0: flags |= 0x8       # DDS_PITCH
	if mipmaps  > 1: flags |= 0x20000   # DDS_MIPMAPCOUNT
	if shape[2] > 1: flags |= 0x800000  # DDS_DEPTH

	if mipmaps > 1: caps0   |= 0x400000  # DDSCAPS_MIPMAP
	if cubemap:     caps1   |= 0xfe00    # DDSCAPS2_CUBEMAP, DDSCAPS2_CUBEMAP_ALL_FACES
	if cubemap:     flags10 |= 0x4       # DDS_RESOURCE_MISC_TEXTURECUBE

	if shape[0] > 1: dims = 3
	if shape[2] > 1: dims = 4

	out.write(struct.pack("=I II IIII I IIII IIII III III I IIII IIII I II III",
		0x20534444, 124, flags,                           # magic, size, flags
		shape[1], shape[0], pitch, shape[2], mipmaps,     # width, height, pitch, depth, mipmaps
		0,0,0,0, 0,0,0,0, 0,0,0,                          # reserved[11]
		32, 0x04, 0x30315844, 0, 0, 0, 0, 0,              # pf_size, flags:FOURCC, fourcc:DX10, bits, rmask, gmask, bmask, amask
		caps0, caps1, 0, 0, 0,                            # caps0, caps1, caps2, caps3, reserved
		fmt[PF_DXGI], dims, flags10, shape[3], 0))        # dx10_fmt, dimensions, flags, array_count, reserved

	for image in pixels:
		for mipmap in image:
			out.write(mipmap)

	out.close()



np.set_printoptions(threshold=np.nan)


argv = parse_command_line()



if argv.list_formats:

	pwarn("Supported formats:")

	for fmt, num in sorted(PIXELFORMATS.items(), key=lambda x: x[1][PF_VULKAN]):
		sys.stderr.write("\t%s\n" % fmt)
		sys.stderr.flush()

	sys.stderr.write("\n")
	sys.exit(0)


if not argv.format in PIXELFORMATS:
	perror("Unsupported pixelformat %s!" % argv.format)


if argv.directory:
	try:
		os.chdir(argv.directory)

	except Exception:
		perror("Failed to change working directory to %s" % argv.directory)


for image, pixels in (image_load_combined(argv.images) if argv.merge or argv.cubemap else image_load(argv.images)):

	if argv.reshape:
		shape = [ int(x) for x in argv.reshape.split(':') + [ '1', '1', '1', '1' ]][0: 4]

		try:
			pixels = pixels.reshape(shape + [ 4 ])

		except:
			perror("New shape (%d:%d:%d:%d) is incompatible with old shape (%d:%d:%d:%d)" % (
				shape[0], shape[1], shape[2], shape[3],
				pixels.shape[0], pixels.shape[1], pixels.shape[2], pixels.shape[3]))

	if argv.scale:
		pixels *= np.asarray([ float(x) for x in argv.scale.split(':') + [ '1', '1', '1', '1' ]][0: 4], dtype='float32')

	if argv.bias:
		pixels += np.asarray([ float(x) for x in argv.bias.split(':') + [ '0', '0', '0', '0' ]][0: 4], dtype='float32')

	data = image_convert(pixels, PIXELFORMATS[argv.format], argv.generate_mipmaps)

	name   = image[:image.rindex('.')] if '.' in image else image
	output = argv.output
	output = output.replace('{input}', name)

	dds_write(output, data, pixels.shape, PIXELFORMATS[argv.format], argv.cubemap)

