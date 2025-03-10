# Compilers
CC      = gcc
CXX     = g++
# Flags (adjust as needed)
CFLAGS    = -fPIC -O2 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV -DGLSLANG_OSINCLUDE_UNIX
CXXFLAGS  = -fPIC -std=c++20 -fno-rtti -fno-exceptions -O2 -DSUPPORT_VULKAN_BACKEND=1 -DSUPPORT_GLSL_PARSER=1 -DENABLE_SPIRV -DGLSLANG_OSINCLUDE_UNIX
INCLUDEFLAGS = -Iinclude -Iamalgamation/ -Iamalgamation/glslang -Iamalgamation/SPIRV-Reflect
# C++ source files
SRC_CPP = src/InitWindow.cpp \
          src/glsl_support.cpp \
          src/models.cpp \
          src/raygpu.cpp \
          src/backend_vulkan/backend_vulkan.cpp \
          src/backend_vulkan/vulkan_textures.cpp \
          src/backend_vulkan/wgvk.cpp \
          src/backend_vulkan/vulkan_pipeline.cpp \
          src/rshapes.cpp \
          src/shader_parse.cpp

SRC_GLSL += amalgamation/glslang/glslang/MachineIndependent/parseConst.cpp \
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

# C source files
SRC_C = src/sinfl_impl.c \
        src/msf_gif_impl.c \
        src/cgltf_impl.c \
        src/windows_stuff.c \
        src/rtext.c \
        src/stb_impl.c \
        examples/core_headless.c \

SRC_C += amalgamation/SPIRV-Reflect/spirv_reflect.c
# Objective-C++ source file
#SRC_MM = src/utils_metal.mm

# Precompiled objects that you want to link directly
PRE_OBJS = src/rtext.o src/msf_gif_impl.o src/cgltf_impl.o

# Generate object file names from sources
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ_GLSL = $(SRC_GLSL:.cpp=.o)
OBJ_C   = $(SRC_C:.c=.o)
OBJ_MM  = $(SRC_MM:.mm=.o)

# All object files (compiled from source + precompiled)
OBJS = $(OBJ_CPP) $(OBJ_C) $(OBJ_MM) $(PRE_OBJS)

# Final executable
TARGET = program
LIBGLSLANG = libglslang.a
# Default target
all: $(TARGET)
$(LIBGLSLANG): $(OBJ_GLSL)
	ar rcs libglslang.a $^

# Link all object files using the C++ linker
$(TARGET): $(OBJS) $(LIBGLSLANG)
	$(CXX) -o $@ $^ -lvulkan

# Pattern rules for compiling source files

# Compile C++ sources
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDEFLAGS)

# Compile C sources
%.o: %.c
	$(CC) $(INCLUDEFLAGS) $(CFLAGS) -c $< -o $@

# Compile Objective-C++ sources
%.o: %.mm
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -x objective-c++ -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJS) $(TARGET) $(OBJ_GLSL) $(LIBGLSLANG)
