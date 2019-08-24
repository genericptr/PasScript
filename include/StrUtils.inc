{$ifdef INTERFACE}

// TODO: we need to make generic versions of these because ansistrings need to allocate new strings
// wait until we get the implicit spez patch applied

type
  LongString = ansistring;
  LongStringIndex = longint;

  ShortString = string;
  ShortStringIndex = integer;

type
  TCharSet = set of char;

const
  WhiteSpaceCharacters = [chr(32), chr(12), chr(10)];

function Explode(input: ansistring; delimiter: string): TList; 
function Implode(constref list: TList; glue: string = ''): ansistring;
function Substr(subject: ansistring; const start: longint; len: longint = 0): ansistring; inline;
function Substr(subject: string; const start: integer; len: integer = 0): string; inline;
function Strpos(subject: ansistring; search: ansistring; offset: longint = 0): longint;
function Replace(search: LongString; replacement: LongString; subject: LongString; replaceAll: boolean = true): LongString; overload;
function Replace(subject: LongString; replacement: LongString; start: LongStringIndex; len: LongStringIndex): LongString; overload;
function StrCopy(subject: LongString; offset: LongStringIndex; count: LongStringIndex): LongString;
function StrCopyAt(subject: LongString; startPos, endPos: LongStringIndex): LongString;
function StrCaseCmp(left, right: string): boolean;

function HasPrefix(subject: ansistring; prefix: string): boolean;
function HasSuffix(subject: ansistring; suffix: string): boolean;

function Trim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;
function LTrim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;
function RTrim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;

function Wrap(subject: ansistring; str: string = '"'): ansistring; inline;
function AddSlashes(subject: LongString): LongString; inline;

// Aliases
function str_replace(search: LongString; replacement: LongString; subject: LongString): LongString;
// TODO: substr_replace
// https://www.php.net/manual/en/function.substr-replace.php
function is_numeric(value: ansistring): boolean; inline;

type
  TLongStringHelpers = type helper for LongString
    procedure Show; inline;
    function Length: LongStringIndex; inline;
    function High: LongStringIndex; inline;
    function Explode(delimiter: string): TList; inline;
    function Sub(start: LongStringIndex; len: LongStringIndex = 0): LongString; inline;
    function Pos(search: string): LongStringIndex; inline;
    procedure Expand(count: LongStringIndex); inline;
    procedure Contract(count: LongStringIndex); inline;
    function HasPrefix(prefix: string): boolean; inline;
    function HasSuffix(suffix: string): boolean; inline;
    function ToInt: integer; inline;
    function ToFloat: float; inline;
    function ToBool: boolean; inline;
  end;

type
  TShortStringHelpers = type helper for ShortString
    procedure Show; inline;
    function Length: ShortStringIndex; inline;
    function High: ShortStringIndex; inline;
    function Explode(delimiter: string): TList; inline;
    function Sub(start: ShortStringIndex; len: ShortStringIndex = 0): ShortString; inline;
    function Pos(search: string): ShortStringIndex; inline;
    function HasPrefix(prefix: string): boolean; inline;
    function HasSuffix(suffix: string): boolean; inline;
    function ToInt: integer; inline;
    function ToFloat: float; inline;
    function ToBool: boolean; inline;
  end;

{ Implicit String Conversions }
operator := (right: longint): string;
operator := (right: float): string;

{ String/Numeric conversions }
function ToStr(value: longint): string; overload; inline;
function ToStr(value: double): string; overload; inline;
function ToStr(value: boolean): string; overload; inline;

function ToInt(value: string): integer; inline;
function ToFloat(value: string): float; inline;
function ToBool(value: string): boolean; inline;

function FormatString(value: ansistring; formatting: TStringFormatting): ansistring;

type
  TIntegerStringHelper = type helper for integer
    function ToStr: string; inline;
  end;

type
  TFloatStringHelper = type helper for float
    function ToStr: string; inline;
  end;

type
  TBooleanStringHelper = type helper for float
    function ToStr: string; inline;
  end;

{$endif}

{$ifdef IMPLEMENTATION}

//$bookmark -
//$bookmark PRIVATE FUNCTIONS
//$bookmark -

type
  TPrivateLongStringHelpers = type helper for LongString
    procedure AppendBytes(buffer: pointer; len: integer);
  end;

