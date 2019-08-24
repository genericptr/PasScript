{$ifdef INTERFACE}

// assignment
operator explicit (right: Variant): TValue; {$i inline_operator}

operator explicit (source: TValue): integer;
operator explicit (source: TValue): string;
operator explicit (source: TValue): float;
operator explicit (source: TValue): boolean;

operator := (right: TValue): integer; {$i inline_operator}
operator := (right: TValue): float; {$i inline_operator}
operator := (right: TValue): string; {$i inline_operator}
operator := (right: TValue): ansistring; {$i inline_operator}
operator := (right: TValue): pchar; {$i inline_operator}
operator := (right: TValue): boolean; {$i inline_operator}

operator := (right: integer): TValue; {$i inline_operator}
operator := (right: float): TValue; {$i inline_operator}
operator := (right: string): TValue; {$i inline_operator}
operator := (right: boolean): TValue; {$i inline_operator}

// comparison
// note: we need add {} to fix syntax parsing for "operator = "
{} operator = (left: TValue; right: integer): boolean; {$i inline_operator}
{} operator = (left: TValue; right: float): boolean; {$i inline_operator}
{} operator = (left: TValue; right: string): boolean; {$i inline_operator}
{} operator = (left: TValue; right: boolean): boolean; {$i inline_operator}
{} 
{} operator = (left: integer; right: TValue): boolean; {$i inline_operator}
{} operator = (left: float; right: TValue): boolean; {$i inline_operator}
{} operator = (left: string; right: TValue): boolean; {$i inline_operator}
{} operator = (left: boolean; right: TValue): boolean; {$i inline_operator}

operator > (left: TValue; right: integer): boolean; {$i inline_operator}
operator > (left: TValue; right: float): boolean; {$i inline_operator}
operator > (left: TValue; right: string): boolean; {$i inline_operator}
operator > (left: TValue; right: boolean): boolean; {$i inline_operator}

operator < (left: TValue; right: integer): boolean; {$i inline_operator}
operator < (left: TValue; right: float): boolean; {$i inline_operator}
operator < (left: TValue; right: string): boolean; {$i inline_operator}
operator < (left: TValue; right: boolean): boolean; {$i inline_operator}

//operator >= (left: TValue; right: integer): boolean; {$i inline_operator}
//operator >= (left: TValue; right: float): boolean; {$i inline_operator}
//operator >= (left: TValue; right: string): boolean; {$i inline_operator}
//operator >= (left: TValue; right: boolean): boolean; {$i inline_operator}

//operator <= (left: TValue; right: integer): boolean; {$i inline_operator}
//operator <= (left: TValue; right: float): boolean; {$i inline_operator}
//operator <= (left: TValue; right: string): boolean; {$i inline_operator}
//operator <= (left: TValue; right: boolean): boolean; {$i inline_operator}

//operator <> (left: TValue; right: integer): boolean; {$i inline_operator}
//operator <> (left: TValue; right: float): boolean; {$i inline_operator}
//operator <> (left: TValue; right: string): boolean; {$i inline_operator}
//operator <> (left: TValue; right: boolean): boolean; {$i inline_operator}

// arithmetic
operator + (left: TValue; right: integer): integer; {$i inline_operator}
operator + (left: TValue; right: float): float; {$i inline_operator}
operator + (left: TValue; right: string): string; {$i inline_operator}

operator + (left: integer; right: TValue): integer; {$i inline_operator}
operator + (left: float; right: TValue): float; {$i inline_operator}
operator + (left: string; right: TValue): string; {$i inline_operator}

// unary
operator not (left: TValue): boolean;

{$endif}

{$ifdef IMPLEMENTATION}

//$bookmark assignment operators

//  TValueType = (TIntegerType, TFloatType, TStringType, TBoolType, TListType, TDictType);
operator explicit (source: TValue): integer;
begin
  case source.typ of
    TIntegerType:
      result := TIntegerValue(source).value;
    TStringType:
      result := TStringValue(source).value.ToInt;
    TFloatType:
      result := trunc(TFloatValue(source).value);
    TBoolType:
      begin
        if TBooleanValue(source).value then
          result := 1
        else
          result := 0;
      end;
    otherwise
      Fatal('explicit operator failed');
  end;
end;

operator explicit (source: TValue): string;
begin
  case source.typ of
    TIntegerType:
      result := TIntegerValue(source).value.ToStr;
    TStringType:
      result := TStringValue(source).value;
    TFloatType:
      result := TFloatValue(source).value.ToStr;
    TBoolType:
      result := ToStr(TBooleanValue(source).value);
    otherwise
      Fatal('explicit operator => string failed');
  end;
end;

operator explicit (source: TValue): float;
begin
  case source.typ of
    TIntegerType:
      result := TIntegerValue(source).value;
    TStringType:
      result := TStringValue(source).value.ToFloat;
    TFloatType:
      result := TFloatValue(source).value;
    TBoolType:
      begin
        if TBooleanValue(source).value then
          result := 1
        else
          result := 0;
      end;
    otherwise
      Fatal('explicit operator => float failed');
  end;
