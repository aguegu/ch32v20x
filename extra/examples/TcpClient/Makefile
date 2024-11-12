BIN     = ../../riscv-embedded-gcc/bin
BIN_PREFIX = riscv-none-embed
CFLAGS  = -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g
DEFINES = -DCH32V20x_D8W

OUTPUT_DIR = ./dist

TARGET := $(addprefix $(OUTPUT_DIR)/, $(notdir $(CURDIR)))

Project_ROOT = .
Project_SRC_DIR = $(Project_ROOT)/src
Project_INC_DIR = $(Project_ROOT)/inc
Project_SOURCE = $(notdir $(wildcard $(Project_SRC_DIR)/*.c))
Project_OBJECTS := $(addprefix $(OUTPUT_DIR)/, $(Project_SOURCE:.c=.o))

Startup_ROOT = ../../../EVT/EXAM/SRC/Startup
Startup_SOURCE = startup_ch32v20x_D8W.S
Startup_OBJECT = $(addprefix $(OUTPUT_DIR)/Startup/, $(Startup_SOURCE:.S=.o))

Debug_ROOT = ../../../EVT/EXAM/SRC/Debug
Debug_SOURCE = debug.c
Debug_OBJECT := $(addprefix $(OUTPUT_DIR)/Debug/, $(Debug_SOURCE:.c=.o))

Core_ROOT = ../../../EVT/EXAM/SRC/Core
Core_SOURCE = core_riscv.c
Core_OBJECT := $(addprefix $(OUTPUT_DIR)/Core/, $(Core_SOURCE:.c=.o))

Peripheral_ROOT = ../../../EVT/EXAM/SRC/Peripheral
Peripheral_INC_DIR = $(Peripheral_ROOT)/inc
Peripheral_SRC_DIR = $(Peripheral_ROOT)/src
Peripheral_SOURCES = $(notdir $(wildcard $(Peripheral_SRC_DIR)/*.c))
# Peripheral_SOURCES = ch32v20x_gpio.c ch32v20x_usart.c ch32v20x_rcc.c ch32v20x_misc.c ch32v20x_dbgmcu.c
Peripheral_OBJECTS := $(addprefix $(OUTPUT_DIR)/Peripheral/, $(Peripheral_SOURCES:.c=.o))

NetLib_ROOT = ../../../EVT/EXAM/ETH/NetLib
NetLib_SOURCES = $(notdir $(wildcard $(NetLib_ROOT)/*.c))
NetLib_OBJECTS := $(addprefix $(OUTPUT_DIR)/NetLib/, $(NetLib_SOURCES:.c=.o))

LINK_FILE = ../../Link.ld

INCLUDE = -I $(Debug_ROOT) -I $(Peripheral_INC_DIR) -I $(Core_ROOT) -I $(Project_INC_DIR) -I $(NetLib_ROOT)

VPATH = $(Project_SRC_DIR):$(Startup_ROOT):$(Core_ROOT):$(Peripheral_SRC_DIR):$(Debug_ROOT):$(NetLib_ROOT)

LIBS := -lwchnet

$(OUTPUT_DIR)/Startup/%.o: %.S
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/Core/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-I $(Core_ROOT) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/Peripheral/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(Project_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/Debug/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(Project_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/NetLib/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(Project_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-I $(NetLib_ROOT) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(INCLUDE) $(DEFINES) -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

all: $(OUTPUT_DIR) $(TARGET).hex

$(TARGET).elf: ${Project_OBJECTS} $(Startup_OBJECT) $(Core_OBJECT) $(Peripheral_OBJECTS) $(Debug_OBJECT) $(NetLib_OBJECTS)
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-L "$(NetLib_ROOT)" \
		-T "$(LINK_FILE)" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"$(TARGET).map" --specs=nano.specs --specs=nosys.specs -o "$@" $^ $(LIBS)

$(TARGET).hex: $(TARGET).elf
	$(BIN)/$(BIN_PREFIX)-objcopy -O ihex "$<" "$@"

flash: $(TARGET).hex
	sudo ../../isp/WCHISPTool_CMD -p /dev/ch37x0 -c ../../isp/config.ini -o program -f $<

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)/Peripheral
	mkdir -p $(OUTPUT_DIR)/Startup
	mkdir -p $(OUTPUT_DIR)/Core
	mkdir -p $(OUTPUT_DIR)/Peripheral
	mkdir -p $(OUTPUT_DIR)/Debug
	mkdir -p $(OUTPUT_DIR)/NetLib

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: clean flash
