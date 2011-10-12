
.DEFAULT_GOAL = all

# See http://www.gnu.org/software/make/manual/make.html#Secondary-Expansion
# Needed for definition of $(1)_dependencies (below) which is double-escaped
.SECONDEXPANSION:

module_makefile_name = morkfile

empty :=
space := $(empty) $(empty)
remove_spaces = $(subst $(space),,$(1))

# Figures out the platform, loads the corresponding config file
include morkfiles/platform.makefile

# Load the rules for building different types of module
include morkfiles/types/common/native.makefile
include $(wildcard morkfiles/types/*.makefile)

# The template for loading a module
define load_module_definition
this = $(notdir $(1))
$$(this)_path = $(1)
$$(this)_dependencies = $$$$(foreach r,$$$$($$(this)_requires),$$$$($$$$(r)_main))
include $(1)/$(module_makefile_name)
$$(eval $$(call module_type_$$($$(this)_type),$(1),$$(this)))
endef

# Find all directories that have a module makefile
module_paths := $(patsubst ./%/,%,$(dir $(shell find . -name $(module_makefile_name))))

# Load all the modules
$(foreach m,$(module_paths),$(eval $(call load_module_definition,$(m))))

modules := $(notdir $(module_paths))

# Default rule: depends on the main rule of every module
all: $(foreach m,$(modules),$($(m)_main))

# Clean rule: all trash must be in separate dirs, specified by each module
clean:
	rm -rf $(wildcard $(foreach m,$(modules),$($(m)_output_dirs)))

.PHONY: all clean

