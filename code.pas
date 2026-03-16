program ProgramaValido;
var
   contador, limite, total : integer;
   nombre : string;
begin
   contador := 0;
   limite := 5;
   total := 0;
   nombre := 'Gaby';

   while contador < limite do
   begin
      total := total + contador;
      contador := contador + 1;
   end;

   if total > 0 then
   begin
      writeln(nombre);
      writeln(total);
   end
   else
   begin
      writeln('Sin datos');
   end;
end.
