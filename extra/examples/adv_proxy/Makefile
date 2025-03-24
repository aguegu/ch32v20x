BIN     = ../../riscv-embedded-gcc/bin
BIN_PREFIX = riscv-none-embed
CFLAGS  = -march=rv32imacxw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g
DEFINES = -DCH32V20x_D8W -DBLE_MAC=TRUE -DHAL_KEY=1

OUTPUT_DIR = ./dist

TARGET := $(addprefix $(OUTPUT_DIR)/, $(notdir $(CURDIR)))

Startup_ROOT = ../../../EVT/EXAM/SRC/Startup
Startup_SOURCE = startup_ch32v20x_D8W.S
Startup_OBJECT = $(addprefix $(OUTPUT_DIR)/Startup/, $(Startup_SOURCE:.S=.o))

APP_ROOT = ./APP
APP_INC_DIR = $(APP_ROOT)/include
APP_SOURCES = $(notdir $(wildcard $(APP_ROOT)/*.c))
APP_OBJECTS = $(addprefix $(OUTPUT_DIR)/APP/, $(APP_SOURCES:.c=.o))

HAL_ROOT =	../../BLE/HAL
HAL_INC_DIR = $(HAL_ROOT)/include
HAL_SOURCES = $(notdir $(wildcard $(HAL_ROOT)/*.c))
HAL_OBJECTS = $(addprefix $(OUTPUT_DIR)/HAL/, $(HAL_SOURCES:.c=.o))

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

BleLib_ROOT = ../../BLE/LIB
BleLib_SOURCE = ble_task_scheduler.S
BleLib_OBJECT = $(addprefix $(OUTPUT_DIR)/BleLib/, $(BleLib_SOURCE:.S=.o))

MeshLib_ROOT = ../../BLE/MESH/MESH_LIB

LINK_FILE = ../../BLE/HAL/Link.ld

VPATH = $(APP_ROOT):$(Startup_ROOT):$(Core_ROOT):$(Peripheral_SRC_DIR):$(Debug_ROOT):$(HAL_ROOT):$(APP_ROOT):$(HAL_ROOT):$(BleLib_ROOT)

LIBS := -lwchble -lmesh

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
		-I $(APP_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/Debug/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(APP_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/HAL/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(APP_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-I $(HAL_INC_DIR) \
		-I $(BleLib_ROOT) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/BleLib/%.o: %.S
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

$(OUTPUT_DIR)/APP/%.o: %.c
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) $(DEFINES) \
		-I $(Core_ROOT) \
		-I $(Debug_ROOT) \
		-I $(APP_INC_DIR) \
		-I $(Peripheral_INC_DIR) \
		-I $(HAL_INC_DIR) \
		-I $(BleLib_ROOT) \
		-I $(MeshLib_ROOT) \
		-std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

all: $(OUTPUT_DIR) $(TARGET).hex

$(TARGET).elf: $(Startup_OBJECT) $(Core_OBJECT) $(Peripheral_OBJECTS) $(Debug_OBJECT) $(APP_OBJECTS) $(HAL_OBJECTS) $(BleLib_OBJECT)
	$(BIN)/$(BIN_PREFIX)-gcc $(CFLAGS) \
		-L "$(BleLib_ROOT)" \
		-L "$(MeshLib_ROOT)" \
		-T "$(LINK_FILE)" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"$(TARGET).map" --specs=nano.specs --specs=nosys.specs -o "$@" $^ $(LIBS)

$(TARGET).hex: $(TARGET).elf
	$(BIN)/$(BIN_PREFIX)-objcopy -O ihex "$<" "$@"

flash: $(TARGET).hex
	sudo ../../isp/WCHISPTool_CMD -p /dev/ch37x0 -c ./config.ini -o program -f $<

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)/Peripheral
	mkdir -p $(OUTPUT_DIR)/Startup
	mkdir -p $(OUTPUT_DIR)/Core
	mkdir -p $(OUTPUT_DIR)/Peripheral
	mkdir -p $(OUTPUT_DIR)/Debug
	mkdir -p $(OUTPUT_DIR)/APP
	mkdir -p $(OUTPUT_DIR)/HAL
	mkdir -p $(OUTPUT_DIR)/BleLib

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: clean flash
