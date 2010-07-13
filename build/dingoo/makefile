MIPSTOOLS = /home/djdron/mipsel-tools/mipsel-4.1.2-nopic
DINGOO_SDK = /home/djdron/dingoo-sdk
PATH := $(MIPSTOOLS)/bin:$(PATH)

CXX       = mipsel-linux-g++
C         = mipsel-linux-gcc
LD        = mipsel-linux-ld
OBJCOPY   = mipsel-linux-objcopy
ELF2APP   = $(DINGOO_SDK)/tools/elf2app/elf2app 

PROJECT_NAME = unreal_speccy_portable
LD_SCRIPT = $(DINGOO_SDK)/lib/dingoo.xn

SRC_PATH = ../..
OBJ_BASE = Release
DIST_PATH = $(OBJ_BASE)
OBJ_PATH = $(OBJ_BASE)

SRCFOLDERS = . z80 tools snapshot ui platform platform/custom_ui platform/dingoo devices devices/fdd devices/input devices/sound 
LIB_PATH  = $(DINGOO_SDK)/lib
LIBS      = -lsml -lc -ljz4740 -lgcc

INCLUDE   = -I$(DINGOO_SDK)/include -I$(MIPSTOOLS)/mipsel-linux/include -I$
STD_OPTS  = -G0 -O3 $(INCLUDE) -Wall -finline-functions -fomit-frame-pointer -msoft-float -fno-builtin -fno-exceptions -mips32 -mno-abicalls -fno-pic -c -DMPU_JZ4740 -DNDEBUG -D_DINGOO -DUSE_UI 
CPP_OPTS  = $(STD_OPTS) -fno-rtti -fno-threadsafe-statics  
C_OPTS    = $(STD_OPTS) 
LD_OPTS   = -nodefaultlibs --script $(LD_SCRIPT) -L$(LIB_PATH) -L$(LIBS) -Map=$(BIN_TARGET).map

CXXSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.cpp))
CSRCS = $(foreach dir, $(SRCFOLDERS), $(wildcard $(SRC_PATH)/$(dir)/*.c))

CXX_OBJS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(CXXSRCS))
CXX_DEPS = $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.d,$(CXXSRCS))
C_OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(CSRCS))
C_DEPS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.d,$(CSRCS))

BIN_TARGET = $(DIST_PATH)/$(PROJECT_NAME)

build: mkdirs $(BIN_TARGET).app

mkdirs:
	mkdir -p $(DIST_PATH)
	mkdir -p $(foreach dir, $(SRCFOLDERS), $(OBJ_PATH)/$(dir))

$(CXX_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CXX) $(CPP_OPTS) -o $@ $<

$(C_OBJS): $(OBJ_PATH)/%.o : $(SRC_PATH)/%.c
	$(C) $(C_OPTS) -o $@ $<

$(BIN_TARGET).elf: $(CXX_OBJS) $(C_OBJS)
	$(LD) $(CXX_OBJS) $(LD_OPTS) -o $(BIN_TARGET).elf

$(BIN_TARGET).app: $(BIN_TARGET).elf
	$(ELF2APP) $(BIN_TARGET)

clean:
	rm -rf $(CXX_OBJS)
	rm -rf $(CXX_DEPS)
	rm -rf $(C_OBJS)
	rm -rf $(C_DEPS)
	rm -rf $(BIN_TARGET).app $(BIN_TARGET).elf $(BIN_TARGET).bin $(BIN_TARGET).map

.PHONY: mkdirs clean build