function StrAppendBytes(var subject: LongString; buffer: pointer; len: integer): LongString;
begin
  subject.Expand(len);
  Move(buffer^, subject[length(subject) - len + 1], len);
  result := subject;
end;

procedure TPrivateLongStringHelpers.AppendBytes(buffer: pointer; len: integer);
begin
  StrAppendBytes(self, buffer, len);
end;

//$bookmark -
//$bookmark OPERATORS
//$bookmark -

operator := (right: integer): string;
begin
  result := IntToStr(right);
end;

operator := (right: float): string;
begin
  result := FloatToStr(right);
end;

function ToStr(value: longint): string;
begin
  result := IntToStr(value);
end;

function ToStr(value: double): string;
begin
  result := FloatToStr(value);
end;

function ToStr(value: boolean): string;
begin
  if value then
    result := 'true'
  else
    result := 'false';
end;

function ToInt(value: string): integer;
begin
  result := StrToInt(value);
end;

function ToFloat(value: string): float;
begin
  result := StrToFloat(value);
end;

function ToBool(value: string): boolean;
begin
  if strcasecmp(value, 'true') then
    result := true
  else if strcasecmp(value, 'false') then
    result := false
  else
    fatal('toint string is not valid boolean.');
end;

function FormatString(value: ansistring; formatting: TStringFormatting): ansistring;
begin
  result := value;
  if TStringFormatJSON in formatting then
    result := '"'+result+'"';
end;

//$bookmark -
//$bookmark ORDINAL HELPERS
//$bookmark -

function TIntegerStringHelper.ToStr: string;
begin
  result := PasScript.ToStr(self);
end;

function TFloatStringHelper.ToStr: string;
begin
  result := PasScript.ToStr(self);
end;

function TBooleanStringHelper.ToStr: string;
begin
  result := PasScript.ToStr(self);
end;


//$bookmark -
//$bookmark LONG STRING HELPERS
//$bookmark -

procedure TLongStringHelpers.Show;
begin
  writeln(self);
end;

function TLongStringHelpers.Length: LongStringIndex;
begin
  result := System.Length(self);
end;

function TLongStringHelpers.High: LongStringIndex;
begin
  result := System.Length(self) - 1;
end;

function TLongStringHelpers.Explode(delimiter: string): TList; 
begin
  result := PasScript.Explode(self, delimiter);
end;

function TLongStringHelpers.Sub(start: LongStringIndex; len: LongStringIndex = 0): LongString;
begin
  result := PasScript.Substr(self, start, len);
end;

function TLongStringHelpers.Pos(search: string): LongStringIndex;
begin
  result := PasScript.Strpos(self, search);
end;

procedure TLongStringHelpers.Expand(count: LongStringIndex);
begin
  SetLength(self, System.Length(self) + count);
end;

procedure TLongStringHelpers.Contract(count: LongStringIndex);
begin
  SetLength(self, System.Length(self) - count);
end;

function TLongStringHelpers.HasPrefix(prefix: string): boolean;
begin
  result := PasScript.HasPrefix(self, prefix);  
end;

function TLongStringHelpers.HasSuffix(suffix: string): boolean;
begin
  result := PasScript.HasSuffix(self, suffix)  
end;

function TLongStringHelpers.ToInt: integer;
begin
  result := PasScript.ToInt(self);
end;
function TLongStringHelpers.ToFloat: float;
begin
  result := PasScript.ToFloat(self);
end;
function TLongStringHelpers.ToBool: boolean;
begin
  result := PasScript.ToBool(self);
end;

//$bookmark -
//$bookmark SHORT STRING HELPERS
//$bookmark -

procedure TShortStringHelpers.Show;
begin
  writeln(self);
end;

function TShortStringHelpers.Length: ShortStringIndex;
begin
  result := System.Length(self);
end;

function TShortStringHelpers.High: ShortStringIndex;
begin
  result := System.Length(self) - 1;
end;

function TShortStringHelpers.Explode(delimiter: string): TList; 
begin
  result := PasScript.Explode(self, delimiter);
end;

function TShortStringHelpers.Sub(start: ShortStringIndex; len: ShortStringIndex = 0): ShortString;
begin
  result := PasScript.Substr(self, start, len);
end;

function TShortStringHelpers.Pos(search: string): ShortStringIndex;
begin
  result := PasScript.Strpos(self, search);
