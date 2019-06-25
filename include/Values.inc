{$ifdef INTERFACE}

type
  float = single;

type
  TValueType = (TIntegerType, TFloatType, TStringType, TBoolType, TListType, TDictType);
  TValue = class
    private
      typ: TValueType;
    private
      function GetValue(index: integer): TValue; virtual;
      procedure SetValue(index: integer; newValue: TValue); virtual;
    public
      function Copy: TValue; virtual; abstract;
      function Str: string; virtual;
      function DebugString: string; virtual;
      function IsValid: boolean; virtual;
      procedure Show;
      property Subscript[index: integer]: TValue read GetValue write SetValue; default;
  end;
  TIntegerValue = class (TValue)
    public
      class function DefaultValue: integer;
      constructor Create (newValue: integer);
      function Copy: TValue; override;
      function Str: string; override;
      function IsValid: boolean; override;
    public
      value: integer;
  end;
  TFloatValue = class (TValue)
    public
      class function DefaultValue: float;
      constructor Create (newValue: float);
      function Copy: TValue; override;
      function Str: string; override;
      function IsValid: boolean; override;
    public
      value: float;
  end;
  TStringValue = class (TValue)
    public
      class function DefaultValue: string;
      constructor Create (newValue: ansistring);
      function Copy: TValue; override;
      function Str: string; override;
      function IsValid: boolean; override;
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
      function Str: string; override;
      function IsValid: boolean; override;
    public
      value: boolean;
  end;

// TODO: this is crap. lets ask fpc if they'll let us make class operators
// that that DONT return the actual class types
{$define inline_operator}

operator := (right: TValue): integer; {$i inline_operator}
operator := (right: TValue): float; {$i inline_operator}
operator := (right: TValue): string; {$i inline_operator}
operator := (right: TValue): ansistring; {$i inline_operator}
operator := (right: TValue): pchar; {$i inline_operator}
operator := (right: TValue): boolean; {$i inline_operator}

operator := (right: integer): TValue; {$i inline_operator}
operator := (right: float): TValue; {$i inline_operator}
operator := (right: string): TValue; {$i inline_operator}

operator = (left: TValue; right: integer): boolean; {$i inline_operator}
operator = (left: TValue; right: float): boolean; {$i inline_operator}
operator = (left: TValue; right: string): boolean; {$i inline_operator}

operator = (left: integer; right: TValue): boolean; {$i inline_operator}
operator = (left: float; right: TValue): boolean; {$i inline_operator}
operator = (left: string; right: TValue): boolean; {$i inline_operator}

operator + (left: TValue; right: integer): integer; {$i inline_operator}
operator + (left: TValue; right: float): float; {$i inline_operator}
operator + (left: TValue; right: string): string; {$i inline_operator}

operator + (left: integer; right: TValue): integer; {$i inline_operator}
operator + (left: float; right: TValue): float; {$i inline_operator}
operator + (left: string; right: TValue): string; {$i inline_operator}

{$endif}

{$ifdef IMPLEMENTATION}

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

function TIntegerValue.Str: string;
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

function TFloatValue.Str: string;
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

function TStringValue.Str: string;
begin
  result := value;
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

function TBooleanValue.Str: string;
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

//$bookmark BASE VALUE

function TValue.Str: string;
begin
  result := '';
end;

procedure TValue.Show;
begin
  writeln(DebugString);
end;

function TValue.DebugString: string;
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

//$bookmark -
//$bookmark OPERATORS
//$bookmark -

//$bookmark assignment operators

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

{$endif}

{$ifdef CODE}
{$endif}