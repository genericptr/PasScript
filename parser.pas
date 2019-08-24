{$i settings}

program parser;
uses
  Sockets, BaseUnix,
  SQLite3, PasScript;

// TODO: make another unit for RTL support? PasScriptLib?
{$include include/BaseUnix.inc}
{$include include/CNet.inc}

type
  TPatterns = record
    routine: TPattern;
  end;

type
  TStatements = record
    findSymbol: psqlite3_stmt;
  end;

type
  TSQLite3 = class
    public
      constructor Create(path: ansistring);
      function Exec(query: ansistring): boolean; overload;
      function Exec(stmt: psqlite3_stmt): boolean; overload;
      function Prepare_v2(query: ansistring): psqlite3_stmt;
    private
      db: psqlite3;
      destructor Destroy; override;
  end;

constructor TSQLite3.Create(path: ansistring);
begin
  if sqlite3_open(@path[1], @db) <> SQLITE_OK then
    fatal('failed to open database');
end;

function TSQLite3.Exec(query: ansistring): boolean;
var
  err: pansichar;
begin
  query += #0;
  writeln(query);
  if sqlite3_exec(db, @query[1], nil, nil, @err) <> SQLITE_OK then
    begin
      writeln('sqlite3_exec: ',err);
      result := false;
    end
  else
    result := true;
end;

function TSQLite3.Exec(stmt: psqlite3_stmt): boolean;
var
  err: pansichar;
begin
  while sqlite3_step(stmt) = SQLITE_ROW do
    begin
      //name := sqlite3_column_text(statement, 0);
      //source := sqlite3_column_text(statement, 1);
      //line := sqlite3_column_int(statement, 2);
      writeln('got it');
    end;
end;

function TSQLite3.Prepare_v2(query: ansistring): psqlite3_stmt;
begin
  if not sqlite3_prepare_v2(db, @query[1], -1, @result, nil) = SQLITE_OK then
    result := nil;
end;

destructor TSQLite3.Destroy;
begin
  sqlite3_close(db);
end;

function sql_quote(subject: ansistring): ansistring; inline;
begin
  result := wrap(subject, '''');
end;

function StrAppendBytes(var subject: LongString; buffer: pointer; len: integer): LongString;
begin
  subject.Expand(len);
  Move(buffer^, subject[length(subject) - len + 1], len);
  result := subject;
end;

procedure TestSockets; 
const
  BUFFER_SIZE = 64;
var
  error: integer;
  byteCount: integer;
  socket: longint;
  dataLength: integer;
  buffer: array[0..BUFFER_SIZE - 1] of char;
  str: string;
  header: pchar;
  output: ansistring;
  response: ansistring;
  line: ansistring;
  offset, pos: integer;
  key, value: string;
  parts: TList;
begin
  socket := make_connected_socket(nil, '3490');
  if socket <> -1 then
    begin
      //send_all(socket, 'a', 1);
      // listen for the client

      //function listen(socket:longint; backlog:longint):longint;cdecl;external;

      byteCount := high(byteCount);
      response := '';
      offset := 0;
      writeln('reading.... ');
      while byteCount > 0 do
        begin
          byteCount := recv(socket, @buffer[0], BUFFER_SIZE, 0);
          //writeln('byteCount:',byteCount);
          if byteCount > 0 then
            begin
              //response.AppendBytes(@buffer[0], byteCount);
              StrAppendBytes(response, @buffer[0], byteCount);
            end
          else if (byteCount = -1) and (SocketError <> ESysEAGAIN) then
            begin
              writeln('error ', SocketError);
              break;
            end;
        end;
      writeln('done');
      writeln(output);
      shutdown(socket, 2);
    end
  else
    writeln('test sockets failed');
end;


var
  fd: TFileDesc;
  line: ansistring;
  lines: TList;
  patterns: TPatterns;
  statements: TStatements;
  matches: TList;
  startTime: double;
  path: ansistring;
var
  db: TSQLite3;
  query, where: ansistring;
  statement: psqlite3_stmt;
  err: pansichar;
begin
  // https://sqlite.org/cintro.html
  // https://sqlite.org/lang.html
  TestSockets;
  halt;

  //unlink('/Developer/Projects/FPC/PasScript/symbols.db');
  db := TSQLite3.Create('/Developer/Projects/FPC/PasScript/symbols.db');

  db.Exec(`CREATE TABLE IF NOT EXISTS routines (
            name    varchar(255),
            args    varchar(255),
            line    integer,
            UNIQUE(name)
            );`);


  db.Exec('DELETE FROM routines');


  //if sqlite3_prepare_v2(db, @query[1], -1, @statement, nil) = SQLITE_OK then
  //  begin
  //    writeln('got it!');
  //    sqlite3_finalize(statement);
  //  end;
  //while sqlite3_step(statement) = SQLITE_ROW do
  //  begin
  //    name := sqlite3_column_text(statement, 0);
  //    source := sqlite3_column_text(statement, 1);
  //    line := sqlite3_column_int(statement, 2);
  //  end;

  //lines := file_get_lines('/Developer/Projects/FPC/PasScript/parser_test.pas', true);
  //print_r(lines);

  patterns.routine := preg_pattern('(generic)*\s*(function|procedure)+\s+(\w+)\s*(\((.*)\))*\s*;');
  startTime := TimeSinceNow;

  path := '/Developer/ObjectivePascal/fpc-git/compiler/pexpr.pas';
  //path := '/Developer/Projects/FPC/PasScript/parser_test.pas';

  fd := fopen(path, [mode_read]);
  if fd then
    begin
      while readline(fd, line) do
        begin
          //writeln(line);
          if preg_match(patterns.routine, line, matches) then
            begin
              //matches.show;
              // TODO: we need to always get the same count of captures!
              if matches.count < 6 then
                continue;

              //where := 'WHERE NOT EXISTS (SELECT * FROM routines WHERE name='+sql_quote(matches[3].str)+')';
              query := 'INSERT INTO routines VALUES ('+sql_quote(matches[3].str)+', '+sql_quote(matches[5].str)+', '+ToStr(0)+')';
              //query := `insert into routines values('aaa','bbb',0)`;
              //SELECT 'aaa' Where not exists(select * from routines where name='aaa')
              db.exec(query);
            end;
        end;
      close(fd);
    end;

  writeln('finished in: ', tostr((TimeSinceNow - startTime)/1000));

  db.Free;
end.