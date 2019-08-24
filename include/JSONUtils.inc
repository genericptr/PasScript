{$ifdef INTERFACE}
type
  TJSON = record
    private
      // TODO: using broken overloads until overloaded array props are added
      // https://bugs.freepascal.org/view.php?id=35772
      // https://bugs.freepascal.org/view.php?id=35809
      function GetValue(k0: variant): TValue; overload; inline;
      function GetValue(k0,k1: variant): TValue; overload; inline;
      function GetValue(k0,k1,k2: variant): TValue; overload; inline;
      function GetValue(k0,k1,k2,k3: variant): TValue; overload; inline;

      procedure SetValue(k0: variant; value: TValue); overload; inline;
      procedure SetValue(k0,k1: variant; value: TValue); overload; inline;
      procedure SetValue(k0,k1,k2: variant; value: TValue); overload; inline;
      procedure SetValue(k0,k1,k2,k3: variant; value: TValue); overload; inline;
    public
      list: TList;
      dict: TDict;
    public
      property InternalValues[key: variant]: TValue read GetValue write SetValue; default;

      function IsArray: boolean; inline;
      function IsObject: boolean; inline;
      function Values: TList; inline;
      function Keys: TList; inline;

      procedure Show;
      function Str(formatted: boolean = true): ansistring;
      function MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;

    private
      root: TValueType;

      procedure GetValue(base: TValue; k0: variant; out value: TValue); overload;
      procedure SetValue(base: TValue; k0: variant; value: TValue); overload;
  end;

function json_decode(contents: ansistring): TJSON; inline;
//function json_encode(constref list: TList): ansistring; overload; inline;
//function json_encode(constref dict: TDict): ansistring; overload; inline;

operator := (right: TJSON): TValue; {$i inline_operator}
operator := (right: TJSON): TDict; {$i inline_operator}
operator := (right: TJSON): TList; {$i inline_operator}

{$endif}

{$ifdef IMPLEMENTATION}

type
  TJSONScanner = class(TScanner)
    public
      procedure Parse; override;
    private
      data: TValue;
      procedure ParseArray(list: TListValue);
      procedure ParseObject(dict: TDictValue);
      function ParseValue: variant;
  end;

function json_decode(contents: ansistring): TJSON;
var
  scanner: TJSONScanner;
begin
  scanner := TJSONScanner.Create(contents);
  scanner.Parse;

  fatal(scanner.data = nil, 'json_decode failed');

  if scanner.data.InheritsFrom(TListValue) then
    begin
      result.list := TListValue(scanner.data).value;
      result.root := TListType;
    end
  else if scanner.data.InheritsFrom(TDictValue) then
    begin
      result.dict := TDictValue(scanner.data).value;
      result.root := TDictType;
    end
  else
    Fatal('invalid type');

  scanner.Free;
end;

//$bookmark -
//$bookmark JSON RECORD
//$bookmark -

operator := (right: TJSON): TValue;
begin
  if right.IsArray then
    result := TListValue.Create(right.list)
  else
    result := TDictValue.Create(right.dict);

  result.temp := true;
end;

operator := (right: TJSON): TDict;
begin
  result := right.dict;
end;

operator := (right: TJSON): TList;
begin
  result := right.list;
end;

function TJSON.IsArray: boolean;
begin
  result := (root = TListType);
end;

function TJSON.IsObject: boolean;
begin
  result := (root = TDictType);
end;

function TJSON.Values: TList;
begin
  if IsObject then
    result := dict.Values
  else
    result := list;
end;

function TJSON.Keys: TList;
var
  i: integer;
begin
  if IsObject then
    result := dict.keys
  else
    fatal('JSON array has no keys');
end;

procedure TJSON.Show;
begin
  writeln(Str);
end;

function TJSON.Str(formatted: boolean = true): ansistring;
begin
  if formatted then
    result := MakeFormattedString('', [TStringFormatJSON])
  else
    begin
      if root = TListType then
        result := list.Str
      else
        result := dict.Str;
    end;
end;

function TJSON.MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;
begin
  if IsArray then
    result := list.MakeFormattedString(indent, formatting)
  else
    result := dict.MakeFormattedString(indent, formatting);
end;

procedure TJSON.GetValue(base: TValue; k0: variant; out value: TValue);
begin
  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          value := list[integer(k0)]
        else
          value := TListValue(base).value[integer(k0)];
      end;
    varString:
      begin
        if base = nil then
          value := dict[string(k0)]
        else
          value := TDictValue(base).value[string(k0)];
      end;
    otherwise
      fatal('invalid key type for JSON');
  end;
end;

function TJSON.GetValue(k0: variant): TValue;
begin
  GetValue(nil, k0, result);
end;

function TJSON.GetValue(k0, k1: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, result);
end;

function TJSON.GetValue(k0, k1, k2: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, result);
end;

