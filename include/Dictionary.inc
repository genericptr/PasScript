{$ifdef INTERFACE}

// TODO: rename to TDict

type
  TDictionaryKey = string[40];
  TDictionaryKeyArray = array of TDictionaryKey;
  TDictionaryValueArray = array of TValue;
  TDictionaryPtr = ^TDictionary;
  TDictionaryEntry = record
    key: TDictionaryKey;
    value: TValue;
  end;
  TDictionaryEntryPtr = ^TDictionaryEntry;
  TDictionary = record
    public type
      TDictionaryEntryArray = array[0..high(integer)] of TDictionaryEntry;
      TDictionaryEntryArrayPtr = ^TDictionaryEntryArray;
      TDictionaryHeader = record
        capacity: integer;
        hashesChanged: boolean;
        valueCount: integer;
        refCount: integer;
        entries: TDictionaryEntryArrayPtr;
      end;
      TDictionaryHeaderPtr = ^TDictionaryHeader;        
      TEnumerator = record
        private
          source: TDictionaryPtr;
          index: integer;
        public
          function MoveNext: Boolean;
          procedure Reset;
          function GetCurrent: TDictionaryEntryPtr;
          property Current: TDictionaryEntryPtr read GetCurrent;
      end;
    private
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
      procedure Show;

      property EntryAccess[key: TDictionaryKey]: TValue read GetValue write SetValue; default;
    private
      class operator := (right: array of TDictionaryEntry): TDictionary;
      class operator Finalize(var a: TDictionary); 
      class operator Initialize(var a: TDictionary);
      class operator AddRef(var a: TDictionary);
      class operator Copy(constref src: TDictionary; var dest: TDictionary);
    private
      header: TDictionaryHeaderPtr;
      function Capacity: integer; inline;
      function Hash (key: TDictionaryKey): integer;
      function IsSet (index: integer): boolean; inline;
      function GetKey (index: integer): TDictionaryKey; inline;
      function GetValue (index: integer): TValue; inline;
      function KeyOfValue (value: TValue): TDictionaryKey;
      function CompareValues (a, b: TValue): boolean; inline;
      procedure SetValue (index: integer; key: TDictionaryKey; value: TValue);
      procedure Rehash (elements: integer);
      procedure ReleaseValue (index: integer);
      procedure RetainValue (value: TValue);
      procedure IncRefCount; inline;
      procedure DecRefCount;
  end;


{$endif}

{$ifdef IMPLEMENTATION}
const
  kHeaderSize = sizeof(TDictionary.TDictionaryHeader) - sizeof(TDictionary.TDictionaryEntryArrayPtr);

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

function TDictionary.TEnumerator.GetCurrent: TDictionaryEntryPtr;  
begin  
  Result := @(source.header.entries[index]);  
end;  
 
function TDictionary.GetEnumerator: TEnumerator;  
begin  
  Result.source := @self;  
  Result.index := -1;  
end;

procedure TDictionary.Show;
var
  i: integer;
  indent: string;
begin
  writeln('{');
  indent := '  ';
  for i := 0 to Capacity - 1 do
    if IsSet(i) then
      begin
        writeln(indent, header.entries[i].key, ': ', header.entries[i].value.DebugString);
        // TODO: recurse? lists?
      end;
  writeln('}');
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

procedure TDictionary.SetValue (key: TDictionaryKey; value: TValue);
begin
  SetValue(Hash(key), key, value);
end;

// TODO: rename to entryCount
function TDictionary.Capacity: integer;
begin
  result := header.capacity;
end;

procedure TDictionary.SetValue (index: integer; key: TDictionaryKey; value: TValue);
begin
  ReleaseValue(index);
  header.entries[index].key := key;
  header.entries[index].value := value;
  RetainValue(value);
  header.hashesChanged := true;
end;

function TDictionary.GetValue (index: integer): TValue;
begin
  result := header.entries[index].value;
end;

function TDictionary.GetKey (index: integer): TDictionaryKey;
begin
  result := header.entries[index].key;
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
  result := assigned(header.entries[index].value);
end;

function TDictionary.ContainsValue (value: TValue): boolean;
begin
  result := KeyOfValue(value) <> '';
end;

function TDictionary.ContainsKey (key: TDictionaryKey): boolean;
begin
  result := assigned(GetValue(key));
end;

function TDictionary.Count: integer;
var
  i: integer;
begin
  // recalculate if hashes changed
  if header.hashesChanged then
    begin
      header.valueCount := 0;
      for i := 0 to Capacity - 1 do
        if IsSet(i) then
          header.valueCount += 1;
      header.hashesChanged := false;
    end;
  result := header.valueCount;
end;

function TDictionary.Values: TList;
var
  i: integer;
begin
  for i := 0 to Capacity - 1 do
    if IsSet(i) then
      // TODO: we need to copy tvalue and pass into list
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
  header.entries[index].key := '';
  header.entries[index].value := nil;
