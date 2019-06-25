{$ifdef INTERFACE}

// TODO: we need to make generic versions of these because ansistrings need to allocate new strings
// wait until we get the implicit spez patch applied
ddd

type
  LongString = ansistring;
  LongStringIndex = longint;

  ShortString = string;
  ShortStringIndex = integer;

function Explode(input: ansistring; delimiter: string): TList; 
function Implode(constref list: TList; glue: string = ''): ansistring;
function Substr(subject: ansistring; const start: longint; len: longint = 0): ansistring; inline;
function Substr(subject: string; const start: integer; len: integer = 0): string; inline;
function Strpos(subject: ansistring; search: ansistring; offset: longint = 0): longint;
function Replace(search: LongString; replacement: LongString; subject: LongString; replaceAll: boolean = true): LongString; overload;
function Replace(subject: LongString; replacement: LongString; start: LongStringIndex; len: LongStringIndex): LongString; overload;
function StrCopy(subject: LongString; offset: LongStringIndex; count: LongStringIndex): LongString; inline;

function HasPrefix(subject: ansistring; prefix: string): boolean;
function HasSuffix(subject: ansistring; suffix: string): boolean;

type
  TLongStringHelpers = type helper for LongString
    function Length: LongStringIndex; inline;
    function High: LongStringIndex; inline;
    function Explode(delimiter: string): TList; inline;
    function Sub(start: LongStringIndex; len: LongStringIndex = 0): LongString; inline;
    function Pos(search: string): LongStringIndex; inline;
    procedure Expand(count: LongStringIndex); inline;
    procedure Contract(count: LongStringIndex); inline;
    function HasPrefix(prefix: string): boolean; inline;
    function HasSuffix(suffix: string): boolean; inline;
  end;

type
  TShortStringHelpers = type helper for ShortString
    function Length: ShortStringIndex; inline;
    function High: ShortStringIndex; inline;
    function Explode(delimiter: string): TList; inline;
    function Str(start: ShortStringIndex; len: ShortStringIndex = 0): ShortString; inline;
    function Pos(search: string): ShortStringIndex; inline;
    function HasPrefix(prefix: string): boolean; inline;
    function HasSuffix(suffix: string): boolean; inline;
  end;

// implicit string conversions
operator := (right: integer): string;
operator := (right: float): string;

// string conversions
function tostr(value: integer): string; overload; inline;
function tostr(value: float): string; overload; inline;
function tostr(value: boolean): string; overload; inline;

function toint(value: string): integer; overload; inline;
function toint(value: string): float; overload; inline;
function toint(value: string): boolean; overload; inline;

{$endif}

{$ifdef IMPLEMENTATION}

operator := (right: integer): string;
begin
  result := IntToStr(right);
end;

operator := (right: float): string;
begin
  result := FloatToStr(right);
end;

function tostr(value: integer): string;
begin
  result := IntToStr(value);
end;

function tostr(value: float): string;
begin
  result := FloatToStr(value);
end;

function tostr(value: boolean): string;
begin
  if value then
    result := 'true'
  else
    result := 'false';
end;

function toint(value: string): integer;
begin
  result := StrToInt(value);
end;

function toint(value: string): float;
begin
  result := StrToFloat(value);
end;

function toint(value: string): boolean;
begin
  if value = 'true' then
    result := true
  else if value = 'false' then
    result := false
  else
    fatal('toint string is not valid boolean.');
end;


//$bookmark -
//$bookmark STRING HELPERS
//$bookmark -

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

function TLongStringHelpers.Substr(start: LongStringIndex; len: LongStringIndex = 0): LongString;
begin
  result := PasScript.Substr(self, start, len);
end;

function TLongStringHelpers.Strpos(search: string): LongStringIndex;
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
  result := HasPrefix(self, prefix);  
end;

function TLongStringHelpers.HasSuffix(suffix: string): boolean;
begin
  result := HasSuffix(self, prefix)  
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

function TShortStringHelpers.Substr(start: ShortStringIndex; len: ShortStringIndex = 0): ShortString;
begin
  result := PasScript.Substr(self, start, len);
end;

function TShortStringHelpers.Strpos(search: string): ShortStringIndex;
begin
  result := PasScript.Strpos(self, search);
end;

function TShortStringHelpers.HasPrefix(prefix: string): boolean;
begin
  result := HasPrefix(self, prefix);  
end;

function TShortStringHelpers.HasSuffix(suffix: string): boolean;
begin
  result := HasSuffix(self, prefix)  
end;

//$bookmark -
//$bookmark FUNCTIONS
//$bookmark -

function Compare(a, b: char): boolean; inline
begin
  result := a = b;
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
  result := true;
  for i := length(prefix) - length(suffix) to length(prefix) do
    if not Compare(subject[i], prefix[i]) then
      exit(false);
end;


// https://www.php.net/manual/en/function.substr.php
generic function substr_internal<S, U>(subject: S; const start: U; len: U; offset: integer): S;
var
  i: longint;
  max: integer;
begin
  result := '';
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