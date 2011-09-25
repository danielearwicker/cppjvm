#
# commonvars.makefile
#
# To be included at the START of a makefile template
#


# Bring in the configuration
include $(buildroot)/buildscripts/config.makefile


# Automatically include all C++ source files for compilation
sources = $(wildcard *.cpp)


# Subdirectory to store obj files (and other per-module temporary output)
objdir = obj


# Make a list of object file names from the source file names
object_names = $(patsubst %.cpp,%$(compiler_object_suffix),$(sources))
objects = $(addprefix $(objdir)/,$(object_names))


# Figure out the physical directory for each dependency
requires_dirs = $(addprefix $(buildroot)/,$(requires))


# And the include sub-directory for each dependency
requires_includes = $(addsuffix /include,$(requires_dirs))


# And hence C++ compiler flags to include them
requires_cppflags = $(addprefix -I,$(requires_includes))


# Find packages under thirdparty
thirdparty_packages := $(wildcard $(buildroot)/thirdparty/*/$(thirdparty_platform)/)
thirdparty_cppflags = $(foreach tp,$(thirdparty_packages),-I$(tp)include)


# Combined standard includes for the C++ compiler
compiler_includes = -Iinclude $(thirdparty_cppflags) $(requires_cppflags) -DCONFIG_VARIANT_$(config_variant)


# Standard location for Java sources
java_sources_dir = src


# Place to put executables (and shared libraries/DLLs)
bindir = $(buildroot)/bin


# Place to put static libs
staticlibdir = $(buildroot)/lib


# Basis for making target names
base_target_name = $(notdir $(CURDIR))


# Name that would be used to make a static library
staticlib_name = $(staticlibdir)/lib$(base_target_name)$(staticlib_suffix)


# Name that would be used to make an executable
executable_name = $(bindir)/$(base_target_name)$(executable_suffix)