end;

function TShortStringHelpers.HasPrefix(prefix: string): boolean;
begin
  result := PasScript.HasPrefix(self, prefix);  
end;

function TShortStringHelpers.HasSuffix(suffix: string): boolean;
begin
  result := PasScript.HasSuffix(self, suffix); 
end;

function TShortStringHelpers.ToInt: integer;
begin
  result := PasScript.ToInt(self);
end;
function TShortStringHelpers.ToFloat: float;
begin
  result := PasScript.ToFloat(self);
end;
function TShortStringHelpers.ToBool: boolean;
begin
  result := PasScript.ToBool(self);
end;

//$bookmark -
//$bookmark FUNCTIONS
//$bookmark -

function Compare(a, b: char): boolean; inline;
begin
  result := a = b;
end;

function Trim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;
var
  i: integer;
  left, right: integer;
begin
  i := 1;

  // trim left
  while subject[i] in mask do
    inc(i);
  left := i;

  // trim right
  i := length(subject);
  while subject[i] in mask do
    dec(i);
  right := i;

  result := strcopy(subject, left, (right - left) + 1);
end;

function LTrim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;
var
  i: integer;
  left: integer;
begin
  i := 1;
  while subject[i] in mask do
    inc(i);
  writeln(i);
  result := strcopy(subject, i, (length(subject) - i) + 1);
end;

// https://www.php.net/manual/en/function.rtrim.php
function RTrim(subject: ansistring; mask: TCharSet = WhiteSpaceCharacters): ansistring;
var
  i: integer;
  right: integer;
begin
  i := length(subject);
  while subject[i] in mask do
    dec(i);
  result := strcopy(subject, 1, i);
end;

// TODO: case sensitivity
function HasPrefix(subject: ansistring; prefix: string): boolean;
var
  i: longint;
begin
  result := true;
  for i := 1 to length(prefix) do
    if not Compare(subject[i], prefix[i]) then
      exit(false);
end;

function HasSuffix(subject: ansistring; suffix: string): boolean;
var
  i: longint;
begin
  fatal('not done');
  result := true;
  //for i := length(suffix) - length(suffix) to length(prefix) do
  //  if not Compare(subject[i], prefix[i]) then
  //    exit(false);
end;

function Wrap(subject: ansistring; str: string = '"'): ansistring;
begin
  result := str+subject+str;
end;

// https://www.php.net/manual/en/function.substr.php
generic function substr_internal<S, U>(subject: S; const start: U; len: U; offset: integer): S;
var
  i: longint;
  max: integer;
begin
  result := '';
  if len = 0 then
    exit;
  max := subject.high + offset;
  if len = 0 then
    len := max;
  if start >= 0 then
    begin
      if len >= 0 then
        begin
          i := start;
          while len > 0 do
            begin
              result += subject[i + offset];
              inc(i);
              dec(len);
            end;
        end
      else
        begin
          for i := start to (max - 1) + len do
            result += subject[i + offset];
        end;
    end
  else
    begin
      if len >= 0 then
        begin
          i := max + start;
          while len > 0 do
            begin
              result += subject[i + offset];
              inc(i);
              dec(len);
            end;
        end
      else
        begin
          for i := max + start to (max - 1) + len do
            result += subject[i + offset];
        end;
    end;
end;

function Substr(subject: ansistring; const start: longint; len: longint = 0): ansistring;
begin
  result := specialize substr_internal<ansistring, longint>(subject, start, len, 1);
end;

function Substr(subject: string; const start: integer; len: integer = 0): string;
begin
  result := specialize substr_internal<string, integer>(subject, start, len, 0);
end;

// https://www.php.net/manual/en/function.strpos.php
// https://www.php.net/manual/en/function.strrpos.php

// TODO: case sensitivity
function Strpos(subject: ansistring; search: ansistring; offset: longint = 0): longint;
var
  i, j: longint;
begin
  result := -1;
  for i := offset + 1 to length(subject) do
    begin
      if subject[i] = search[1] then
        begin
          result := i - 1;
          for j := 1 to length(search) do
            if search[j] <> subject[i + j - 1] then
              begin
                result := -1;
                break;
              end;
          if result <> -1 then
            exit;
        end;
    end;
end;

