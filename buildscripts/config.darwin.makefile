
thirdparty_platform = macosx

compiler = g++
compiler_global_flags = -c -DPOSIX
#compiler_global_flags += -g
compiler_global_flags += -I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers/
compiler_global_flags += -O3 -Os
compiler_global_flags += -fno-stack-protector -funsafe-loop-optimizations
compiler_global_flags += -U_FORTIFY_SOURCE

#compiler_global_flags += -arch x86_64 

compiler_object_name_prefix = -o 
compiler_object_suffix = .o

staticlib_suffix = .lib
static_linker = libtool
static_linker_global_flags = -static
static_linker_output_name_prefix = -o 

executable_linker = g++
executable_linker_global_flags = -framework JavaVM 
executable_linker_output_name_prefix = -o 
executable_linker_library_path_prefix = -L
executable_linker_library_name_prefix = 


