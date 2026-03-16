program PruebaSemantico;
var
   entero : integer;
   texto  : string;
begin
   entero := 10;
   texto := 'hola';

   entero := texto;
   entero := 10 / 0;
   entero := 5.7;
   entero := texto + 1;
   entero := noDeclarada + 2;
end.
