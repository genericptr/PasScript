{$ifdef INTERFACE}

type
  TFileMode = (mode_read, mode_write);
  TFileModes = set of TFileMode;

type
  PFileDesc = ^TFileDesc;
  TFileDescEnumerator = record
    private
      fd: PFileDesc;
      line: ansistring;
    public
      function MoveNext: boolean;
      property Current: ansistring read line;
  end;
  TFileDesc = record
    private
      fd: pointer;
      buffer: pchar;
      exitCode: integer;
    public
      function GetEnumerator: TFileDescEnumerator;
    private
      procedure Free;
  end;

function exec(command: ansistring): ansistring; overload; inline;
function exec(command: ansistring; out exitCode: integer): ansistring; overload;
function passthru(command: ansistring): integer;

function popen(command: ansistring; modes: TFileModes = [mode_read]): TFileDesc; overload; inline;
function popen(command: ansistring; var exitCode: pinteger; modes: TFileModes = [mode_read]): TFileDesc; overload;
function pclose(var fd: TFileDesc): integer; inline;
function eof(fd: TFileDesc): boolean; inline;
function readline(fd: TFileDesc): ansistring; overload;
function readline(fd: TFileDesc; var line: ansistring): boolean; overload;
function pread(command: ansistring; var line: ansistring): boolean; overload;

{$endif}

{$ifdef IMPLEMENTATION}

type
  TProcess = class
    fd: TFileDesc;
  end;

const
  kBufferSize = 1024 * 4;

threadvar
  SharedProcess: TProcess;

function TFileDesc.GetEnumerator: TFileDescEnumerator;
begin
  result.fd := @self;
  result.line := '';
end;

function TFileDescEnumerator.MoveNext: boolean;
begin
  // TODO: this just passes around the buffer
  // should we just kill readline and use the buffer directly?
  result := readline(fd^, line);

  // cleanup
  if not result then
    begin
      fd.exitCode := pclose(fd^);
      SharedProcess.Free;
      SharedProcess := nil;
    end;
end;

procedure TFileDesc.Free;
begin
  if assigned(buffer) then
    strdispose(buffer);
end;

function pread(command: ansistring; var line: ansistring): boolean;
begin
  if not assigned(SharedProcess) then
    SharedProcess.fd := popen(command);

  result := readline(SharedProcess.fd, line);

  // cleanup
  if not result then
    begin
      pclose(SharedProcess.fd);
      SharedProcess.Free;
      SharedProcess := nil;
    end;
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
  if not assigned(SharedProcess) then
    SharedProcess := TProcess.Create;

  if mode_read in modes then
    ms += 'r';
  if mode_write in modes then
    ms += 'w';
  if ms = '' then
    fatal('popen invald mode');
  ms += chr(0);

  result.fd := _popen(pchar(command+chr(0)), @ms[1]);
  result.buffer := nil;

  exitCode := @SharedProcess.fd.exitCode;
end;

function pclose(var fd: TFileDesc): integer;
begin
  if fd.fd = nil then
    exit(0);
  result := _pclose(fd.fd);
  fd.fd := nil;
end;

function eof(fd: TFileDesc): boolean;
begin
  if fd.fd = nil then
    exit(true);
  result := _feof(fd.fd) <> 0;
end;

function readline(fd: TFileDesc; var line: ansistring): boolean;
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

function readline(fd: TFileDesc): ansistring;
begin
  if not readline(fd, result) then
    result := '';
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