#!/usr/bin/python

import argparse
import math
import os
import shlex
import struct
import subprocess
import sys
import tempfile

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont


FONT_MAGIC    = 0x66796c67 # 'glyf'
FONT_REVISION = 0



def perror(str):
	sys.stderr.write("fonttool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("fonttool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def get_power_of_two(x):
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x += 1;
	return x;



def parse_command_line():

	argp = argparse.ArgumentParser(description="Generate font glyphs.");
	argp.add_argument('-a', '--alpha',       action='store_true', default=False, help='Generate alpha channel');
	argp.add_argument('-r', '--resolution',  action='store',      type=int, help='Set font resolution (16)');
	argp.add_argument('-s', '--supersample', action='store',      type=int, help='Set supersampling factor (1)');
	argp.add_argument('-c', '--compression', action='store',      type=str, help='Use this command to compress the font image. (nvcompress {in} {out})');
	argp.add_argument('-o', '--output',      action='store',      type=str, help='Set output name (output.glyph)');
	argp.add_argument('script', nargs='?')

	argv = argp.parse_args()

	if not argv.compression:
		argv.compression = "nvcompress {in} {out}"

	if not argv.resolution:
		argv.resolution = 16

	if not argv.supersample:
		argv.supersample = 1

	if not argv.output:
		argv.output = "output.glyph"

	if not argv.script:
		perror("No font script specified")

	return argv;



def generate_symbols(encoding, chars):

	symbols  = [ -1 for n in xrange(256) ]
	dst, src = chars.split('=') if '=' in chars else (chars, "")

	dst_n0 = int(dst[:dst.find('-')],   0) if '-' in dst else int(dst, 0);
	dst_n1 = int(dst[dst.find('-')+1:], 0) if '-' in dst else dst_n0;

	if dst_n0 < 0 or dst_n1 < 0 or dst_n0 > 255 or dst_n1 > 255:
		pwarn("Glyph range %d-%d is out of bounds, skipping." % (dst_n0, dst_n1));
		return symbols;

	if src != "":
		if src[-1] == "+":
			src_n0 = int(src[:-1], 0);
			src_n1 = src_n0 + dst_n1 - dst_n0;

			for n in range(dst_n0, dst_n1+1):
				symbols[n] = src_n0 + n - dst_n0;

		else:
			src_n0 = int(src, 0);

			for n in range(dst_n0, dst_n1+1):
				symbols[n] = ord(chr(src_n0).decode(encoding));

	else:
		for n in range(dst_n0, dst_n1+1):
			try:
				symbols[n] = ord(chr(n).decode(encoding));

			except Exception:
				symbols[n] = -1;

	return symbols;



def generate_glyphs(ttf, size, sampling, symbols):

	font = None

	try:
		font = ImageFont.truetype(ttf, size * sampling)

	except Exception:
		perror("Failed to open %s" % ttf)

	glyphs  = []
	aligned = []
	extent  = size * sampling * 4
	center  = extent / 2
	ascent  = extent
	descent = 0;

	for symbol in symbols:

		if symbol < 0:
			glyphs.append(None);
			continue;

		char  = unichr(symbol);
		glyph = Image.new('L', (extent, extent), 0)

		ImageDraw.Draw(glyph).text((center, center), char, font=font, fill="white");

		rect = glyph.getbbox()

		if rect:
			if rect[1] < ascent:  ascent  = rect[1]
			if rect[3] > descent: descent = rect[3]

		glyphs.append(glyph);

	for glyph in glyphs:
		if glyph:
			rect = glyph.getbbox()
			if rect:
				glyph = glyph.crop((rect[0], ascent, rect[2], descent))
				glyph = glyph.resize((size * (rect[2] - rect[0]) / (descent - ascent), size), Image.ANTIALIAS)

			else:
				glyph = Image.new('L', (size, size), 0)

		aligned.append(glyph)

	return aligned;



def execute_font_script(script, resolution, supersample):

	page_symbols = { 0: [ -1   for n in xrange(256) ] }
	page_glyphs  = { 0: [ None for n in xrange(256) ] }
	page_count   = 0;

	font_name     = ""
	curr_page     = 0
	curr_source   = ""
	curr_encoding = "iso-8859-7"
	line_no       = 0

	try:
		with open(script, 'r') as sc:
			for line in sc:

				if '#' in line:
					line = line[0:line.index('#')]

				line     = line.strip()
				line_no += 1

				if not line:
					continue

				args = shlex.split(line)
				cmd  = args.pop(0).lower()

				if cmd == "encoding":

					if len(args) != 1:
						perror("%s:%d: Error: Invalid arguments" % (script, line_no))

					curr_encoding = str(args[0])

				elif cmd == "glyphs":

					if len(args) < 1:
						perror("%s:%d: Error: Invalid arguments" % (script, line_no))

					pwarn("Rendering glyphs for page %d, encoding %s from %s..." % (curr_page, curr_encoding, os.path.basename(curr_source)))

					for arg in args:
						symbols = generate_symbols(curr_encoding, arg)
						glyphs  = generate_glyphs(curr_source, resolution, supersample, symbols)

						for num in xrange(256):
							if symbols[num] >= 0: page_symbols[curr_page][num] = symbols[num];
							if glyphs[num]:       page_glyphs[curr_page][num]  = glyphs[num];

				elif cmd == "name":

					if font_name != "":
						perror("%s:%d: Error: Font name already defined" % (script, line_no))

					elif len(args) != 1:
						perror("%s:%d: Error: Invalid arguments" % (script, line_no))

					font_name = str(args[0])

				elif cmd == "page":

					if len(args) != 1:
						perror("%s:%d: Error: Invalid arguments" % (script, line_no))

					curr_page = int(args[0])

					if not curr_page in page_symbols: page_symbols[curr_page] = [ -1   for n in xrange(256) ]
					if not curr_page in page_glyphs:  page_glyphs[ curr_page] = [ None for n in xrange(256) ]

				elif cmd == "truetype":

					if len(args) != 1:
						perror("%s:%d: Error: Invalid arguments" % (script, line_no))

					curr_source = os.path.realpath(os.path.expandvars(os.path.expanduser(str(args[0]))))

				else:
					perror("%s:%d: Error: Unknown directive '%s'" % (script, line_no, cmd))

	except Exception:
		raise

	page_count   = max(page_symbols.keys()) + 1
	symbol_table = [ -1   for n in xrange(256 * page_count) ]
	glyph_table  = [ None for n in xrange(256 * page_count) ]

	for page in xrange(page_count):
		if page in page_symbols: symbol_table[page * 256 + 0:page * 256 + 256] = page_symbols[page]
		if page in page_glyphs:  glyph_table[ page * 256 + 0:page * 256 + 256] = page_glyphs[page]

	return font_name, page_count, symbol_table, glyph_table



def generate_layout(glyphs):

	rows = 32;
	amax = 4;
	best = None;
	curr = None;
	boxw = 0;
	boxh = 0;

	for m in range(rows):

		curr = [];
		boxw = 0;
		boxh = -1;

		for n in range(m + 1):
			row = dict();
			row['glyphs'] = [];
			row['size']   = 0;
			curr.append(row);

		for g in range(len(glyphs)):
			if glyphs[g]:

				row = 0;

				for r in range(m+1):
					if curr[r]['size'] < curr[row]['size']:
						row = r;

				curr[row]['glyphs'].append(g);
				curr[row]['size'] += glyphs[g].size[0];

				if boxw < curr[row]['size']:
					boxw = curr[row]['size'];

				if boxh < 0:
					boxh = glyphs[g].size[1] * (m + 1);

		texw  = get_power_of_two(boxw);
		texh  = get_power_of_two(boxh);
		waste = float(texw * texh) / float(boxw * boxh) * max(float(boxw) / float(boxh), float(boxh) / float(boxw));

		if best == None or best['waste'] > waste:
			best = dict();
			best['boxw']   = boxw;
			best['boxh']   = boxh;
			best['texw']   = texw;
			best['texh']   = texh;
			best['waste']  = waste;
			best['mosaic'] = curr;

	pwarn("Selected layout %dx%d, with coverage %.3f%%" % (best['texw'], best['texh'], 100.0 / best['waste']))
	return best;



def compile_metrics(page_count, size, mosaic, glyphs):

	pwarn("Compiling font metrics")

	metrics = [ [ 0.0, 0.0, 0.0, 0.0, 0.0 ] for n in range(256 * page_count) ];

	rtexw = 1.0 / float(mosaic['texw']);
	rtexh = 1.0 / float(mosaic['texh']);

	x = 0;
	y = 0;
	f = False;

	for row in mosaic['mosaic']:
		for gid in row['glyphs']:

			glyph = glyphs[gid];

			if f:
				x  = 0;
				y += glyph.size[1];
				f  = False;

			top    = y * rtexh + rtexh / 2.0;
			left   = x * rtexw + rtexw / 2.0;
			bottom = (y + float(glyph.size[1])) * rtexh - rtexh / 2.0;
			right  = (x + float(glyph.size[0])) * rtexw - rtexw / 2.0;

			metrics[gid] = [
				1.0 - top,    left,
				1.0 - bottom, right,
				float(glyph.size[0]) / float(glyph.size[1])
			];

			x += glyph.size[0];

		f = True;

	return metrics



def compile_composite(size, mosaic, glyphs):

	pwarn("Compositing glyphs onto final surface")

	composite = Image.new('L', (mosaic['texw'], mosaic['texh']), 0);

	rtexw = 1.0 / float(mosaic['texw']);
	rtexh = 1.0 / float(mosaic['texh']);

	x = 0;
	y = 0;
	f = False;

	for row in mosaic['mosaic']:
		for gid in row['glyphs']:

			glyph = glyphs[gid];

			if f:
				x  = 0;
				y += glyph.size[1];
				f  = False;

			composite.paste(glyph, (x, y));

			x += glyph.size[0];

		f = True;

	return composite



def compress_composite(composite, compress, alpha):

	png_file = tempfile.mktemp(".png", "glyph_")
	dds_file = tempfile.mktemp(".dds", "glyph_")

	pwarn("Compressing font image")

	try:
		if alpha:
			Image.merge('LA', (composite, composite)).save(png_file, 'PNG')

		else:
			composite.save(png_file, 'PNG');

		cmd = compress.replace("{in}", "\"%s\"" % png_file).replace("{out}", "\"%s\"" % dds_file)

		pwarn("Attempting to execute '%s'..." % cmd)

		if subprocess.call(cmd, shell=True) == 0:

			pwarn("Success")

			with open(dds_file, 'rb') as dds:
				return dds.read()

		else:
			perror("Failed")

	except Exception:
			perror("Image compressor failed")

	finally:
		try:
			os.remove(png_file)
			os.remove(dds_file)

		except Exception:
			pass



def export_glyphs(output, name, size, pages, metrics, image):

	pwarn("Exporting %s..." % output)

	def mkstrings(strs):
		strings = ""
		for s in strs:
			strings += s + "\0"
		while len(strings) & 3:
			strings += "\0"
		return strings

	try:
		with open(output, 'wb') as gfile:

			gfile.write(struct.pack('@IIfII', FONT_MAGIC, FONT_REVISION, size, 0, pages));
			gfile.write("".join(struct.pack('@fffff', *m) for m in metrics))

			strings = mkstrings([ name ])
			gfile.write(struct.pack('@I', len(strings)))
			gfile.write(strings)

			gfile.write(image)

			pwarn("Export complete")

	except Exception:
		perror("Export failed")



argv = parse_command_line()

(font_name,
 page_count,
 symbol_table,
 glyph_table) = execute_font_script(argv.script, int(argv.resolution), int(argv.supersample))

layout    = generate_layout(glyph_table)
metrics   = compile_metrics(page_count, int(argv.resolution), layout, glyph_table)
composite = compile_composite(int(argv.resolution), layout, glyph_table)
image     = compress_composite(composite, str(argv.compression), argv.alpha)

export_glyphs(
		argv.output,            font_name,
		float(argv.resolution), page_count, metrics, image);

