{$ifdef INTERFACE}

type
  TListHelpers = record helper for TList
    // https://www.php.net/manual/en/ref.array.php 
    //function Pop: TValue;   
    //function Implode(glue: string = ''): string;
    //procedure Apply(func: TListApplyFunction; context: pointer);
    {
      merge (append)
      unique - remove duplicate values
    }
  end;

type
  TListValue = class (TValue)
    private
      function GetValue(index: integer): TValue; override;
      procedure SetValue(index: integer; newValue: TValue); override;
    public
      value: TList;
      constructor Create (constref newValue: TList); overload;
      constructor Create; overload;
      function Str: ansistring; override;
      function MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring; override;
      function Copy: TValue; override;
  end;

type
  TListValueHelper = class helper for TValue
    procedure Add(value: variant);
    procedure Remove(index: integer);
    // TODO: not in list yet
    //procedure Replace(index: integer);
    //procedure Insert(index: integer);
    procedure Clear;
  end;

operator := (right: array of const): TValue;
operator := (right: TList): TValue; {$i inline_operator}
operator explicit(right: TList): TValue; {$i inline_operator}

{$endif}

{$ifdef IMPLEMENTATION}

//$bookmark -
//$bookmark LIST VALUE HELPER
//$bookmark -

procedure TListValueHelper.Add(value: variant);
begin
  TListValue(self).value.AddTemp(TValue(value));
end;

procedure TListValueHelper.Remove(index: integer);
begin
  TListValue(self).value.Remove(index);
end;

procedure TListValueHelper.Clear;
begin
  TListValue(self).value.Clear;
end;

operator explicit(right: TList): TValue;
begin
  result := TListValue.Create(right);
end;

operator :=(right: TList): TValue;
begin
  result := TListValue.Create(right);
end;

operator := (right: array of const): TValue;
var
  i: integer;
begin
  writeln(':= to array of const');
  result := TListValue.Create;
  result.temp := true;
  // https://wiki.lazarus.freepascal.org/TVarRec
  for i := 0 to system.high(right) do
    begin
      case right[i].vtype of
        vtInteger:
          result.Add(right[i].vinteger);
        vtExtended:
          result.Add(right[i].vextended^);
        vtString:
          result.Add(right[i].vstring^);
        vtAnsiString:
          result.Add(ansistring(right[i].vansistring));
        vtChar:
          result.Add(right[i].vchar);
        otherwise
          fatal('variable argument value type '+IntToStr(right[i].vtype)+' is invalid');
      end;
    end;
end;

//$bookmark -
//$bookmark LIST VALUE
//$bookmark -

constructor TListValue.Create (constref newValue: TList);
begin
  value := newValue;
  typ := TListType;
end;

constructor TListValue.Create;
begin
  typ := TListType;
end;

function TListValue.Str: ansistring;
begin
  result := value.Str;
end;

function TListValue.Copy: TValue;
begin
  result := TListValue.Create(value);
end;

function TListValue.GetValue(index: integer): TValue;
begin
  result := value[index];
end;

procedure TListValue.SetValue(index: integer; newValue: TValue);
begin
  value[index] := newValue;
end;

function TListValue.MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring;
begin
  result := value.MakeFormattedString(indent, formatting);
end;


{$endif}

{$ifdef CODE}

{$endif}