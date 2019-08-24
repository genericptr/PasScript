{$ifdef INTERFACE}

function exec(command: ansistring): ansistring; overload; inline;
function exec(command: ansistring; out exitCode: integer): ansistring; overload;
function passthru(command: ansistring): integer;

function popen(command: ansistring; modes: TFileModes = [mode_read]): TFileDesc; overload; inline;
function popen(command: ansistring; var exitCode: pinteger; modes: TFileModes = [mode_read]): TFileDesc; overload;
function pclose(var fd: TFileDesc): integer;

{$endif}

{$ifdef IMPLEMENTATION}

type
  TProcess = class
    fd: TFileDesc;
  end;

function popen(command: ansistring; modes: TFileModes = [mode_read]): TFileDesc;
var 
  exitCode: pinteger;
begin
  result := popen(command, exitCode, modes);
end;

function popen(command: ansistring; var exitCode: pinteger; modes: TFileModes = [mode_read]): TFileDesc;
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

  result.fd := _popen(pchar(command+chr(0)), @ms[1]);
  result.buffer := nil;
  result.typ := fs_process;

  exitCode := @popen_exitcode;
end;

function pclose(var fd: TFileDesc): integer;
begin
  if fd.fd = nil then
    exit(0);    
  result := _pclose(fd.fd);
  fd.fd := nil;
end;

// https://www.php.net/manual/en/function.exec.php
function exec(command: ansistring; out exitCode: integer): ansistring;
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
      if _fgets(buffer, kBufferSize, fd) <> nil then
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
var
  fd: pointer;
  buffer: pchar;
begin
  command := command+chr(0);
  fd := _popen(pchar(command), 'r');
  buffer := stralloc(kBufferSize);
  while true do 
    begin
      if _fgets(buffer, kBufferSize, fd) <> nil then
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