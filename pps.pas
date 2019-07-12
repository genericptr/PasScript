{$mode objfpc}

program pps;
uses
  PasScript, SysUtils;

// TODO: make these options:
const
  fpc_version = '3.3.1';  // -v 3.3.1
  fpc_arch = 'ppcx64';    // -a ppcx64

const
  flag_debug = 'g';
  flag_build_only = 'b';

var
  i: integer;
  source: string;
  version: string;
  arch: string;
  options: ansistring = '';
  exec: ansistring;
  fpc: string;
  root: ansistring;
  pps_options: ansistring;
  line: ansistring;
  command: ansistring;
  exitCode: pinteger;
begin
  if ParamCount >= 1 then
    begin
      source := ParamStr(1);
      if exists(source) then
        begin
          fpc := '/usr/local/lib/fpc/'+fpc_version+'/'+fpc_arch;
          exec := GetTempDir+basename(source, false);

          // find the root directory of pps
          root := dirname(ParamStr(0));

          // c-style output formatting
          options += '-vbr ';

          // debugging
          if argv[flag_debug] then
            begin
              options += '-gw ';
              options += '-godwarfcpp ';
            end;

          options += '-Fl/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib ';
          options += '-FU"'+GetTempDir+'" ';
          options += '-Fu"'+root+'" ';
          options += '-Fi"'+root+'" ';
          // TODO: we need to compile and keep passcript.o in the same location
          // but we also want to output local script to temp dir
          //options += '-Fu"'+root+'/output" ';

          // exeutable output path
          options += '-o"'+exec+'"';

          if exists(fpc) then
            begin
              command := fpc+' '+source+' '+options+' 2>&1';
              //writeln(command);

              for line in popen(command, exitCode) do
                if preg_match('^/(.*).(pas|pp)+:(\d+): error:', line) then
                  writeln(line);

              // TODO: how can we get back the error code from popen? a pointer maybe?
              if exitCode^ = 0 then
                if argv[flag_build_only] then
                  begin
                    // print the executable so it can be parsed out later
                    pps_options := '-'+FLAG_PPS_SRC+'="'+source+'"';
                    writeln('$exec:', exec, ' ', pps_options);
                  end
                else
                  begin
                    if not exists(exec) then
                      halt(1, 'executable doesn''t exist: '+exec);
                    writeln('running ', exec, '...');
                    halt(passthru(exec));
                  end;
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