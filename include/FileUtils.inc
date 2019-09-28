{$ifdef INTERFACE}

const
  INVALID = -1;

type
  TFileHandle = PDir;
  TFileMode = (mode_read, mode_write);
  TFileModes = set of TFileMode;
  TFileStreamType = ( fs_file, 
                      fs_directory, 
                      fs_process, 
                      fs_socket);

type
  PFileDesc = ^TFileDesc;
  TFileDescEnumerator = record
    private
      fd: PFileDesc;
      line: ansistring;
    public
      procedure Reset;
      function MoveNext: boolean;
      property Current: ansistring read line;
  end;
  TFileDesc = record
    private
      fd: pointer;
      buffer: pchar;
      typ: TFileStreamType;
    public
      function GetEnumerator: TFileDescEnumerator;
      class operator := (constref right: TFileDesc): boolean; {$i inline_operator}
      procedure Close;
    private
      procedure Free;
  end;

type
  TFileParts = record
    name: ansistring;
    dir: ansistring;
    ext: ansistring;
  end;

{ Reading/Writing Files }
function file_get_contents(path: ansistring): ansistring;
function file_put_contents(path: ansistring; contents: ansistring): boolean;
function file_get_lines(path: ansistring; ignoreEmptyLines: boolean = false): TList;

{ File Streams }
function fopen(command: ansistring; modes: TFileModes = [mode_read]): TFileDesc;
function eof(var fd: TFileDesc): boolean; inline;
function readline(var fd: TFileDesc): ansistring; overload;
function readline(var fd: TFileDesc; var line: ansistring): boolean; overload;
function read(var fd: TFileDesc; count: integer; var line: ansistring): integer; overload;
function read(var fd: TFileDesc; count: integer): ansistring; overload;
procedure close(var fd: TFileDesc);

{ Directory Streams }
// TODO: opendir needs to be TFileDesc now! 
function opendir(path: ansistring): TFileHandle;
function readdir(handle: TFileHandle; out name: pchar): boolean;
function readdir(path: ansistring): TFileDesc; inline;
function scandir(path: ansistring; fullPath: boolean = false): TList;
procedure closedir(handle: TFileHandle);

{ File Utilities }
function is_dir(path: ansistring): boolean;
function is_file(path: ansistring): boolean;
function is_link(path: ansistring): boolean;
function is_hidden(path: ansistring): boolean;
function readlink(path: ansistring): ansistring; inline;
function file_exists(path: ansistring): boolean;
function file_parts(path: ansistring): TFileParts;
function basename(path: ansistring; extension: boolean = true): ansistring;
function dirname(path: ansistring): ansistring;

procedure unlink(path: ansistring);
function mkdir(path: ansistring): boolean;
function copyfile(source: ansistring; dest: ansistring): boolean;
function rename(source: ansistring; dest: ansistring): boolean; inline;

function getcwd: ansistring; inline;
procedure chdir(dir: ansistring); inline;
function tmpfile: ansistring;
function tmpdir: ansistring;

{ Mac Utilities }
function ExpandTilde(str: ansistring): ansistring;

{ Path Utilities }
function MakePath (parts: array of const): ansistring;

//$bookmark DATE & TIME 

function TimeSinceNow: longint;

//$bookmark STRING HELPERS

type
  TLongStringFileHelpers = type helper for LongString
    function ExpandTilde: ansistring;
    function FileExists: boolean;
    function AddComponent(component: LongString): LongString;
    function AddExtension(extension: ShortString): LongString;
  end;

type
  TShortStringFileHelpers = type helper for ShortString
    function ExpandTilde: string;
    function FileExists: boolean;
    function AddComponent(component: ShortString): ShortString;
    function AddExtension(extension: ShortString): ShortString;
  end;

{$endif}

{$ifdef IMPLEMENTATION}

const
  kBufferSize = 1024 * 4;

threadvar
  popen_exitcode: integer;

class operator TFileDesc.:= (constref right: TFileDesc): boolean;
begin
  result := assigned(right.fd);
end;

procedure TFileDesc.Close;
begin
  PasScript.close(self);
end;

function TFileDesc.GetEnumerator: TFileDescEnumerator;
begin
  result.fd := @self;
  result.line := '';
end;

procedure TFileDescEnumerator.Reset;
begin
end;

