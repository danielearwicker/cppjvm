#
# executable.makefile
#
# Builds an executable
#

include $(buildroot)/buildscripts/commonvars.makefile


# Physical name (also used by 'clean' rule)
fulltargetname = $(executable_name)


# Default target
all = $(fulltargetname)


# Static library (if any) for each dependency
requires_libs = $(foreach req,$(requires_dirs),$(wildcard $(staticlibdir)/lib$(notdir $(req))$(staticlib_suffix)))


# And hence flags to link them
requires_linkage = $(foreach reqlib,$(requires_libs),$(executable_linker_library_name_prefix)$(reqlib))


# Libs under thirdparty
thirdparty_libs := $(foreach tp,$(thirdparty_packages),$(wildcard $(tp)lib/*))
thirdparty_linkage := $(foreach tpl,$(thirdparty_libs),\
	$(executable_linker_library_path_prefix)$(dir $(tpl)) \
	$(executable_linker_library_name_prefix)$(tpl))


# The rule to make the executable 
$(fulltargetname) : $(if $(minimal),,$(requires_dirs)) $(objdir) $(objects) $(bindir)
	$(executable_linker) $(executable_linker_global_flags) \
		$(executable_linker_output_name_prefix)$(fulltargetname) \
		$(objects) \
		$(executable_linker_library_path_prefix)$(staticlibdir) \
		$(requires_linkage) $(thirdparty_linkage)
	$(post_executable_link_step)


include $(buildroot)/buildscripts/commonrules.makefile
