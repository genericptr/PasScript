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
  TStringFormat = (TStringFormatJSON, TStringFormatDebug);
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

{$endif}

{$ifdef CODE}
{$endif}