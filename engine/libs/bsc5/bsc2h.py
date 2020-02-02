#! /usr/bin/python -B

import math
import sys

#
# THE BSC5 FORMAT
# ---------------
# Fd Datum       Byte(s) Comments/Occurrences               Format
# ----------------------------------------------------------------------
#  1 HR            1-  4 Bright Star number                  I4
#  2 Name          5- 14 Bayer, Flamsteed                    A10
#  3 DM           15- 25 Supplemental ID in 25               A2,I3,I5,A1
#  4 HD           26- 31 Henry Draper number                 I6
#  5 SAO          32- 37 SAO Catalog number                  I6 (A6)
#  6 FK5          38- 41 Fifth Fundamental Catalog           I4 (A4)
#  7 CIO flag         42 I                                   A1
#  8 CIO ' flag       43 '                                   A1
#  9 Double code      44 W, D, S                             A1
# 10 ADS          45- 49 Number if in Aitken catalogue       A5
# 11 ADS comp.    50- 51 Letter identifications              A2
# 12 Var          52- 60 Name, number (NSV), "var"           A9
# 13 RAH B1900.0  61- 62 hours of time                       I2
# 14 RAM          63- 64 minutes of time                     I2
# 15 RAS          65- 68 seconds of time                     F4.1
# 16 DECD B1900.0 69- 71 degrees of arc                      I3 (A1,I2)
# 17 DECM         72- 73 minutes of arc                      I2
# 18 DECS         74- 75 seconds of arc                      I2
# 19 RAH J2000.0  76- 77 hours of time                       I2
# 20 RAM          78- 79 minutes of time                     I2
# 21 RAS          80- 83 seconds of time                     F4.1
# 22 DECD J2000.0 84- 86 degrees of arc                      I3 (A1,I2)
# 23 DECM         87- 88 minutes of arc                      I2
# 24 DECS         89- 90 seconds of arc                      I2
# 25 Gal. long.   91- 96 degrees                             F6.2
# 26 Gal. lat.    97-102 degrees                             F6.2
# 27 V mag.      103-107 UBV system if available             F5.2
# 28 V code          108 R, H if not UBV                     A1
# 29 V unc. code     109 :, ?                                A1
# 30 B-V         110-114 UBV system                          F5.2
# 31 B-V code        115 :, ?                                A1
# 32 U-B         116-120 UBV system                          F5.2
# 33 U-B code        121 :, ?                                A1
# 34 R-I         122-126 Johnson system if available         F5.2
# 35 R-I code        127 C, E, :, ?, D (Cousins, Eggen)      A1
# 36 Sp. type    128-147 MK type if available                A20
# 37 Sp.T. code      148 e, v, t                             A1
# 38 PMRA J2000  149-154 on FK5 system                       F6.3
# 39 PMDC J2000  155-160 on FK5 system                       F6.3
# 40 Par. code       161 D (dynamical)                       A1
# 41 Par.        162-166 Trigonometric if code blank         F5.3
# 42 RV          167-170 Line-of-sight velocity (helio.)     I4
# 43 RV comms.   171-174 Variable, SB, etc.                  A4
# 44 vsini code      175 = if limit                          A1
# 45 vsini code      176 < or >                              A1
# 46 vsini       177-179 Slettebak system                    I3
# 47 vsini code      180 : if uncertain                      A1
# 48 Delta mag   181-184 Components of multiple              F4.1
# 49 Separation  185-190 For identified components           F6.1
# 50 Components  191-194 Letter identifications              A4
# 51 # comp.     195-196 Total number known                  I2
# 52 Remark code     197 * if remark(s) in file              A1
#


bsc = []

try:
	with open("bsc5.dat", 'rb') as catalog:
		for star in catalog:

			if star[102:107].strip() == "":
				continue

			ra_h  = float(star[75:77])
			ra_m  = float(star[77:79])
			ra_s  = float(star[79:83])
			dec_n = float(star[83] + '1')
			dec_d = float(star[84:86])
			dec_m = float(star[86:88])
			dec_s = float(star[88:90])
			mag_v = float(star[102:107])

			ra  =         (ra_h  * 3600.0 + ra_m  * 60.0 + ra_s)  * math.pi /  43200.0
			dec = dec_n * (dec_d * 3600.0 + dec_m * 60.0 + dec_s) * math.pi / 324000.0

			bsc.append([ ra, dec, mag_v ])

except:
	raise
	sys.stderr.write("BSC5: Failed to open bsc5.dat!\n");
	sys.exit(1)


print
print
print "#ifndef __BRIGHT_STAR_CATALOG_H"
print "#define __BRIGHT_STAR_CATALOG_H"
print
print
print "namespace BSC5 {"
print
print "\tstruct _star_t {"
print
print "\t\tfloat x, y, z;"
print "\t\tfloat magnitude;"
print
print "\t};"
print
print "\tconst uint    bright_star_count = %u;" % len(bsc)
print "\tconst _star_t bright_star_catalog[]={"
print

for ra, dec, mag in sorted(bsc, key=lambda star: star[2]):

	x = -math.cos(dec) * math.cos(ra)
	y = +math.sin(dec)
	z = +math.cos(dec) * math.sin(ra)

	print "\t\t{ %+1.15f, %+1.15f, %+1.15f, %+1.3f }," % (x, y, z, mag)

print
print "\t};"
print
print "}"
print
print "#endif"
print
