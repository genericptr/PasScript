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
      constructor Create (constref newValue: TList);
      function Str: string; override;
    private
      value: TList;
  end;

{$endif}

{$ifdef IMPLEMENTATION}

constructor TListValue.Create (constref newValue: TList);
begin
  value := newValue;
  typ := TListType;
end;

function TListValue.Str: string;
begin
  // TODO: TList.Str
  result := 'TODO: [list]';
end;

function TListValue.GetValue(index: integer): TValue;
begin
  result := value[index];
end;

procedure TListValue.SetValue(index: integer; newValue: TValue);
begin
  value[index] := newValue;
end;

{$endif}

{$ifdef CODE}

{$endif}