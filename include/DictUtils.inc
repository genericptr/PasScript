{$ifdef INTERFACE}

type
  TDictionaryHelpers = record helper for TDictionary
    // https://www.php.net/manual/en/ref.array.php 
    {
      merge (only merge values from other dict if they don't exist)
    }
  end;

type
  TDictValue = class (TValue)
    private
      function GetValue(index: integer): TValue; override;
      procedure SetValue(index: integer; newValue: TValue); override;
    public
      value: TDictionary;
      constructor Create (constref newValue: TDictionary); overload;
      constructor Create; overload;
      function Str: ansistring; override;
      function MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring; override;
      function Copy: TValue; override;
  end;

operator explicit(right: TDictionary): TValue; {$i inline_operator}
operator := (right: TDictionary): TValue; {$i inline_operator}

{$endif}

{$ifdef IMPLEMENTATION}

operator explicit(right: TDictionary): TValue;
begin
  result := TDictValue.Create(right);
end;

operator := (right: TDictionary): TValue;
begin
  result := TDictValue.Create(right);
end;

constructor TDictValue.Create (constref newValue: TDictionary);
begin
  value := newValue;
  typ := TDictType;
end;

constructor TDictValue.Create;
begin
  typ := TDictType;
end;

function TDictValue.Str: ansistring;
begin
  result := value.Str;
end;

function TDictValue.MakeFormattedString(indent: string; formatting: TStringFormatting = []): ansistring;
begin
  result := value.MakeFormattedString(indent, formatting);
end;

function TDictValue.Copy: TValue;
begin
  result := TDictValue.Create(value);
end;

function TDictValue.GetValue(index: integer): TValue;
begin
  result := value[index];
end;

procedure TDictValue.SetValue(index: integer; newValue: TValue);
begin
  value[index] := newValue;
end;

{$endif}

{$ifdef CODE}

{$endif}