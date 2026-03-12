program MiPrograma;
var
   numero, edad, x, y : integer;
   nombre            : string;
begin
   numero := 10;
   nombre := 'Juan';
   edad := 20;         

   writeln('Hola mundo');

   if edad > 18 then
   begin
      writeln('Mayor');
   end
   else
   begin
      writeln('Menor');
      while numero < 5 do
      begin
         numero := numero + 1;
         writeln(numero);
         x := 10;
         y := 20;
      end;
   end;
end.  
