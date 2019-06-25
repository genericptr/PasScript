{$ifdef INTERFACE}

function exec(command: ansistring): ansistring; overload;
function exec(command: ansistring; out exitCode: integer): ansistring; overload;

function passthru(command: ansistring): integer;

type
  TFileMode = (mode_read, mode_write);
  TFileModes = set of TFileMode;

type
  TFileDesc = record
    fd: pointer;
    buffer: pchar;
  end;
  PFileDesc = ^TFileDesc;
  PProcess = pointer;

function popen(command: ansistring; modes: TFileModes = [mode_read]): PProcess;
function pclose(fd: PProcess): integer; inline;
function eof(fd: PProcess): boolean; inline;
function readline(fd: PProcess): ansistring; overload;
function readline(fd: PProcess; var line: ansistring): boolean; overload;

{$endif}

{$ifdef IMPLEMENTATION}

function popen (command: ansistring; modes: TFileModes = [mode_read]): PProcess;
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
  result := _popen(pchar(command+chr(0)), @ms[1]);
end;

function pclose (fd: PProcess): integer;
begin
  if fd = nil then
    exit(0);
  result := _pclose(fd);
end;

function eof(fd: PProcess): boolean;
begin
  if fd = nil then
    exit(true);
  result := feof(fd) <> 0;
end;

function readline(fd: PProcess; var line: ansistring): boolean;
const
  kBufferSize = 1024 * 4;
var
  buffer: pchar;
begin
  // TODO: store this in a new record
  buffer := stralloc(kBufferSize);
  if fgets(buffer, kBufferSize, fd) <> nil then
    begin
      line := buffer;
      // TODO: this can't be good
      line.contract(1);
      result := true;
    end
  else
    result := false;
  strdispose(buffer);
end;

function readline(fd: PProcess): ansistring;
begin
  if not readline(fd, result) then
    result := '';
end;

// https://www.php.net/manual/en/function.exec.php
function exec(command: ansistring; out exitCode: integer): ansistring;
const
  kBufferSize = 1024 * 4;
var
  fd: pointer;
  buffer: pchar;
begin
  command := command+chr(0);
  fd := _popen(pchar(command), 'r');
  buffer := stralloc(kBufferSize);
  result := '';
  while true do 
    begin
      if fgets(buffer, kBufferSize, fd) <> nil then
        result += buffer
      else
        break;
    end;
  strdispose(buffer);
  exitCode := _pclose(fd);
  // removing trailing null terminator
  if length(result) > 0 then
    SetLength(result, length(result) - 1);
end; 

function exec(command: ansistring): ansistring;
var
  exitCode: integer;
begin
  result := exec(command, exitCode);
end;

function passthru(command: ansistring): integer;
const
  kBufferSize = 1024 * 4;
var
  fd: pointer;
  buffer: pchar;
begin
  command := command+chr(0);
  fd := _popen(pchar(command), 'r');
  buffer := stralloc(kBufferSize);
  while true do 
    begin
      if fgets(buffer, kBufferSize, fd) <> nil then
        write(buffer)
      else
        break;
    end;
  strdispose(buffer);
  result := _pclose(fd);
end; 

{$endif}

{$ifdef CODE}

{$endif}