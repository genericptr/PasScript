{$ifdef darwin}

type
  PFile = pointer;

// https://linux.die.net/man/3/popen
function _popen (command: pchar; mode: pchar): PFile; cdecl; external name 'popen';
function _pclose (fd: PFile): cint; cdecl; external name 'pclose';

const
  SEEK_SET  = 0; { Seek from the beginning of data }
  SEEK_CUR  = 1; { Seek relative to current read point }
  SEEK_END  = 2; { Seek relative to the end of data }

function fopen(filename: pchar; mode: pchar): pointer; cdecl; external;
procedure fclose(fp: pointer); cdecl; external;
function fseek(fp: pointer; recPos: clong; mode: pchar): clong; cdecl; external;
function fgets (buffer: pointer; bufferSize: cint; fd: pointer): pointer; cdecl; external;
function fread (buffer: pointer; elementSize: cint; count: cint; fd: pointer): cint; cdecl; external;
function fwrite (buffer: pointer; elementSize: cint; count: cint; fd: pointer): cint; cdecl; external;
function ftell(fp: pointer): clong; cdecl; external;
function feof(fp: pointer): clong; cdecl; external;
function fflush (fd: pointer): cint; cdecl; external;

type
  Ppasswd = ^passwd;
  passwd = record
    pw_name : Pchar;
    pw_passwd : Pchar;
    pw_uid : cint;
    pw_gid : cint;
    pw_gecos : Pchar;
    pw_dir : Pchar;
    pw_shell : Pchar;
  end;
  PPpasswd = ^Ppasswd;

function getpwuid(uid: cint):Ppasswd; cdecl; external clib name 'getpwuid';

{$endif}