#!/usr/bin/python

import argparse
import os
import re
import sys
import struct



def perror(str):
	sys.stderr.write("blobtool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("blobtool: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Process and integrate resources")
	argp.add_argument('-c',  '--create',        action='store_const', dest='action', const='crt', help='Combine files and create archive')
	argp.add_argument('-x',  '--extract',       action='store_const', dest='action', const='xtr', help='Extract archive')
	argp.add_argument('-l',  '--list',          action='store_const', dest='action', const='lst', help='List archive contents')
	argp.add_argument('-C',  '--directory',     action='store',       type=str,                   help='Set working directory')
	argp.add_argument('-Fh', '--format-header', action='store_const', dest='format', const='cxx', help='Produce C header')
	argp.add_argument('-Fa', '--format-asm',    action='store_const', dest='format', const='asm', help='Produce assembly code')
	argp.add_argument('-Fb', '--format-blob',   action='store_const', dest='format', const='blb', help='Produce .blob file (default)')
	argp.add_argument('pack', nargs='?')
	argp.add_argument('dirs', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.action:
		perror("One of create, extract or list must be specified")

	if not argv.format:
		argv.format = 'blb'

	if not argv.directory:
		argv.directory = ""

	if not argv.pack:
		argv.pack = (
			"out.blob" if argv.format == 'blb' else
			"out.h"    if argv.format == 'cxx' else
			"out.s"    if argv.format == 'asm' else None)

	if len(argv.dirs) < 1:
		argv.dirs.append('.')

	return argv



def gather_filenames(root, dirs):

	curr  = os.getcwd()
	files = []

	if root:
		try:
			os.chdir(root)

		except Exception:
			perror("Failed to change working directory to %s" % root)

	for path in dirs:
		for parent, _, filelist in os.walk(path):
			for file in filelist:
				base = os.path.join(parent, file)
				files.append(list((
					os.path.normpath(base),
					os.path.realpath(base))))

	if root:
		os.chdir(curr)

	return files



def gather_filestats(filelist):

	stats = []

	for file in filelist:
		try:
			with open(file[1], 'rb') as src:
				src.seek(0, 2)
				stats.append(list((file[0], file[1], src.tell())))

		except Exception:
			pwarn("Failed to access %s" % file[1])

	return stats



def blob_generate_blob(output, files):

	offsets = {}
	names   = {}

	strings = ""

	for entry in files:
		names[entry[0]] = len(strings)
		strings += entry[0] + "\x00"

	offset          = struct.calcsize("@IIQQ")
	offset_entries  = offset
	offset         += struct.calcsize("@QQII") * len(files)
	offset_strings  = offset
	offset         += len(strings)

	for entry in files:

		while offset & 15:
			offset += 1

		offsets[entry[0]] = offset
		offset += entry[2] + 1


	try:
		with open(output, 'wb') as out:

			out.write(struct.pack("@IIQQ", 0x626f6c62, len(files), offset_entries, offset_strings))

			for entry in sorted(files, key=lambda f: f[0]):
				out.write(struct.pack("@QQII", offsets[entry[0]], entry[2], 0x10000, names[entry[0]] ))

			out.write(strings)

			for entry in files:

				pwarn("Processing %s" % entry[0])

				while out.tell() & 15:
					out.write("\x00")

				with open(entry[1], 'rb') as src:
					while True:

						data = src.read(4096)

						if not data:
							break

						out.write(data)

				out.write("\x00")

	except Exception:
		perror("Failed to write %s" % output)



def blob_extract_blob(path):
	pass



def blob_list_blob(path):

	def readcstr(f, o):
		s = bytearray()
		f.seek(o, 0)
		while True:
			b = f.read(1)
			if b is None or b == '\0':
				return str(s)
			s.append(b)

	try:
		with open(path, 'rb') as blob:

			magic, count, ofs_entries, ofs_strings = struct.unpack("@IIQQ", blob.read(struct.calcsize("@IIQQ")))
			entries = []

			if magic != 0x626f6c62:
				perror("Wrong magic in %s" % path)

			blob.seek(ofs_entries, 0)

			for n in xrange(count):
				entries.append(list(struct.unpack("@QQII", blob.read(struct.calcsize("@QQII")))))

			for offset, length, revision, name in entries:
				print "{0:12d}  {1:16x}  {2:d}.{3:d}.{4:d}  {5}".format(
						length,
						offset,
						(revision >> 16) & 255,
						(revision >>  8) & 255,
						(revision >>  0) & 255,
						readcstr(blob, ofs_strings + name))

	except Exception:
		perror("Failed to read %s" % path)



def blob_generate_asm(output, files):

	index = dict([ (entry[0], id) for id, entry in enumerate(files) ])

	try:
		with open(output, 'w') as out:

			out.write("\n")
			out.write("\n")
			out.write("\t.section .rodata\n")
			out.write("\t.global __resource_index\n")
			out.write("\t.global __resource_count\n")
			out.write("\n")

			for entry in files:

				pwarn("Processing %s" % entry[0])

				out.write("\n\t.align\n")
				out.write("__resource_%d_data:\n" % index[entry[0]])

				with open(entry[1], 'rb') as src:
					while True:

						data = src.read(16)

						if not data:
							break

						out.write("\t.byte " + ", ".join([ "%3d" % (ord(byte)) for byte in data ]) + "\n")

				out.write("\t.byte " + ", ".join(["  0"] * (4 - entry[2] % 4)) + "\n")
				out.write("\n")

			out.write("\n\t.align\n\n")

			for entry in files:
				out.write("\n__resource_%d_name:\n\t.asciz \"%s\"\n\n" % (index[entry[0]], entry[0]))

			out.write("\n\t.align\n")
			out.write("__resource_index:\n")

			for entry in sorted(files, key=lambda f: f[0]):

				out.write("\t.int __resource_%d_name\n" % index[entry[0]])
				out.write("\t.int __resource_%d_data\n" % index[entry[0]])
				out.write("\t.int %d\n" % entry[2])
				out.write("\n")

			out.write("\n\t.align\n")
			out.write("__resource_count:\n")
			out.write("\t.int %d\n" % len(files))
			out.write("\n")

	except:
		perror("Failed to write %s" % output)



def blob_generate_cxx(output, files):

	try:
		with open(output, 'w') as out:

			out.write("\n")
			out.write("\n")
			out.write("struct __resource_t {\n\n")
			out.write("\tconst char *name;\n")
			out.write("\tconst void *data;\n\n")
			out.write("\tint length;\n\n")
			out.write("};\n\n")

			out.write("extern const __resource_t __resource_index[] = {\n")

			for entry in sorted(files, key=lambda f: f[0]):

				pwarn("Processing %s" % entry[0])

				out.write("\t{\n\t\t\"%s\",\n" % entry[0])
				out.write("\t\t{\n")

				with open(entry[1], 'rb') as src:
					while True:

						data = src.read(16)

						if not data:
							break

						out.write("\t\t\t" + ", ".join([ "%3d" % (ord(byte)) for byte in data ]) + ",\n")

				out.write("\t\t\t" + ", ".join(["  0"] * (4 - entry[2] % 4)) + "\n")
				out.write("\t\t},\n")
				out.write("\t\t%d\n\t},\n" % entry[2])

			out.write("};\n")
			out.write("\nextern const int __resource_count = %d;\n\n\n" % len(files))
			out.close()

	except:
		perror("Failed to write %s" % output)



argv = parse_command_line()

if argv.action == 'crt':

	pwarn("Gathering files")

	files = gather_filenames(argv.directory, argv.dirs)
	stats = gather_filestats(files)

	pwarn("Writing blob to %s" % argv.pack)

	if   argv.format == 'cxx': blob_generate_cxx( argv.pack, stats)
	elif argv.format == 'asm': blob_generate_asm( argv.pack, stats)
	elif argv.format == 'blb': blob_generate_blob(argv.pack, stats)
	else:
		perror("Unknown output format")

	count  = len(stats)
	length = sum([ stat[2] for stat in stats ])

	pwarn("Package contains %d entries in %d bytes" % (count, length))


elif argv.action == 'xtr':
	pass

elif argv.action == 'lst':
	blob_list_blob(argv.pack)

else:
	perror("Internal error")

