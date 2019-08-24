
{$ifdef INTERFACE}

// TODO: ansistring has to go because it can't grow smart enough

type
  generic TLongString<T> = record
    private type
      TStringChar = T;
      TStringIndex = integer;
      TInternalString = array[0..high(word)] of TStringChar;
      PInternalString = ^TInternalString;
      TArrayEnumerator = record
        private
          list: TInternalString;
          currentValue: TStringChar;
          currentIndex: TArrayIndex;
          lastElement: TArrayIndex;
        public
          constructor Create(source: TInternalString; _lastElement: TArrayIndex); 
          function MoveNext: boolean;
          property Current: TStringChar read currentValue;
      end;
    public
      procedure SetCapacity(newValue: integer);
      property Values[index: integer]: TStringChar read GetChar write SetChar; default;
    private
      class operator := (right: ansistring): TLongString;
      class operator := (right: string): TLongString;
      class operator + (left: TLongString; right: ansistring): TLongString; overload;
      class operator + (left: TLongString; right: string): TLongString; overload;
      class operator = (left: TLongString; right: ansistring): boolean; overload;
      class operator = (left: TLongString; right: string): TLongString; overload;
    private
      m_string: PInternalString;
  end;
  LongString = specialize TLongString<char>;
  LongUnicodeString = specialize TLongString<unicodechar>;

{$endif}

{$ifdef IMPLEMENTATION}

{$endif}
