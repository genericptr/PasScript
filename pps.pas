{$mode objfpc}

program pps;
uses
  PasScript, SysUtils;

const
  fpc_version = '3.3.1';
  fpc_arch = 'ppcx64';
  fpc_path = '/usr/local/lib/fpc';

const
  flag_debug = 'g';
  flag_build_only = 'b';
  flag_run_in_terminal = 't';
  flag_arch = 'a';
  flag_compiler = 'v';

function run_in_terminal(script: ansistring): integer;
var
  command: ansistring;
begin
  command := '/usr/bin/osascript <<EOF'+#10;
  command += 'tell application "Terminal"'+#10;
  command += '  if (count of windows) is 0 then'+#10;
  command += '    do script "'+script+'"'+#10;
  command += '  else'+#10;
  command += '    do script "'+script+'" in window 1'+#10;
  command += '  end if'+#10;
  command += '  activate'+#10;
  command += 'end tell'+#10;
  command += 'EOF'+#0;
  result := passthru(command);
end;

function which(name: string): ansistring;
var
  paths: TList;
  path: ansistring;
  root: ansistring;
  newPath: ansistring;
begin
  // TODO: how can we get $PATH to read from?
  ///Volumes/Ryan/fsl/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:/opt/X11/bin:/Library/Frameworks/Mono.framework/Versions/Current/Commands:/Applications/Wireshark.app/Contents/MacOS
  paths := ['/usr/local/bin', '/usr/bin'];
  root := '';
  for path in paths do
    begin
      newPath := path+'/'+name;
      if file_exists(newPath) then
        begin
          root := newPath;
          break;
        end;
    end;
  if is_link(root) then
    root := readlink(root);
  result := root;
end;

function CompareVersionNumbers(left, right: TValue; context: pointer): TComparatorResult;
var
  i: integer;
  v1, v2: TList;
begin
  preg_match('(\d+)\.(\d+)\.(\d+)', left, v1);
  preg_match('(\d+)\.(\d+)\.(\d+)', right, v2);
  for i := 1 to 3 do
    begin
      if integer(v1[i]) > integer(v2[i]) then
        begin
          result := kOrderedDescending;
          break;
        end;
      if integer(v1[i]) < integer(v2[i]) then
        begin
          result := kOrderedAscending;
          break;
        end;
      result := kOrderedSame;
    end;
end;

function GetLatestFPCVersion: string;
var
  files, versions: TList;
  name: string;
begin
  files := scandir('/usr/local/lib/fpc');
  for name in files do
    if preg_match('\d+\.\d+\.\d+', name) then
      versions += name;
  versions := versions.Sort(@CompareVersionNumbers);
  result := versions[0];
end;

var
  i: integer;
  source: string;
  version: string;
  arch: string;
  options,
  exec,
  fpc,
  compiler,
  root,
  path,
  pps_options,
  line,
  command: ansistring;
  exitCode: pinteger;
  code: integer;
begin
  if ParamCount >= 1 then
    begin
      source := ParamStr(1);
      if file_exists(source) then
        begin

          // get compiler arch
          arch := argv[flag_arch];
          if arch = '' then
            arch := fpc_arch;

          // get compiler path
          compiler := argv[flag_compiler];
          if not file_exists(compiler) then
            begin
              version := argv[flag_compiler];
              if version = 'latest' then
                version := GetLatestFPCVersion;
              fpc := fpc_path+'/'+version+'/'+arch
            end
          else
            fpc := compiler;
          
          //writeln('arch:',arch);
          //writeln('version:',version);
          //writeln('fpc:',fpc);
          //writeln('source:',source);
          //argv.show;
          //halt;

          exec := GetTempDir+basename(source, false);

          // ParamStr(0) will return "pps" so we need to search $PATH
          // and find the actual binary
          root := dirname(which('pps'));
          root := dirname(root);

          // c-style output formatting
          options := '';
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

          if file_exists(fpc) then
            begin
              command := fpc+' '+source+' '+options+' 2>&1';
              writeln('[', command, ']');

              for line in popen(command, exitCode) do
                if preg_match('^/(.*).(pas|pp)+:(\d+): error:', line) then
                  writeln(line);

              if exitCode^ = 0 then
                if argv[flag_build_only] then
                  begin
                    // print the executable so it can be parsed out later
                    pps_options := '--'+FLAG_PPS_SRC+'='+wrap(source, '''');
                    writeln(FLAG_PPS_EXEC, ':', exec, ' ', pps_options);
                  end
                else
                  begin
                    if not file_exists(exec) then
                      halt(1, 'executable doesn''t exist: '+exec);
                    pps_options := '--'+FLAG_PPS_SRC+'='+wrap(source, '''');
                    command := exec+' '+pps_options;
                    writeln('[', command, ']');
                    if argv[flag_run_in_terminal] then
                      code := run_in_terminal(command)
                    else
                      code := passthru(command);
                    halt(code);
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