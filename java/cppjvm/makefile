include vars.makefile

include $(buildroot)/buildscripts/commonvars.makefile

java_classes = \
	java.util.regex.MatchResult \
	java.util.regex.Matcher \
	java.util.regex.Pattern \
	java.lang.System \
	java.io.ByteArrayOutputStream

$(staticlib_name) : $(if $(minimal),,$(requires_dirs))
	java -cp ../cppwrap/build/cppwrap.jar com.earwicker.cppjvm.cppwrap.CppWrap . $(java_classes)
	$(MAKE) -f lib.makefile

include $(buildroot)/buildscripts/commonrules.makefile

clean:
	-rm *.cpp
	-rm -rf include
	$(MAKE) -f lib.makefile clean
