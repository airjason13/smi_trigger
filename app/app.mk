PROG = ../prog/smi_trigger

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

$(PROG): $(SRCS)
	mkdir -p ../prog
	echo "seq LIB:"$(LIBS)
	$(CC) $^ $(CFLAGS) -Wl,-Map,$(PROG).map $(LIBS) $(LDFLAGS) -o $@
	echo  "smi_trigger ok! $(notdir $(PROG)) from $^" 

.PHONY:clean
clean:
	rm -f $(OBJS) $(PROG)
	rm -rf ../prog ../libs
	echo "Remove objects : $(OBJS)"
