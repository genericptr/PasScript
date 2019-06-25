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
      constructor Create (constref newValue: TDictionary);
      function Str: string; override;
    private
      value: TDictionary;
  end;

{$endif}

{$ifdef IMPLEMENTATION}

constructor TDictValue.Create (constref newValue: TDictionary);
begin
  value := newValue;
  typ := TDictType;
end;

function TDictValue.Str: string;
begin
  // TODO: TDictionary.Str
  result := 'TODO: [dict]';
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