{$i settings}
{$include include/targetos.inc}
{$macro on}

unit PasScript;
interface
uses
  CTypes, Math, SysUtils, BaseUnix;

{$define INTERFACE}
{$include include/Values.inc}
{$include include/List.inc}
{$include include/Dictionary.inc}
{$include include/Regex.inc}
{$include include/Utils.inc}
{$include include/ListUtils.inc}
{$include include/DictUtils.inc}
{$include include/StrUtils.inc}
{$include include/FileUtils.inc}
{$include include/ProcessUtils.inc}
{$undef INTERFACE}

var
  argv: TDictionary;

var
  FPC_FILE_PATH: ansistring = '';
  FPC_FILE_NAME: ansistring = '';
  FPC_DIR: ansistring = '';

implementation

{$define IMPLEMENTATION}
{$include include/BaseUnix.inc}
{$include include/PrivateUtils.inc}
{$include include/Values.inc}
{$include include/List.inc}
{$include include/Dictionary.inc}
{$include include/Regex.inc}
{$include include/Utils.inc}
{$include include/ListUtils.inc}
{$include include/DictUtils.inc}
{$include include/StrUtils.inc}
{$include include/FileUtils.inc}
{$include include/ProcessUtils.inc}
{$undef IMPLEMENTATION}

begin
  ParseCommandLine;

  // add executable to argv
  argv['exec'] := ParamStr(0);

  // TODO: this will be wrong if we use pps because the exec is in temporary dir!
  // pps needs to pas source file as params --fpc_source
  FPC_FILE_PATH := ParamStr(0)+'.pas';
  FPC_FILE_NAME := basename(FPC_FILE_PATH);
  FPC_DIR := dirname(FPC_FILE_PATH);


  {$define CODE}
  {$include include/Values.inc}
  {$include include/List.inc}
  {$include include/Dictionary.inc}
  {$include include/Regex.inc}
  {$include include/Utils.inc}
  {$include include/ListUtils.inc}
  {$include include/DictUtils.inc}
  {$include include/StrUtils.inc}
  {$include include/FileUtils.inc}
  {$include include/ProcessUtils.inc}
  {$undef CODE}
end.