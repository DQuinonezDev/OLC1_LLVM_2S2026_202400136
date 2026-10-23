CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -g
TARGET  = compilador

SRCS    = main.c ast.c
GEN     = parser.tab.c parser.tab.h lex.yy.c

all: $(TARGET)

$(TARGET): $(SRCS) parser.tab.c lex.yy.c ast.h
	$(CC) $(CFLAGS) -o $@ $(SRCS) parser.tab.c lex.yy.c

parser.tab.c parser.tab.h: parser.y
	bison -d -Wall -Wcounterexamples parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

clean:
	rm -f $(TARGET) $(TARGET).exe $(GEN) parser.output *.o

.PHONY: all clean
