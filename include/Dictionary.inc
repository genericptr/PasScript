{$ifdef INTERFACE}

type
  TDictionaryKey = string[40];
  TDictionaryKeyArray = array of TDictionaryKey;
  TDictionaryValueArray = array of TValue;
  TDictionaryPtr = ^TDictionary;
  TDictionaryEntry = record
    key: TDictionaryKey;
    value: TValue;
    function IsSet: boolean; inline;
  end;
  PDictionaryEntry = ^TDictionaryEntry;
  TDictionary = record
    public type
      TArrayIndex = longint;
      TDictionaryEntryArray = class(specialize TRefCountedArray<TDictionaryEntry>)
        capacity: integer;
        hashesChanged: boolean;
        valueCount: integer;
        sorted: boolean;
        function Copy: TDictionaryEntryArray;
      end;
      TEnumerator = record
        private
          source: TDictionaryPtr;
          index: integer;
        public
          function MoveNext: Boolean;
          procedure Reset;
          function GetCurrent: PDictionaryEntry;
          property Current: PDictionaryEntry read GetCurrent;
      end;
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
      property EntryAccess[key: variant]: TValue read GetValue write SetValue; default;
    public
      { Setting/Removing }
      procedure Remove (key: TDictionaryKey);    
      procedure Clear;
      
      { Getting }
      function GetValue (key: TDictionaryKey; out value: TValue): boolean; inline; overload;
      function Keys: TList;
      function Values: TList;
      function GetEnumerator: TEnumerator;
      function Count: integer;
      
      { Querying }
      function ContainsKey (key: TDictionaryKey): boolean;
      function ContainsValue (value: TValue): boolean;
      function Str: ansistring;
      function MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;
      procedure Show;

    private
      class operator := (right: array of TDictionaryEntry): TDictionary;
      class operator := (right: array of const): TDictionary;
      class operator Finalize (var dict: TDictionary); 
      class operator Initialize (var dict: TDictionary);
      class operator AddRef (var dict: TDictionary);
      class operator Copy (constref src: TDictionary; var dest: TDictionary);
    private
      entries: TDictionaryEntryArray;

      function GetValueInternal (key: TDictionaryKey): TValue; inline;
      function GetValueAtIndex (index: TArrayIndex): TValue; inline;
      procedure SetValueAtIndex (index: TArrayIndex; key: TDictionaryKey; value: TValue);
      procedure SetValueInternal (key: TDictionaryKey; value: TValue);
      procedure GetValue (base: TValue; k0: variant; out value: TValue);
      procedure SetValue (base: TValue; k0: variant; value: TValue);

      function GetKey (index: TArrayIndex): TDictionaryKey; inline;
      function Capacity: integer; inline;
      function Hash (key: TDictionaryKey): integer;
      function IsSet (index: TArrayIndex): boolean; inline;
      function KeyOfValue (value: TValue): TDictionaryKey;
      function CompareValues (a, b: TValue): boolean; inline;
      procedure Rehash (newSize: integer);
      procedure ReleaseValue (index: TArrayIndex);
    private
      class var UnassignedValue: TUnassignedValue;
  end;

type
  TPair = PDictionaryEntry;
  TDict = TDictionary;

{$endif}

{$ifdef IMPLEMENTATION}

var
  EmptyDictEntry: TDictionaryEntry = (key: ''; value: nil);

function TDictionaryEntry.IsSet : boolean;
begin
  result := assigned(value);
end;

function TDictionary.TDictionaryEntryArray.Copy: TDictionaryEntryArray;
var
  i: integer;
begin
  result := TDictionaryEntryArray.Create;
  
  result.SetLength(GetLength);
  for i := 0 to result.GetLength - 1 do
    begin
      result.refs[i].key := self.refs[i].key;
      if assigned(self.refs[i].value) then
        result.refs[i].value := self.refs[i].value.Copy
      else
        result.refs[i].value := nil;
    end;

  result.capacity := self.capacity;
  result.hashesChanged := self.hashesChanged;
  result.sorted := self.sorted;
  result.valueCount := self.valueCount;