end;

operator explicit (source: TValue): boolean;
begin
  case source.typ of
    TIntegerType:
      result := TIntegerValue(source).value = TIntegerValue.DefaultValue;
    TStringType:
      result := TStringValue(source).value = TStringValue.DefaultValue;
    TFloatType:
      result := TFloatValue(source).value = TFloatValue.DefaultValue;
    TBoolType:
      result := TBooleanValue(source).value
    otherwise
      Fatal('explicit operator => boolean failed');
  end;
end;


// https://wiki.freepascal.org/Variant
operator explicit (right: Variant): TValue;
begin
  case varType(right) of
    varString, varStrArg, varOleStr:
      result := TStringValue.Create(ansistring(right));
    varSingle, varDouble:
      result := TFloatValue.Create(float(right));
    varShortInt, varSmallint, varInteger, varByte, varWord:
      result := TIntegerValue.Create(integer(right));
    varBoolean:
      result := TBooleanValue.Create(boolean(right));
    otherwise
      fatal('variant conversion not supported '+varToStr(right));
  end;
end;

operator := (right: TValue): integer;
begin
  if right.typ = TIntegerType then
    result := TIntegerValue(right).value
  else
    result := TIntegerValue.DefaultValue;
end;

operator := (right: TValue): float;
begin
  if right.typ = TFloatType then
    result := TFloatValue(right).value
  else
    result := TFloatValue.DefaultValue;
end;

operator := (right: TValue): string;
begin
  result := right.str;
end;

operator := (right: TValue): ansistring;
begin
  result := right.str;  
end;

operator := (right: TValue): pchar;
begin
  if right.typ = TStringType then
    result := TStringValue(right).GetPChar
  else
    result := '';
end;

operator := (right: TValue): boolean;
begin
  result := right.IsValid;
end;

operator := (right: integer): TValue;
begin
  result := TIntegerValue.Create(right);
end;

operator := (right: float): TValue;
begin
  result := TFloatValue.Create(right);
end;

operator := (right: string): TValue;
begin
  result := TStringValue.Create(right);
end;

operator := (right: boolean): TValue;
begin
  result := TBooleanValue.Create(right);
end;

//$bookmark arithmetic operators

operator + (left: TValue; right: integer): integer;
begin
  result := TIntegerValue(left).value + right;
end;

operator + (left: TValue; right: float): float;
begin
  result := TFloatValue(left).value + right;
end;

operator + (left: TValue; right: string): string;
begin
  result := TStringValue(left).value + right;
end;

operator + (left: integer; right: TValue): integer;
begin
  result := left + TIntegerValue(right).value;
end;

operator + (left: float; right: TValue): float;
begin
  result := left + TFloatValue(right).value;
end;

operator + (left: string; right: TValue): string;
begin
  result := left + TStringValue(right).value;
end;

//$bookmark equality operators

operator = (left: TValue; right: integer): boolean;
begin
  result := TIntegerValue(left) = right;
end;

operator = (left: TValue; right: float): boolean;
begin
  result := TFloatValue(left) = right;
end;

operator = (left: TValue; right: string): boolean;
begin
  result := TStringValue(left) = right;
end;

operator = (left: TValue; right: boolean): boolean;
begin
  result := TBooleanValue(left) = right;
end;

operator = (left: integer; right: TValue): boolean;
begin
  result := left = TIntegerValue(right);
end;

operator = (left: float; right: TValue): boolean;
begin
  result := left = TFloatValue(right);
end;

operator = (left: string; right: TValue): boolean;
begin
  result := left = TStringValue(right);
end;

operator = (left: boolean; right: TValue): boolean;
begin
  result := left = TBooleanValue(right);
end;

//$bookmark relational
operator > (left: TValue; right: integer): boolean;
begin
  result := TIntegerValue(left).value > right;
end;

operator > (left: TValue; right: float): boolean;
begin
  result := TFloatValue(left).value = right;
end;

operator > (left: TValue; right: string): boolean;
begin
  result := TStringValue(left).value = right;
end;

operator > (left: TValue; right: boolean): boolean;
begin
  result := TBooleanValue(left).value = right;
end;

operator < (left: TValue; right: integer): boolean;
begin
  result := TIntegerValue(left).value > right;
end;

operator < (left: TValue; right: float): boolean;
begin
  result := TFloatValue(left).value = right;
end;

operator < (left: TValue; right: string): boolean;
begin
  result := TStringValue(left).value = right;
end;

operator < (left: TValue; right: boolean): boolean;
begin
  result := TBooleanValue(left).value = right;
end;

//$bookmark unary operators
operator not (left: TValue): boolean;
begin
  result := not left.IsValid;
end;

{$endif}

{$ifdef CODE}
{$endif}