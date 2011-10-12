
os := $(shell uname)

ifeq ($(findstring Linux,$(os)),Linux)
	config_variant = linux
endif
 
ifeq ($(findstring CYGWIN,$(os)),CYGWIN)
	config_variant = windows
endif

ifeq ($(findstring Darwin,$(os)),Darwin)
	config_variant = darwin
endif

ifeq (,$(config_variant))
	config_variant = unrecognised_platform # include statement below will produce error
endif

include morkfiles/platforms/$(config_variant).makefile

