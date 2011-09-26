#
# commonrules.makefile
#
# To be included at the END of a makefile template
#


# Rule to create the executable output directory
$(bindir) :
	mkdir $(bindir) $(foreach dl,$(wildcard $(foreach tp,$(thirdparty_packages),$(tp)dll/*)),; cp $(dl) $(bindir))


# Rule to create the object output directory
$(objdir) :
	mkdir $(objdir)


# Rule to run make in each directory specified in 'requires' variable
$(requires_dirs) :
	cd $@ && $(MAKE)


# The generic clean rule
clean :
	-rm $(fulltargetname)
	-rm -rf $(objdir)
	-rm -rf $(bindir)


# C++ compilation
$(objdir)/%$(compiler_object_suffix) : %.cpp
	$(compiler) $(compiler_global_flags) $(compiler_additional_flags) $(compiler_includes) $(compiler_object_name_prefix)$@ $<


# Java compilation
%.class: %.java
	javac -classpath $(java_sources_dir) $<


# Declare some phony rules
.PHONY : clean $(requires_dirs)