end;

procedure TDictionary.Clear;
var
  i: integer;
begin
  for i := 0 to Capacity - 1 do
    ReleaseValue(i);
  // NOTE: shrink memory or clear?
  FillChar(header.entries[0], Capacity * sizeof(TDictionaryEntry), 0);
end;

// TODO: rename elemenets to entries
procedure TDictionary.Rehash (elements: integer);
var
  i: integer;
  temp: TDictionaryEntryArrayPtr = nil;
  oldElemenents: integer = 0;
  init: boolean;
begin 

  if header <> nil then
    begin
      oldElemenents := Capacity;
      if oldElemenents > 0 then
        begin
          temp := TDictionaryEntryArrayPtr(GetMem(oldElemenents * sizeof(TDictionaryEntry)));
          Move(header.entries[0], temp[0], oldElemenents * sizeof(TDictionaryEntry));
        end;
    end;

  init := header = nil;
  // TODO: don't resize if new elements is less than old
  if elements > oldElemenents then
    ReAllocMem(header, kHeaderSize + (elements * sizeof(TDictionaryEntry)));

  // TODO: why do we need +1?
  FillChar(header.entries, (elements + 1) * sizeof(TDictionaryEntry), 0);
  header.capacity := elements;
  header.hashesChanged := false;
  // TODO: Arithmetic "+" on untyped pointer is unportable to {$T+}, suggest typecast
  header.entries := @header.entries + sizeof(TDictionaryHeader);
  if init then
    begin
      header.valueCount := 0;
      header.refCount := 1;
    end;

  // insert old entries again
  for i := 0 to oldElemenents - 1 do
    if  assigned(temp^[i].value) then
      SetValue(temp^[i].key, temp^[i].value);
  
  if temp <> nil then
    FreeMem(temp);
    
  header.hashesChanged := true;
end;

function TDictionary.Hash (key: TDictionaryKey): integer;
const
  kInitialValue = 5381;
  kM = 33;
var
  hashval: integer = kInitialValue;
  i: integer;
begin
  for i := 1 to Length(key) do
    hashval := kM * hashval + Ord(key[i]);
  result := abs(hashval mod Capacity);
end;

procedure TDictionary.ReleaseValue (index: integer);
begin
  if assigned(header.entries[index].value) then
    begin
      header.entries[index].value.Free;
      header.entries[index].value := nil;
    end;
end;

procedure TDictionary.RetainValue (value: TValue);
begin
end;


procedure TDictionary.IncRefCount;
begin
  header.refCount += 1;
end;

procedure TDictionary.DecRefCount;
begin
  if header.refCount > 0 then
    begin
      header.refCount -= 1;
      if header.refCount = 0 then
        begin
          // TODO: actually free this shit
          //FreeMem(header);
          header := nil;
        end;
    end;
end;

class operator TDictionary.AddRef(var a: TDictionary);
begin
  if a.header.refCount > 0 then
    begin
      writeln('addref ', HexStr(@a), ' refCount=', a.header.refCount);
      a.IncRefCount;
      // NOTE: not passing by reference is probably not a good idea
      // unless you're certain the caller will not access it again
      // so consider making this an exception
      //raise Exception.Create('dictionaries must be passed by reference.');
    end;
end;

class operator TDictionary.Copy(constref src: TDictionary; var dest: TDictionary);
var
  i: integer;
begin
  writeln('copy ', HexStr(@src), ' to ', HexStr(@dest), ' refCount=', dest.header.refCount, ' bytes=', MemSize(dest.header));
  writeln(src.capacity, ' to ', dest.capacity);

  if src.Capacity > dest.Capacity then
    ReAllocMem(dest.header, kHeaderSize + (src.capacity * sizeof(TDictionaryEntry)));

  dest.header.capacity := src.header.capacity;
  dest.header.hashesChanged := false;
  dest.header.valueCount := 0;
  dest.header.refCount := 1;
  //dest.header.entries: TDictionaryEntryArrayPtr;
  // TODO: Arithmetic "+" on untyped pointer is unportable to {$T+}, suggest typecast
  dest.header.entries := @dest.header.entries + sizeof(TDictionaryHeader);

  for i := 0 to dest.Capacity do
    begin
      dest.header.entries[i].key := src.header.entries[i].key;
      if assigned(src.header.entries[i].value) then
        dest.header.entries[i].value := src.header.entries[i].value.Copy
      else
        dest.header.entries[i].value := nil;
    end;
end;

class operator TDictionary.Initialize(var a: TDictionary);
begin
  writeln('init dict ', HexStr(@a));
  a.Rehash(8);
end;

class operator TDictionary.Finalize(var a: TDictionary);
begin
  if assigned(a.header) and (a.header.refCount > 0) then
    begin
      writeln('finalize dict ', HexStr(@a), ' refCount=', a.header.refCount - 1);
      a.DecRefCount;
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

{$endif}