function TFileDescEnumerator.MoveNext: boolean;
begin
  case fd.typ of
    fs_directory:
      begin
        result := readdir(fd.fd, fd.buffer);
        if not result then
          closedir(fd.fd);
      end;
    fs_process:
      begin
        // TODO: this just passes around the buffer
        // should we just kill readline and use the buffer directly?
        result := readline(fd^, line);
        if not result then
          popen_exitcode := pclose(fd^);
      end;
    otherwise
      fatal('stream is not enumeratable');
  end;

end;

procedure TFileDesc.Free;
begin
  if assigned(buffer) then
    strdispose(buffer);
end;

//function SystemTime: double;
//var
//  now: TDateTime;
//begin
//  result := TimeStampToMSecs(DateTimeToTimeStamp(Now)) / 1000;
//end;  
  
function TimeSinceNow: longint;
begin
  result := round(TimeStampToMSecs(DateTimeToTimeStamp(Now)));
end;    

function TLongStringFileHelpers.ExpandTilde: ansistring;
begin
  result := PasScript.ExpandTilde(self);
end;

function TLongStringFileHelpers.FileExists: boolean;
begin
  result := file_exists(self);
end;

function TLongStringFileHelpers.AddComponent(component: LongString): LongString;
begin
  result := self+'/'+component;
end;

function TLongStringFileHelpers.AddExtension(extension: ShortString): LongString;
begin
  result := self+'.'+extension;
end;

function TShortStringFileHelpers.ExpandTilde: string;
begin
  result := PasScript.ExpandTilde(self);
end;

function TShortStringFileHelpers.FileExists: boolean;
begin
  result := file_exists(self);
end;

function TShortStringFileHelpers.AddComponent(component: ShortString): ShortString;
begin
  result := self+'/'+component;
end;

function TShortStringFileHelpers.AddExtension(extension: ShortString): ShortString;
begin
  result := self+'.'+extension;
end;

function MakePath (parts: array of const): ansistring;
var
  i: integer;
begin
  result := '';
  for i := 0 to high(parts) do
    begin
      case parts[i].vtype of
        vtString:
          if parts[i].vstring^[1] = '/' then
            result += parts[i].vstring^
          else
            result += '/'+parts[i].vstring^;
        vtAnsiString:
          if ansistring(parts[i].vansistring)[1] = '/' then
            result += ansistring(parts[i].vansistring)
          else
            result += '/'+ansistring(parts[i].vansistring);
        otherwise
          fatal('can''t make path from non-string parts');
      end;
    end;
end;

var
  SystemUserName: string = '';

function ExpandTilde(str: ansistring): ansistring;
var
  pw: Ppasswd;
begin
  if SystemUserName = '' then
    begin
      pw := getpwuid(FpGetuid);
      SystemUserName := pw.pw_name;
    end;
  result := replace('~', '/Users/'+SystemUserName, str, false);
end;

function file_exists(path: ansistring): boolean;
var
  info : stat;
begin
  if fpstat(path, info) = 0 then
    begin
      if fpS_ISDIR(info.st_mode) then
        result := DirectoryExists(path)
      else if fpS_ISREG(info.st_mode) then
        result := FileExists(path)
      else
        result := false;
    end
  else
    result := false;
end;

function file_parts(path: ansistring): TFileParts;
var
  i: integer;
  extOffset: integer = INVALID;
  nameOffset: integer = INVALID;
  len: integer;
begin
  for i := length(path) downto 1 do
    begin
      if (extOffset = INVALID) and (path[i] = '.') then
        extOffset := i
      else if (nameOffset = INVALID) and (path[i] = '/') then
        begin
          nameOffset := i;
          break;
        end;
    end;

  if extOffset <> INVALID then
    len := extOffset - nameOffset - 1
  else
    len := length(path) - nameOffset;

  result.name := strcopy(path, nameOffset + 1, len);
  result.dir := strcopy(path, 0, nameOffset);

  if extOffset <> INVALID then
    result.ext := strcopy(path, extOffset + 1, length(path) - extOffset);
end;

function getcwd: ansistring;
begin
  result := GetCurrentDir;
end;

procedure chdir(dir: ansistring);
begin
  SetCurrentDir(dir);
end;

// https://www.php.net/manual/en/function.tmpfile.php
function tmpfile: ansistring;
begin
  result := GetTempDir.AddComponent(UUIDGen);
  if not file_exists(result) then
    file_put_contents(result, '');
end;

function tmpdir: ansistring;
begin
  result := GetTempDir.AddComponent(UUIDGen);
  if not file_exists(result) then
    mkdir(result);
