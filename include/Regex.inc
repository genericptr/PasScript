{$ifdef INTERFACE}

{$ifdef TARGET_OS_MAC_X86}
{$linklib libpcre_x64.a}
{$else}
{$linklib libpcre_i386.a}
{$endif}

type
  TMatchValue = class (TStringValue)
    public
      constructor Create (newValue: ansistring; inStart, inLength: longint);
      function Copy: TValue; override;
      function DebugString: ansistring; override;
    private
      start, len: longint;
  end;

type
  PPattern = ^TPattern;
  TPattern = record
    ref: pointer;
    extraInfo: pointer;
    class operator := (right: string): TPattern;
    class operator Finalize(var rec: TPattern);
  end;

function preg_match(pattern: string; subject: ansistring): boolean; overload;
function preg_match(pattern: string; subject: ansistring; out matches: TList): boolean; overload;
function preg_match(constref pattern: TPattern; subject: ansistring): boolean; overload;
function preg_match(constref pattern: TPattern; subject: ansistring; out matches: TList): boolean; overload;

function preg_match_all(pattern: string; subject: ansistring; out matches: TList): boolean;
function preg_split(pattern: string; subject: ansistring): TList;
function preg_replace(pattern: string; replacement: string; subject: ansistring): ansistring;
function preg_pattern(pattern: string; var compiled: TPattern): boolean;
function preg_pattern(pattern: string): TPattern;

type
  TPatternArray = array[0..0] of TPattern;
  PPatternArray = ^TPatternArray;

function preg_match(patterns: PPatternArray; patternCount: integer; subject: ansistring): boolean; overload;
function preg_match(patterns: array of TPattern; subject: ansistring): boolean; overload;

{$endif}

{$ifdef IMPLEMENTATION}

constructor TMatchValue.Create (newValue: ansistring; inStart, inLength: longint);
begin
  value := newValue;
  start := inStart;
  len := inLength;
  typ := TStringType;
end;

function TMatchValue.Copy: TValue;
begin
  result := TMatchValue.Create(value, start, len);
end;

function TMatchValue.DebugString: ansistring;
begin
  result := value+' '+IntToStr(start)+'/'+IntToStr(len);
end;


