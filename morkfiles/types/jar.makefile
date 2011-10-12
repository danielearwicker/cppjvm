
define module_type_jar

$(1)/classes/%.class: $(1)/src/%.java
	mkdir -p $$(dir $$@)
	javac -d $(1)/classes -sourcepath $(1)/src -classpath $(1)/classes$$(call remove_spaces,$$(foreach j,$$($(2)_requires_jars),$(env_separator)$$(j))) $$< 

$(2)_java_sources = $(subst ./,,$(shell find $(1)/src -name *.java))
$(2)_java_classes = $$($(2)_java_sources:$(1)/src/%.java=$(1)/classes/%.class)

$(2)_main = $(1)/jar/$(2).jar
$(2)_output_dirs = $(1)/jar $(1)/classes

$(2)_requires_jars = $$(call stable_unique_first, $$(foreach r,$$($(2)_requires), $$($$(r)_jars)))
$(2)_jars = $$($(2)_main) $$($(2)_requires_jars)

$$($(2)_main) : $$($(2)_java_classes) $$($(2)_requires_jars)
	$($(2)_prebuild)
	mkdir -p $(1)/jar
	jar cvf $$($(2)_main) -C $(1)/classes .

endef

