{$ifdef INTERFACE}
  
// TODO: allows enabling constref in setters
// https://bugs.freepascal.org/view.php?id=35821
{$varpropsetter on}
type
  TListIndex = longint;
  PList = ^TList;
  TList = record
    private type
      TInternalArray = array of TValue;
      TRefCountedValueArray = specialize TRefCountedArray<TValue>;
      TEnumerator = record
        private
          list: TInternalArray;
          currentValue: TValue;
          currentIndex: TListIndex;
          lastElement: TListIndex;
        public
          constructor Create(source: TInternalArray; _lastElement: TListIndex); 
          function MoveNext: boolean;
          property Current: TValue read currentValue;
      end;
      TListArray = class(specialize TRefCountedArray<TValue>)
        currentIndex: integer;
        function Copy: TListArray;
      end;
      TComparator = function (left, right: TValue; context: pointer): TComparatorResult;
      TComparator2 = function (left, right: TValue; context: pointer): TComparatorResult is nested;
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
      property Values[index: variant]: TValue read GetValue write SetValue; default;
    public
      constructor Create(_capacity: integer);

      { Methods }
      // TODO: when implicit function specialization gets added we can kill this
      //generic procedure Add<T>(value: T);
      procedure Add(value: TValue); overload;
      procedure Add(value: integer); overload;
      procedure Add(value: float); overload;
      procedure Add(value: string); overload;
      procedure Add(value: ansistring); overload;
      procedure Add(value: boolean); overload;

      { Removing }
      procedure Remove(index: integer);
      procedure Clear;

      { Searching }
      // TODO: replace with variant
      function IndexOf(value: integer): integer; overload;
      function IndexOf(value: float): integer; overload;
      function IndexOf(value: string): integer; overload;
      function IndexOf(value: ansistring): integer; overload;
      function Sort(comparator: TComparator = nil; context: pointer = nil): TList; overload;
      function Sort(comparator: TComparator2; context: pointer = nil): TList; overload;

      { Element Access }
      function Tail: TValue; inline;

      { Introspection }
      function Count: integer; inline;
      function High: integer; inline;
      function Capacity: integer; inline;
      function GetEnumerator: TEnumerator;
      procedure Show;
      function Str: ansistring;
      function MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;

    public
      class operator + (constref left: TList; right: TValue): TList;

      class operator in (constref left: integer; constref right: TList): boolean; overload;
      class operator in (constref left: string; constref right: TList): boolean; overload;
      class operator in (constref left: ansistring; constref right: TList): boolean; overload;
    private
      class operator := (right: array of const): TList;
      class operator Initialize(var list: TList);
      class operator Finalize(var list: TList);
      class operator AddRef(var list: TList);
      class operator Copy(constref src: TList; var dest: TList);
    private
      m_array: TListArray;

      { Internal Member Access }
      procedure AddWithoutCopy(value: TValue);
      procedure GetValue(base: TValue; k0: variant; out value: TValue);
      procedure SetValue(base: TValue; k0: variant; value: TValue);
      function GetValueInternal(index: integer): TValue; inline;
      procedure SetValueInternal(index: integer; value: TValue); inline;

      function GetCurrentIndex: TListIndex; inline;
      procedure SetCurrentIndex(newValue: TListIndex); inline;
      property CurrentIndex: TListIndex read GetCurrentIndex write SetCurrentIndex;

      procedure QuickSort (var x: TInternalArray; first, last: TListIndex; comparator: TComparator; context: pointer);
      procedure QuickSort2 (var x: TInternalArray; first, last: TListIndex; comparator: TComparator2; context: pointer);
      procedure Expand(newSize: integer);
      procedure GrowToFit(newSize: integer);
  end;
{$varpropsetter off}

{$endif}

{$ifdef IMPLEMENTATION}

//$bookmark LIST ARRAY

function TList.TListArray.Copy: TListArray;
var
  i: TListIndex;
begin
  result := TListArray.Create(GetLength);

  for i := 0 to result.GetLength - 1 do
    if assigned(self[i]) then
      result[i] := self[i].Copy;

  result.currentIndex := currentIndex;
end;

//$bookmark OPERATORS

class operator TList.in (constref left: integer; constref right: TList): boolean;
begin
  result := right.IndexOf(left) <> -1;
end;

class operator TList.in (constref left: string; constref right: TList): boolean;
begin
  result := right.IndexOf(left) <> -1;
end;

class operator TList.in (constref left: ansistring; constref right: TList): boolean;
begin
  result := right.IndexOf(left) <> -1;
end;

class operator TList.+ (constref left: TList; right: TValue): TList;
begin
  left.Add(right);
  result := left;
end;

class operator TList.:= (right: array of const): TList;
var
  i: integer;
begin
  result.GrowToFit(length(right));
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
        //vtClass      :
        //  Writeln (’Class reference, name :’,Args[i].VClass.Classname);
        vtAnsiString:
          result.Add(ansistring(right[i].vansistring));
        vtChar:
          result.Add(right[i].vchar);
        otherwise
          fatal('variable argument value type '+IntToStr(right[i].vtype)+' is invalid.');
      end;
    end;
