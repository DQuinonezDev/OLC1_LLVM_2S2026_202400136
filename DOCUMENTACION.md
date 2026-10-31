# Documentación técnica — Compilador de expresiones a LLVM IR (AVR)

**Curso:** Organización de Lenguajes y Compiladores 1 — 2S2026
**Carné:** 202400136 → `202400136 mod 3 = 0` → **Arduino Mega 2560 (AVR / ATmega2560)**

---

## 1. Descripción general

Este proyecto es un compilador pequeño escrito en C que recibe un programa con
variables, expresiones aritméticas e impresión, y produce un archivo **LLVM IR**
(`.ll`). Ese archivo se traduce después con el backend AVR de LLVM (`llc`) y se
enlaza con un runtime para obtener el firmware (`.hex`) que se carga en la
Arduino Mega 2560. La salida de `print` aparece en el monitor serial.

```text
programa.olc ─► Flex ─► Bison ─► AST ─► Semántico ─► LLVM IR ─► llc ─► avr-gcc ─► .hex ─► Mega
               tokens  gramática       tipos+tabla   programa.ll  .S    .elf        avrdude
└──────────────────── front-end (este compilador) ────────────────┘└──── backend AVR ────┘
```

Fases implementadas:

| Fase | Archivo | Resultado |
|---|---|---|
| Léxico | `src/lexer/lexer.l` | Tokens con línea y columna |
| Sintáctico | `src/parser/parser.y` | AST |
| Semántico | `src/semantic/semantic.c`, `symtab.c` | AST tipado + tabla de símbolos |
| Generación | `src/codegen/llvm_codegen.c` | Archivo `.ll` |
| Runtime | `runtime/mega2560/` | `print_i32`, `print_f64`, `mb_led_set` sobre el hardware |

## 2. Instrucciones para compilar y ejecutar

### 2.1 Requisitos (Windows con MSYS2)

| Terminal MSYS2 | Paquetes | Para qué |
|---|---|---|
| **UCRT64** | `mingw-w64-ucrt-x86_64-gcc make bison flex` | construir el compilador |
| **CLANG64** | `mingw-w64-clang-x86_64-llvm` (llc, llvm-as), `mingw-w64-clang-x86_64-clang` (opcional, para ejecutar en PC), `mingw-w64-clang-x86_64-avr-gcc avr-libc avr-binutils avrdude` | backend AVR y carga |

```bash
# UCRT64
pacman -S mingw-w64-ucrt-x86_64-gcc make bison flex
# CLANG64
pacman -S mingw-w64-clang-x86_64-llvm mingw-w64-clang-x86_64-clang \
          mingw-w64-clang-x86_64-avr-gcc mingw-w64-clang-x86_64-avr-libc \
          mingw-w64-clang-x86_64-avr-binutils mingw-w64-clang-x86_64-avrdude
```

### 2.2 Construir el compilador (UCRT64)

```bash
make                    # genera bin/compilador.exe
make test               # 5 programas válidos + 10 programas con errores
```

### 2.3 Usar el compilador

```bash
./bin/compilador.exe [--ast] [--tabla] [--entry nombre] [-o salida.ll] entrada.olc
```

| Opción | Efecto |
|---|---|
| `-o salida.ll` | Archivo de salida (por defecto, el mismo nombre con `.ll`) |
| `--ast` | Imprime el AST con el tipo de cada nodo |
| `--tabla` | Imprime la tabla de símbolos (nombre, tipo, valor LLVM) |
| `--entry nombre` | Nombre de la función generada (por defecto `minibot_program`, la que llama el runtime) |

Código de salida: `0` si se generó el `.ll`; `1` si hubo cualquier error (en ese caso
**no** se crea el archivo `.ll`).

### 2.4 Generar el firmware para la Mega (CLANG64)

```bash
make mega FILE=examples/completo.olc      # build/completo.ll -> .S -> .elf -> .hex
make upload FILE=examples/completo.olc PUERTO=COM9
```

Para comprobar el `.ll` sin placa: `make host FILE=examples/completo.olc`
(requiere `clang` en CLANG64) o `make run-tests-host`, que ejecuta todos los
ejemplos y compara su salida con los archivos `.esperado`.

## 3. Estructura del proyecto

