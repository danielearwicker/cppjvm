#
# staticlib.makefile
#
# Builds every subdirectory that has its own makefile
#

include $(buildroot)/buildscripts/commonvars.makefile

subdirs := $(subst /,,$(wildcard */))

submakes := $(dir $(foreach subdir,$(subdirs),$(wildcard $(subdir)/makefile)))

all : $(submakes)

$(submakes) :
	cd $@ && $(MAKE)

.PHONY : $(submakes)

clean : 
	-for d in $(submakes); do (cd $$d; $(MAKE) clean ); done