BIN     = ../../riscv-embedded-gcc/bin
BIN_PREFIX = riscv-none-embed
CFLAGS  = -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g
DEFINES = -DCH32V20x_D8W

OUTPUT_DIR = ./dist

TARGET := $(addprefix $(OUTPUT_DIR)/, $(notdir $(CURDIR)))

Project_ROOT = .
Project_SRC_DIR = $(Project_ROOT)
Project_INC_DIR = $(Project_ROOT)/include
Project_SOURCE = $(notdir $(wildcard $(Project_SRC_DIR)/*.c))
Project_OBJECTS := $(addprefix $(OUTPUT_DIR)/, $(Project_SOURCE:.c=.o))

Core_ROOT = ../../../EVT/EXAM/SRC/Core
Core_SOURCE = core_riscv.c
Core_OBJECT := $(addprefix $(OUTPUT_DIR)/Core/, $(Core_SOURCE:.c=.o))

Peripheral_ROOT = ../../../EVT/EXAM/SRC/Peripheral
Peripheral_INC_DIR = $(Peripheral_ROOT)/inc
Peripheral_SRC_DIR = $(Peripheral_ROOT)/src
Peripheral_SOURCES = ch32v20x_gpio.c ch32v20x_rcc.c
Peripheral_OBJECTS := $(addprefix $(OUTPUT_DIR)/Peripheral/, $(Peripheral_SOURCES:.c=.o))

Startup_ROOT = ../../../EVT/EXAM/SRC/Startup
Startup_SOURCE = startup_ch32v20x_D8W.S
Startup_OBJECT = $(addprefix $(OUTPUT_DIR)/Startup/, $(Startup_SOURCE:.S=.o))

LINK_FILE = ../../Link.ld

INCLUDES = -I $(Peripheral_INC_DIR) -I $(Core_ROOT) -I $(Project_INC_DIR)

VPATH = $(Project_ROOT):$(Startup_ROOT):$(Core_ROOT):$(Peripheral_SRC_DIR)

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
		-I $(Project_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		 $(DEFINES) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) -MMD -MP $(INCLUDES) $(DEFINES) -std=gnu99 -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

all: $(OUTPUT_DIR) $(TARGET).hex

$(TARGET).elf: ${Project_OBJECTS} $(Startup_OBJECT) $(Core_OBJECT) $(Peripheral_OBJECTS)
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-T "$(LINK_FILE)" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"$(TARGET).map" --specs=nano.specs --specs=nosys.specs -o "$@" $^

$(TARGET).hex: $(TARGET).elf
	$(BIN)/$(BIN_PREFIX)-objcopy -O ihex "$<" "$@"

flash: $(TARGET).hex
	sudo ../../isp/WCHISPTool_CMD -p /dev/ch37x0 -c ../../isp/config.ini -o program -f $<

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)/Peripheral
	mkdir -p $(OUTPUT_DIR)/Startup
	mkdir -p $(OUTPUT_DIR)/Core
	mkdir -p $(OUTPUT_DIR)/Peripheral

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: clean flash
