######################################
# K1986VK025 RISC-V Example Project by A_D
# https://adelectronics.ru/
######################################

######################################
# Compile flags
######################################

COMPILE_NMSIS = 1

######################################
# User sources
######################################

C_USER_SOURCES = \
Src/main.c \
DriversExt/fonts.c \
DriversExt/ST7735.c

######################################
# User includes
######################################

C_USER_INCLUDES = \
-IInc \
-ISrc \
-IDriversExt

######################################
# Driver sources
######################################

C_SOURCES = \
Drivers/MLDR187_SPL/src/MLDR187_adc.c \
Drivers/MLDR187_SPL/src/MLDR187_adcui.c \
Drivers/MLDR187_SPL/src/MLDR187_bkp.c \
Drivers/MLDR187_SPL/src/MLDR187_clk_msr.c \
Drivers/MLDR187_SPL/src/MLDR187_crc.c \
Drivers/MLDR187_SPL/src/MLDR187_cryptoAes.c \
Drivers/MLDR187_SPL/src/MLDR187_cryptoKuz.c \
Drivers/MLDR187_SPL/src/MLDR187_dma.c \
Drivers/MLDR187_SPL/src/MLDR187_eeprom.c \
Drivers/MLDR187_SPL/src/MLDR187_gpio.c \
Drivers/MLDR187_SPL/src/MLDR187_i2c.c \
Drivers/MLDR187_SPL/src/MLDR187_iso7816.c \
Drivers/MLDR187_SPL/src/MLDR187_iwdg.c \
Drivers/MLDR187_SPL/src/MLDR187_otp.c \
Drivers/MLDR187_SPL/src/MLDR187_power.c \
Drivers/MLDR187_SPL/src/MLDR187_rng.c \
Drivers/MLDR187_SPL/src/MLDR187_rst_clk.c \
Drivers/MLDR187_SPL/src/MLDR187_sensors.c \
Drivers/MLDR187_SPL/src/MLDR187_ssp.c \
Drivers/MLDR187_SPL/src/MLDR187_timer.c \
Drivers/MLDR187_SPL/src/MLDR187_uart.c \
Drivers/MLDR187_SPL/src/MLDR187_wwdg.c

ifeq ($(COMPILE_NMSIS), 1)
C_SOURCES += \
Drivers/NMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c \
Drivers/NMSIS/DSP/Source/FastMathFunctions/FastMathFunctions.c \
Drivers/NMSIS/DSP/Source/TransformFunctions/TransformFunctions.c \
Drivers/NMSIS/DSP/Source/FilteringFunctions/FilteringFunctions.c \
Drivers/NMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c \
Drivers/NMSIS/DSP/Source/CommonTables/CommonTables.c \
Drivers/NMSIS/DSP/Source/CommonTables/riscv_twiddlecoef_tables.c
endif

ASM_SOURCES = startup_MLDR187.S

######################################
# Driver includes
######################################

C_INCLUDES = \
-IDrivers/MLDR187_SPL/inc

ifeq ($(COMPILE_NMSIS), 1)
C_INCLUDES += \
-IDrivers/NMSIS/Core/Include \
-IDrivers/NMSIS/DSP/Include \
-IDrivers/NMSIS/NN/Include
endif

######################################
# Build dir, name
######################################

TARGET = Firmware
TARGET_RAM = Firmware_RAM
BUILD_DIR = build
GDB_FLASH_FILE = JLink/gdb_writefile.txt

#######################################
# Binaries
#######################################

PREFIX = riscv-none-embed-

ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc -std=gnu11
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# Definitions
#######################################

ASM_DEFS =

C_DEFS = -DHXTAL_VALUE=25000000 -D__SYSTEM_CLOCK_108M_PLL_HXTAL=108000000 -DRISCV_ALL_FFT_TABLES

#######################################
# Flags
#######################################

ARCH = rv32imac
ABI = ilp32

COMP_OPT = -Og -flto -g
MCU = -march=$(ARCH) -mabi=$(ABI) -fno-pie
DEPENDS = -MMD -MP
WARNINGS = -Wall -Wextra -Wshadow -Wformat=2 -Wformat-truncation=2 -Wundef
WARNINGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-undef -Wno-sign-compare

ASFLAGS = $(COMP_OPT) $(MCU) -ggdb -pipe $(DEPENDS) $(WARNINGS) $(ASM_DEFS) $(C_INCLUDES) $(C_USER_INCLUDES)
CFLAGS = $(COMP_OPT) $(MCU) -ggdb -pipe $(DEPENDS) $(WARNINGS) $(C_DEFS) $(C_INCLUDES) $(C_USER_INCLUDES)
CFLAGS += -fno-common -ffunction-sections -fdata-sections
CFLAGS += -ffreestanding -ftls-model=local-exec

LDSCRIPT = MLDR187_Flash.ld
LDSCRIPT_RAM = MLDR187_RAM.ld

LIBS = -lgcc -lc

LDFLAGS = $(COMP_OPT) $(MCU) $(LIBS) -static -Wl,-O1,--gc-sections,--relax,--build-id=none -nostdlib
LDFLAGS_RAM = $(COMP_OPT) $(MCU) $(LIBS) -static -Wl,-O1,--gc-sections,--relax,--build-id=none -nostdlib
LDFLAGS += -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS_RAM += -T$(LDSCRIPT_RAM) -Wl,-Map=$(BUILD_DIR)/$(TARGET_RAM).map

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
all: $(BUILD_DIR)/$(TARGET_RAM).elf $(BUILD_DIR)/$(TARGET_RAM).hex $(BUILD_DIR)/$(TARGET_RAM).bin

#######################################
# Build
#######################################

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_USER_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.c $(sort $(dir $(C_USER_SOURCES)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo $<
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@echo $<
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET_RAM).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS_RAM) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	@echo 'set $$StartAddr = 0x00000000'> $(GDB_FLASH_FILE)
	@echo 'set $$FileSize = '`stat -L -c %s $(BUILD_DIR)/$(TARGET).bin `>> $(GDB_FLASH_FILE)

$(BUILD_DIR):
	mkdir $@

#######################################
# Clean
#######################################

clean:
	-rm -fR $(BUILD_DIR)

#######################################
# Dependencies
#######################################

-include $(wildcard $(BUILD_DIR)/*.d)
