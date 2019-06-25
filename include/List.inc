{$ifdef INTERFACE}
type
  PList = ^TList;
  TList = record
    private 
      type
        TArrayIndex = integer;
        TInternalArray = array of TValue;
        TArrayEnumerator = record
          private
            list: TInternalArray;
            currentValue: TValue;
            currentIndex: TArrayIndex;
            lastElement: TArrayIndex;
          public
            constructor Create(source: TInternalArray; _lastElement: TArrayIndex); 
            function MoveNext: boolean;
            property Current: TValue read currentValue;
        end;
    private
      function GetValue(index: integer): TValue; inline;
      procedure SetValue(index: integer; value: TValue); inline; 
    public
      { Methods }
      procedure Add(value: TValue); overload;
      procedure Add(value: integer); overload;
      procedure Add(value: float); overload;
      procedure Add(value: string); overload;

      { Removing }
      procedure Remove(value: integer);
      procedure Clear;

      { Searching }
      function IndexOf(value: integer): integer; overload;
      function IndexOf(value: float): integer; overload;
      function IndexOf(value: string): integer; overload;

      { Element Access }
      property Values[index: integer]: TValue read GetValue write SetValue; default;

      { Introspection }
      function Count: integer; inline;
      function High: integer; inline;
      function Capacity: integer; inline;
      function GetEnumerator: TArrayEnumerator;
      procedure Show;

    private
      class operator := (right: array of const): TList;
      class operator Initialize(var list: TList);
      class operator Finalize(var list: TList);
    private
      m_array: TInternalArray;
      currentIndex: integer;

      procedure Expand(newSize: integer);
      procedure GrowToFit(newSize: integer);
  end;
{$endif}

{$ifdef IMPLEMENTATION}
constructor TList.TArrayEnumerator.Create(source: TInternalArray; _lastElement: TArrayIndex);
begin
  list := source;
  lastElement := _lastElement;
  currentValue := nil;
  currentIndex := 0;
end;
  
function TList.TArrayEnumerator.MoveNext: boolean;
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

procedure TList.Show;
var
  i: integer;
begin
  for i := 0 to High do
    m_array[i].Show;
end;

class operator TList.Initialize(var list: TList);
begin
  list.currentIndex := 0;
end;

class operator TList.Finalize(var list: TList);
begin
  list.Clear;
end;

class operator TList.:= (right: array of const): TList;
var
  i: integer;
begin
  writeln(':= array of const');
  result.GrowToFit(length(right));
  result.Clear;
  for i := 0 to system.high(right) do
    begin
      case right[i].vtype of
        vtinteger:
          result.Add(right[i].vinteger);
        vtextended:
          result.Add(right[i].vextended^);
        vtString:
          result.Add(right[i].vstring^);
        //vtPointer:
        //  Writeln (’Pointer, value : ’,Longint(Args[i].VPointer));
        //vtObject:
        //  value := TObject(args[i].VObject);
        //vtClass      :
        //  Writeln (’Class reference, name :’,Args[i].VClass.Classname);
        vtAnsiString:
          result.Add(ansistring(right[i].vansistring));
        otherwise
          // TODO: fatal
          //Fatal('TArray: variable argument value type '+IntToStr(args[i].vtype)+' is invalid.');
          begin
            writeln('dead meat');
            halt;
          end;
      end;
    end;
end;

function TList.Capacity: integer;
begin
  result := length(m_array);
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
  i: integer;
begin
  // TODO: shrink?
  for i := 0 to High do
    begin
      m_array[i].Free;
      m_array[i] := nil;
    end;
  currentIndex := 0;
end;

procedure TList.Add(value: TValue);
begin
  GrowToFit(Count + 1);
  m_array[currentIndex] := value.Copy;
  currentIndex += 1;
end;


procedure TList.Add(value: integer);
begin
  GrowToFit(Count + 1);
  m_array[currentIndex] := value;
  currentIndex += 1;
end;

procedure TList.Add(value: float);
begin
  GrowToFit(Count + 1);
  m_array[currentIndex] := value;
  currentIndex += 1;
end;

procedure TList.Add(value: string);
begin
  GrowToFit(Count + 1);
  m_array[currentIndex] := value;
  currentIndex += 1;
end;

procedure TList.Remove(value: integer);
begin
  // TODO: shrink list
end;

function TList.IndexOf(value: integer): integer;
var
  i: integer;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i] = value then
      exit(i);
end;

function TList.IndexOf(value: float): integer;
var
  i: integer;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i] = value then
      exit(i);
end;

function TList.IndexOf(value: string): integer;
var
  i: integer;
begin
  result := -1;
  for i := 0 to High do
    if m_array[i] = value then
      exit(i);
end;

function TList.GetEnumerator: TArrayEnumerator;
begin
  result := TArrayEnumerator.Create(m_array, High);
end;

procedure TList.Expand(newSize: integer);
begin
  // TODO: allow shrinking
  SetLength(m_array, newSize);
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

function TList.GetValue(index: integer): TValue;
begin
  result := m_array[index];
end;

procedure TList.SetValue(index: integer; value: TValue);
begin
  m_array[index] := TValue.Create;
end;
{$endif}

{$ifdef CODE}

{$endif}