{$i settings}
{$include include/targetos.inc}
{$macro on}

{$ifdef DARWIN}
{$linkframework Cocoa}
{$modeswitch objectivec1}
{$endif}

unit PasScript;
interface
uses
  {$ifdef DARWIN}
  MacOSAll, CocoaAll,
  {$endif}
  CTypes, Math, SysUtils, BaseUnix, Sockets, Scanner, Variants;

{$define inline_operator}

{$define INTERFACE}
{$include include/Globals.inc}
{$include include/Math.inc}
{$include include/Values.inc}
{$include include/Conversions.inc}
{$include include/List.inc}
{$include include/Dictionary.inc}
{$include include/Regex.inc}
{$include include/ListUtils.inc}
{$include include/DictUtils.inc}
{$include include/StrUtils.inc}
{$include include/FileUtils.inc}
{$include include/ProcessUtils.inc}
{$include include/WebUtils.inc}
{$include include/JSONUtils.inc}
{$include include/XMLUtils.inc}
{$include include/Utils.inc}
{$undef INTERFACE}

var
  argv: TDictionary;

var
  FPC_FILE_PATH: ansistring = '';
  FPC_FILE_NAME: ansistring = '';
  FPC_DIR: ansistring = '';

const
  FLAG_PPS_SRC = 'pps_src';
  FLAG_PPS_EXEC = 'pps_exec';

procedure InitThread;

implementation

{$define IMPLEMENTATION}
{$include include/Globals.inc}
{$include include/Math.inc}
{$include include/BaseUnix.inc}
{$include include/PrivateUtils.inc}
{$include include/Values.inc}
{$include include/Conversions.inc}
{$include include/List.inc}
{$include include/Dictionary.inc}
{$include include/Regex.inc}
{$include include/ListUtils.inc}
{$include include/DictUtils.inc}
{$include include/StrUtils.inc}
{$include include/FileUtils.inc}
{$include include/ProcessUtils.inc}
{$include include/WebUtils.inc}
{$include include/JSONUtils.inc}
{$include include/XMLUtils.inc}
{$include include/Utils.inc}
{$undef IMPLEMENTATION}

procedure InitThread;
begin
  SharedRegex := nil;
end;

begin
  {$define CODE}
  {$include include/Math.inc}
  {$include include/Values.inc}
  {$include include/Conversions.inc}
  {$include include/List.inc}
  {$include include/Dictionary.inc}
  {$include include/Regex.inc}
  {$include include/ListUtils.inc}
  {$include include/DictUtils.inc}
  {$include include/StrUtils.inc}
  {$include include/FileUtils.inc}
  {$include include/ProcessUtils.inc}
  {$include include/WebUtils.inc}
  {$include include/JSONUtils.inc}
  {$include include/XMLUtils.inc}
  {$include include/Utils.inc}
  {$undef CODE}

  ParseCommandLine;

  // add executable to argv
  argv['exec'] := ParamStr(0);

  // if the file was run from pps then we can get
  // additional command line arguments about the script
  if argv[FLAG_PPS_SRC] then
    begin
      FPC_FILE_PATH := argv[FLAG_PPS_SRC];
      FPC_FILE_NAME := basename(FPC_FILE_PATH);
      FPC_DIR := dirname(FPC_FILE_PATH);
    end;

  // init main thread
  InitThread;
end.