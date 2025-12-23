###############################################################################
# Variables
###############################################################################

.DEFAULT_GOAL := all

CC ?= clang
AR ?= ar
CXX ?= clang++
SUPPORT_GLFW ?= 1
OPT_OR_DEBUG_FLAGS ?= -O3
# Compiler/Linker flags
CFLAGS   = -fPIC $(OPT_OR_DEBUG_FLAGS) -DSUPPORT_VULKAN_BACKEND=1 -DRG_STATIC=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV
CXXFLAGS = -fPIC -std=c++20 -fno-rtti -fno-exceptions -DRG_STATIC=1 $(OPT_OR_DEBUG_FLAGS) -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV
ifeq ($(SUPPORT_GLFW), 1)
CFLAGS   += -DSUPPORT_GLFW=1
CXXFLAGS += -DSUPPORT_GLFW=1
endif

# Include paths
INCLUDEFLAGS  = -Iinclude \
                -Isrc/internal_include \
                -Iamalgamation/glfw-3.4/include \
                -Iamalgamation/ \
                -Iamalgamation/vulkan_headers/include \
                -Iamalgamation/glslang \
                -Iamalgamation/SPIRV-Reflect \
                -I/home/manuel/Documents/raygpu/relbuild/_deps/dawn-src/third_party/vulkan-headers/src/include/ \
                -Idl

# Platform detection
PLATFORM_OS     ?= WINDOWS
TARGET_PLATFORM ?= PLATFORM_DESKTOP

# Output directories
OBJ_DIR = obj
LIB_DIR = lib

# Wayland directory for generated protocol files (Linux)
WL_OUT_DIR = wl_include

# Library output name
LIB_OUTPUT = $(LIB_DIR)/libraygpu.a

###############################################################################
# WGVK auto-download (into ./dl if missing)
###############################################################################
DL_DIR          = dl
WGVK_URL_BASE   = https://raw.githubusercontent.com/manuel5975p/WGVK/refs/heads/master
DL_WGVK_C       = $(DL_DIR)/wgvk.c
DL_WGVK_H       = $(DL_DIR)/wgvk.h
DL_WGVK_STRUCTS = $(DL_DIR)/wgvk_structs_impl.h
DL_WGVK_CONFIG  = $(DL_DIR)/wgvk_config.h
DL_WEBGPU_H     = $(DL_DIR)/webgpu/webgpu.h
DL_VOLK_H       = $(DL_DIR)/external/volk.h
DL_VOLK_C       = $(DL_DIR)/src/volk.c
DL_ALL          = $(DL_WGVK_C) $(DL_WGVK_H) $(DL_WGVK_STRUCTS) $(DL_WGVK_CONFIG) $(DL_WEBGPU_H) $(DL_VOLK_H) $(DL_VOLK_C)

$(DL_DIR):
	@mkdir -p $(DL_DIR)

$(DL_DIR)/webgpu:
	@mkdir -p $(DL_DIR)/webgpu

$(DL_DIR)/src:
	@mkdir -p $(DL_DIR)/src

$(DL_DIR)/external:
	@mkdir -p $(DL_DIR)/external

$(DL_WGVK_C): | $(DL_DIR)
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/src/wgvk.c -o $@; fi

$(DL_WGVK_H): | $(DL_DIR)
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/include/wgvk.h -o $@; fi

$(DL_WGVK_STRUCTS): | $(DL_DIR)
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/include/wgvk_structs_impl.h -o $@; fi

$(DL_WGVK_CONFIG): | $(DL_DIR)
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/include/wgvk_config.h -o $@; fi

$(DL_WEBGPU_H): | $(DL_DIR)/webgpu
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/include/webgpu/webgpu.h -o $@; fi

$(DL_VOLK_H): | $(DL_DIR)/external
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/include/external/volk.h -o $@; fi

$(DL_VOLK_C): | $(DL_DIR)/src
	@if [ ! -f $@ ]; then echo "Downloading $@"; curl -fsSL $(WGVK_URL_BASE)/src/volk.c -o $@; fi

dl_deps: $(DL_ALL)


