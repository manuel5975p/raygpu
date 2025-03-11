###############################################################################
# Variables and Compiler Flags
###############################################################################
CC           = clang
CXX          = clang++
CFLAGS       = -fPIC -O3 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV -DGLSLANG_OSINCLUDE_UNIX -DSUPPORT_GLFW=1
CXXFLAGS     = -fPIC -std=c++20 -fno-rtti -fno-exceptions -O3 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DSUPPORT_GLFW=1 -DENABLE_SPIRV -DGLSLANG_OSINCLUDE_UNIX
INCLUDEFLAGS = -Iinclude -Iamalgamation/ -Iamalgamation/glslang -Iamalgamation/SPIRV-Reflect

###############################################################################
# Source Files
###############################################################################
# Main C++ source files
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

# glslang sources
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

# GLFW sources
SRC_GLFW = amalgamation/glfw-3.4/src/context.c \
           amalgamation/glfw-3.4/src/egl_context.c \
           amalgamation/glfw-3.4/src/glx_context.c \
           amalgamation/glfw-3.4/src/init.c \
           amalgamation/glfw-3.4/src/input.c \
           amalgamation/glfw-3.4/src/linux_joystick.c \
           amalgamation/glfw-3.4/src/monitor.c \
           amalgamation/glfw-3.4/src/null_init.c \
           amalgamation/glfw-3.4/src/null_joystick.c \
           amalgamation/glfw-3.4/src/null_monitor.c \
           amalgamation/glfw-3.4/src/null_window.c \
           amalgamation/glfw-3.4/src/osmesa_context.c \
           amalgamation/glfw-3.4/src/platform.c \
           amalgamation/glfw-3.4/src/posix_module.c \
           amalgamation/glfw-3.4/src/posix_poll.c \
           amalgamation/glfw-3.4/src/posix_thread.c \
           amalgamation/glfw-3.4/src/posix_time.c \
           amalgamation/glfw-3.4/src/vulkan.c \
           amalgamation/glfw-3.4/src/window.c \
           amalgamation/glfw-3.4/src/wl_init.c \
           amalgamation/glfw-3.4/src/wl_monitor.c \
           amalgamation/glfw-3.4/src/wl_window.c \
           amalgamation/glfw-3.4/src/x11_init.c \
           amalgamation/glfw-3.4/src/x11_monitor.c \
           amalgamation/glfw-3.4/src/x11_window.c \
           amalgamation/glfw-3.4/src/xkb_unicode.c

# C sources
SRC_C = src/sinfl_impl.c \
        src/msf_gif_impl.c \
        src/cgltf_impl.c \
        src/windows_stuff.c \
        src/rtext.c \
        src/stb_impl.c \
        amalgamation/SPIRV-Reflect/spirv_reflect.c

# Precompiled objects (if any)
PRE_OBJS = src/rtext.o src/msf_gif_impl.o src/cgltf_impl.o

###############################################################################
# Wayland Protocols and Generated Files
###############################################################################
# List of Wayland protocol names
WL_PROTOS = xdg-shell fractional-scale-v1 idle-inhibit-unstable-v1 pointer-constraints-unstable-v1 relative-pointer-unstable-v1 viewporter wayland xdg-activation-v1 xdg-decoration-unstable-v1

# Output directory for generated Wayland files
WL_OUT_DIR = wl_include
WL_XML_DIR = amalgamation/glfw-3.4/deps/wayland

# Generated headers and code files
WL_CLIENT_HEADERS = $(addprefix $(WL_OUT_DIR)/, $(addsuffix -client-protocol.h, $(WL_PROTOS)))
WL_CLIENT_CODES   = $(addprefix $(WL_OUT_DIR)/, $(addsuffix -client-protocol-code.h, $(WL_PROTOS)))
WL_ALL            = $(WL_CLIENT_HEADERS) $(WL_CLIENT_CODES)

# Pattern rule: generate client protocol header using wayland-scanner
$(WL_OUT_DIR)/%-client-protocol.h: $(WL_XML_DIR)/%.xml
	@mkdir -p $(WL_OUT_DIR)
	wayland-scanner client-header $< $@

# Pattern rule: generate client protocol code using wayland-scanner (private-code)
$(WL_OUT_DIR)/%-client-protocol-code.h: $(WL_XML_DIR)/%.xml
	@mkdir -p $(WL_OUT_DIR)
	wayland-scanner private-code $< $@

###############################################################################
# Object File Lists
###############################################################################
OBJ_CPP  = $(SRC_CPP:.cpp=.o)
OBJ_GLSL = $(SRC_GLSL:.cpp=.o)
OBJ_GLFW = $(SRC_GLFW:.c=.o)
OBJ_C    = $(SRC_C:.c=.o)
# Uncomment if you have Objective-C++ sources
# OBJ_MM  = $(SRC_MM:.mm=.o)

# Combine all objects (adjust if OBJ_MM is used)
OBJS = $(OBJ_CPP) $(OBJ_GLSL) $(OBJ_GLFW) $(OBJ_C) $(PRE_OBJS)

###############################################################################
# Final Targets
###############################################################################
TARGET = libraygpu.a

#LIBGLSLANG = libglslang.a

all: $(TARGET)

# Create the static library archive; ensure Wayland files are generated first
$(TARGET): $(OBJS) $(WL_ALL)
	ar rcs $@ $^

###############################################################################
# Compile Rules
###############################################################################
# Main C++ sources
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDEFLAGS)

# glslang sources
amalgamation/glslang/%.o: amalgamation/glslang/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDEFLAGS)

# GLFW sources (depends on generated Wayland files)
amalgamation/glfw-3.4/src/%.o: amalgamation/glfw-3.4/src/%.c $(WL_ALL)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDEFLAGS) -D_GLFW_X11 -D_GLFW_WAYLAND -Iwl_include

# C sources (general rule)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDEFLAGS)

# (Optional) Objective-C++ sources
%.o: %.mm
	$(CXX) $(CXXFLAGS) -x objective-c++ -c $< -o $@ $(INCLUDEFLAGS)

###############################################################################
# Clean
###############################################################################
clean:
	rm -f $(OBJS) $(TARGET) $(WL_ALL)
