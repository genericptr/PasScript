{$ifdef INTERFACE}

type
  TDictionaryKey = string[40];
  TDictionaryKeyArray = array of TDictionaryKey;
  TDictionaryValueArray = array of TValue;
  TDictionaryPtr = ^TDictionary;
  TDictionaryEntry = record
    key: TDictionaryKey;
    value: TValue;
  end;
  PDictionaryEntry = ^TDictionaryEntry;
  TDictionary = record
    public type
      TArrayIndex = longint;
      TDictionaryEntryArray = class(specialize TRefCountedArray<TDictionaryEntry>)
        capacity: integer;
        hashesChanged: boolean;
        valueCount: integer;
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
      TComparator = function (left, right: pointer; context: pointer): TComparatorResult;
    private
      // TODO: do we need get safe now? I think the overloaded property will fix this
      function GetValueSafe (key: TDictionaryKey): TValue; inline; overload;
      function GetValue (key: TDictionaryKey): TValue; inline; overload;
      procedure SetValue (key: TDictionaryKey; value: TValue); overload;
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

      procedure SortKeys(comparator: TComparator = nil; context: pointer = nil);
      procedure SortValues(comparator: TComparator = nil; context: pointer = nil);

      property EntryAccess[key: TDictionaryKey]: TValue read GetValueSafe write SetValue; default;
    private
      class operator := (right: array of TDictionaryEntry): TDictionary;
      class operator Finalize(var dict: TDictionary); 
      class operator Initialize(var dict: TDictionary);
      class operator AddRef(var dict: TDictionary);
      class operator Copy(constref src: TDictionary; var dest: TDictionary);
    private
      entries: TDictionaryEntryArray;

      function Capacity: integer; inline;
      function Hash (key: TDictionaryKey): integer;
      function IsSet (index: TArrayIndex): boolean; inline;
      function GetKey (index: TArrayIndex): TDictionaryKey; inline;
      function GetValue (index: TArrayIndex): TValue; inline;
      function KeyOfValue (value: TValue): TDictionaryKey;
      function CompareValues (a, b: TValue): boolean; inline;
      procedure SetValue (index: TArrayIndex; key: TDictionaryKey; value: TValue);
      procedure Rehash (elements: integer);
      procedure ReleaseValue (index: TArrayIndex);
      procedure QuickSort (src: TDictionaryEntryArray; first, last: TArrayIndex; comparator: TComparator; context: pointer);
    private
      class var UnassignedValue: TUnassignedValue;
  end;

type
  TPair = PDictionaryEntry;
  TDict = TDictionary;

{$endif}

{$ifdef IMPLEMENTATION}

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
  writeln(Str);
end;

procedure TDictionary.QuickSort (src: TDictionaryEntryArray; first, last: TArrayIndex; comparator: TComparator; context: pointer);
var
  pivot,j,i: TArrayIndex;
  temp: PDictionaryEntry;
begin
  if (first < last) then
    begin
      pivot:=first;
      i:=first;
      j:=last;
      while(i<j)do
        begin
          while(comparator(src.refs[i], src.refs[pivot], context).LessThanOrEqualTo and (i<last)) do
            inc(i);
          while(comparator(src.refs[j], src.refs[pivot], context).GreaterThan) do
            dec(j);
          if i<j then
            begin
              temp:=src.refs[i];
              src.refs[i]:=src.refs[j];
              src.refs[j]:=temp;
            end;
        end;
      temp:=src.refs[pivot];
      src.refs[pivot]:=src.refs[j];
      src.refs[j]:=temp;
      QuickSort(src,first,j-1,comparator,context);
      QuickSort(src,j+1,last,comparator,context);
    end;
end;

function DefaultKeyComparator(left, right: pointer; context: pointer): TComparatorResult;
var
  r: integer;
begin
  if (TPair(left).key = '') and (TPair(right).key = '') then
    exit(kOrderedSame)
  else if (TPair(left).key <> '') and (TPair(right).key = '') then
    exit(kOrderedAscending)
  else if (TPair(left).key = '') and (TPair(right).key <> '') then
    exit(kOrderedDescending);

  r := CompareText(TPair(left).key, TPair(right).key);
  if r > 0 then
    result := kOrderedAscending
  else if r < 0 then
    result := kOrderedDescending
  else
    result := kOrderedSame;

  writeln(TPair(left).key,' <-> ', TPair(right).key, ': ', result);
end;

function DefaultValueComparator(left, right: pointer; context: pointer): TComparatorResult;
begin
  result := PValue(left).Compare(PValue(right)^);
end;

procedure TDictionary.SortKeys(comparator: TComparator = nil; context: pointer = nil);
begin
  if comparator = nil then
    comparator := @DefaultKeyComparator;

  QuickSort(entries, 0, entries.GetLength - 1, comparator, context);
end;

procedure TDictionary.SortValues(comparator: TComparator = nil; context: pointer = nil);
begin
  if comparator = nil then
    comparator := @DefaultValueComparator;

  QuickSort(entries, 0, entries.GetLength - 1, comparator, context);
end;

function TDictionary.GetValue (key: TDictionaryKey; out value: TValue): boolean;
begin
  value := GetValue(key);
  result := assigned(value);
end;

function TDictionary.GetValue (key: TDictionaryKey): TValue;
begin
  result := GetValue(Hash(key));
end;

function TDictionary.GetValueSafe (key: TDictionaryKey): TValue;
begin
  result := GetValue(Hash(key));
  if not assigned(result) then
    result := UnassignedValue;
end;

procedure TDictionary.SetValue (key: TDictionaryKey; value: TValue);
 // http://stackoverflow.com/questions/1100311/what-is-the-ideal-growth-rate-for-a-dynamically-allocated-array
const
  kGrowSize = 2;
var
  index: TArrayIndex;
begin
  index := Hash(key);

  // available location, set value
  if not IsSet(index) then
    SetValue(index, key, value)
  else
    begin
      // there is a collision because the key is not the same
      // but the index is occupied by a value 
      if GetKey(index) <> key then
        begin
          Rehash(Trunc(Capacity * kGrowSize));
          SetValue(key, value);
        end
      else 
        SetValue(index, key, value); // replace value since the key is the same
    end;
end;

function TDictionary.Capacity: integer;
begin
  result := entries.capacity;
end;

procedure TDictionary.SetValue (index: TArrayIndex; key: TDictionaryKey; value: TValue);
begin
  ReleaseValue(index);
  entries.refs[index].key := key;
  entries.refs[index].value := value;
  entries.hashesChanged := true;
end;

function TDictionary.GetValue (index: TArrayIndex): TValue;
begin
  result := entries.refs[index].value;
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
  result := assigned(entries.refs[index].value);
end;

function TDictionary.ContainsValue (value: TValue): boolean;
begin
  result := KeyOfValue(value) <> '';
end;

function TDictionary.ContainsKey (key: TDictionaryKey): boolean;
begin
  result := assigned(GetValue(key));
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
      result.Add(GetValue(i));
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

procedure TDictionary.Rehash (elements: TArrayIndex);
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

  entries.SetLength(elements);
  FillChar(entries.values[0], elements * sizeof(TDictionaryEntry), 0);

  for i := 0 to oldElemenents - 1 do
    if assigned(temp[i].value) then
      SetValue(temp[i].key, temp[i].value);
  
  entries.capacity := elements;
  entries.hashesChanged := true;
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