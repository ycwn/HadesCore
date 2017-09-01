

subsystems = \
	core \
	game \
	gr   \
	sg   \
	ui

datasources =


source  = $(foreach system,$(subsystems) .,$(sort $(wildcard source/$(system)/*.c source/$(system)/*.d)))
scripts = $(sort $(notdir $(wildcard script/*)))
shaders = $(sort $(notdir $(wildcard shaders/*)))
cores   = 0
arch    = unknown
autosrc =

export subsystems source   autosrc
export scripts    shaders


.PHONY: all build purge clean tarball
.PHONY: .FORCE
.FORCE:


all: world

ifneq ("$(wildcard config.rules)","")
include config.rules
else
$(error config.rules not found, run configure first)
endif

ifeq ("$(wildcard engine/arch_$(arch)/platform.rules)","")
$(error engine/arch_$(arch)/platform.rules not found, are you sure '$(arch)' is a supported platform)
endif


include $(wildcard engine/build_make/*.rules)
include $(wildcard engine/arch_$(arch)/platform.rules)


world: build autogen platform-pre platform-build platform-pack platform-post

build:
	mkdir -p build/

autogen: $(autosrc)

clean: platform-clean

purge: clean clean-data clean-docs
	rm config.rules

armageddon:
	rm -rf build/

tarball: armageddon
	(cd .. && tar Jcvf `date +hades-%y%m%d.%H%M.tar.xz` hades/)