###############################################################################
# Determine PLATFORM_OS when required
###############################################################################
ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), PLATFORM_DESKTOP))
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS = WINDOWS
        ifndef PLATFORM_SHELL
            PLATFORM_SHELL = cmd
        endif
    else
        UNAMEOS = $(shell uname)
        ifeq ($(UNAMEOS),Linux)
            PLATFORM_OS = LINUX
        endif
        ifeq ($(UNAMEOS),FreeBSD)
            PLATFORM_OS = BSD
        endif
        ifeq ($(UNAMEOS),OpenBSD)
            PLATFORM_OS = BSD
        endif
        ifeq ($(UNAMEOS),NetBSD)
            PLATFORM_OS = BSD
        endif
        ifeq ($(UNAMEOS),DragonFly)
            PLATFORM_OS = BSD
        endif
        ifeq ($(UNAMEOS),Darwin)
            PLATFORM_OS = OSX
        endif
        ifndef PLATFORM_SHELL
            PLATFORM_SHELL = sh
        endif
    endif
endif

# Windowing selection macros from detected platform
ifeq ($(PLATFORM_OS), LINUX)
CFLAGS   += -DRAYGPU_USE_X11=1 -DRAYGPU_USE_WAYLAND=1
CXXFLAGS += -DRAYGPU_USE_X11=1 -DRAYGPU_USE_WAYLAND=1
endif
ifeq ($(PLATFORM_OS), WINDOWS)
CFLAGS   += -DRAYGPU_USE_WIN32=1
CXXFLAGS += -DRAYGPU_USE_WIN32=1
endif
ifeq ($(PLATFORM_OS), OSX)
CFLAGS   += -DRAYGPU_USE_METAL=1
CXXFLAGS += -DRAYGPU_USE_METAL=1
endif
ifeq ($(PLATFORM_OS), BSD)
CFLAGS   += -DRAYGPU_USE_X11=1
CXXFLAGS += -DRAYGPU_USE_X11=1
endif

ifeq ($(SUPPORT_GLFW), 1)
CFLAGS   += -DSUPPORT_GLFW=1
CXXFLAGS += -DSUPPORT_GLFW=1
endif
###############################################################################
# Source Files
###############################################################################
SRC_CPP = src/glsl_support.cpp

SRC_GLSL = amalgamation/glslang/glslang/MachineIndependent/parseConst.cpp \
           amalgamation/glslang/glslang/MachineIndependent/Versions.cpp \
           amalgamation/glslang/glslang/MachineIndependent/IntermTraverse.cpp \
           amalgamation/glslang/glslang/MachineIndependent/intermOut.cpp \
           amalgamation/glslang/glslang/MachineIndependent/Scan.cpp \
           amalgamation/glslang/glslang/MachineIndependent/reflection.cpp \
           amalgamation/glslang/glslang/MachineIndependent/PoolAlloc.cpp \
           amalgamation/glslang/glslang/MachineIndependent/SymbolTable.cpp \
           amalgamation/glslang/glslang/MachineIndependent/attribute.cpp \
           amalgamation/glslang/glslang/MachineIndependent/ParseContextBase.cpp \
           amalgamation/glslang/glslang/MachineIndependent/linkValidate.cpp \
           amalgamation/glslang/glslang/MachineIndependent/glslang_tab.cpp \
           amalgamation/glslang/glslang/MachineIndependent/ParseHelper.cpp \
           amalgamation/glslang/glslang/MachineIndependent/iomapper.cpp \
           amalgamation/glslang/glslang/MachineIndependent/Constant.cpp \
           amalgamation/glslang/glslang/MachineIndependent/propagateNoContraction.cpp \
           amalgamation/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp \
           amalgamation/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp \
           amalgamation/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp \
           amalgamation/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp \
           amalgamation/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp \
           amalgamation/glslang/glslang/MachineIndependent/limits.cpp \
           amalgamation/glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp \
           amalgamation/glslang/glslang/MachineIndependent/Intermediate.cpp \
           amalgamation/glslang/glslang/MachineIndependent/RemoveTree.cpp \
           amalgamation/glslang/glslang/MachineIndependent/ShaderLang.cpp \
           amalgamation/glslang/glslang/MachineIndependent/Initialize.cpp \
           amalgamation/glslang/glslang/MachineIndependent/InfoSink.cpp \
           amalgamation/glslang/glslang/GenericCodeGen/CodeGen.cpp \
           amalgamation/glslang/glslang/GenericCodeGen/Link.cpp \
           amalgamation/glslang/SPIRV/disassemble.cpp \
           amalgamation/glslang/SPIRV/doc.cpp \
           amalgamation/glslang/SPIRV/GlslangToSpv.cpp \
           amalgamation/glslang/SPIRV/InReadableOrder.cpp \
           amalgamation/glslang/SPIRV/Logger.cpp \
           amalgamation/glslang/SPIRV/SpvBuilder.cpp \
           amalgamation/glslang/SPIRV/SpvPostProcess.cpp \
           amalgamation/glslang/SPIRV/SpvTools.cpp

