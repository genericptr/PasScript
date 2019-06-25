{$mode objfpc}

program pps;
uses
  PasScript, SysUtils;

const
  fpc_version = '3.3.1';
  fpc_arch = 'ppcx64';

var
  i: integer;
  source: string;
  version: string;
  arch: string;
  options: ansistring = '';
  exec: ansistring;
  fpc: string;
  command: ansistring;
begin
  if ParamCount = 1 then
    begin
      source := ParamStr(0);
      if file_exists(source) then
        begin
          fpc := '/usr/local/lib/fpc/'+fpc_version+'/'+fpc_arch;
          exec := GetTempDir+basename(source);
          options += '-vbr ';
          options += '-gw ';
          options += '-godwarfcpp ';
          options += '-Fl/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib ';
          options += '-FU"'+GetTempDir+'" ';
          options += '-Fu"/Developer/Projects/FPC/Scripting" ';
          options += '-e"'+exec+'"';
          if file_exists(fpc) then
            begin
              command := fpc+' '+source+' '+options;
              writeln(command);
              if passthru(command) = 0 then
                // TODO: isn't there a way to just invoke the process and pipe STDOUT?
                // seems wrong we're capturing it here and using writeln...
                halt(passthru(exec));
            end
          else
            halt(1, 'fpc "'+fpc+'" can''t be found.');
        end
      else
        halt(1, 'file doesn''t exist.');
    end
  else
    halt(1, 'must specify file to run.');
end.