

subsystems = \
	ai        \
	angel     \
	animation \
	audio     \
	core      \
	dynamics  \
	game           \
	game/animation \
	game/driver    \
	game/hud       \
	game/intro     \
	game/menu      \
	gl    \
	math  \
	net   \
	scene \
	ui

datasources = \
	resources

cflags   = -Iinclude/ -DDEBUG -O3
cxxflags = $(cflags) -fno-rtti -fno-exceptions -fpermissive -std=gnu++0x -Werror=overloaded-virtual
ldflags  = #-Ldata/ -lres

daemon = 31415
telnet = 31415

source  = $(foreach system,$(subsystems),$(sort $(wildcard source/$(system)/*.cpp))) $(sort $(wildcard source/*.cpp))
scripts = $(sort $(notdir $(wildcard script/*)))
shaders = $(sort $(notdir $(wildcard shaders/*)))
cores   = $(shell grep -c processor /proc/cpuinfo)
autosrc =

export subsystems source   autosrc
export scripts    shaders
export cflags     cxxflags ldflags

.PHONY: all build purge clean deploy log log-clean single-core exec trace tarball
.PHONY: .FORCE
.FORCE:

all: world

ifneq ("$(wildcard config.rules)","")
include config.rules
else
$(error config.rules not found, run configure first)
endif

include $(wildcard engine/make/*.rules)


world: build autogen native-droid package-droid

build:
	mkdir -p build/

autogen: $(autosrc)

purge: clean clean-data clean-docs
	rm config.rules

clean:
	rm -rf build/

deploy: deploy-debug


deploy-debug:
#	ant -f engine/android/build.xml installd
	adb install -r build/bin/CharonCore-debug.apk

deploy-release:
	ant -f engine/android/build.xml installr

log:
	adb logcat | grep DEBUG

log-clean:
	adb logcat -c

exec: autoconst-start
	adb shell am start -n org.quantumsingularity.charoncore.test/android.app.NativeActivity

trace:
	$(shell ndk-which addr2line) -e build/obj/local/armeabi-v7a/libcharoncore.so $(A)

tarball: purge
	(cd .. && tar Jcvf `date +charon-%y%m%d.%H%M.tar.xz` charon/)

