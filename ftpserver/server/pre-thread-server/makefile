CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../../common

OBJS = pre-thread.o $(SHDIR)/common.o

all: pre-thread

pre-thread: $(OBJS)
	@$(CC) -pthread -o pre-thread $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o pre-thread
	@rm -f ../../common/*.o
	@echo Done cleaning