function TJSON.GetValue(k0, k1, k2, k3: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  GetValue(next, k3, result);
end;

procedure TJSON.SetValue(base: TValue; k0: variant; value: TValue);
begin
  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          list[integer(k0)] := value
        else
          TListValue(base).value[integer(k0)] := value;
      end;
    varString:
      begin
        if base = nil then
          dict[string(k0)] := value
        else
          TDictValue(base).value[string(k0)] := value;
      end;
    otherwise
      fatal('invalid key type for JSON');
  end;
end;

procedure TJSON.SetValue(k0: variant; value: TValue);
begin
  GetValue(nil, k0, value);
end;

procedure TJSON.SetValue(k0,k1: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  SetValue(next, k1, value);
end;

procedure TJSON.SetValue(k0,k1,k2: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  SetValue(next, k2, value);
end;

procedure TJSON.SetValue(k0,k1,k2,k3: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  SetValue(next, k3, value);
end;

//$bookmark -
//$bookmark JSON SCANNER
//$bookmark -

function TJSONScanner.ParseValue: variant;
begin
  if token = kTokenInteger then
    begin
      result := StrToInt(pattern);
      Consume(token);
    end
  else if token = kTokenRealNumber then
    begin
      result := StrToFloat(pattern);
      Consume(token);
    end
  else if token = kTokenStringDoubleQuote then
    begin
      result := pattern;
      Consume(token);
    end
  else if token = kTokenID then
    begin
      // ID can only be true/false or NULL
      if strcasecmp(pattern, 'null') then
        result := nil
      else if strcasecmp(pattern, 'true') then
        result := true
      else if strcasecmp(pattern, 'false') then
        result := false
      else
        ParserError('identifier "'+pattern+'" isn''t a value valid');
      Consume(token);
    end
  else
    ParserError('expected value type but got "'+TokenToStr(token)+'"');

  //writeln('got:',VarType(result), ':', VarToStr(result));
end;

procedure TJSONScanner.ParseArray(list: TListValue);
var
  value: variant;
begin
  // empty array
  if token = kTokenSquareBracketClosed then
    exit;

  while true do
    begin
      //writeln('next:',TokenToStr(token));
      // TODO: must have value before first , [, {
      case token of
        kTokenComma:
          Consume(token);
        kTokenSquareBracketOpen:
          begin
            //writeln('new array');
            Consume(token);
            list.value.AddTemp(TListValue.Create);
            ParseArray(TListValue(list.value.Tail));
          end;
        kTokenCurlyBracketOpen:
          begin
            //writeln('new object');
            Consume(token);
            list.value.AddTemp(TDictValue.Create);
            ParseObject(TDictValue(list.value.Tail));
          end;
        kTokenSquareBracketClosed:
          begin
            //writeln('close array');
            Consume(token);
            break;
          end;
        otherwise
          begin
            value := ParseValue;
            //writeln('got:',value);
            list.value.AddTemp(TValue(value));
          end;
      end;
    end;
end;

procedure TJSONScanner.ParseObject(dict: TDictValue);
var
  value: variant;
  key: TDictionaryKey;
  child: TValue;

  procedure CheckInvalidKey; inline;
  begin
    if key = '' then
      ParserError('object got value without key');
  end;

begin
  // empty object
  if token = kTokenCurlyBracketClosed then
    exit;

  key := '';

  while true do
    begin
      //writeln('next:',TokenToStr(token));
      case token of
        kTokenComma:
          begin
            // TODO: do we have a full key?
            Consume(token);
            key := '';
            value := '';
          end;
        kTokenSquareBracketOpen:
          begin
            CheckInvalidKey;
            //writeln('new array');
            //writeln('add ', key, ' -> array');
            Consume(token);
            // TODO: this doesn't copy but we don't have a copy option for dicts yet
            child := TListValue.Create;
            dict.value[key] := child;
            ParseArray(TListValue(child));
          end;
        kTokenCurlyBracketOpen:
          begin
            CheckInvalidKey;
            //writeln('new object');
            //writeln('add ', key, ' -> object');
            Consume(token);
            // TODO: this doesn't copy but we don't have a copy option for dicts yet
            child := TDictValue.Create;
            dict.value[key] := child;
            ParseObject(TDictValue(child));
          end;
        kTokenCurlyBracketClosed:
          begin
            // TODO: check if we're open!
            //writeln('close object');
            Consume(token);
            break;
          end;
        kTokenStringDoubleQuote:
          begin
            if key = '' then
              begin
                key := pattern;
                //writeln('got key: ', key);
                Consume(token);
                Consume(kTokenColon);
              end
            else
              begin
                CheckInvalidKey;
                //writeln('add ', key, ' -> ', pattern);
                dict.value[key] := TValue(pattern);
                Consume(token);
                key := '';
                value := '';
              end;
          end;
        otherwise
          begin
            CheckInvalidKey;
            //writeln('add ', key, ' -> ', pattern);
            value := ParseValue;
            dict.value[key] := TValue(value);
            key := '';
            value := '';
          end;
      end;
    end;

end;

procedure TJSONScanner.Parse;
begin
  ReadToken;

  case token of
    kTokenSquareBracketOpen:
      begin
        Consume(token);
        data := TListValue.Create;
        ParseArray(TListValue(data));
      end;
    kTokenCurlyBracketOpen:
      begin
        Consume(token);
        data := TDictValue.Create;
        ParseObject(TDictValue(data));
      end;
    otherwise
      Consume(token);
  end;
end;

{$endif}

{$ifdef CODE}

{$endif}