end;

//$bookmark ENUMERATOR

constructor TList.TEnumerator.Create(source: TInternalArray; _lastElement: TListIndex);
begin
  list := source;
  lastElement := _lastElement;
  currentValue := nil;
  currentIndex := 0;
end;
  
function TList.TEnumerator.MoveNext: boolean;
begin
  if currentIndex <= lastElement then
    begin
      currentValue := list[currentIndex];
      inc(currentIndex);
      result := true;
    end
  else
    result := false;
end;

procedure TList.QuickSort (var x: TInternalArray; first, last: TListIndex; comparator: TComparator; context: pointer);
var
  pivot,j,i: TListIndex;
  temp: TValue;
begin
  if (first < last) then
    begin
      pivot:=first;
      i:=first;
      j:=last;
      while(i<j)do
        begin
          while(comparator(x[i], x[pivot], context).LessThanOrEqualTo and (i<last)) do
            inc(i);
          while(comparator(x[j], x[pivot], context).GreaterThan) do
            dec(j);
          if i<j then
            begin
              temp:=x[i];
              x[i]:=x[j];
              x[j]:=temp;
            end;
        end;
      temp:=x[pivot];
      x[pivot]:=x[j];
      x[j]:=temp;
      QuickSort(x,first,j-1,comparator,context);
      QuickSort(x,j+1,last,comparator,context);
    end;
end;

procedure TList.QuickSort2 (var x: TInternalArray; first, last: TListIndex; comparator: TComparator2; context: pointer);
var
  pivot,j,i: TListIndex;
  temp: TValue;
begin
  if (first < last) then
    begin
      pivot:=first;
      i:=first;
      j:=last;
      while(i<j)do
        begin
          while(comparator(x[i], x[pivot], context).LessThanOrEqualTo and (i<last)) do
            inc(i);
          while(comparator(x[j], x[pivot], context).GreaterThan) do
            dec(j);
          if i<j then
            begin
              temp:=x[i];
              x[i]:=x[j];
              x[j]:=temp;
            end;
        end;
      temp:=x[pivot];
      x[pivot]:=x[j];
      x[j]:=temp;
      QuickSort2(x,first,j-1,comparator,context);
      QuickSort2(x,j+1,last,comparator,context);
    end;
end;

function DefaultListComparator(left, right: TValue; context: pointer): TComparatorResult;
begin
  result := left.Compare(right);
end;

function TList.Sort(comparator: TComparator2; context: pointer = nil): TList;
begin
  if Count < 2 then
    exit;
  result := self;
  QuickSort2(result.m_array.values, 0, result.High, comparator, context);
end;

function TList.Sort(comparator: TComparator = nil; context: pointer = nil): TList;
begin
  if Count < 2 then
    exit;

  if comparator = nil then
    comparator := @DefaultListComparator;

  result := self;
  QuickSort(result.m_array.values, 0, result.High, comparator, context);
end;

function TList.Str: ansistring;
var
  i: integer;
begin
  result := '[';
  for i := 0 to high do
    begin
      if i = high then
        result += m_array[i].Str
      else
        result += m_array[i].Str+',';
    end;
  result += ']';
end;

function TList.MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;
var
  i: integer;
  prefix, suffix: string;
begin
  result := '['+#10;
  for i := 0 to high do
    begin
      if not assigned(values[i]) then
        fatal('trying to print invalid index '+i+'/'+ToStr(High));
      prefix := indent;
      suffix := ',';
      if TStringFormatDebug in formatting then
        begin
          prefix := indent+'  '+tostr(i)+': ';
          suffix := '';
        end;
      if i = high then
        result += prefix+'  '+values[i].MakeFormattedString(indent+'  ', formatting)+#10
      else
        result += prefix+'  '+values[i].MakeFormattedString(indent+'  ', formatting)+suffix+#10;
    end;
  result += indent+']';
end;


procedure TList.Show;
begin
  print_r(self);
end;

function TList.GetCurrentIndex: TListIndex;
begin
  fatal(m_array = nil, 'TList internal array nil');
  result := m_array.currentIndex;
end;

procedure TList.SetCurrentIndex(newValue: TListIndex);
begin
  m_array.currentIndex := newValue;
end;

function TList.Capacity: integer;
begin
  result := m_array.GetLength;
end;

function TList.Count: integer;
begin
  result := currentIndex;
end;

function TList.High: integer;
begin
  result := Count - 1;
end;

procedure TList.Clear;
var
  i: TListIndex;
begin
  // TODO: shrink?
  for i := 0 to High do
    begin
      m_array[i].Free;
      m_array[i] := nil;
    end;
  currentIndex := 0;
end;

procedure TList.AddWithoutCopy(value: TValue);
begin
  Fatal(value = nil, 'adding nil value to list');
  GrowToFit(Count + 1);
  m_array[currentIndex] := value;
  currentIndex := currentIndex + 1;
end;

