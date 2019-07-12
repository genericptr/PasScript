{$ifdef INTERFACE}

function file_get_contents(path: ansistring): ansistring;
function file_put_contents(path: ansistring; contents: ansistring): boolean;

const
  INVALID = -1;

type
  TFileHandle = PDir;

type
  TFileParts = record
    name: ansistring;
    dir: ansistring;
    ext: ansistring;
  end;

function opendir(path: ansistring): TFileHandle;
function readdir(handle: TFileHandle; out name: pchar): boolean;
function scandir(path: ansistring; fullPath: boolean = false): TList;
procedure closedir(handle: TFileHandle);

function is_dir(path: ansistring): boolean;
function is_file(path: ansistring): boolean;
function is_link(path: ansistring): boolean;
function readlink(path: ansistring): ansistring; inline;

function exists(path: ansistring): boolean;
function file_parts(path: ansistring): TFileParts;
function basename(path: ansistring; extension: boolean = true): ansistring;
function dirname(path: ansistring): ansistring;

function getcwd: ansistring; inline;
procedure chdir(dir: ansistring); inline;

function ExpandTilde(str: ansistring): ansistring;

//$bookmark DATE & TIME 

function TimeSinceNow: longint;

//$bookmark STRING HELPERS

  type
    TLongStringFileHelpers = type helper for LongString
      function ExpandTilde: ansistring;
    end;

  type
    TShortStringFileHelpers = type helper for ShortString
      function ExpandTilde: string;
    end;

{$endif}

{$ifdef IMPLEMENTATION}

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

function TShortStringFileHelpers.ExpandTilde: string;
begin
  result := PasScript.ExpandTilde(self);
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

function exists(path: ansistring): boolean;
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

function basename(path: ansistring; extension: boolean = true): ansistring;
begin
  if extension then
    result := ExtractFileName(path)
  else
    begin
      result := ExtractFileName(path);
      result := copy(result,1,pos(ExtractFileExt(result),result)-1);
    end;
end;

function dirname(path: ansistring): ansistring;
begin
  result := ExtractFileDir(path);
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
  info : stat;
begin
  if fpStat(path,info) = 0 then
    result := fpS_ISLNK(info.st_mode)
  else
    result := false;
end;

function readlink(path: ansistring): ansistring;
begin
  result := fpReadLink(path);
end;

function opendir(path: ansistring): TFileHandle;
begin
  result := TFileHandle(fpOpenDir(path));
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

// TODO: we have some more generic FPC functions for file access
// probably better to use this than darwin
// https://www.freepascal.org/docs-html/rtl/sysutils/filecreate.html

function file_get_contents(path: ansistring): ansistring;
var
  f: File;
begin
  if not exists(path) then
    exit('');
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