end;

function copyfile(source: ansistring; dest: ansistring): boolean;
var
  stream: TMemoryStream;
begin
  {$ifdef DARWIN}
  // on mac we need to use system calls or extended attributes won't get copied
  source += #0;
  dest += #0;
  result := NSFileManager.defaultManager.copyItemAtPath_toPath_error(NSSTR(pchar(source)), NSSTR(pchar(dest)), nil);
  {$else}
  stream := TMemoryStream.Create;
  try
    stream.LoadFromFile(source);
    stream.SaveToFile(dest); 
    result := true;
  except
    result := false;
  end;
  stream.Free;
  {$endif}
end;

function rename(source: ansistring; dest: ansistring): boolean;
begin
  result := fpRename(source, dest) = 0;
end;

function basename(path: ansistring; extension: boolean = true): ansistring;
var
  ext: string;
begin
  if extension then
    result := ExtractFileName(path)
  else
    begin
      result := ExtractFileName(path);
      ext := ExtractFileExt(result);
      if ext <> '' then
        result := Copy(result, 1, pos(ext, result) - 1);
    end;
end;

function dirname(path: ansistring): ansistring;
begin
  result := ExtractFileDir(path);
end;

procedure unlink(path: ansistring);
begin
  // TODO: where is this is system frameworks?
  fpunlink(path);
end;

function mkdir(path: ansistring): boolean;
begin
  // TODO: where is this is system frameworks?
  //system.mkdir(path);
  result := CreateDir(path);
end;

function is_dir(path: ansistring): boolean;
var
  info : stat;
begin
  if fpstat(path,info) = 0 then
    result := fpS_ISDIR(info.st_mode)
  else
    result := false;
end;

function is_file(path: ansistring): boolean;
var
  info : stat;
begin
  if fpstat(path,info) = 0 then
    result := fpS_ISREG(info.st_mode)
  else
    result := false;
end;

function is_link(path: ansistring): boolean;
var
  info: stat;
  {$ifdef DARWIN}
  attrs: NSDictionary;
  value: boolean;
  url: CFURLRef;
  {$endif}
begin
  if fpStat(path,info) = 0 then
    begin
      result := fpS_ISLNK(info.st_mode);
      if not result then
        begin
          {$ifdef DARWIN}
          attrs := NSFileManager.defaultManager.attributesOfItemAtPath_error(NSSTR(pchar(path)), nil);
          result := attrs.objectForKey(NSFileType) = NSFileTypeSymbolicLink;
          attrs.release;
          //writeln(attrs.description.utf8string);
          //fileSystemRepresentation := path+chr(0);
          //if FSPathMakeRef('/usr/local/bin/pps', ref, nil) = noErr then
          //  begin
          //    FSIsAliasFile(ref, isAlias, isFolder);
          //    writeln('isAlias:',isAlias);
          //  end;
          {$endif}
        end;
    end
  else
    result := false;
end;

function is_hidden(path: ansistring): boolean;
begin
  {$ifdef DARWIN}
  result := path[1] <> '.';
  {$else}
  fatal('is_hidden not implemented for this system');
  {$endif}
end;

function readlink(path: ansistring): ansistring;
begin
  result := fpReadLink(path);
end;

function opendir(path: ansistring): TFileHandle;
begin
  result := TFileHandle(fpOpenDir(path));
end;

function readdir(path: ansistring): TFileDesc;
begin
  result.fd := opendir(path);
  result.buffer := nil;
  result.typ := fs_directory;
end;

function readdir(handle: TFileHandle; out name: pchar): boolean;
var
  entry: PDirent;
begin
  while true do
    begin
      entry := fpReadDir(handle);
      if assigned(entry) then
        begin
          name := pchar(@entry.d_name[0]);
          if (name = '.') or (name = '..') then
            continue;
          exit(true);
        end
      else
        exit(false);
    end;
end;

function scandir(path: ansistring; fullPath: boolean = false): TList;
var
  handle: TFileHandle;
  entry: PDirent;
  name: pchar;
begin
  handle := TFileHandle(fpOpenDir(path));
  if assigned(handle) then
    begin
      while true do
        begin
          entry := fpReadDir(handle);
          if assigned(entry) then
            begin
              name := pchar(@entry.d_name[0]);
              if (name = '.') or (name = '..') then
                continue;
              if fullPath then
                result.Add(path+'/'+name)
              else
                result.Add(name);
            end
          else
            break;
        end;
      fpCloseDir(handle);
    end;
