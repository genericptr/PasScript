{$ifdef INTERFACE}

type
  float = single;

const
  kOrderedDescending = -1;
  kOrderedAscending = 1;
  kOrderedSame = 0;
type
  TOrdered = integer;

type
  TComparatorResult = integer;
  TComparatorResultHelper = type helper for TComparatorResult
    function GreaterThan: boolean;
    function LessThan: boolean;
    function GreaterThanOrEqualTo: boolean;
    function LessThanOrEqualTo: boolean;
    function EqualTo: boolean;
  end;

// TODO: implement these for TValue and TLIst, TDict
// TODO: use variant instead of overloads
type
  ICollection = interface
    procedure Add(value: variant);
    procedure Add(key: string; value: variant);
  end;

type
  TStringFormat = (TStringFormatJSON);
  TStringFormatting = set of TStringFormat;

type
  TValueType = (TIntegerType, TFloatType, TStringType, TBoolType, TListType, TDictType);
  PValue = ^TValue;
  TValue = class
    // TODO: make this part of the type data set
    public
      temp: boolean;
    private
      typ: TValueType;
    private
      function GetValue(index: integer): TValue; virtual;
      procedure SetValue(index: integer; newValue: TValue); virtual;
    public
      function Copy: TValue; virtual;
      function Str: ansistring; virtual;
      function DebugString: ansistring; virtual;
      function MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring; virtual;
      function IsValid: boolean; virtual;
      function Compare(other: TValue): TOrdered; virtual;
      procedure Show;
      property Subscript[index: integer]: TValue read GetValue write SetValue; default;
  end;
  TIntegerValue = class (TValue)
    public
      class function DefaultValue: integer;
      constructor Create (newValue: integer);
      function Copy: TValue; override;
      function Str: ansistring; override;
      function IsValid: boolean; override;
      function Compare(other: TValue): TOrdered; override;
    public
      value: integer;
  end;
  TFloatValue = class (TValue)
    public
      class function DefaultValue: float;
      constructor Create (newValue: float);
      function Copy: TValue; override;
      function Str: ansistring; override;
      function IsValid: boolean; override;
      function Compare(other: TValue): TOrdered; override;
    public
      value: float;
  end;
  TStringValue = class (TValue)
    public
      class function DefaultValue: string;
      constructor Create (newValue: ansistring);
      function Copy: TValue; override;
      function Str: ansistring; override;
      function MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring; override;
      function IsValid: boolean; override;
      function Compare(other: TValue): TOrdered; override;
    public
      value: ansistring;
    private
      m_pchar: pchar;
      function GetPChar: pchar;
      procedure BeforeDestruction; override;
  end;
  TBooleanValue = class (TValue)
    public
      class function DefaultValue: boolean;
      constructor Create (newValue: boolean);
      function Copy: TValue; override;
      function Str: ansistring; override;
      function IsValid: boolean; override;
      function Compare(other: TValue): TOrdered; override;
    public
      value: boolean;
  end;
  TUnassignedValue = class (TValue)
    public
      function Copy: TValue; override;
  end;

// TODO: allows enabling constref in setters
// https://bugs.freepascal.org/view.php?id=35821
{$varpropsetter on}
type
  generic TRefCountedArray<T> = class
    public type
      TInternalArray = array of T;
      TArrayIndex = longint;
    private type
      TReference = ^T;
    private
     refCount: integer;
     values: TInternalArray;

     constructor Create(newLength: integer = 0);
     function GetLength: TArrayIndex;
     procedure SetLength(len: TArrayIndex);

     // ref counting
     procedure AddRef;
     procedure Finalize;

     // access
     function GetValue(index: TArrayIndex): T; inline;
     procedure SetValue(index: TArrayIndex; constref value: T); inline; 
     property PrivateValues[index: TArrayIndex]: T read GetValue write SetValue; default;

     // references
     function GetReference(index: TArrayIndex): TReference; inline;
     procedure SetReference(index: TArrayIndex; value: TReference); inline; 
     property Refs[index: TArrayIndex]: TReference read GetReference write SetReference;
  end;
{$varpropsetter off}

// assignment
operator explicit (right: Variant): TValue; {$i inline_operator}

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

//$bookmark -
//$bookmark COMPARATOR
//$bookmark -

function TComparatorResultHelper.GreaterThan: boolean;
begin
  result := (self = kOrderedAscending);
end;

function TComparatorResultHelper.LessThan: boolean;
begin
  result := (self = kOrderedDescending);
end;

function TComparatorResultHelper.GreaterThanOrEqualTo: boolean;
begin
  result := (self = kOrderedAscending) or (self = kOrderedSame);
end;

function TComparatorResultHelper.LessThanOrEqualTo: boolean;
begin
  result := (self = kOrderedDescending) or (self = kOrderedSame);
end;

function TComparatorResultHelper.EqualTo: boolean;
begin
  result := (self = kOrderedSame);
end;

//$bookmark -
//$bookmark REF COUNTED ARRAY
//$bookmark -

constructor TRefCountedArray.Create(newLength: integer = 0);
begin
  refCount := 1;
  if newLength > 0 then
    SetLength(newLength);
end;

procedure TRefCountedArray.AddRef;
begin
  refCount += 1;
end;

