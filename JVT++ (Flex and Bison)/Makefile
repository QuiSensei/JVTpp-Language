CC = gcc
CFLAGS = -Wall -g
# Suppress warnings only for generated files
GENERATED_CFLAGS = -g -Wno-unused-function -Wno-unused-label

all: jvtpp

jvtpp: JVTpp.tab.c lex.yy.c JVTpp_tools.c
	$(CC) $(GENERATED_CFLAGS) -o jvtpp JVTpp.tab.c lex.yy.c JVTpp_tools.c

JVTpp.tab.c: JVTpp.y
	@echo "Compiling JVT++ interpreter..."
	bison -d JVTpp.y

lex.yy.c: JVTpp.l JVTpp.tab.h
	flex JVTpp.l

clean:
	@echo "Cleaning up..."
	rm -f jvtpp lex.yy.c JVTpp.tab.c JVTpp.tab.h

test: jvtpp
	@echo "Running test..."
	@echo 'develop[NUMERAL x = 5; NUMERAL y = 10; show("x + y = %d", x + y);]finish' | ./jvtpp

.PHONY: all clean test