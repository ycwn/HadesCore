#!/usr/bin/python

import os
import sys
import re
import argparse
import zipfile
import xml.parsers.expat
import PIL.Image as Image



def perror(str):
	sys.stderr.write("skintool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("skintool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Export an Orion Core skin from a list of images")
	argp.add_argument('-i',  '--input',       action='append',           help='Read image paths from file')
	argp.add_argument('-C',  '--directory',   action='store',  type=str, help='Set working directory')
	argp.add_argument('-g',  '--granularity', action='store',  type=int, help='Set element granularity (1)')
	argp.add_argument('-m',  '--margin',      action='store',  type=int, help='Set element margin (0)')
	argp.add_argument('-p',  '--strip',       action='store',  type=int, help='Remove this number of path elements')
	argp.add_argument('-n',  '--name',        action='store',  type=str, help='Set skin name (default: skin)')
	argp.add_argument('-o',  '--output',      action='store',  type=str, help='Set output filename (default: name)')
	argp.add_argument('images', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.input:
		argv.input = []

	if not argv.directory:
		argv.directory = ""

	if not argv.granularity:
		argv.granularity = 1

	if not argv.margin:
		argv.margin = 0

	if not argv.strip:
		argv.strip = 0

	if not argv.name:
		argv.name = "skin"

	if not argv.output:
		argv.output = argv.name

	return argv



def parse_image_paths(sources, initial=[]):

	paths = [ img for img in initial ]

	for source in sources:
		with open(source, 'r') as src:
			for img in src:

				if '#' in img:
					img = img[:img.index('#')]

				img = img.strip()

				if img != "":
					paths.append(img)

	return paths



def load_images(paths, level):

	images = {}

	for p in paths:

		sym = p

		for n in xrange(int(level)):
			if not os.path.sep in sym:
				break
			sym = sym[sym.index(os.path.sep) + 1:]

		if '.' in sym:
			sym = sym[:sym.index('.')]

		images[sym] = Image.open(p)

	return images



def generate_mosaic(images, margin, granularity):

	boxes = []

	for sym, img in images.iteritems():
		boxes.append([ sym, img.size[0], img.size[1] ])

	boxes.sort(key=lambda box: box[1] * box[2], reverse=True)

	mosaic        = []
	mosaic_width  = 1 << (boxes[0][1] - 1).bit_length()
	mosaic_height = 1 << (boxes[0][2] - 1).bit_length()

	for box_sym, box_w, box_h in boxes:
		while True:
			placed = False

			for x, y in [ (x, y)
					for y in xrange(0, mosaic_width  - box_w + 1, granularity)
					for x in xrange(0, mosaic_height - box_h + 1, granularity) ]:

				collision = False

				for rs, rx, ry, rw, rh in mosaic:
					if ((x < rx + rw + margin) and (rx < x + box_w + margin) and
					    (y < ry + rh + margin) and (ry < y + box_h + margin)):
						collision = True
						break

				if collision:
					continue

				placed = True
				mosaic.append([ box_sym, x, y, box_w, box_h ])
				break

			if placed:
				break

			if mosaic_width > mosaic_height:
				mosaic_height *= 2

			else:
				mosaic_width *= 2

	return mosaic, mosaic_width, mosaic_height



def export_composite(output, images, mosaic, width, height):

	pwarn("Writing composite to '%s'..." % output)

	composite = Image.new('RGBA', (width, height), 0);

	for sym, x, y, w, h in mosaic:
		img = images[sym]
		composite.paste(img, (x, y), img);

	composite.save(output, 'PNG')



def export_metrics(output, name, mosaic, width, height):

	pwarn("Writing metrics to '%s'..." % output)

	hds = 0.5 / float(width)
	hdt = 0.5 / float(height)

	METRIC = "\t<metric name=\"%s\" left=\"%f\" bottom=\"%f\" right=\"%f\" top=\"%f\" width=\"%f\" height=\"%f\" />\n"

	with open(output, 'w') as xml:

		xml.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
		xml.write("<skin name=\"%s\">\n" % name)

		for sym, x, y, w, h in mosaic:

			l =       (2.0 * float(x)     * hds + hds)
			b = 1.0 - (2.0 * float(y + h) * hdt - hdt)
			r =       (2.0 * float(x + w) * hds - hds)
			t = 1.0 - (2.0 * float(y)     * hdt + hdt)

			xml.write(METRIC % (sym, l, b, r, t, w, h))

		xml.write("</skin>\n")



argv  = parse_command_line()
paths = parse_image_paths(argv.input, argv.images)
curr  = os.getcwd()

if argv.directory:
	try:
		os.chdir(argv.directory)

	except Exception:
		perror("Failed to change working directory to %s" % argv.directory)

images                = load_images(paths, argv.strip)
mosaic, width, height = generate_mosaic(images, argv.margin, argv.granularity)

export_composite(argv.output + ".png", images,    mosaic, width, height)
export_metrics(  argv.output + ".xml", argv.name, mosaic, width, height)

if argv.directory:
	os.chdir(curr)

