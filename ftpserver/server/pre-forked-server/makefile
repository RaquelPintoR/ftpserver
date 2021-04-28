CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../../common

OBJS = pre-forked.o $(SHDIR)/common.o

all: pre-forked

pre-forked: $(OBJS)
	@$(CC) -o pre-forked $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o pre-forked
	@rm -f ../../common/*.o
	@echo Done cleaning
