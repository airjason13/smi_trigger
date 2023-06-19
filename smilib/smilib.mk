MODULE = libsmi
LIB = $(MAKE_DIR)/libs/$(MODULE).a

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

$(LIB): $(OBJS)
	mkdir -p $(MAKE_DIR)/libs
	echo "LIB = $(LIB)"
	$(AR) cr $@ $^
	echo "Archive $(notdir $@)"

$(OBJS): $(SRCS)
	$(CC) -I$(MAKE_DIR)/utildbglib/ -lrt -lutildbg -c $^
	echo " CC $(CFLAGS) $(OBJS)"


.PHONY:clean
clean:
	$(RM) -f $(LIB) $(OBJS)
	$(RM) -f *expand
	echo "Remove Objects: $(OBJS)"
	echo "Remove Objects: $(notdir $(LIB))"
