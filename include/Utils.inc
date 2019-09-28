{$ifdef INTERFACE}

procedure Sleep(seconds: float);
function uuidgen: string;

procedure print_r (constref list: TList);
procedure print_r (constref dict: TDict);
procedure print_r (constref json: TJSON);

// ANSI Color Codes
const
  ANSI_FORE_BLACK           = 30;
  ANSI_FORE_RED             = 31;
  ANSI_FORE_GREEN           = 32;
  ANSI_FORE_YELLOW          = 33;
  ANSI_FORE_BLUE            = 34;
  ANSI_FORE_MAGENTA         = 35;
  ANSI_FORE_CYAN            = 36;
  ANSI_FORE_WHITE           = 37;
  ANSI_FORE_RESET           = 39;

  ANSI_BACK_BLACK           = 40;
  ANSI_BACK_RED             = 41;
  ANSI_BACK_GREEN           = 42;
  ANSI_BACK_YELLOW          = 43;
  ANSI_BACK_BLUE            = 44;
  ANSI_BACK_MAGENTA         = 45;
  ANSI_BACK_CYAN            = 46;
  ANSI_BACK_WHITE           = 47;
  ANSI_BACK_RESET           = 49;

  ANSI_STYLE_BOLD           = 1;
  ANSI_STYLE_ITALIC         = 3;
  ANSI_STYLE_UNDERLINE      = 4;
  ANSI_STYLE_BLINK          = 5;


procedure PrintColor(code: byte; str: ansistring);
procedure PrintColor(codes: array of byte; str: ansistring);

{$endif}

{$ifdef IMPLEMENTATION}

procedure PrintColor(code: byte; str: ansistring);
begin
  writeln(#&033,'[',code,'m',str,#&033,'[',0,'m');
end;

procedure PrintColor(codes: array of byte; str: ansistring);
var
  attrs: string;
  code: byte;
begin
  attrs := '';
  for code in codes do
    begin
      if attrs <> '' then
        attrs += ';';
      attrs += string(code);
    end;
  writeln(#&033,'[',attrs,'m',str,#&033,'[',0,'m');
end;

//procedure PrintColor(foreColor, backColor, style: byte; str: ansistring);
//var
//  attrs: string;
//begin
//  attrs := '';

//  if foreColor > 0 then
//    begin
//      if attrs <> '' then
//        attrs += ';';
//      attrs += string(foreColor);
//    end;
  
//  if style > 0 then
//    begin
//      if attrs <> '' then
//        attrs += ';';
//      attrs += string(style);
//    end;

//  if backColor > 0 then
//    begin
//      if attrs <> '' then
//        attrs += ';';
//      attrs += string(backColor);
//    end;

//  writeln(#&033,'[',attrs,'m',str,#&033,'[',0,'m');
//end;

procedure Sleep(seconds: float);
begin
  SysUtils.Sleep(Trunc(seconds * 1000));
end;

function uuidgen: string;
var
  guid: TGUID;
begin
  if CreateGUID(guid) = 0 then
    //result := GUIDToString(guid)
    // TODO: make our own converter
    // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    // strips { and } from string
    // https://docs.getlazarus.org/#rtl+system+tguid#
    result := substr(GUIDToString(guid), 2, -1)
  else
    Fatal('failed to generate UUID');
end;

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