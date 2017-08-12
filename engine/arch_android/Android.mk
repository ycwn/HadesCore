

LOCAL_PATH := .
LOCAL_LIBS := engine/libs

LIBEXPAT     := $(LOCAL_LIBS)/expat-2.1.0/lib
LIBEXPAT_INC := $(LIBEXPAT)
LIBEXPAT_SRC := xmlparse.c xmlrole.c xmltok.c xmltok_impl.c xmltok_ns.c

LIBSTB     := $(LOCAL_LIBS)/stb
LIBSTB_INC := $(LIBSTB)
LIBSTB_SRC := stb_image.c stb_vorbis.c

LIBANGEL     := $(LOCAL_LIBS)/angelscript-2.30.2/angelscript/source
LIBANGEL_INC := $(LOCAL_LIBS)/angelscript-2.30.2/angelscript/include
LIBANGEL_SRC := \
	as_atomic.cpp         as_builder.cpp          as_bytecode.cpp           as_callfunc_arm.cpp \
	as_callfunc.cpp       as_callfunc_mips.cpp    as_callfunc_ppc_64.cpp    as_callfunc_ppc.cpp \
	as_callfunc_sh4.cpp   as_callfunc_x64_gcc.cpp as_callfunc_x64_mingw.cpp as_callfunc_x64_msvc.cpp \
	as_callfunc_x86.cpp   as_callfunc_xenon.cpp   as_compiler.cpp           as_configgroup.cpp \
	as_context.cpp        as_datatype.cpp         as_gc.cpp                 as_generic.cpp \
	as_globalproperty.cpp as_memory.cpp           as_module.cpp             as_objecttype.cpp \
	as_outputbuffer.cpp   as_parser.cpp           as_restore.cpp            as_scriptcode.cpp \
	as_scriptengine.cpp   as_scriptfunction.cpp   as_scriptnode.cpp         as_scriptobject.cpp \
	as_string.cpp         as_string_util.cpp      as_thread.cpp             as_tokenizer.cpp \
	as_typeinfo.cpp       as_variablescope.cpp \
	as_callfunc_arm_gcc.S


LIBANGELX     := $(LOCAL_LIBS)/angelscript-2.30.2/add_on
LIBANGELX_INC := $(LOCAL_LIBS)/angelscript-2.30.2/add_on
LIBANGELX_SRC := \
	contextmgr/contextmgr.cpp \
	debugger/debugger.cpp \
	scriptany/scriptany.cpp \
	scriptarray/scriptarray.cpp \
	scriptbuilder/scriptbuilder.cpp \
	scriptdictionary/scriptdictionary.cpp \
	scriptfile/scriptfile.cpp \
	scriptfile/scriptfilesystem.cpp \
	scriptgrid/scriptgrid.cpp \
	scripthandle/scripthandle.cpp \
	scripthelper/scripthelper.cpp \
	scriptmath/scriptmathcomplex.cpp \
	scriptmath/scriptmath.cpp \
	scriptstdstring/scriptstdstring.cpp \
	scriptstdstring/scriptstdstring_utils.cpp \
	serializer/serializer.cpp \
	weakref/weakref.cpp


LIBEIGEN     := $(LOCAL_LIBS)/eigen-3.1
LIBEIGEN_INC := $(LIBEIGEN)
LIBEIGEN_SRC :=


LIBBSC5     := $(LOCAL_LIBS)/bsc5
LIBBSC5_INC := $(LIBBSC5)
LIBBSC5_SRC :=


#
# Build libexpat
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libexpat
LOCAL_SRC_FILES          := $(addprefix $(LIBEXPAT)/,$(LIBEXPAT_SRC))
LOCAL_CFLAGS             := -I$(LIBEXPAT_INC) $(cflags) -DHAVE_MEMMOVE
LOCAL_EXPORT_C_INCLUDES  := $(LIBEXPAT_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_LIBEXPAT
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build libSTB
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libSTB
LOCAL_SRC_FILES          := $(addprefix $(LIBSTB)/,$(LIBSTB_SRC))
LOCAL_CFLAGS             := -I$(LIBSTB_INC) $(cflags) -DSTBI_NO_STDIO -DSTB_VORBIS_NO_STDIO
LOCAL_EXPORT_C_INCLUDES  := $(LIBSTB_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_LIBSTB
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build AngelScript
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libangel
LOCAL_SRC_FILES          := $(addprefix $(LIBANGEL)/,$(LIBANGEL_SRC))
LOCAL_CFLAGS             := -I$(LIBANGEL_INC) $(cflags) -DAS_NO_EXCEPTIONS -DAS_NO_STDIO -DAS_NO_THREADS
LOCAL_EXPORT_C_INCLUDES  := $(LIBANGEL_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_ANGEL
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build AngelScript AddOns
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libangelx
LOCAL_SRC_FILES          := $(addprefix $(LIBANGELX)/,$(LIBANGELX_SRC))
LOCAL_CFLAGS             := -I$(LIBANGEL_INC) -I$(LIBANGELX_INC) $(cflags) -DAS_NO_EXCEPTIONS -DAS_NO_STDIO -DAS_NO_THREADS
LOCAL_EXPORT_C_INCLUDES  := $(LIBANGELX_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_ANGELX
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build Eigen
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libeigen
LOCAL_SRC_FILES          := $(addprefix $(LIBEIGEN)/,$(LIBEIGEN_SRC))
LOCAL_CFLAGS             := -I$(LIBEIGEN_INC) $(cflags)
LOCAL_EXPORT_C_INCLUDES  := $(LIBEIGEN_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_EIGEN -DEIGEN_MPL2_ONLY -DEIGEN_DEFAULT_TO_ROW_MAJOR -DEIGEN_FAST_MATH -DEIGEN_NO_MALLOC
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build the Bright Star Catalog
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE           := arm
LOCAL_MODULE             := libbsc5
LOCAL_SRC_FILES          := $(addprefix $(LIBBSC5)/,$(LIBBSC5_SRC))
LOCAL_CFLAGS             := -I$(LIBBSC5_INC) $(cflags)
LOCAL_EXPORT_C_INCLUDES  := $(LIBBSC5_INC)
LOCAL_EXPORT_CFLAGS      := -DHAVE_BSC5
LOCAL_EXPORT_LDLIBS      :=

include $(BUILD_STATIC_LIBRARY)


#
# Build the Core
#
include $(CLEAR_VARS)

LOCAL_ARM_MODE          := arm
LOCAL_MODULE            := charoncore
LOCAL_CFLAGS            := $(cflags) -Isource/
LOCAL_CXXFLAGS          := $(cxxflags) -Isource/
LOCAL_SRC_FILES         := $(autosrc) $(source)
LOCAL_LDLIBS            := $(ldflags) -llog -landroid -lEGL -lGLESv2 -lOpenSLES
LOCAL_STATIC_LIBRARIES  := android_native_app_glue libangel libangelx libbsc5 libeigen libexpat libSTB

include $(BUILD_SHARED_LIBRARY)


$(call import-module,android/native_app_glue)

