
thirdparty_platform = linux

compiler = g++
compiler_global_flags = -c -DPOSIX
compiler_object_name_prefix = -o
compiler_object_suffix = .o

staticlib_suffix = .lib
static_linker = ar
static_linker_global_flags = -cr
static_linker_output_name_prefix = -o 

executable_linker = g++
executable_linker_global_flags = -L$(JAVA_HOME)/jre/lib/i386/client/ -ljvm

executable_linker_output_name_prefix = -o 
executable_linker_library_path_prefix = -L
executable_linker_library_name_prefix = 