const
  PCRE_CASELESS             = $00000001;  // case insensitive: same as Perl /i
  PCRE_MULTILINE            = $00000002;  // same as Perl /m option
  PCRE_DOTALL               = $00000004;  // same as Perl /s option
  PCRE_EXTENDED             = $00000008;  // same as Perl /x option
  PCRE_ANCHORED             = $00000010;
  PCRE_DOLLAR_ENDONLY       = $00000020;
  PCRE_EXTRA                = $00000040;  // turn ON pcre functionality incompatible with Perl
  PCRE_NOTBOL               = $00000080;
  PCRE_NOTEOL               = $00000100;
  PCRE_UNGREEDY             = $00000200;  // make the match ungreedy: incompatible with Perl
  PCRE_NOTEMPTY             = $00000400;
  PCRE_UTF8                 = $00000800;  // evaluate both the pattern and the subject as UTF8 strings
  PCRE_NO_AUTO_CAPTURE      = $00001000;  // disables the use of numbered capturing
  PCRE_NO_UTF8_CHECK        = $00002000;  // skip validity check for UTF8 pattern
  PCRE_AUTO_CALLOUT         = $00004000;  // automatically insert callout items
  PCRE_PARTIAL              = $00008000;  // turns on partial matching
  PCRE_DFA_SHORTEST         = $00010000;  // return only the shortest match
  PCRE_DFA_RESTART          = $00020000;  // this is a restart after a partial match
  PCRE_FIRSTLINE            = $00040000;  // force matching to be before newline
  PCRE_DUPNAMES             = $00080000;
  PCRE_NEWLINE_CR           = $00100000;
  PCRE_NEWLINE_LF           = $00200000;
  PCRE_NEWLINE_CRLF         = $00300000;

  // Exec-time and get-time error codes
  PCRE_ERROR_NOMATCH        = -1;
  PCRE_ERROR_NULL           = -2;
  PCRE_ERROR_BADOPTION      = -3;
  PCRE_ERROR_BADMAGIC       = -4;
  PCRE_ERROR_UNKNOWN_NODE   = -5;
  PCRE_ERROR_NOMEMORY       = -6;
  PCRE_ERROR_NOSUBSTRING    = -7;
  PCRE_ERROR_MATCHLIMIT     = -8;
  PCRE_ERROR_CALLOUT        = -9; // never used by PCRE itself
  PCRE_ERROR_BADUTF8        = -10;
  PCRE_ERROR_BADUTF8_OFFSET = -11;
  PCRE_ERROR_PARTIAL        = -12;
  PCRE_ERROR_BADPARTIAL     = -13;
  PCRE_ERROR_INTERNAL       = -14;
  PCRE_ERROR_BADCOUNT       = -15;
  PCRE_ERROR_DFA_UITEM      = -16;
  PCRE_ERROR_DFA_UCOND      = -17;
  PCRE_ERROR_DFA_UMLIMIT    = -18;
  PCRE_ERROR_DFA_WSSIZE     = -19;
  PCRE_ERROR_DFA_RECURSE    = -20;
  PCRE_ERROR_RECURSIONLIMIT = -21;

  // Request types for pcre_fullinfo()
  PCRE_INFO_OPTIONS         = 0;
  PCRE_INFO_SIZE            = 1;
  PCRE_INFO_CAPTURECOUNT    = 2;
  PCRE_INFO_BACKREFMAX      = 3;
  PCRE_INFO_FIRSTCHAR       = 4;
  PCRE_INFO_FIRSTTABLE      = 5;
  PCRE_INFO_LASTLITERAL     = 6;
  PCRE_INFO_FIRSTBYTE       = 4;
  PCRE_INFO_NAMEENTRYSIZE   = 7;
  PCRE_INFO_NAMECOUNT       = 8;
  PCRE_INFO_NAMETABLE       = 9;
  PCRE_INFO_STUDYSIZE       = 10;
  PCRE_INFO_DEFAULT_TABLES  = 11;

  // Request type for pcre_config()
  PCRE_CONFIG_UTF8                    = 0;
  PCRE_CONFIG_NEWLINE                 = 1;
  PCRE_CONFIG_LINK_SIZE               = 2;
  PCRE_CONFIG_POSIX_MALLOC_THRESHOLD  = 3;
  PCRE_CONFIG_MATCH_LIMIT             = 4;
  PCRE_CONFIG_STACKRECURSE            = 5;
  PCRE_CONFIG_UNICODE_PROPERTIES      = 6;
  PCRE_CONFIG_MATCH_LIMIT_RECURSION   = 7;

  // Bit flags for the pcre_extra structure
  PCRE_EXTRA_STUDY_DATA               = $0001;
  PCRE_EXTRA_MATCH_LIMIT              = $0002;
  PCRE_EXTRA_CALLOUT_DATA             = $0004;
  PCRE_EXTRA_TABLES                   = $0008;
  PCRE_EXTRA_MATCH_LIMIT_RECURSION    = $0010; 
  
type
  PCRERef = pointer;
  PCRETableRef = pointer;

const
  MAX_CAPTURING_SUBPATTERNS = 65535;
  
type
  PIntArray = ^TIntArray;
  TIntArray = array[0..((MAX_CAPTURING_SUBPATTERNS+1) * 3)-1] of Integer;
  
type 
  PCREExtraRec = record
    flags: UInt32;
    study_data: pointer;
    match_limit: UInt32;
    callout_data: pointer;
    tables: pchar;                 // Pointer to character tables
    match_limit_recursion: UInt32; // Max recursive calls to match()
  end;
  PPCREExtraRec = ^PCREExtraRec;
  
type
  PCRERange = record
    location: integer;
    length: integer;
  end;

type
  PCREMatchInfo = record
    text: string;
    range: PCRERange;
    fullString: ansistring;
  end;

type
  TPCRERegex = class
    vectors: PIntArray;
    vectorSize: cint;
    vectorCount: cint;
    constructor Create;
    destructor Destroy; override;
  end;

threadvar
  SharedRegex: TPCRERegex;

var 
  pcre_options: cint = PCRE_CASELESS + PCRE_UTF8;

