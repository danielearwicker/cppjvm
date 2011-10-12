
cpp_tool = g++
cpp_flags = -c -DPOSIX -g -O0 -fno-inline -MD -MP -fPIC -m32
# We generate dependencies at same time as compiling (-MD)
cpp_generate_dependencies = 
cpp_output = -o 
cpp_output_dir = obj
cpp_output_suffix = .o

static_tool = ar
static_flags = -cr
static_output = -o 
static_output_dir = lib
static_output_prefix = lib
static_output_suffix = .lib

shared_tool = gcc -shared -Wl,-soname,$(1) -o $(1)
shared_output_dir = so
shared_output_prefix = 
shared_output_suffix = .so
shared_output_link = $(1)

exe_tool = g++ -m32
exe_flags = 
exe_output = -o  
exe_output_dir = bin
exe_output_suffix =
exe_libpath_prefix = -L
exe_libname_prefix = 

env_keyword = export
env_separator = :
env_libpath = LD_LIBRARY_PATH