// add TValue directly always copies unless it's
// explicity marked as temp
procedure TList.Add(value: TValue);
begin
  GrowToFit(Count + 1);  
  if value.temp then
    m_array[currentIndex] := value
  else
    m_array[currentIndex] := value.Copy;
  currentIndex := currentIndex + 1;
end;

procedure TList.Add(value: integer);
begin
  AddWithoutCopy(TValue(value));
end;

procedure TList.Add(value: float);
begin
  AddWithoutCopy(TValue(value));
end;

procedure TList.Add(value: string);
begin
  AddWithoutCopy(TValue(value));
end;

procedure TList.Add(value: ansistring);
begin
  AddWithoutCopy(TValue(value));
end;

procedure TList.Add(value: boolean);
begin
  AddWithoutCopy(TValue(value));
end;

procedure TList.Remove(index: integer);
begin
  // TODO: shrink list
  fatal('TList.Remove not implemented');
end;

function TList.IndexOf(value: integer): integer;
var
  i: TListIndex;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i].Compare(value) = kOrderedSame then
      exit(i);
end;

function TList.IndexOf(value: float): integer;
var
  i: TListIndex;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i].Compare(value) = kOrderedSame then
      exit(i);
end;

function TList.IndexOf(value: string): integer;
var
  i: TListIndex;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i].Compare(value) = kOrderedSame then
      exit(i);
end;

function TList.IndexOf(value: ansistring): integer; overload;
var
  i: TListIndex;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i].Compare(value) = kOrderedSame then
      exit(i);
end;

function TList.GetEnumerator: TEnumerator;
begin
  result := TEnumerator.Create(m_array.values, High);
end;

procedure TList.Expand(newSize: integer);
begin
  // TODO: allow shrinking
  m_array.SetLength(newSize);
end;

procedure TList.GrowToFit(newSize: integer);
begin
  if Capacity < newSize then
    begin
      if Capacity = 0 then
        Expand(4)
      else
        Expand(newSize * 2);
    end;
end;

function TList.Tail: TValue;
begin
  result := m_array[High];
end;

function TList.GetValueInternal(index: integer): TValue;
begin
  result := m_array[index];
end;

procedure TList.SetValueInternal(index: integer; value: TValue);
begin
  m_array[index] := value{TValue.Create};
end;

procedure TList.GetValue(base: TValue; k0: variant; out value: TValue);
begin
  // trying to index into an invalid list
  if assigned(base) and (base.typ = TUnassignedType) then
    begin
      value := base;
      exit;
    end;

  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          value := m_array[integer(k0)]
        else
          value := TListValue(base).value.GetValueInternal(integer(k0));
      end;
    varString:
      begin
        if base = nil then
          Fatal('can not use strings for indexing lists')
        else
          value := TDictValue(base).value.GetValueInternal(string(k0));
      end;
    otherwise
      fatal('invalid index type for list');
  end;
end;

function TList.GetValue(k0: variant): TValue;
begin
  GetValue(nil, k0, result);
end;

function TList.GetValue(k0, k1: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, result);
end;

function TList.GetValue(k0, k1, k2: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, result);
end;

function TList.GetValue(k0, k1, k2, k3: variant): TValue;
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  GetValue(next, k3, result);
end;

procedure TList.SetValue(base: TValue; k0: variant; value: TValue);
begin
  case VarType(k0) of
    varShortInt, varSmallint, varInteger:
      begin
        if base = nil then
          m_array[integer(k0)] := value
        else
          TListValue(base).value.SetValueInternal(integer(k0), value);
      end;
    varString:
      begin
        if base = nil then
          Fatal('can not use strings for indexing lists')
        else
          TDictValue(base).value.SetValue(string(k0), value);
      end;
    otherwise
      fatal('invalid index type for list');
  end;
end;

procedure TList.SetValue(k0: variant; value: TValue);
begin
  SetValue(nil, k0, value);
end;

procedure TList.SetValue(k0,k1: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  SetValue(next, k1, value);
end;

procedure TList.SetValue(k0,k1,k2: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  SetValue(next, k2, value);
end;

procedure TList.SetValue(k0,k1,k2,k3: variant; value: TValue);
var
  next: TValue;
begin
  GetValue(nil, k0, next);
  GetValue(next, k1, next);
  GetValue(next, k2, next);
  SetValue(next, k3, value);
end;

constructor TList.Create(_capacity: integer);
begin
  Expand(_capacity);
  currentIndex := 0;
end;

class operator TList.AddRef(var list: TList);
begin
  list.m_array.AddRef;
end;

class operator TList.Copy(constref src: TList; var dest: TList);
begin
  if assigned(src.m_array) then
    dest.m_array := src.m_array.Copy
  else
    dest.m_array := TListArray.Create;
end;

class operator TList.Initialize(var list: TList);
begin
  list.m_array := TListArray.Create;
end;

class operator TList.Finalize(var list: TList);
begin
  with list do
    begin
      //m_array.Finalize;
      //if m_array.refCount = 0 then
      //  begin
      //    Clear;
      //    FreeAndNil(m_array);
      //  end;
    end;
end;

{$endif}

{$ifdef CODE}

{$endif}