; LLVM IR generado por el compilador OLC1 a partir de completo.olc
source_filename = "completo.olc"

; Funciones del runtime de la placa (salida por el monitor serial)
declare void @print_i32(i32)
declare void @print_f64(double)
declare void @mb_led_set(i32, i32)

define void @minibot_program() {
entry:
  ; linea 2: let a : i32
  ; linea 3: let b : i32
  ; linea 4: let resultado : i32
  %t1 = mul i32 20, 2
  %t2 = add i32 10, %t1
  ; linea 5: print : i32
  call void @print_i32(i32 %t2)
  ; linea 7: let ancho : double
  ; linea 8: let alto : double
  ; linea 9: let area : double
  %t3 = fmul double 10.5, 20.0
  ; linea 10: print : double
  call void @print_f64(double %t3)
  ; linea 12: print : i32
  %t4 = add i32 10, 20
  %t5 = mul i32 %t4, 2
  call void @print_i32(i32 %t5)
  ; linea 13: print : i32
  %t6 = sub i32 9, 3
  %t7 = sub i32 %t6, 1
  call void @print_i32(i32 %t7)
  ; linea 14: print : i32
  %t8 = sdiv i32 20, 2
  %t9 = sdiv i32 %t8, 2
  call void @print_i32(i32 %t9)
  ; linea 15: print : i32
  %t10 = sdiv i32 7, 2
  call void @print_i32(i32 %t10)
  ; linea 16: print : double
  %t11 = fdiv double 7.0, 2.0
  call void @print_f64(double %t11)
  ; linea 17: print : double
  %t12 = fadd double 10.5, 20.0
  %t13 = fdiv double %t12, 2.0
  call void @print_f64(double %t13)
  ; linea 18: print : double
  %t14 = fdiv double 1.0, 3.0
  call void @print_f64(double %t14)
  ret void
}