end;

procedure closedir(handle: TFileHandle);
begin
  fpCloseDir(handle);
end;

function fopen(command: ansistring; modes: TFileModes = [mode_read]): TFileDesc;
var
  ms: string = '';
begin
  if mode_read in modes then
    ms += 'r';
  if mode_write in modes then
    ms += 'w';
  if ms = '' then
    fatal('popen invald mode');
  ms += chr(0);

  result.fd := _fopen(pchar(command+chr(0)), @ms[1]);
  result.buffer := nil;
  result.typ := fs_file;
end;

function eof(var fd: TFileDesc): boolean;
begin
  if fd.fd = nil then
    exit(true);
  result := _feof(fd.fd) <> 0;
end;

procedure close(var fd: TFileDesc);
begin
  _fclose(fd.fd);
  fd.fd := nil;
end;

function readline(var fd: TFileDesc; var line: ansistring): boolean;
begin
  if not assigned(fd.buffer) then
    fd.buffer := stralloc(kBufferSize);
  if _fgets(fd.buffer, kBufferSize, fd.fd) <> nil then
    begin
      line := fd.buffer;
      // TODO: we want to trim the last eol char but this can't be good
      line.contract(1);
      result := true;
    end
  else
    result := false;
end;

function readline(var fd: TFileDesc): ansistring;
begin
  if not readline(fd, result) then
    result := '';
end;

function read(var fd: TFileDesc; count: integer; var line: ansistring): integer;
begin
  if not assigned(fd.buffer) then
    fd.buffer := stralloc(kBufferSize)
  else if StrBufSize(fd.buffer) < count then
    begin
      strdispose(fd.buffer);
      fd.buffer := stralloc(count); 
    end;
  result := _fread(fd.buffer, 1, count, fd.fd);
  if result > 0 then
    begin
      line := fd.buffer;
      // TODO: we want to trim the last eol char but this can't be good
      line.contract(1);
    end;
end;

function read(var fd: TFileDesc; count: integer): ansistring;
begin
  if read(fd, count, result) < 1 then
    result := '';
end;

(*

char * buffer = 0;
long length;
FILE * f = fopen (filename, "rb");

if (f)
{
  fseek (f, 0, SEEK_END);
  length = ftell (f);
  fseek (f, 0, SEEK_SET);
  buffer = malloc (length);
  if (buffer)
  {
    fread (buffer, 1, length, f);
  }
  fclose (f);
}

if (buffer)
{
  // start to process your data / extract strings here...
}

FILE* file = fopen(filename, "rb");
char buffer[4];

if (file) {
    /* File was opened successfully. */

    /* Attempt to read */
    while (fread(buffer, 1, 4, file) == 4) {
        /* byte swap here */
    }

    fclose(file);
}

*)

function file_get_lines(path: ansistring; ignoreEmptyLines: boolean = false): TList;
var
  fd: TFileDesc;
  line: ansistring;
  i: integer;
  valid: boolean;
begin
  fd := fopen(path,[mode_read]);
  if fd then
    begin
      while readline(fd, line) do
        begin
          if ignoreEmptyLines then
            begin
              valid := false;
              for i := 1 to length(line) do
                if not (line[i] in WhiteSpaceCharacters) then
                  begin
                    valid := true;
                    break;
                  end;
              if not valid then
                continue;
            end;
          result += line;
        end;
      close(fd);
    end;
end;



// TODO: we have some more generic FPC functions for file access
// probably better to use this than darwin
// https://www.freepascal.org/docs-html/rtl/sysutils/filecreate.html

function file_get_contents(path: ansistring): ansistring;
var
  f: File;
begin
  if not file_exists(path) then
    fatal('file "'+path+'" doesn''t exist');
  try
    AssignFile(f, path);
    FileMode := fmOpenRead;
    Reset(f, 1);
    SetLength(result, FileSize(f));
    BlockRead(f, pointer(result)^, FileSize(f));
    CloseFile(f);
  except
    on E:Exception do
      writeln(path+': ', E.Message);
  end;
end;


function file_put_contents(path: ansistring; contents: ansistring): boolean;
var
  f: TextFile;
begin
  try
    AssignFile(f, path);
    Rewrite(f);
    Write(f, contents);
    CloseFile(f);
  except
    on E:Exception do
      writeln(path, ': ', E.Message);
  end;
end;

{$endif}

{$ifdef CODE}
{$endif}