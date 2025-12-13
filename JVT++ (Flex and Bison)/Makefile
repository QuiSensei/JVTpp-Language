CC = gcc
CFLAGS = -Wall -g

all: jvtpp

JVTpp.tab.c JVTpp.tab.h: JVTpp.y
	bison -d JVTpp.y

lex.yy.c: JVTpp.l JVTpp.tab.h
	flex JVTpp.l

jvtpp: lex.yy.c JVTpp.tab.c JVTpp_tools.c
	$(CC) $(CFLAGS) -o jvtpp JVTpp.tab.c lex.yy.c JVTpp_tools.c

clean:
	rm -f jvtpp lex.yy.c JVTpp.tab.c JVTpp.tab.h


.PHONY: all clean test