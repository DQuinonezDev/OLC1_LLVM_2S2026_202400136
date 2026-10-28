# ============================================================================
#  Compilador OLC1 -> LLVM IR -> Arduino Mega 2560 (AVR)     carnet 202400136
#
#  1) Terminal MSYS2 UCRT64   (gcc, bison, flex, make)
#       make                      construye bin/compilador.exe
#       make ll   FILE=...        genera build/programa.ll
#       make test                 pruebas del compilador (errores incluidos)
#
#  2) Terminal MSYS2 CLANG64  (llc, clang, avr-gcc, avrdude)
#       make host FILE=...        ejecuta el .ll en la PC (sin placa)
#       make mega FILE=...        .ll -> .S -> .elf -> .hex para la Mega
#       make upload PUERTO=COM9   sube el .hex a la placa con avrdude
#       make run-tests-host       compara la salida de todos los ejemplos
# ============================================================================

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Isrc/ast -Isrc/semantic -Isrc/codegen -Isrc/util -Ibuild

BISON = bison
FLEX  = flex

ifeq ($(OS),Windows_NT)
EXEEXT = .exe
endif

TARGET = bin/compilador$(EXEEXT)

OBJECTS = build/main.o build/ast.o build/fuente.o build/symtab.o \
          build/semantic.o build/llvm_codegen.o \
          build/parser.tab.o build/lex.yy.o

# Programa a compilar. examples/led.olc -> build/led.ll, build/led.hex, ...
FILE   ?= examples/completo.olc
NAME   ?= $(basename $(notdir $(FILE)))
LL      = build/$(NAME).ll

# ---------------------------- Backend AVR -----------------------------------
LLC      ?= llc
CLANG    ?= clang
AVR_GCC  ?= avr-gcc
OBJCOPY  ?= avr-objcopy
AVRDUDE  ?= avrdude
MCU       = atmega2560
PUERTO   ?= COM9
# -mdouble=64: avr-gcc usa double de 4 bytes por defecto; LLVM usa 8.
AVR_FLAGS = -mmcu=$(MCU) -mdouble=64 -DF_CPU=16000000UL -Os -std=c11 -Wall -Wextra

.PHONY: all ll test host mega upload size run-tests-host clean

all: $(TARGET)

$(TARGET): $(OBJECTS) | bin
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# -Wcounterexamples: si hubiera un conflicto, Bison muestra un ejemplo.
build/parser.tab.c build/parser.tab.h: src/parser/parser.y | build
	$(BISON) -Wall -Wcounterexamples -d -o build/parser.tab.c $<

build/lex.yy.c: src/lexer/lexer.l build/parser.tab.h | build
	$(FLEX) -o $@ $<

HEADERS = src/ast/ast.h src/semantic/symtab.h src/semantic/semantic.h \
          src/codegen/llvm_codegen.h src/util/fuente.h

build/main.o: src/main.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
build/ast.o: src/ast/ast.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
build/fuente.o: src/util/fuente.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
build/symtab.o: src/semantic/symtab.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
build/semantic.o: src/semantic/semantic.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
build/llvm_codegen.o: src/codegen/llvm_codegen.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
# El codigo que generan Flex/Bison no es c11 estricto: se compila sin -Wpedantic.
build/parser.tab.o: build/parser.tab.c $(HEADERS)
	$(CC) $(CPPFLAGS) -std=gnu11 -g -c $< -o $@
build/lex.yy.o: build/lex.yy.c build/parser.tab.h $(HEADERS)
	$(CC) $(CPPFLAGS) -std=gnu11 -g -c $< -o $@

build bin:
	mkdir -p $@

# ------------------------------- Front-end ----------------------------------
ll: $(LL)

$(LL): $(FILE) $(TARGET) | build
	./$(TARGET) --ast --tabla $(FILE) -o $@

test: $(TARGET)
	sh tests/run_tests.sh ./$(TARGET)

# -------------------- Ejecutar el mismo .ll en la PC -------------------------
host: $(LL)
	$(CLANG) -Wno-override-module $(LL) runtime/host/runtime_host.c \
		runtime/common/formato.c -o build/$(NAME)_host$(EXEEXT)
	./build/$(NAME)_host$(EXEEXT)

run-tests-host: $(TARGET)
	sh tests/run_tests.sh ./$(TARGET) --host

# --------------------------- Arduino Mega 2560 -------------------------------
mega: build/$(NAME).hex size

# 1. llc: LLVM IR (independiente) -> ensamblador AVR (dependiente de la placa)
build/$(NAME).S: $(LL)
	$(LLC) -mtriple=avr -mcpu=$(MCU) -filetype=asm $< -o $@

# 2. avr-gcc ensambla el .S generado por LLVM
build/$(NAME).o: build/$(NAME).S
	$(AVR_GCC) -mmcu=$(MCU) -c $< -o $@

# 3. runtime de la placa (USART + LED) y su main()
build/mega_runtime.o: runtime/mega2560/runtime_mega.c runtime/runtime.h runtime/common/formato.h | build
	$(AVR_GCC) $(AVR_FLAGS) -c $< -o $@
build/mega_formato.o: runtime/common/formato.c runtime/common/formato.h | build
	$(AVR_GCC) $(AVR_FLAGS) -c $< -o $@
build/mega_main.o: runtime/mega2560/main_mega.c runtime/runtime.h | build
	$(AVR_GCC) $(AVR_FLAGS) -c $< -o $@

# 4. enlazar programa + runtime -> firmware ELF
build/$(NAME).elf: build/$(NAME).o build/mega_runtime.o build/mega_formato.o build/mega_main.o
	$(AVR_GCC) -mmcu=$(MCU) -mdouble=64 -Wl,--no-warn-execstack $^ -o $@

# 5. ELF -> Intel HEX (el formato que entiende el bootloader de la Mega)
build/$(NAME).hex: build/$(NAME).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

size: build/$(NAME).elf
	avr-size -C --mcu=$(MCU) $<

# 6. subir por USB. -c wiring = bootloader stk500v2 de la Mega
upload: build/$(NAME).hex
	$(AVRDUDE) -p m2560 -c wiring -P $(PUERTO) -b 115200 -D -U flash:w:$<:i

clean:
	rm -rf build bin
