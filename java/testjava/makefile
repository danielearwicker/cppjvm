buildroot = ../..

requires = java/cppjvm java/jniutil

post_executable_link_step = export LD_LIBRARY_PATH="$(JAVA_HOME)/jre/lib/i386/client/" ; $(executable_name)

include $(buildroot)/buildscripts/executable.makefile