SRC_GLFW = amalgamation/glfw-3.4/src/context.c \
           amalgamation/glfw-3.4/src/egl_context.c \
           amalgamation/glfw-3.4/src/init.c \
           amalgamation/glfw-3.4/src/input.c \
           amalgamation/glfw-3.4/src/monitor.c \
           amalgamation/glfw-3.4/src/null_init.c \
           amalgamation/glfw-3.4/src/null_joystick.c \
           amalgamation/glfw-3.4/src/null_monitor.c \
           amalgamation/glfw-3.4/src/null_window.c \
           amalgamation/glfw-3.4/src/osmesa_context.c \
           amalgamation/glfw-3.4/src/platform.c \
           amalgamation/glfw-3.4/src/posix_poll.c \
           amalgamation/glfw-3.4/src/posix_thread.c \
           amalgamation/glfw-3.4/src/posix_time.c \
           amalgamation/glfw-3.4/src/vulkan.c \
           amalgamation/glfw-3.4/src/window.c

ifeq ($(PLATFORM_OS), LINUX)
SRC_GLFW += amalgamation/glfw-3.4/src/glx_context.c \
            amalgamation/glfw-3.4/src/linux_joystick.c \
            amalgamation/glfw-3.4/src/wl_init.c \
            amalgamation/glfw-3.4/src/wl_monitor.c \
            amalgamation/glfw-3.4/src/wl_window.c \
            amalgamation/glfw-3.4/src/x11_init.c \
            amalgamation/glfw-3.4/src/x11_monitor.c \
            amalgamation/glfw-3.4/src/x11_window.c \
            amalgamation/glfw-3.4/src/xkb_unicode.c \
            amalgamation/glfw-3.4/src/posix_module.c
GLFW_BUILD_FLAGS += -D_GLFW_WAYLAND=1 -D_GLFW_X11=1
endif

ifeq ($(PLATFORM_OS), WINDOWS)
SRC_GLFW += amalgamation/glfw-3.4/src/win32_init.c \
            amalgamation/glfw-3.4/src/win32_module.c \
            amalgamation/glfw-3.4/src/win32_time.c \
            amalgamation/glfw-3.4/src/win32_thread.c \
            amalgamation/glfw-3.4/src/win32_joystick.c \
            amalgamation/glfw-3.4/src/win32_monitor.c \
            amalgamation/glfw-3.4/src/win32_window.c \
            amalgamation/glfw-3.4/src/wgl_context.c
GLFW_BUILD_FLAGS += -D_GLFW_WIN32=1
endif

SRC_C = src/sinfl_impl.c \
        src/msf_gif_impl.c \
        src/cgltf_impl.c \
        src/windows_stuff.c \
        src/rtext.c \
        src/wgsl_parse_lite.c \
        src/raygpu.c \
        src/models.c \
        src/rshapes.c \
        src/backend_wgpu.c \
        src/InitWindow.c \
        $(DL_DIR)/wgvk.c \
        src/stb_impl.c \
        amalgamation/SPIRV-Reflect/spirv_reflect.c

###############################################################################
# Wayland Protocols (Linux only)
###############################################################################
WL_PROTOS       = xdg-shell fractional-scale-v1 idle-inhibit-unstable-v1 \
                  pointer-constraints-unstable-v1 relative-pointer-unstable-v1 \
                  viewporter wayland xdg-activation-v1 xdg-decoration-unstable-v1

