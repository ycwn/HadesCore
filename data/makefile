

build := ../build
data  := $(build)/data
temp  := $(build)/temp

configsrc  := $(wildcard *.xml)
fontsrc    := $(wildcard fonts/*.conf)
modelsrc   := $(shell grep -v '\#' models/models.conf)
musicsrc   := $(wildcard music/*.ogg)
scriptsrc  := $(shell find ../script -type f -iname '*.as')
shadersrc  := $(wildcard ../shaders/*.shader)
rendrc     := $(wildcard ../shaders/*.conf)
skinsrc    := $(shell cat hud/skins.conf)
uidefrc    := $(shell find hud/ -type f -iname '*.ui')
soundfxsrc :=
tracksrc   := $(shell grep -v '\#' tracks/tracks.conf)
textures_bc1 := $(shell cat textures/textures_bc1.conf)
textures_bc2 := $(shell cat textures/textures_bc2.conf)
textures_bc3 := $(shell cat textures/textures_bc3.conf)

font_res := 64
font_aa  := 4

ft := ../engine/tools/fonttool.py
gt := ../engine/tools/geotool.py
pt := ../engine/tools/blobtool.py
sc := ../engine/tools/shaderc.py
st := ../engine/tools/skintool.py


config    = $(configsrc:%=$(data)/%)
fonts     = $(fontsrc:%.conf=$(data)/%.glyph)
models    = $(foreach mdl,$(modelsrc),$(data)/models/$(mdl).geo)
models   += $(patsubst %.png,$(data)/%.dds,$(foreach mdl,$(modelsrc),$(wildcard models/$(mdl)_rgb.png)))
models   += $(patsubst %.png,$(data)/%.dds,$(foreach mdl,$(modelsrc),$(wildcard models/$(mdl)_rgba.png)))
models   += $(patsubst %.png,$(data)/%.dds,$(foreach mdl,$(modelsrc),$(wildcard models/$(mdl)_rgbs.png)))
models   += $(patsubst %.png,$(data)/%.dds,$(foreach mdl,$(modelsrc),$(wildcard models/$(mdl)_norm.png)))
music     = $(musicsrc:%=$(data)/%)
scripts   = $(scriptsrc:../%=$(data)/%)
shaders   = $(shadersrc:../%.shader=$(data)/%.spirv)
rendrcs   = $(rendrc:../%.conf=$(data)/%.conf)
skins     = $(skinsrc:%=$(data)/hud/%.dds)
skins    += $(skinsrc:%=$(data)/hud/%.xml)
uidefs    = $(uidefrc:%=$(data)/%)
soundfx   =
tracks    = $(tracksrc:%=$(data)/tracks/%.xml)
textures  = $(textures_bc1:%=$(data)/textures/%.dds)
textures += $(textures_bc2:%=$(data)/textures/%.dds)
textures += $(textures_bc3:%=$(data)/textures/%.dds)

datafiles = $(sort $(config) $(fonts) $(models) $(music) $(scripts) $(shaders) $(rendrcs) $(skins) $(uidefs) $(textures) $(tracks))
datadirs  = $(patsubst $(data)/%,%,$(sort $(dir $(datafiles))))

include texture-$(texture_profile).rules

ftflags   := -c "$(font_compression)" -a -s $(font_aa) -r $(font_res)
gtflags   := -vt -vn -mp models/materials.ini
cubefaces := _0_pos_x _1_neg_x _2_pos_y _3_neg_y _4_pos_z _5_neg_z



all:

clean:
	rm -f $(build)/data0.blob
	rm -rf $(data) $(temp)

build:
	mkdir -p $(datadirs:%=$(data)/%) $(datadirs:%=$(temp)/%)


$(build)/data0.blob: build $(datafiles)
	$(pt) -c -C $(data) -Fb $@


#
# Copy over XML configurations
#
$(config): $(data)/%: %
	@cp -v $< $@


#
# Generate fonts
#
$(fonts): $(data)/%.glyph: %.conf
	$(ft) $(ftflags) -o $@ $<


#
# Copy scripts and shaders over
#
$(scripts): $(data)/%: ../%
	@cp -v $< $@


#
# Compile shaders
#
$(shaders): $(data)/%.spirv: ../%.shader
	$(sc) -I../shaders/sys -I../interface -K -o $@ $< ../shaders/etc/glslangValidator.conf


#
# Copy over renderer configuration
#
$(rendrcs): $(data)/%: ../%
	echo "$(tracks)" "$@" "$<"
	@cp -v $< $@


#
# Copy over track definitions
#
$(tracks): $(data)/%: %
	echo "$(tracks)" "$@" "$<"
	@cp -v $< $@


#
# Convert FBX modeils
#
$(filter %.geo,$(models)): $(data)/%.geo: %.fbx
	$(gt) $(gtflags) -o "$@" "$<"


#
# Copy music and SFX over
#
$(music): $(data)/%: %
	@cp -v $< $@


#
# Generate HUD skin metrics and texture
#
$(temp)/%.png $(temp)/%.xml: %
	$(st) -p2 -n "$(notdir $<)" -o "$(temp)/$<" $(wildcard $</*)


#
# Copy HUD skin metrics
#
$(filter %.xml,$(skins)): $(data)/%.xml: $(temp)/%.xml
	@cp -v $< $@


#
# Copy UI definitions
#
$(uidefs): $(data)/%: %
	@cp -v $< $@