function StrCopy(subject: LongString; offset: LongStringIndex; count: LongStringIndex): LongString;
begin
  SetLength(result, count);
  Move(subject[offset], result[1], count);
end;

function StrCopyAt(subject: LongString; startPos, endPos: LongStringIndex): LongString;
var
  count, max: LongStringIndex;
begin
  SetLength(result, count);
  max := length(subject) - startPos;
  if endPos = -1 then
    count := max
  else
    begin
      count := endPos - startPos;
      if count > max then
        count := max;
    end;
  Move(subject[startPos], result[1], count);
end;

function StrCaseCmp(left, right: string): boolean;
var
  i: integer;
begin
  result := true;
  for i := 1 to length(left) do
    if UpperCase(left[i]) <> UpperCase(right[i]) then
      exit(false);
end;

// TODO: case sensitivity
// TODO: replace all
// https://www.php.net/manual/en/function.str-replace.php
function Replace(search: LongString; replacement: LongString; subject: LongString; replaceAll: boolean = true): LongString;
var
  i, j: LongStringIndex;
  last, offset: LongStringIndex;
  len, prev: LongStringIndex;
  output: LongString;
begin
  output := '';
  last := 1;
  i := 1;
  while i <= length(subject) do
    begin
      if subject[i] = search[1] then
        begin
          offset := i - 1;
          for j := 1 to length(search) do
            if search[j] <> subject[i + j - 1] then
              begin
                offset := -1;
                break;
              end;
          if offset <> -1 then
            begin
              len := offset - last + 1;
              prev := Length(output);
              if len > 0 then
                begin
                  output.Expand(len + Length(replacement));
                  Move(subject[last], output[prev + 1], len);
                end;
              // append replacement
              output += replacement;
              last := offset + Length(search) + 1;
              i := last - 1;
              // end search
              if not replaceAll then
                break;
            end;
        end;
      inc(i);
    end;
  if last <> i then
    begin
      len := length(subject) - last + 1;
      output += strcopy(subject, last, len);
    end;
  result := output;
end;


// alias for PHP
// https://www.php.net/manual/en/function.str-replace.php
function str_replace(search: LongString; replacement: LongString; subject: LongString): LongString;
begin
  result := Replace(search, replacement, subject, true);
end;

// TODO: make a custom function for this
function is_numeric(value: ansistring): boolean;
var
  i: integer;
  f: float;
begin
  if TryStrToInt(value, i) then
    result := true
  else if TryStrToFloat(value, f) then
    result := true
  else
    result := false;
end;

function AddSlashes(subject: LongString): LongString;
begin
  result := Replace('"', '\"', subject, true);
end;

// TODO: needs short string version
function Replace(subject: LongString; replacement: LongString; start: LongStringIndex; len: LongStringIndex): LongString;
begin
  SetLength(result, length(subject) - len + length(replacement));
  Move(subject[1], result[1], start);
  Move(replacement[1], result[start + 1], length(replacement));
  Move(subject[start + len + 1], result[start + length(replacement) + 1], length(subject) - start + len);
end;

function Implode(constref list: TList; glue: string = ''): ansistring;
var
  i: integer;
begin
  result := '';
  for i := 0 to list.high do
    if i < list.high then
      result += list[i] + glue
    else
      result += list[i];
end;

function Explode(input: ansistring; delimiter: string): TList;
var
  index: integer = 0;

  function c: char; inline;
  begin
    result := input[index];
  end;

var
  i: integer;
  part: ansistring;
  dstr: string;
begin
  result.Clear;

  if length(input) = 0 then
    exit;

  index := 1;
  part := '';
  dstr := '';
  while index <= length(input) do
    begin
      if c = delimiter[1] then
        begin
          dstr := '';
          for i := 0 to length(delimiter) - 1 do
            begin
              if input[index + i] = delimiter[i + 1] then
                dstr += input[index + i]
              else
                begin
                  part += dstr;
                  index += length(dstr);
                  dstr := '';
                  break;
                end;
            end;
          if length(dstr) = length(delimiter) then
            begin
              if length(part) > 0 then
                result.Add(part);
              part := '';
              index += length(dstr);
              dstr := '';
            end;
        end
      else
        begin
          part += c;
          index += 1;
        end;
    end;
  if length(part) > 0 then
    result.Add(part);
end;

{$endif}

{$ifdef CODE}
{$endif}