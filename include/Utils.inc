{$ifdef INTERFACE}

procedure Fatal(condition: boolean; msg: string); overload;
procedure Fatal(msg: string); overload;

procedure Halt(code: integer = 0; msg: string = '');
{$endif}

{$ifdef IMPLEMENTATION}
procedure Fatal(condition: boolean; msg: string);
begin
  if condition then
    Fatal(msg);
end;

procedure Fatal(msg: string);
begin
  writeln('FATAL: '+msg);
  //system.halt(code);
  raise Exception.Create(msg);
end;

procedure Halt(code: integer = 0; msg: string = '');
begin
  if msg <> '' then
    writeln(msg);
  system.halt(code);
end;

{$endif}

{$ifdef CODE}

{$endif}