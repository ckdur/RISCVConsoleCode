BOOTROM_DIR?=$(abspath .)

ISACONF?=RV64GC
CROSSCOMPILE?=riscv64-unknown-elf
CC=$(CROSSCOMPILE)-gcc
CCX=$(CROSSCOMPILE)-g++
OBJCOPY=$(CROSSCOMPILE)-objcopy
OBJDUMP=$(CROSSCOMPILE)-objdump
ifeq ($(ISACONF),RV64GC)
CFLAGS_ARCH=-march=rv64imafdc_zicsr -mabi=lp64d
else ifeq ($(ISACONF),RV64IMAC)
CFLAGS_ARCH=-march=rv64imac_zicsr -mabi=lp64
else ifeq ($(ISACONF),RV32GC)
CFLAGS_ARCH=-march=rv32imafdc_zicsr -mabi=ilp32d
else #RV32IMAC
CFLAGS_ARCH=-march=rv32imac_zicsr -mabi=ilp32
endif

CFLAGS=$(CFLAGS_ARCH) -mcmodel=medany -O1 -std=gnu11 -Wall -nostartfiles 
CFLAGS+= -fno-common -g -DENTROPY=0 -DNONSMP_HART=0 
CFLAGS+= -I $(BOOTROM_DIR)/include -I. -I./src -I./kprintf -I./lib -I./clkutils -I./libfdt -I./spi $(ADD_OPTS)
LFLAGS=-static -nostdlib --specs=nosys.specs -L $(BOOTROM_DIR)/linker -T memory.lds
BUILD_DIR?=$(abspath ./build)

LIB_FS_O= \
	src/start.o \
	src/main.o \
	src/test.o \
	src/dilithium_test.o \
	src/bls12_381_test.o \
	uart/uart.o \
	$(BUILD_DIR)/version.o \
	clkutils/clkutils.o \
	kprintf/kprintf.o \
	htif/htif.o \
	plic/plic_driver.o \
	i2c/driver.o \
	spi/spi.o \
	lib/memcpy.o \
	lib/memset.o \
	lib/strcmp.o \
	lib/strncmp.o \
	lib/strlen.o \
	lib/memchr.o \
	lib/memmove.o \
	lib/memcmp.o \
	lib/strrchr.o \
	lib/strnlen.o \
	libfdt/fdt.o libfdt/fdt_ro.o libfdt/fdt_wip.o libfdt/fdt_sw.o libfdt/fdt_rw.o libfdt/fdt_strerror.o libfdt/fdt_empty_tree.o \
	libfdt/fdt_addresses.o libfdt/fdt_check.o

$(BUILD_DIR)/version.c:
	mkdir -p $(BUILD_DIR)
	echo "const char *gitid = \"$(shell git describe --always --dirty)\";" > $(BUILD_DIR)/version.c
	echo "const char *gitdate = \"$(shell git log -n 1 --date=short --format=format:"%ad.%h" HEAD)\";" >> $(BUILD_DIR)/version.c
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" >> $(BUILD_DIR)/version.c

%.o: %.S
	$(CC) $(CFLAGS) -DFSBL_TARGET_ADDR=$(FSBL_TARGET_ADDR) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -DFSBL_TARGET_ADDR=$(FSBL_TARGET_ADDR) -c $< -o $@

%.o: %.cpp
	$(CCX) $(CFLAGS) -DFSBL_TARGET_ADDR=$(FSBL_TARGET_ADDR) -c $< -o $@

elf := $(BUILD_DIR)/out.elf
$(elf): $(LIB_FS_O)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LFLAGS) -T link.lds -o $@ $(LIB_FS_O) -lgcc -lm -lgcc -lc

elf_qspi := $(BUILD_DIR)/out.qspi.elf
$(elf_qspi): $(LIB_FS_O)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LFLAGS) -T link.qspi.lds -o $@ $(LIB_FS_O) -lgcc -lm -lgcc -lc

bin := $(BUILD_DIR)/out.bin
bin_qspi := $(BUILD_DIR)/out.qspi.bin
hex := $(BUILD_DIR)/out.hex
hex_qspi := $(BUILD_DIR)/out.qspi.hex

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	$(OBJDUMP) -d $^ > $@.dump

%.hex: %.bin
	od -t x4 -An -w4 -v $< > $@

elf: $(elf) $(elf_qspi)
bin: $(bin) $(bin_qspi)
hex: $(hex) $(hex_qspi)

.PHONY: clean
clean::
	rm -rf $(elf) $(elf_qspi) $(bin) $(bin_qspi) $(hex) $(hex_qspi) $(LIB_FS_O) build
