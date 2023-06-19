PROG = ../prog/lcsmi

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

$(PROG): $(SRCS)
	mkdir -p ../prog
	echo "seq LIB:"$(LIBS)
	$(CC) $^ $(CFLAGS) -Wl,-Map,$(PROG).map $(LIBS) $(LDFLAGS) -o $@
	echo  "ledclient ok! $(notdir $(PROG)) from $^" 

.PHONY:clean
clean:
	rm -f $(OBJS) $(PROG)
	rm -rf ../prog ../libs
	echo "Remove objects : $(OBJS)"