constructor TPCRERegex.Create;
const
  MAX_VECTOR_SIZE = 99;
begin
  vectorCount := MAX_VECTOR_SIZE;
  vectorSize := (1 + MAX_VECTOR_SIZE) * 3;
  vectors := PIntArray(GetMem(sizeof(cint) * vectorSize));
end;

destructor TPCRERegex.Destroy;
begin
  if assigned(vectors) then
    FreeMem(vectors);
end;

// https://www.pcre.org/original/doc/html/index.html
function pcre_compile (pattern: pchar; options: cint; var errptr: pchar; var erroffset: cint; tableptr: PCRETableRef): PCRERef; cdecl; external name 'pcre_compile';
function pcre_exec (pcre: PCRERef; pcre_extra: PPCREExtraRec; subject: pchar; length: cint; startoffset: cint; options: cint; ovector: PIntArray; ovecsize: cint): cint; cdecl; external name 'pcre_exec';
function pcre_study(pcre: PCRERef; options: cint; var errptr: pchar): PPCREExtraRec; cdecl; external name 'pcre_study';
procedure pcre_free (pcre: PCRERef); cdecl; external name 'pcre_free';
procedure pcre_free_study (extraInfo: PPCREExtraRec); cdecl; external name 'pcre_free_study';
function pcre_get_substring(subject: pchar; ovector: PIntArray; stringcount: cint; stringnumber: cint; stringptr: ppchar): cint; external name 'pcre_get_substring';

// https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html

class operator TPattern.:= (right: string): TPattern;
begin
  result := preg_pattern(right);
end;

class operator TPattern.Finalize(var rec: TPattern);
begin
  // TODO: crashes!
  //pcre_free(rec.ref);
  //pcre_free_study(rec.extraInfo);
end;

function preg_pattern(pattern: string; var compiled: TPattern): boolean;
var
  options: cint;
  err: pchar;
  errOffset: cint;
begin
  if SharedRegex = nil then
    SharedRegex := TPCRERegex.Create;

  pattern := pattern+chr(0);
  options := pcre_options;
  compiled.ref := pcre_compile(@pattern[1], options, err, errOffset, nil);

  result := assigned(compiled.ref);

  // https://www.pcre.org/original/doc/html/pcre_study.html
  if result then
    compiled.extraInfo := pcre_study(compiled.ref, 0, err);
end;

function preg_pattern(pattern: string): TPattern;
begin
  if not preg_pattern(pattern, result) then
    fatal('pattern '+wrap(pattern)+' failed to compile');
end;

function preg_match_internal(pattern: string; compiledPattern: PPattern; subject: ansistring; extraInfo: PPCREExtraRec; findAll: boolean; matches: PList): boolean;
const
  MAX_VECTOR_SIZE = 99;
var
  ref: PCRERef;
  options: cint;
  res: cint;
  startOffset: longint;
  matchStart: longint;
  matchEnd: longint;
  matchStr: ansistring;
  matchCount: integer;
  err: pchar;
  i: integer;
  errOffset: cint;