WL_XML_DIR        = amalgamation/glfw-3.4/deps/wayland
WL_CLIENT_HEADERS = $(addprefix $(WL_OUT_DIR)/, $(addsuffix -client-protocol.h, $(WL_PROTOS)))
WL_CLIENT_CODES   = $(addprefix $(WL_OUT_DIR)/, $(addsuffix -client-protocol-code.h, $(WL_PROTOS)))
WL_ALL            = $(WL_CLIENT_HEADERS) $(WL_CLIENT_CODES)

ifeq ($(PLATFORM_OS), LINUX)
WAYLAND_DEPS = $(WL_ALL)
else
WAYLAND_DEPS =
endif

###############################################################################
# Object Files
###############################################################################
OBJ_CPP  = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_CPP))
OBJ_GLSL = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_GLSL))
OBJ_GLFW = $(patsubst %.c,   $(OBJ_DIR)/%.o, $(SRC_GLFW))
OBJ_C    = $(patsubst %.c,   $(OBJ_DIR)/%.o, $(SRC_C))

OBJS = $(OBJ_CPP) $(OBJ_GLSL) $(OBJ_C)
ifeq ($(SUPPORT_GLFW), 1)
OBJS += $(OBJ_GLFW)
endif

###############################################################################
# Phony Targets
###############################################################################
.PHONY: all clean glfw_objs glsl_objs cpp_objs c_objs wayland-protocols dl_deps

all: dl_deps $(LIB_OUTPUT)

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(WL_OUT_DIR) $(DL_DIR)

###############################################################################
# Library Build
###############################################################################
ifeq ($(SUPPORT_GLFW), 1)
$(LIB_OUTPUT): glsl_objs glfw_objs cpp_objs c_objs | $(LIB_DIR)
	@echo "Archiving objects into $(LIB_OUTPUT)"
	$(AR) rcs $@ $(OBJS)
else
$(LIB_OUTPUT): glsl_objs cpp_objs c_objs glfw_objs | $(LIB_DIR)
	@echo "Archiving objects into $(LIB_OUTPUT)"
	$(AR) rcs $@ $(OBJS)
endif

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

###############################################################################
# Build Group Targets
###############################################################################
glfw_objs: $(OBJ_GLFW)
glsl_objs: $(OBJ_GLSL)
cpp_objs:  $(OBJ_CPP)
c_objs:    $(OBJ_C)

# Ensure C objs wait for downloaded headers too
$(OBJ_DIR)/$(DL_DIR)/wgvk.o: $(DL_WGVK_H) $(DL_WGVK_STRUCTS) $(DL_WGVK_CONFIG)

###############################################################################
# Compile Rules
###############################################################################
# Make compiles wait for downloads so dl files are present
$(OBJ_DIR)/%.o: %.cpp $(DL_ALL)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c $(DL_ALL)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c $< -o $@

ifeq ($(SUPPORT_GLFW), 1)
$(OBJ_DIR)/amalgamation/glfw-3.4/src/%.o: amalgamation/glfw-3.4/src/%.c $(WAYLAND_DEPS) $(DL_ALL)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(GLFW_BUILD_FLAGS) -I $(WL_OUT_DIR) -c $< -o $@
endif

###############################################################################
# Wayland Protocol Generation (Linux)
###############################################################################
ifeq ($(PLATFORM_OS), LINUX)
$(OBJ_GLFW): wayland-protocols

wayland-protocols: $(WL_ALL)
	@echo "Wayland protocols generated."

$(WL_OUT_DIR):
	@mkdir -p $(WL_OUT_DIR)

$(WL_OUT_DIR)/%-client-protocol.h: $(WL_XML_DIR)/%.xml | $(WL_OUT_DIR)
	@echo "Generating $@ ..."
	wayland-scanner client-header $< $@

$(WL_OUT_DIR)/%-client-protocol-code.h: $(WL_XML_DIR)/%.xml | $(WL_OUT_DIR)
	@echo "Generating $@ ..."
	wayland-scanner private-code $< $@
endif

