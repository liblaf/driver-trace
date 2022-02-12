# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := driver-trace-recorder.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
# Prerequisites
	@rm --force --recursive *.d
# Object files
	@rm --force --recursive *.o *.ko *.obj *.elf
# Linker output
	@rm --force --recursive *.ilk *.map *.exp
# Precompiled Headers
	@rm --force --recursive *.gch *.pch
# Libraries
	@rm --force --recursive *.lib *.a *.la *.lo
# Shared objects (inc. Windows DLLs)
	@rm --force --recursive *.dll *.so *.so.* *.dylib
# Executables
	@rm --force --recursive *.exe *.out *.app *.i*86 *.x86_64 *.hex
# Debug files
	@rm --force --recursive *.dSYM/ *.su *.idb *.pdb
# Kernel Module Compile Results
	@rm --force --recursive *.mod* .*.cmd .tmp_versions/ modules.order Module.symvers Mkfile.old dkms.conf

endif
