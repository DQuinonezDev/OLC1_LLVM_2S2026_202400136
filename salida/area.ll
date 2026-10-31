; LLVM IR generado por el compilador OLC1 a partir de area.olc
source_filename = "area.olc"

; Funciones del runtime de la placa (salida por el monitor serial)
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