procedure TRefCountedArray.Finalize;
begin
  refCount -= 1;
end;

function TRefCountedArray.GetLength: integer;
begin
  result := Length(values);
end;

procedure TRefCountedArray.SetLength(len: integer);
begin
  System.SetLength(values, len);
end;

function TRefCountedArray.GetReference(index: TArrayIndex): TReference;
begin
  result := @values[index]
end;

procedure TRefCountedArray.SetReference(index: TArrayIndex; value: TReference);
begin
  values[index] := value^;
end;

function TRefCountedArray.GetValue(index: TArrayIndex): T;
begin
  result := values[index];
end;

procedure TRefCountedArray.SetValue(index: TArrayIndex; constref value: T);
begin
  values[index] := value;
end;

//$bookmark UNASSIGNED VALUE

function TUnassignedValue.Copy: TValue;
begin
  fatal('unassigned values can not be copied');
  result := nil;
end;


//$bookmark INTEGER VALUE

class function TIntegerValue.DefaultValue: integer;
begin
  result := 0;
end;

constructor TIntegerValue.Create (newValue: integer);
begin
  value := newValue;
  typ := TIntegerType;
end;

function TIntegerValue.Str: ansistring;
begin
  result := tostr(value);
end;

function TIntegerValue.Copy: TValue;
begin
  result := TIntegerValue.Create(value);
end;

function TIntegerValue.IsValid: boolean;
begin
  result := (value <> DefaultValue);
end;

function TIntegerValue.Compare(other: TValue): TOrdered;
begin
  if other > value then
    result := kOrderedAscending
  else if other < value then
    result := kOrderedDescending
  else
    result := kOrderedSame;
end;

//$bookmark FLOAT VALUE

class function TFloatValue.DefaultValue: float;
begin
  result := 0.0;
end;

constructor TFloatValue.Create (newValue: float);
begin
  value := newValue;
  typ := TFloatType;
end;

function TFloatValue.Str: ansistring;
begin
  result := tostr(value);
end;

function TFloatValue.Copy: TValue;
begin
  result := TFloatValue.Create(value);
end;

function TFloatValue.IsValid: boolean;
begin
  result := (value <> DefaultValue);
end;

function TFloatValue.Compare(other: TValue): TOrdered;
begin
  if other > value then
    result := kOrderedAscending
  else if other < value then
    result := kOrderedDescending
  else
    result := kOrderedSame;
end;

//$bookmark STRING VALUE

class function TStringValue.DefaultValue: string;
begin
  result := '';
end;

constructor TStringValue.Create (newValue: ansistring);
begin
  value := newValue;
  typ := TStringType;
end;

function TStringValue.Str: ansistring;
begin
  result := value;
end;

function TStringValue.MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring;
begin
  result := FormatString(value, formatting);
end;

function TStringValue.GetPChar: pchar;
begin
  if m_pchar = nil then
    begin
      m_pchar := StrAlloc(length(value) + 1);
      strpcopy(m_pchar, value);
    end;
  result := m_pchar;
end;

procedure TStringValue.BeforeDestruction;
begin
  StrDispose(m_pchar);
  m_pchar := nil;
end;

function TStringValue.Copy: TValue;
begin
  result := TStringValue.Create(value);
end;

function TStringValue.IsValid: boolean;
begin
  result := (value <> DefaultValue);
end;

function TStringValue.Compare(other: TValue): TOrdered;
var
  r: integer;
begin
  r := CompareText(value, other.str);
  if r > 0 then
    result := kOrderedAscending
  else if r < 0 then
    result := kOrderedDescending
  else
    result := kOrderedSame;
end;

//$bookmark BOOLEAN VALUE

class function TBooleanValue.DefaultValue: boolean;
begin
  result := false;
end;

constructor TBooleanValue.Create (newValue: boolean);
begin
  value := newValue;
  typ := TBoolType;
end;

function TBooleanValue.Str: ansistring;
begin
  result := tostr(value);
end;

function TBooleanValue.Copy: TValue;
begin
  result := TBooleanValue.Create(value);
end;

function TBooleanValue.IsValid: boolean;
begin
  result := value <> DefaultValue;
end;

function TBooleanValue.Compare(other: TValue): TOrdered;
begin
  if other > value then
    result := kOrderedAscending
  else if other < value then
    result := kOrderedDescending
  else
    result := kOrderedSame;
end;

//$bookmark BASE VALUE

function TValue.Copy: TValue;
begin
  result := nil;
  if ClassType = TValue then
    fatal('can''t copy generic value')
  else
    fatal(ClassName+' must implement copy');
end;

function TValue.Str: ansistring;
begin
  result := '';
end;

procedure TValue.Show;
begin
  writeln(DebugString);
end;

function TValue.DebugString: ansistring;
begin
  result := Str;
end;

function TValue.MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring;
begin
  result := Str;
end;

function TValue.IsValid: boolean;
begin
  result := false;
end;

function TValue.GetValue(index: integer): TValue;
begin
  result := nil;
end;

procedure TValue.SetValue(index: integer; newValue: TValue);
begin
end;

function TValue.Compare(other: TValue): TOrdered;
begin
  fatal('can''t compare untyped values');
end;

//$bookmark -
//$bookmark OPERATORS
//$bookmark -

//$bookmark assignment operators

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