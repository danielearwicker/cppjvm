#
# staticlib.makefile
#
# Builds a static library (using the 'ar' tool)
#

include $(buildroot)/buildscripts/commonvars.makefile


# Physical name (also used by 'clean' rule)
fulltargetname = $(staticlib_name)


# The rule to make the library from the objects
$(fulltargetname) : $(if $(minimal),,$(requires_dirs)) $(objdir) $(objects) $(staticlibdir)
	$(static_linker) $(static_linker_global_flags) $(static_linker_output_name_prefix)$(fulltargetname) $(objects)


# The rule to ensure the output directory exists
$(staticlibdir) :
	mkdir $(staticlibdir)


include $(buildroot)/buildscripts/commonrules.makefile
