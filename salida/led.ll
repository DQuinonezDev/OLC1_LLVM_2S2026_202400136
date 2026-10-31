; LLVM IR generado por el compilador OLC1 a partir de led.olc
source_filename = "led.olc"

; Funciones del runtime de la placa (salida por el monitor serial)
declare void @print_i32(i32)
declare void @print_f64(double)
declare void @mb_led_set(i32, i32)

define void @minibot_program() {
entry:
  ; linea 2: let x : i32
  ; linea 3: let y : i32
  ; linea 4: print : i32
  %t1 = mul i32 6, 7
  call void @print_i32(i32 %t1)
  ; linea 5: led on
  call void @mb_led_set(i32 13, i32 1)
  ret void
}
