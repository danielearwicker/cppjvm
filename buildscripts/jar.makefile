#
# jar.makefile
#
# Builds a .jar of Java classes
#

include $(buildroot)/buildscripts/commonvars.makefile


builddir = build


# Use the source directory name as the basis for the lib name
fulltargetname = $(builddir)/$(notdir $(CURDIR)).jar


# Find the source files
java_sources := $(shell find $(java_sources_dir) -name *.java)


# Convert to corresponding list of class files
java_classes := $(patsubst %.java,%.class,$(java_sources))


# Make the .jar
$(fulltargetname) : $(java_classes) $(builddir)
	jar cvf $(fulltargetname) -C $(java_sources_dir) .


$(builddir) :
	mkdir $(builddir)


include $(buildroot)/buildscripts/commonrules.makefile