begin
  if SharedRegex = nil then
    SharedRegex := TPCRERegex.Create;

  result := false;
  startOffset := 0;
  pattern := pattern+chr(0);
  subject := subject+chr(0);
  options := pcre_options;

  if assigned(compiledPattern) then
    begin
      ref := compiledPattern.ref;
    end
  else
    begin
      ref := pcre_compile(@pattern[1], options, err, errOffset, nil);
      if ref = nil then
        begin
          result := false;
          exit;
        end;
    end;

  if assigned(matches) then
    begin
      repeat
        matchCount := pcre_exec(ref, extraInfo, pchar(subject), length(subject), startOffset, 0, SharedRegex.vectors, SharedRegex.vectorCount);
        if matchCount > PCRE_ERROR_NOMATCH then
          begin
            i := 0;
            //writeln('matchCount:',matchCount);
            repeat
              // TODO: we can use pcre_get_substring for this but is it better?
              matchStart := SharedRegex.vectors[i + 0];
              matchEnd := SharedRegex.vectors[i + 1];
              startOffset := matchEnd;
              matchStr := substr(subject, matchStart, matchEnd - matchStart);
              matches.AddWithoutCopy(TMatchValue.Create(matchStr, matchStart, matchEnd - matchStart));
              i += 2;
              dec(matchCount);
              result := true;
            until matchCount = 0;
            // done searching
            if not findAll then
              matchCount := -1;
          end;
      until matchCount < 0;
    end
  else
    begin
      res := pcre_exec(ref, extraInfo, pchar(subject), length(subject), startOffset, 0, nil, 0);
      //case PCRE_ERROR_NOMATCH      : printf("String did not match the pattern\n");        break;
      //case PCRE_ERROR_NULL         : printf("Something was null\n");                      break;
      //case PCRE_ERROR_BADOPTION    : printf("A bad option was passed\n");                 break;
      //case PCRE_ERROR_BADMAGIC     : printf("Magic number bad (compiled re corrupt?)\n"); break;
      //case PCRE_ERROR_UNKNOWN_NODE : printf("Something kooky in the compiled re\n");      break;
      //case PCRE_ERROR_NOMEMORY     : printf("Ran out of memory\n");                       break;
      if res > PCRE_ERROR_NOMATCH then
        result := true;
    end;

  // TODO: crashes??
  if not assigned(compiledPattern) then
    //pcre_free(ref);
    ;
end;

function preg_match(pattern: string; subject: ansistring): boolean;
begin
  if pattern = '' then
    exit(false);
  result := preg_match_internal(pattern, nil, subject, nil, false, nil);
end;

function preg_match(pattern: string; subject: ansistring; out matches: TList): boolean;
begin
  if pattern = '' then
    exit(false);
  result := preg_match_internal(pattern, nil, subject, nil, false, @matches);
end;

function preg_match(constref pattern: TPattern; subject: ansistring): boolean;
begin
  result := preg_match_internal('', @pattern, subject, pattern.extraInfo, false, nil);
end;

function preg_match(constref pattern: TPattern; subject: ansistring; out matches: TList): boolean;
begin
  result := preg_match_internal('',  @pattern, subject, pattern.extraInfo, false, @matches);
end;

function preg_match(patterns: PPatternArray; patternCount: integer; subject: ansistring): boolean;
var
  pattern: string;
  i: integer;
begin
  for i := 0 to patternCount - 1 do
    if preg_match(patterns[i], subject) then
      exit(true);
  result := false;
end;

function preg_match(patterns: array of TPattern; subject: ansistring): boolean;
var
  pattern: string;
  i: integer;
begin
  for i := 0 to high(patterns) do
    if preg_match(patterns[i], subject) then
      exit(true);
  result := false;
end;


function preg_match_all(pattern: string; subject: ansistring; out matches: TList): boolean;
begin
  if pattern = '' then
    exit(false);
  result := preg_match_internal(pattern, nil, subject, nil, true, @matches);
end;

// https://www.php.net/manual/en/function.preg-split.php
function preg_split(pattern: string; subject: ansistring): TList;
var
  matches: TList;
  match: TMatchValue;
  offset: longint;
  i: longint;
  part: ansistring;
begin
  if pattern = '' then
    exit;
  // TODO: if we have an optional callback we can avoid all the
  // extra allocations
  if preg_match_internal(pattern, nil, subject, nil, true, @matches) then
    begin
      offset := 0;
      for pointer(match) in matches do
        begin
          writeln(match.start,'/',match.len);
          part := '';
          for i := offset to match.start do
            part += subject[i];
          result.Add(part);
          // advance
          offset := match.start + match.len + 1;
        end;
      // last part
      part := '';
      for i := offset to length(subject) do
        part += subject[i];
      result.Add(part);
    end;
end;

// https://www.php.net/manual/en/function.preg-replace.php
function preg_replace(pattern: string; replacement: string; subject: ansistring): ansistring;
var
  matches: TList;
begin
  if pattern = '' then
    exit(subject);
  // TODO: if we have an optional callback we can avoid all the
  // extra allocations
  if preg_match_internal(pattern, nil, subject, nil, false, @matches) then
    result := replace(subject, replacement, TMatchValue(matches[0]).start, TMatchValue(matches[0]).len)
  else
    result := subject;
end;

{$endif}

{$ifdef CODE}

{$endif}