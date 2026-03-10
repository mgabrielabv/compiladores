program MalPrograma
var
   numero : integer
   nombre : strng
   edad   : ineger
begin
   numero := 10
   nombre := 'Juan
   writeln('Hola mundo')
   
   if edad > 18 then
      writeln('Mayor')
   else
      writeln('Menor')
   
   while numero < 5 do
      numero := numero + 1
      writeln(numero)
   
   procedimientoNoDeclarado()
   
   (* Este comentario no se cierra
   
   begin
      x := 10
      y := 20
   
   { otro comentario no cerrado
   
   finalSinPunto 