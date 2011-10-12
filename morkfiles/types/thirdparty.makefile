define module_type_thirdparty

$(2)_sharedlibs := $(wildcard $(1)/$(config_variant)/prebuilt_shared/*$(shared_output_suffix))
$(2)_staticlibs := $(wildcard $(1)/$(config_variant)/prebuilt_lib/*)

# The include flags needed to compile against this module
$(2)_include = -I$(1)/$(config_variant)/include

# The linker flags needed to link to this module
$(2)_normal_windows_link = $(exe_libpath_prefix)$(1)/$(config_variant)/prebuilt_lib \
		$$(foreach l,$$($(2)_staticlibs),$(exe_libname_prefix)$$(l)) \
		$$(foreach tpd,$$(notdir $$($(2)_sharedlibs)),-DELAYLOAD:$$(tpd))

$(2)_link = $$(or $$($(2)_$(config_variant)_link),$$($(2)_normal_$(config_variant)_link))

$(2)_copy_to_bin = $$($(2)_sharedlibs)

endef