```text
.
├── Makefile                    construcción, pruebas y backend AVR
├── DOCUMENTACION.md
├── src/
│   ├── main.c                  orquesta las fases y procesa argumentos
│   ├── lexer/lexer.l           especificación Flex
│   ├── parser/parser.y         gramática Bison
│   ├── ast/ast.[ch]            nodos del AST, impresión y liberación
│   ├── semantic/symtab.[ch]    tabla de símbolos (entorno global)
│   ├── semantic/semantic.[ch]  verificación de tipos y símbolos
│   ├── codegen/llvm_codegen.[ch]  generación de LLVM IR
│   └── util/fuente.[ch]        muestra la línea del error subrayada
├── runtime/
│   ├── runtime.h               contrato entre el IR y la placa
│   ├── common/formato.[ch]     números a texto sin printf
│   ├── mega2560/runtime_mega.c USART0 a 115200 baudios + pines digitales
│   ├── mega2560/main_mega.c    main(): inicia UART y llama al programa
│   └── host/runtime_host.c     misma ABI con printf, para la PC
├── examples/                   programas válidos y su salida esperada
├── tests/errores/              programas que deben ser rechazados
├── tests/run_tests.sh
└── salida/                     .ll generados por el compilador (entregable)
```

## 4. Uso de Flex y Bison

### 4.1 Flex (`lexer.l`)

| Token | Expresión regular | Valor semántico |
|---|---|---|
| `LET`, `PRINT`, `LED`, `ON`, `OFF` | `"let"`, `"print"`, `"led"`, `"on"`, `"off"` | — |
| `DECIMAL` | `[0-9]+"."[0-9]+` | `double` (con `strtod`) |
| `ENTERO` | `[0-9]+` | `int` (error si supera 2147483647) |
| `ID` | `[a-zA-Z_][a-zA-Z_0-9]*` | copia del texto |
| símbolos | `+ - * / = ; ( )` | el propio carácter |
| ignorados | espacios, tabulaciones, saltos de línea, `// comentario` | — |

Las palabras reservadas aparecen antes que `ID`; ante dos reglas con la misma
longitud de lexema Flex elige la primera, por eso `let` es `LET` y `letra` es `ID`.
La macro `YY_USER_ACTION` guarda en `yylloc` la línea y columna de cada token;
Bison las expone como `@n` y terminan en el AST para los mensajes de error.
Cualquier carácter no reconocido produce un error léxico.

### 4.2 Bison (`parser.y`)

Bison genera un parser LALR(1). Cada producción tiene una acción en C que construye
el nodo del AST correspondiente (traducción dirigida por la sintaxis). La regla
`error ';'` permite recuperarse de un error sintáctico y seguir revisando las
instrucciones siguientes, para reportar varios errores en una sola ejecución.

## 5. Gramática y cómo se evitó la ambigüedad

```bison
programa      : instrucciones ;
instrucciones : instrucciones instruccion | instruccion ;
instruccion   : LET ID '=' expresion ';'
              | PRINT expresion ';'
              | LED ENTERO ON ';'             /* extensión: punto extra del LED */
              | LED ENTERO OFF ';'
              | error ';' ;
expresion     : expresion '+' termino | expresion '-' termino | termino ;
termino       : termino '*' factor    | termino '/' factor    | factor ;
factor        : ENTERO | DECIMAL | ID | '(' expresion ')' ;
```

Una gramática es **ambigua** cuando una misma entrada puede derivarse con dos
árboles distintos. Con `E : E '+' E | E '*' E | num`, la cadena `1 + 2 * 3` admite
`(1 + 2) * 3` y `1 + (2 * 3)`; Bison lo reporta como conflicto shift/reduce.

En esta gramática se eliminó la ambigüedad con la estructura de las producciones,
**sin** `%left`, `%right`, `%nonassoc`, `%precedence` ni `%prec`:

1. **Precedencia por niveles.** `expresion` solo combina `termino`s con `+`/`-`, y
   `termino` solo combina `factor`es con `*`/`/`. Para que un `+` sea operando de un
   `*` tiene que estar dentro de paréntesis (`factor : '(' expresion ')'`). Por eso
   `*` y `/` quedan siempre más abajo en el árbol y se evalúan primero.
