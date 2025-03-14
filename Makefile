###############################################################################
# Variables
###############################################################################

CC ?= clang
AR ?= ar
CXX ?= clang++

# Compiler/Linker flags
CFLAGS        = -fPIC -O3 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV
CXXFLAGS      = -fPIC -std=c++20 -fno-rtti -fno-exceptions -O3 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV
ifeq ($(SUPPORT_GLFW), 1)
CFLAGS += -DSUPPORT_GLFW=1
CXXFLAGS += -DSUPPORT_GLFW=1
endif
# Include paths
INCLUDEFLAGS  = -Iinclude \
                -Iamalgamation/glfw-3.4/include \
                -Iamalgamation/ \
                -Iamalgamation/vulkan_headers/include \
                -Iamalgamation/glslang \
                -Iamalgamation/SPIRV-Reflect \
                -I/home/manuel/Documents/raygpu/relbuild/_deps/dawn-src/third_party/vulkan-headers/src/include/

# Platform detection
PLATFORM_OS   ?= WINDOWS
TARGET_PLATFORM ?= PLATFORM_DESKTOP

# Output directories
OBJ_DIR       = obj
LIB_DIR       = lib

# Wayland directory for generated protocol files (only used on Linux)
WL_OUT_DIR    = wl_include

# Library output name
LIB_OUTPUT    = $(LIB_DIR)/libraygpu.a

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

###############################################################################
# Source Files
###############################################################################
SRC_CPP = src/InitWindow.cpp \
          src/InitWindow_GLFW.cpp \
          src/glsl_support.cpp \
          src/models.cpp \
          src/raygpu.cpp \
          src/backend_vulkan/backend_vulkan.cpp \
          src/backend_vulkan/vulkan_textures.cpp \
          src/backend_vulkan/wgvk.cpp \
          src/backend_vulkan/vulkan_pipeline.cpp \
          src/rshapes.cpp \
          src/shader_parse.cpp

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
           amalgamation/glslang/SPIRV/SPVRemapper.cpp \
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
        src/stb_impl.c \
        amalgamation/SPIRV-Reflect/spirv_reflect.c

###############################################################################
# Wayland Protocols (Linux only)
###############################################################################
WL_PROTOS       = xdg-shell fractional-scale-v1 idle-inhibit-unstable-v1 \
                  pointer-constraints-unstable-v1 relative-pointer-unstable-v1 \
                  viewporter wayland xdg-activation-v1 xdg-decoration-unstable-v1

WL_XML_DIR      = amalgamation/glfw-3.4/deps/wayland
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

# We will create separate object lists for better organization
OBJ_CPP  = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_CPP))
OBJ_GLSL = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_GLSL))
# GLFW object files
OBJ_GLFW = $(patsubst %.c,   $(OBJ_DIR)/%.o, $(SRC_GLFW))
# Other C object files
OBJ_C    = $(patsubst %.c,   $(OBJ_DIR)/%.o, $(SRC_C))

# All objects combined
OBJS     = $(OBJ_CPP) $(OBJ_GLSL) $(OBJ_C)
ifeq ($(SUPPORT_GLFW), 1)
OBJS += $(OBJ_GLFW)
endif
###############################################################################
# Phony Targets
###############################################################################
.PHONY: all clean glfw_objs glsl_objs cpp_objs c_objs wayland-protocols

all: $(LIB_OUTPUT)

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(WL_OUT_DIR)

###############################################################################
# Library Build
###############################################################################
ifeq ($(SUPPORT_GLFW), 1)
$(LIB_OUTPUT): glsl_objs glfw_objs cpp_objs c_objs | $(LIB_DIR)
	@echo "Archiving objects into $(LIB_OUTPUT)"
	$(AR) rcs $@ $(OBJS)
else
$(LIB_OUTPUT): glsl_objs cpp_objs c_objs | $(LIB_DIR)
	@echo "Archiving objects into $(LIB_OUTPUT)"
	$(AR) rcs $@ $(OBJS)
endif
# Make sure the library directory exists
$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

###############################################################################
# Build Group Targets
###############################################################################
glfw_objs: $(OBJ_GLFW)
glsl_objs: $(OBJ_GLSL)
cpp_objs:  $(OBJ_CPP)
c_objs:    $(OBJ_C)

###############################################################################
# Compile Rules
###############################################################################

# Default C++ compile rule (for both normal .cpp and GLSL .cpp)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Default C compile rule (for non-GLFW C files)
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# GLFW C compile rule with extra GLFW_BUILD_FLAGS
# We match the path "amalgamation/glfw-3.4/src" so we can apply the extra flags.
# This uses a more specific pattern and must come *before* the default rule
# or be more specialized, so it only applies to GLFW files.
ifeq ($(SUPPORT_GLFW), 1)
$(OBJ_DIR)/amalgamation/glfw-3.4/src/%.o: amalgamation/glfw-3.4/src/%.c $(WAYLAND_DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(GLFW_BUILD_FLAGS) -I $(WL_OUT_DIR) -c $< -o $@
endif
###############################################################################
# Wayland Protocol Generation (Linux)
###############################################################################
ifeq ($(PLATFORM_OS), LINUX)
# Ensure that glfw objects depend on the generated Wayland protocol files
$(OBJ_GLFW): wayland-protocols

wayland-protocols: $(WL_ALL)
	@echo "Wayland protocols generated."

# Create the wl_include directory
$(WL_OUT_DIR):
	@mkdir -p $(WL_OUT_DIR)

# Generate headers
$(WL_OUT_DIR)/%-client-protocol.h: $(WL_XML_DIR)/%.xml | $(WL_OUT_DIR)
	@echo "Generating $@ ..."
	wayland-scanner client-header $< $@

# Generate private-code
$(WL_OUT_DIR)/%-client-protocol-code.h: $(WL_XML_DIR)/%.xml | $(WL_OUT_DIR)
	@echo "Generating $@ ..."
	wayland-scanner private-code $< $@
endif
