MAKE_DIR = $(PWD)

APP = app
LCDCLILIB = lcdclilib
UTILDBGLIB = utildbglib
JTIMERLIB = jtimerlib
SMILIB = smilib

APP_DIR := $(MAKE_DIR)/$(APP)
LCDCLILIB_DIR := $(MAKE_DIR)/$(LCDCLILIB)
UTILDBGLIB_DIR := $(MAKE_DIR)/$(UTILDBGLIB)
JTIMERLIB_DIR := $(MAKE_DIR)/$(JTIMERLIB)
SMILIB_DIR := $(MAKE_DIR)/$(SMILIB)

INC_SRCH_PATH := -I$(MAKE_DIR)
INC_SRCH_PATH += -I$(COMPAT_DIR)
INC_SRCH_PATH += -I$(LCDCLILIB_DIR)
INC_SRCH_PATH += -I$(UTILDBGLIB_DIR)
INC_SRCH_PATH += -I$(JTIMERLIB_DIR)
INC_SRCH_PATH += -I$(SMILIB_DIR)

LIB_SRCH_PATH := -L$(MAKE_DIR)
LIB_SRCH_PATH += -L$(MAKE_DIR)/libs/

CFLAGS :=
CFLAGS += $(INC_SRCH_PATH) $(LIB_SRCH_PATH)
CFLAGS += -DDEBUG -D_REENTRANT
CFLAGS += -Wall 
CFLAGS += -D_GNU_SOURCE 

LDFLAGS :=
LDFLAGS += -llcdcli
LDFLAGS += -ljtimer
LDFLAGS += -lsmi
LDFLAGS += -lpthread -lm -lrt
LDFLAGS += -lutildbg

export MAKE_DIR CFLAGS LDFLAGS LIBS INC_SRCH_PATH LIB_SRCH_PATH

all: 
	echo $(CFLAGS)
	$(MAKE) -C $(LCDCLILIB_DIR) -f $(LCDCLILIB).mk
	$(MAKE) -C $(UTILDBGLIB_DIR) -f $(UTILDBGLIB).mk
	$(MAKE) -C $(JTIMERLIB_DIR) -f $(JTIMERLIB).mk
	$(MAKE) -C $(SMILIB_DIR) -f $(SMILIB).mk
	$(MAKE) -C $(APP_DIR) -f $(APP).mk
	
.PHONY: clean
clean:
	$(MAKE) -C $(UTILDBGLIB_DIR) -f $(UTILDBGLIB).mk clean
	$(MAKE) -C $(LCDCLILIB_DIR) -f $(LCDCLILIB).mk clean
	$(MAKE) -C $(JTIMERLIB_DIR) -f $(JTIMERLIB).mk clean
	$(MAKE) -C $(SMILIB_DIR) -f $(SMILIB).mk clean
	$(MAKE) -C $(APP_DIR) -f $(APP).mk clean