2. **Asociatividad izquierda por recursión izquierda.** En `expresion : expresion '-'
   termino`, el operando derecho es un `termino`, que no puede contener un `-` suelto.
   `9 - 3 - 1` solo puede agruparse como `(9 - 3) - 1`.
3. **`led`** recibe solo un literal entero seguido de `ON`/`OFF` (igual que en el
   MiniBot del auxiliar), así que no interactúa con las reglas de expresiones.

Bison se invoca con `-Wall -Wcounterexamples` y genera el parser **sin conflictos**
shift/reduce ni reduce/reduce (no imprime ninguna advertencia).

Árbol de `10 + 20 * 3`:

```text
        +
       / \
     10   *
         / \
       20   3
```

## 6. Representación del AST

Todos los nodos usan la estructura `ASTNode` (`src/ast/ast.h`):

| Campo | Uso |
|---|---|
| `type` | `NODE_LET`, `NODE_PRINT`, `NODE_LED`, `NODE_ENTERO`, `NODE_DECIMAL`, `NODE_ID`, `NODE_BINOP` |
| `loc`, `op_loc` | Región del nodo y posición del operador (línea/columna) |
| `op` | `'+'`, `'-'`, `'*'`, `'/'` en `NODE_BINOP` |
| `ival`, `dval`, `name` | Valor del literal o nombre de la variable |
| `left`, `right` | Operandos de `BINOP`; `left` es la expresión de `LET`/`PRINT`, o el `ENTERO` del pin en `LED` |
| `tipo` | `T_INT`, `T_DOUBLE` o `T_ERROR`; lo asigna el análisis semántico |
| `next` | Siguiente instrucción: el programa es una lista enlazada de instrucciones |

Los paréntesis no generan nodo: `'(' expresion ')'` devuelve directamente el nodo
interior, porque la agrupación ya quedó expresada en la forma del árbol.

`./bin/compilador.exe --ast examples/basico.olc` muestra:

```text
PROGRAMA
|-- LET a  (linea 2)
|   `-- ENTERO(10) : i32
|-- LET b  (linea 3)
|   `-- ENTERO(20) : i32
|-- LET resultado  (linea 4)
|   `-- BINOP(+) : i32
|       |-- ID(a) : i32
|       `-- BINOP(*) : i32
|           |-- ID(b) : i32
|           `-- ENTERO(2) : i32
`-- PRINT  (linea 5)
    `-- ID(resultado) : i32
```

## 7. Análisis semántico

Se ejecuta sobre el AST completo **antes** de generar código, con un único entorno
global representado por `TablaSimbolos` (`src/semantic/symtab.c`). Cada entrada
guarda:

| Campo | Lo llena | Ejemplo |
|---|---|---|
| `nombre` | semántico | `area` |
| `tipo` | semántico (tipo de la expresión del `let`) | `double` |
| `valor` | generador: constante o temporal LLVM que representa la variable | `%t1` |
| `decl` | semántico: línea y columna de la declaración | línea 4 |

Reglas:

| Nodo | Regla |
|---|---|
| `ENTERO` / `DECIMAL` | tipo `i32` / `double` |
| `ID` | debe existir en la tabla → **variable no declarada** |
| `BINOP` | ambos operandos con el mismo tipo → **tipos incompatibles**; el resultado tiene ese tipo. No hay conversión implícita |
| `BINOP /` entero con divisor literal `0` | **división entera entre cero** (`sdiv` entre 0 no está definido en LLVM) |
| `LET x = e` | primero se revisa `e`; si `x` ya existe → **declaración duplicada**; si no, se inserta con el tipo de `e` |

Decisiones:

- **Se reportan todos los errores** del archivo, no solo el primero. Un error
  semántico detiene la generación de código, no el análisis.
- **Sin cascadas:** una expresión con error recibe `T_ERROR`, y quien la contiene no
  vuelve a reportar. Una variable cuya expresión falló se inserta con `T_ERROR` para
  que sus usos posteriores no aparezcan como "no declarada".
- En `let x = x + 1;` la `x` derecha se revisa antes de insertar la izquierda, así
  que se reporta como no declarada.

Ejemplo de reporte (`tests/errores/tipos_incompatibles.olc`):

```text
Error semantico en linea 3, columna 11: operacion entre tipos incompatibles
  Tipos    i32 + double
  Causa    el lenguaje no hace conversion implicita entre entero y decimal
     3 | let s = n + f;
       |         ^^^^^
