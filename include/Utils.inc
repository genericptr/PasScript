{$ifdef INTERFACE}

procedure print_r (constref list: TList);
procedure print_r (constref dict: TDict);
procedure print_r (constref json: TJSON);

{$endif}

{$ifdef IMPLEMENTATION}

procedure print_r (constref list: TList);
begin
  writeln(list.MakeFormattedString('', [TStringFormatDebug]));
end;

procedure print_r (constref dict: TDict);
begin
  writeln(dict.MakeFormattedString('', [TStringFormatDebug]));
end;

procedure print_r (constref json: TJSON);
begin
  writeln(json.MakeFormattedString('', [TStringFormatDebug]));
end;

{$endif}

{$ifdef CODE}

{$endif}