end;


function TDictionary.TEnumerator.MoveNext: Boolean;
var
  count: integer;
  found: boolean;
begin
  count := source.Capacity;
  found := false;
  while index < count do
    begin
      index += 1;
      if source.IsSet(index) then
        begin
          found := true;
          break;
        end;
    end;
  result := found;
end;

// TODO: is this getting called?
procedure TDictionary.TEnumerator.Reset;
begin
  index := -1;
end;

function TDictionary.TEnumerator.GetCurrent: PDictionaryEntry;  
begin  
  Result := source.entries.refs[index];  
end;  
 
function TDictionary.GetEnumerator: TEnumerator;  
begin  
  Result.source := @self;  
  Result.index := -1;  
end;

function TDictionary.Str: ansistring;
var
  i: integer;
begin
  result := '{';
  for i := 0 to Capacity - 1 do
    if IsSet(i) then
      result += entries.refs[i].key+': '+entries.refs[i].value.DebugString+' ';
  result += '}';
end;

function TDictionary.MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;
var
  i: integer;
  first: integer = 0;
begin
  result := '{'#10;
  for i := 0 to Capacity - 1 do
    begin
      if IsSet(i) then
        begin
          if first > 1 then
            result += ','+#10;
          result += indent+'  '+FormatString(entries.refs[i].key, formatting)+': '+entries.refs[i].value.MakeFormattedString(indent+'  ', formatting);
          if first = 0 then
            result += ','+#10;
          first += 1;
        end;
    end;
  if first > 0 then
    result += #10;
  result += indent+'}';
end;

procedure TDictionary.Show;
begin
  print_r(self);
end;

procedure TDictionary.GetValue(base: TValue; k0: variant; out value: TValue);
begin  
  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          Fatal('can not use integer for indexing dictionaries')
        else
          value := TListValue(base).value.GetValueInternal(integer(k0));
      end;
    varString:
      begin
        if base = nil then
          value := GetValueInternal(string(k0))
        else
          value := TDictValue(base).value.GetValueInternal(string(k0));
      end;
    otherwise
      fatal('invalid index type for dictionary');
  end;

  if not assigned(value) then
    value := UnassignedValue;
end;

function TDictionary.GetValue(k0: variant): TValue;
begin
  GetValue(nil, k0, result);
end;

function TDictionary.GetValue(k0,k1: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, result);
end;

function TDictionary.GetValue(k0,k1,k2: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, result);
end;

