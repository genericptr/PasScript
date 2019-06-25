{$ifdef IMPLEMENTATION}
procedure BlockMove(dest, src: pointer; size: SizeInt); 
begin 
  Move(src^, dest^, size);
end; 

function CopyMem (src: pointer; offset: integer; count: integer): pointer;
begin
  result := GetMem(count);
  BlockMove(result, src + offset, count);
end;

procedure ParseCommandLine;
const
  AlphaNumericCharSet = ['a'..'z', 'A'..'Z', '1'..'9', '0'];
var
  i, j: integer;
  param: ansistring;
  name: string;
  value: ansistring;
  flag: integer;
  quotes: boolean;
begin
  for i := 1 to ParamCount do
    begin
      param := objpas.paramstr(i);
      if param[1] = '-' then
        begin
          name := '';
          flag := -1;

          // determine length of flag (short or long)
          if param[2] = '-' then
            flag := 2
          else
            flag := 1;

          if flag > -1 then
            begin
              j := flag + 1;
              while j <= length(param) do
                begin
                  if param[j] in AlphaNumericCharSet then
                    begin
                      name += param[j];
                      inc(j);
                    end
                  else if param[j] = '=' then
                    begin
                      value := '';
                      inc(j);
                      // allow optional quote
                      if param[j] = '"' then
                        begin
                          quotes := true;
                          inc(j);
                        end
                      else
                        quotes := false;
                      // parse value
                      while j <= length(param) do
                        begin
                          value += param[j];
                          inc(j);
                        end;
                      // quotes must be termianted
                      if quotes and (param[j] <> '"') then
                        fatal('flag '+name+' value not termianted with double-quote');
                    end
                  else
                    fatal('invalid arg '+param);
                end;
              
              // add to argv
              // TODO: bugs! argv needs to be actually used in unit it was defined
              argv[name] := value;
              //writeln('name:',name, ' value:', value);
            end;
        end
      else
        fatal('invalid arg '+param);
    end;
end;

{$endif}
