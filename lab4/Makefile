INCLUDE_PATH = ./include
CFLAGS  += -g -I $(INCLUDE_PATH)
LDFLAGS += -lncursesw 
PTHREADS_LDFLAGS = -lpthread

LIBDIR 	:= libs
OBJDIR := objs
PROCESS_OBJDIR := $(OBJDIR)/process
PTHREADS_OBJDIR := $(OBJDIR)/pthreads
BINDIR 	:= bin

LD_LIBRARY_PATH := $LD_LIBRARY_PATH:./$(LIBDIR)
COMMON := animal common god list main message reporter

all: pthread-bin process-bin | $(BINDIR)

run-thread: pthread-bin
	./$(BINDIR)/pthread-bin

run-process: process-bin
	./$(BINDIR)/process-bin

pthread-bin: $(addprefix $(PTHREADS_OBJDIR)/, $(addsuffix .o,$(COMMON))) $(PTHREADS_OBJDIR)/simulator-thread.o | $(BINDIR)
	$(CC) $^ $(LDFLAGS) $(PTHREADS_LDFLAGS) -o $(BINDIR)/$@

process-bin: $(addprefix $(PROCESS_OBJDIR)/, $(addsuffix .o,$(COMMON))) $(PROCESS_OBJDIR)/comm.o $(PROCESS_OBJDIR)/simulator-process.o | $(BINDIR)
	$(CC) $^ $(LDFLAGS) -o $(BINDIR)/$@

.PHONY: clean
clean:
	-rm -rf $(BINDIR) $(LIBDIR) $(OBJDIR)

$(PTHREADS_OBJDIR)/%.o: src/%.c include/list.h include/common.h include/entities.h | $(PTHREADS_OBJDIR)
	$(CC) -DTHREADS $(CPPFLAGS) -fPIC $(CFLAGS) -c $< -o $@

$(PROCESS_OBJDIR)/%.o: src/%.c include/list.h include/common.h include/entities.h | $(PROCESS_OBJDIR)
	$(CC) $(CPPFLAGS) -fPIC $(CFLAGS) -c $< -o $@

$(PROCESS_OBJDIR):
	mkdir -p $(PROCESS_OBJDIR)

$(PTHREADS_OBJDIR):
	mkdir -p $(PTHREADS_OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(BINDIR):
	mkdir -p $(BINDIR)
