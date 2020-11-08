INCLUDE_PATH = ./include
CFLAGS += -g -I $(INCLUDE_PATH)

LIBDIR 	:= libs
OBJDIR 	:= objs
BINDIR 	:= bin
TESTDIR := tests

LD_LIBRARY_PATH := $LD_LIBRARY_PATH:./$(LIBDIR)

INPUTDIR  := inputs
OUTPUTDIR := outputs

RUNEXE 	 := $(BINDIR)/imagep-dyn
RUNEXE_S := $(BINDIR)/imagep-static
TESTEXE	 := $(BINDIR)/imagecompare-dyn

TESTS := $(shell ls $(TESTDIR))

INPUTS 	:= $(shell ls $(INPUTDIR))

all: build-so | $(BINDIR)

build-so: $(BINDIR)/imagep-dyn $(BINDIR)/imagecompare-dyn | $(BINDIR)

build-a: $(BINDIR)/imagep-static $(BINDIR)/imagecompare-static | $(BINDIR)

run-so: $(RUNEXE) $(addprefix rundyn/,$(INPUTS))

run-a: $(RUNEXE_S) $(addprefix runstatic/,$(INPUTS))

rundyn/%: $(RUNEXE)
	@echo [RUN]: "\t" $(lastword $(subst /, ,$@))
	-./$(RUNEXE) -i $(INPUTDIR)/$(lastword $(subst /, ,$@)) -o $(OUTPUTDIR)/$(lastword $(subst /, ,$@)) -t2

runstatic/%: $(RUNEXE_S)
	@echo [RUN]: "\t" $(lastword $(subst /, ,$@))
	-./$(RUNEXE_S) -i $(INPUTDIR)/$(lastword $(subst /, ,$@)) -o $(OUTPUTDIR)/$(lastword $(subst /, ,$@)) -t2

tests: T1 T2 $(addsuffix -all,$(TESTS))

T1: $(addsuffix -t1,$(TESTS))

T2: $(addsuffix -t2,$(TESTS))

test%-all: $(RUNEXE_S) $(TESTEXE) $(TESTDIR)/test%
	@echo [TEST]: "  " $@
	-@./$(RUNEXE_S) -i $(TESTDIR)/$(firstword $(subst -, ,$@))/input -o $(TESTDIR)/$(firstword $(subst -, ,$@))/tall -tall
	-@./$(TESTEXE) $(TESTDIR)/$(firstword $(subst -, ,$@))/tall $(TESTDIR)/$(firstword $(subst -, ,$@))/tall.out
	-@rm $(TESTDIR)/$(firstword $(subst -, ,$@))/tall

test%-t1: $(RUNEXE_S) $(TESTEXE) $(TESTDIR)/test%
	@echo [TEST]: "  " $@
	-@./$(RUNEXE_S) -i $(TESTDIR)/$(firstword $(subst -, ,$@))/input -o $(TESTDIR)/$(firstword $(subst -, ,$@))/t1 -t1
	-@./$(TESTEXE) $(TESTDIR)/$(firstword $(subst -, ,$@))/t1 $(TESTDIR)/$(firstword $(subst -, ,$@))/t1.out
	-@rm $(TESTDIR)/$(firstword $(subst -, ,$@))/t1

test%-t2: $(RUNEXE_S) $(TESTEXE) $(TESTDIR)/test% 
	@echo [TEST]: "  " $@
	-@./$(RUNEXE_S) -i $(TESTDIR)/$(firstword $(subst -, ,$@))/input -o $(TESTDIR)/$(firstword $(subst -, ,$@))/t2 -t2
	-@./$(TESTEXE) $(TESTDIR)/$(firstword $(subst -, ,$@))/t2 $(TESTDIR)/$(firstword $(subst -, ,$@))/t2.out
	-@rm $(TESTDIR)/$(firstword $(subst -, ,$@))/t2

$(BINDIR)/imagep-dyn: lib-so $(OBJDIR)/imagep.o | $(BINDIR)
	$(CC) -o $@ $(OBJDIR)/imagep.o -L ./$(LIBDIR) -limage -lmatrixmath -lm

$(BINDIR)/imagep-static: lib-a $(OBJDIR)/imagep.o | $(BINDIR)
	$(CC) -static -o $@ $(OBJDIR)/imagep.o -L ./$(LIBDIR) -Wl,-Bstatic -limage -lmatrixmath -lm

$(BINDIR)/imagecompare-dyn: lib-so $(OBJDIR)/imagecompare.o | $(BINDIR)
	$(CC) -o $@ $(OBJDIR)/imagecompare.o -L ./$(LIBDIR) -limage -lmatrixmath -lm

$(BINDIR)/imagecompare-static: lib-a $(OBJDIR)/imagecompare.o | $(BINDIR)
	$(CC) -static -o $@ $(OBJDIR)/imagecompare.o -L ./$(LIBDIR) -Wl,-Bstatic -limage -lmatrixmath -lm

lib-so: $(LIBDIR)/libmatrixmath.so $(LIBDIR)/libimage.so | $(LIBDIR)

lib-a: $(LIBDIR)/libmatrixmath.a $(LIBDIR)/libimage.a | $(LIBDIR)

.PHONY: clean
clean:
	-rm -rf $(BINDIR) $(LIBDIR) $(OBJDIR)

$(LIBDIR)/libmatrixmath.so: $(OBJDIR)/Matrix.o | $(LIBDIR)
	$(CC) -shared $^ -o $@

$(LIBDIR)/libmatrixmath.a: $(OBJDIR)/Matrix.o | $(LIBDIR)
	$(AR) rcs $@ $^

$(LIBDIR)/libimage.so: $(OBJDIR)/Image.o $(OBJDIR)/ImageTransforms.o | $(LIBDIR)
	$(CC) -shared $^ -o $@

$(LIBDIR)/libimage.a: $(OBJDIR)/Image.o $(OBJDIR)/ImageTransforms.o | $(LIBDIR)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: src/%.c include/ADT/Matrix.h include/Image/Image.h include/Image/ImageTransforms.h | $(OBJDIR)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/ADT/%.c include/ADT/Matrix.h | $(OBJDIR)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/Image/%.c include/Image/Image.h include/Image/ImageTransforms.h | $(OBJDIR)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(BINDIR):
	mkdir -p $(BINDIR)