Compilacion detenida: 1 error(es) semantico(s). No se genero LLVM IR.
```

## 8. Generación de LLVM IR

`src/codegen/llvm_codegen.c` recorre el AST ya validado:

- **Recorrido post-orden.** Para un `BINOP` se generan primero el hijo izquierdo y el
  derecho, y después se emite la instrucción del padre usando lo que devolvieron.
- **Las hojas no emiten instrucciones.** Un literal se usa directamente como
  constante (`10`, `20.0`); un `ID` se reemplaza por el `valor` guardado en la tabla.
- **Cada operación crea un temporal nuevo** `%t1`, `%t2`, … con un contador global
  que nunca se reinicia ni reutiliza (LLVM IR está en forma SSA: cada nombre se
  asigna una sola vez).
- **`let` no emite instrucciones:** solo guarda en la tabla el operando que produjo
  su expresión. Como el lenguaje no permite reasignar, una variable es exactamente
  ese valor, sin necesidad de `alloca`/`store`/`load`.
- **El tipo elige la instrucción:**

| Tipo | LLVM | `+` | `-` | `*` | `/` | print |
|---|---|---|---|---|---|---|
| Entero | `i32` | `add` | `sub` | `mul` | `sdiv` | `call void @print_i32(i32 v)` |
| Decimal | `double` | `fadd` | `fsub` | `fmul` | `fdiv` | `call void @print_f64(double v)` |

- **Constantes `double`:** LLVM exige punto decimal (`20.0`, no `20`). Se escribe la
  representación más corta que recupera exactamente el mismo valor.
- **Independiente del destino:** el `.ll` no lleva `target triple`; la arquitectura
  se elige después con `llc -mtriple=avr`. El mismo archivo se compila también para
  la PC en las pruebas.
- El programa queda dentro de `define void @minibot_program()`, la función que llama
  el `main()` del runtime. Las funciones de la placa solo se **declaran** (`declare`).

Ejemplo (`examples/area.olc` → `salida/area.ll`):

```llvm
declare void @print_i32(i32)
declare void @print_f64(double)
declare void @mb_led_set(i32, i32)

define void @minibot_program() {
entry:
  ; linea 2: let ancho : double
  ; linea 3: let alto : double
  ; linea 4: let area : double
  %t1 = fmul double 10.5, 20.0
  ; linea 5: print : double
  call void @print_f64(double %t1)
  ret void
}
```

## 9. Arquitectura asignada

`202400136 mod 3 = 0` → **Arduino Mega 2560**, microcontrolador **ATmega2560**,
arquitectura **AVR** de 8 bits a 16 MHz, 256 KB de flash y 8 KB de RAM.

El runtime (`runtime/mega2560/runtime_mega.c`) es el único componente que conoce
el hardware:

- `print_i32` / `print_f64`: convierten el número a texto (`runtime/common/formato.c`,
  sin `printf`) y lo envían por **USART0** a **115200 baudios, 8N1**. En la Mega,
  USART0 está conectado al chip USB, así que la salida aparece en el monitor serial.
- `mb_led_set(pin, estado)`: escribe los registros `DDRx` (dirección) y `PORTx`
  (nivel) del pin. El pin 13 corresponde a `PB7`, el LED integrado "L".

**`double` en AVR:** LLVM trata `double` como IEEE-754 de 64 bits, mientras que
`avr-gcc` usa 32 bits por defecto. Si no coinciden, `print_f64` leería mal los
registros. Por eso el runtime se compila y enlaza con **`-mdouble=64`**. Con eso,
los 8 bytes del valor viajan en los registros `r18`–`r25` de la misma forma en ambos
lados. Se verificó en el ensamblador generado: `210.0` (`0x406A400000000000`) se
carga como `r25=0x40, r24=0x6A, r23=0x40, r22..r18=0` antes de `call print_f64`.

## 10. Proceso para generar el código de la arquitectura

| # | Herramienta | Comando (lo ejecuta `make mega`) | Resultado |
|---|---|---|---|
| 1 | este compilador | `./bin/compilador.exe programa.olc -o build/programa.ll` | LLVM IR |
| 2 | `llc` (backend AVR de LLVM) | `llc -mtriple=avr -mcpu=atmega2560 -filetype=asm build/programa.ll -o build/programa.S` | ensamblador AVR |
| 3 | `avr-gcc` (ensamblador) | `avr-gcc -mmcu=atmega2560 -c build/programa.S -o build/programa.o` | objeto |
| 4 | `avr-gcc` (runtime) | `avr-gcc -mmcu=atmega2560 -mdouble=64 -Os -c runtime/...c` | objetos del runtime |
| 5 | `avr-gcc` (enlazador) | `avr-gcc -mmcu=atmega2560 -mdouble=64 *.o -o build/programa.elf` | firmware ELF |
| 6 | `avr-objcopy` | `avr-objcopy -O ihex -R .eeprom build/programa.elf build/programa.hex` | Intel HEX |
| 7 | `avrdude` | `avrdude -p m2560 -c wiring -P COM9 -b 115200 -D -U flash:w:build/programa.hex:i` | programa en la placa |

`-c wiring` usa el protocolo del bootloader de la Mega (STK500v2) y `-D` evita borrar
toda la flash. Comprobaciones útiles: `avr-nm --undefined-only build/programa.elf`
(no debe imprimir nada) y `avr-size -C --mcu=atmega2560 build/programa.elf`
(`completo.olc` ocupa unos 5.7 KB, el 2.2 % de la flash).

Si el docente proporciona su propio runtime, solo cambian los pasos 4–5. Si su
`main()` llama a una función con otro nombre, se usa `--entry nombre`.

## 11. Ejemplos de entrada y resultado esperado

### 11.1 `examples/completo.olc`

```text
let a = 10;
let b = 20;
let resultado = a + b * 2;      // precedencia: 10 + (20 * 2) = 50
print resultado;

