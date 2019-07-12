{$ifdef darwin}

type
  PFile = pointer;

// https://linux.die.net/man/3/popen
function _popen (command: pchar; mode: pchar): PFile; cdecl; external name 'popen';
function _pclose (fd: PFile): cint; cdecl; external name 'pclose';

// TODO: use RTL instead and make wrappers for php style names
// https://www.freepascal.org/docs-html/rtl/sysutils/filecreate.html

const
  SEEK_SET  = 0; { Seek from the beginning of data }
  SEEK_CUR  = 1; { Seek relative to current read point }
  SEEK_END  = 2; { Seek relative to the end of data }

function _fopen(filename: pchar; mode: pchar): pointer; cdecl; external name 'fopen';
procedure _fclose(fp: pointer); cdecl; external name 'fclose';
function _fseek(fp: pointer; recPos: clong; mode: pchar): clong; cdecl; external name 'fseek';
function _fgets(buffer: pointer; bufferSize: cint; fd: pointer): pointer; cdecl; external name 'fgets';
function _fread(buffer: pointer; elementSize: cint; count: cint; fd: pointer): cint; cdecl; external name 'fread';
function _fwrite(buffer: pointer; elementSize: cint; count: cint; fd: pointer): cint; cdecl; external name 'fwrite';
function _ftell(fp: pointer): clong; cdecl; external name 'ftell';
function _feof(fp: pointer): clong; cdecl; external name 'feof';
function _fflush(fd: pointer): cint; cdecl; external name 'fflush';

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