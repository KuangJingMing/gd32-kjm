

%.o : %.c
	$(CC_PREFIX)$(CC) $(C_FLAGS) -c -o "$@" "$<"

%.o : %.S
	$(CC_PREFIX)$(AS) $(S_FLAGS) -c -o "$@" "$<"

build/source_file.mk: source_file.prj
	# sh $(ROOT_DIR)/tools/generate_makefile.sh
	lua $(ROOT_DIR)/utils/generate_makefile.lua source_file.prj $(ROOT_DIR)

clean:
	-$(RM) -rf build/*
	-@echo ' '

.PHONY: all clean dependents