let ancho = 10.5;
let alto = 20.0;
let area = ancho * alto;        // 210.0
print area;

print (a + b) * 2;              // parentesis: 60
print 9 - 3 - 1;                // asociatividad izquierda: (9 - 3) - 1 = 5
print 20 / 2 / 2;               // (20 / 2) / 2 = 5
print 7 / 2;                    // division entera (sdiv): 3
print 7.0 / 2.0;                // division decimal (fdiv): 3.5
print (ancho + alto) / 2.0;     // 15.25
print 1.0 / 3.0;                // 0.333333
```

Salida en el monitor serial (115200 baudios):

```text
50
210.0
60
5
5
3
3.5
15.25
0.333333
```

LLVM IR generado: [`salida/completo.ll`](salida/completo.ll).

### 11.2 Extensión: LED (`examples/led.olc`)

```text
let x = 6;
let y = 7;
print x * y;        // 42 en el monitor serial
led 13 on;          // el LED "L" de la placa queda encendido
```

```llvm
  %t1 = mul i32 6, 7
  call void @print_i32(i32 %t1)
  call void @mb_led_set(i32 13, i32 1)
```

Resultado: `42` en el monitor serial y el LED integrado (pin 13) encendido.
La sintaxis es la misma del MiniBot del auxiliar: `led NUMERO on;` / `led NUMERO off;`,
con el pin como literal entero. Pines soportados por el runtime: 2–13 (cualquier otro
se ignora).

### 11.3 Programas con error (`tests/errores/`)

| Archivo | Error esperado |
|---|---|
| `no_declarada.olc` | Semántico: variable no declarada (`resultado`) |
| `duplicada.olc` | Semántico: declaración duplicada (`n`) |
| `tipos_incompatibles.olc` | Semántico: `i32 + double` |
| `varios_errores.olc` | 4 errores semánticos reportados en una sola ejecución |
| `division_cero.olc` | Semántico: división entera entre cero |
| `led_decimal.olc` | Sintáctico: el pin de `led` debe ser un literal entero |
| `sintaxis.olc` | Sintáctico: falta `;` y falta `)` |
| `lexico.olc` | Léxico: carácter `#` no reconocido |
| `fuera_de_rango.olc` | Léxico: entero mayor que 2147483647 |
| `vacio.olc` | Sintáctico: programa sin instrucciones |

`make test` confirma que los ejemplos válidos compilan y que ningún programa con
error genera un `.ll`.