function TDictionary.GetValue(k0,k1,k2,k3: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  GetValue(next, k3, result);
end;

procedure TDictionary.SetValue(base: TValue; k0: variant; value: TValue);
begin
  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          Fatal('can not use integer for indexing dictionaries')
        else
          TListValue(base).value.SetValueInternal(integer(k0), value);
      end;
    varString:
      begin
        if base = nil then
          SetValueInternal(string(k0), value)
        else
          TDictValue(base).value.SetValueInternal(string(k0), value);
      end;
    otherwise
      fatal('invalid index type for list');
  end;
end;

procedure TDictionary.SetValue(k0: variant; value: TValue);
begin
  SetValue(nil, k0, value);
end;

procedure TDictionary.SetValue(k0,k1: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  SetValue(next, k1, value);
end;

procedure TDictionary.SetValue(k0,k1,k2: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  SetValue(next, k2, value);
end;


procedure TDictionary.SetValue(k0,k1,k2,k3: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  SetValue(next, k3, value);
end;

function TDictionary.GetValue (key: TDictionaryKey; out value: TValue): boolean;
begin
  value := GetValueInternal(key);
  result := assigned(value);
end;

procedure TDictionary.SetValueInternal (key: TDictionaryKey; value: TValue);
const
  kGrowSize = 2;
var
  index: TArrayIndex;
begin
  index := Hash(key);

  // available location, set value
  if not IsSet(index) then
    SetValueAtIndex(index, key, value)
  else
    begin
      // there is a collision because the key is not the same
      // but the index is occupied by a value 
      if GetKey(index) <> key then
        begin
          Rehash(Trunc(Capacity * kGrowSize));
          SetValueInternal(key, value);
          //writeln(index, ' <> ', key, ' grow to ', Capacity);
        end
      else 
        SetValueAtIndex(index, key, value); // replace value since the key is the same
    end;
end;

function TDictionary.Capacity: integer;
begin
  result := entries.capacity;
end;

procedure TDictionary.SetValueAtIndex (index: TArrayIndex; key: TDictionaryKey; value: TValue);
begin
  ReleaseValue(index);
  entries.refs[index].key := key;
  entries.refs[index].value := value;
  entries.hashesChanged := true;
end;

function TDictionary.GetValueAtIndex (index: TArrayIndex): TValue;
begin
  result := entries.refs[index].value;
end;

function TDictionary.GetValueInternal (key: TDictionaryKey): TValue;
var
  index: TArrayIndex;
begin
  index := Hash(key);
  if entries.refs[index].IsSet and (entries.refs[index].key = key) then
    result := entries.refs[index].value
  else
    result := nil;
end;

function TDictionary.GetKey (index: TArrayIndex): TDictionaryKey;
begin
  result := entries.refs[index].key;
end;

function TDictionary.CompareValues (a, b: TValue): boolean;
begin
  result := a = b;
end;

function TDictionary.KeyOfValue (value: TValue): TDictionaryKey;
var
  i: integer;
begin
  result := '';
  for i := 0 to Capacity - 1 do
    if CompareValues(GetValue(i), value) then
      exit(GetKey(i));
end;

function TDictionary.IsSet (index: integer): boolean;
begin
  result := entries.refs[index].IsSet;
end;

function TDictionary.ContainsValue (value: TValue): boolean;
begin
  result := KeyOfValue(value) <> '';
end;

function TDictionary.ContainsKey (key: TDictionaryKey): boolean;
var
  index: TArrayIndex;
begin
  index := Hash(key);
  result := entries.refs[index].IsSet and (entries.refs[index].key = key);
end;

function TDictionary.Count: TArrayIndex;
var
  i: integer;
begin
  // recalculate if hashes changed
  if entries.hashesChanged then
    begin
      entries.valueCount := 0;
      for i := 0 to Capacity - 1 do
        if IsSet(i) then
          entries.valueCount += 1;
      entries.hashesChanged := false;
    end;
  result := entries.valueCount;
end;

function TDictionary.Values: TList;
var
  i: integer;
begin
  for i := 0 to Capacity - 1 do
    if IsSet(i) then
      result.Add(GetValueAtIndex(i));
end;

function TDictionary.Keys: TList;
var
  i: integer;
begin
  if Capacity = 0 then
    exit;
  for i := 0 to Capacity - 1 do
    if IsSet(i) then
      result.Add(GetKey(i));
end;

procedure TDictionary.Remove (key: TDictionaryKey);
var
  index: integer;
begin
  index := Hash(key);
  ReleaseValue(index);
  entries.refs[index].key := '';
  entries.refs[index].value := nil;
end;

procedure TDictionary.Clear;
var
  i: integer;
begin
  for i := 0 to Capacity - 1 do
    ReleaseValue(i);
  // NOTE: shrink memory or clear?
  FillChar(entries.values[0], Capacity * sizeof(TDictionaryEntry), 0);
end;

procedure TDictionary.Rehash (newSize: TArrayIndex);
var
  i: TArrayIndex;
  oldElemenents: TArrayIndex = 0;
  temp: TDictionaryEntryArray.TInternalArray;
begin 
  if entries = nil then
    entries := TDictionaryEntryArray.Create;

  oldElemenents := Capacity;
  if oldElemenents > 0 then
    temp := entries.values;

  entries.SetLength(newSize);
  FillChar(entries.values[0], newSize * sizeof(TDictionaryEntry), 0);

  for i := 0 to oldElemenents - 1 do
    if temp[i].IsSet then
      SetValueAtIndex(Hash(temp[i].key), temp[i].key, temp[i].value);
  
  entries.capacity := newSize;
  entries.hashesChanged := true;
  entries.sorted := false;
end;

function TDictionary.Hash (key: TDictionaryKey): TArrayIndex;
const
  kInitialValue = 5381;
  kM = 33;
var
  hashval: TArrayIndex = kInitialValue;
  i: integer;
begin
  for i := 1 to Length(key) do
    hashval := kM * hashval + Ord(key[i]);
  result := abs(hashval mod Capacity);
end;

procedure TDictionary.ReleaseValue (index: TArrayIndex);
begin
  if (entries.GetLength > 0) and assigned(entries.refs[index].value) then
    begin
      entries.refs[index].value.Free;
      entries.refs[index].value := nil;
    end;
end;

class operator TDictionary.AddRef(var dict: TDictionary);
begin
  dict.entries.AddRef;
end;

class operator TDictionary.Copy(constref src: TDictionary; var dest: TDictionary);
begin
  if assigned(src.entries) then
    dest.entries := src.entries.Copy
  else
    begin
      dest.entries := nil;
      dest.Rehash(8);
    end;
end;

class operator TDictionary.Initialize(var dict: TDictionary);
begin
  dict.entries := nil;
  dict.Rehash(8);
end;

class operator TDictionary.Finalize(var dict: TDictionary);
begin
  with dict do
    begin
      entries.Finalize;
      if entries.refCount = 0 then
        begin
          Clear;
          FreeAndNil(entries);
        end;
    end;
end;

//class operator TDictionary.:= (right: TValue): TDictionary;
//var
//  i: integer;
//  key: TDictionaryKey;
//begin
//  writeln(':= from list');
//  Fatal(right.typ <> TListType, 'can''t assign new dictionary with value');
    
//  for i := 0 to TListValue(right).value.high do
//    begin
//      if i mod 2 = 0 then
//        key := right[i]
//      else
//        begin
//          writeln(key, ' : ', right[i].str);
//          result[key] := right[i].Copy;
//        end;
//    end;
//end;

class operator TDictionary.:= (right: array of const): TDictionary;
var
  i: integer;
  key: TDictionaryKey;
begin
  result.Rehash(length(right));
  // https://wiki.lazarus.freepascal.org/TVarRec
  for i := 0 to system.high(right) do
    begin
      if i mod 2 = 0 then
        begin
          if right[i].vtype = vtString then
            key := right[i].vstring^
          else if right[i].vtype = vtAnsiString then
            key := ansistring(right[i].vansistring)
          else if right[i].vtype = vtChar then
            key := right[i].vchar
          else
            fatal('key type '+IntToStr(right[i].vtype)+' is invalid.');
        end
      else
        begin
          case right[i].vtype of
            vtInteger:
              result[key] := right[i].vinteger;
            vtExtended:
              result[key] := right[i].vextended^;
            vtString:
              result[key] := right[i].vstring^;
            vtAnsiString:
              result[key] := ansistring(right[i].vansistring);
            vtChar:
              result[key] := right[i].vchar;
            otherwise
              fatal('variable argument value type '+IntToStr(right[i].vtype)+' is invalid.');
          end;
        end;
    end;
end;

class operator TDictionary.:= (right: array of TDictionaryEntry): TDictionary;
var
  i: integer;
begin
  writeln(':= array of const');
  // TODO: how do we prevent double-rehash with init?
  if length(right) <> result.Capacity then
    result.Rehash(length(right));

  for i := 0 to high(right) do
    begin
      result[right[i].key] := right[i].value.Copy;
    end;
end;

{$endif}

{$ifdef CODE}
TDictionary.UnassignedValue := TUnassignedValue.Create;
{$endif}