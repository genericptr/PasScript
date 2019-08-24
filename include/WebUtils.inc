{$ifdef INTERFACE}

// TODO: use sockets to read webpage
procedure TestHTTP; 

{ Sockets }
function make_connected_socket (url: pchar; port: pchar): longint; 
function send_all(socket: longint; data: pchar; count: integer): boolean;

{$endif}

{$ifdef IMPLEMENTATION}

// https://beej.us/guide/bgnet/html/single/bgnet.html
// https://stackoverflow.com/questions/10673684/send-http-request-manually-via-socket
// http://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
// https://stackoverflow.com/questions/30470505/http-request-using-sockets-in-c

{$include CNet.inc}

function make_connected_socket (url: pchar; port: pchar): longint; 
const
  BACKLOG = 10;
var
  addr: TSockAddr;
  hints: TAddrInfo;
  error: integer;
  ipstr: ansistring;
  res: PAddrInfo;
begin
  FillChar(hints, sizeof(hints), 0);
  hints.ai_family := {AF_UNSPEC}AF_INET;
  hints.ai_socktype := SOCK_STREAM;
  if url = nil then
    hints.ai_flags := AI_PASSIVE;

  error := getaddrinfo(url, port, @hints, @res);
  if error = 0 then
    begin
      result := socket(res^.ai_family, res^.ai_socktype, res^.ai_protocol);
      if result <> -1 then
        begin
          if hints.ai_flags = AI_PASSIVE then
            begin
              if bind(result, res^.ai_addr, res^.ai_addrlen) = -1 then
                begin
                  writeln('Bind ', errno);
                  result := -1;
                end;
              SetLength(ipstr, INET_ADDRSTRLEN);
              inet_ntop(res^.ai_family, @res^.ai_addr^.sin_addr, pchar(ipstr), length(ipstr));
              writeln('bound to  ', result, ' to ', ipstr, ' @ ', Ntohs(res^.ai_addr^.sin_port));

              if listen(result, BACKLOG) = -1 then
                begin
                  writeln('listen error: ', errno);
                  result := -1;
                end;
            end
          else
            begin
              if connect(result, res^.ai_addr, res^.ai_addrlen) = -1 then
                begin
                  writeln('connect error: ', errno);
                  result := -1;
                end
              else
                begin
                  SetLength(ipstr, INET_ADDRSTRLEN);
                  inet_ntop(res^.ai_family, @res^.ai_addr^.sin_addr, pchar(ipstr), length(ipstr));
                  writeln('connected ', result, ' to ', ipstr, ' @ ', Ntohs(res^.ai_addr^.sin_port));
                end;
            end;

        end
      else
        begin
          writeln('socket error: ', errno);
          result := -1;
        end;
      FreeAddrInfo(res);
    end
  else
    begin
      writeln('GetAddrInfo ', error);
      result := -1;
    end;
end;

function send_all(socket: longint; data: pchar; count: integer): boolean;
var
  byteCount: integer;
begin
  while count > 0 do
    begin
      byteCount := send(socket, data, count, 0);
      if byteCount > 0 then
        begin
          // TODO: inc buffer index
          //Inc(header, byteCount);
          Dec(count, byteCount);
          writeln('sent ', byteCount, ' left ', count);
          if count > 0 then
            fatal('bytes left to send!');
        end
      else
        exit(false)
    end;
  result := true;
end;

function MakeSocketAddress (ip: string; port: integer): TSockAddr;
var
  addr: TSockAddr;
begin
  FillChar(addr, sizeof(addr), 0);
  addr.sin_family := AF_INET;
  addr.sin_port := htons(port);
  addr.sin_addr := StrToNetAddr(ip);
  result := addr;
end;

function ParseHTTPHeader(subject: ansistring): integer;
begin
  // https://www.tutorialspoint.com/http/http_responses
  // TODO: parse up to CRLF and then find empty line
  {
  HTTP/1.1 200 OK
  Cache-Control: max-age=604800
  Content-Type: text/html; charset=UTF-8
  Date: Sun, 21 Jul 2019 20:06:18 GMT
  Etag: "1541025663+ident"
  Expires: Sun, 28 Jul 2019 20:06:18 GMT
  Last-Modified: Fri, 09 Aug 2013 23:54:35 GMT
  Server: ECS (agb/A438)
  Vary: Accept-Encoding
  X-Cache: HIT
  Content-Length: 1270


  HTTP/1.0 505 HTTP Version Not Supported
  Content-Type: text/html
  Content-Length: 379
  Connection: close
  Date: Sun, 21 Jul 2019 20:09:10 GMT
  Server: ECSF (agb/A43D)
  }
end;

(*
function recvtimeout(socket: longint; buffer: pointer; len: size_t; timeout: integer): ssize_t;
var
  fds: fd_set;
  n: integer;
  tv: timeval;
begin
  
end;

int recvtimeout(int s, char *buf, int len, int timeout)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}
*)

procedure TestHTTP; 
const
  CRLF = chr(13)+chr(10){'\r\n'};
  HOST_NAME = 'www.example.com';
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
  //host := HOST_NAME;
  socket := make_connected_socket(HOST_NAME, 'http');
  if socket <> -1 then
    begin
      //same as MSG_DONTWAIT
      //fpFcntl(socket, F_SETFL, O_NONBLOCK);

      header := 'GET / HTTP/1.1'+CRLF+'Host: '+HOST_NAME+CRLF+CRLF;

      // https://www.freepascal.org/docs-html/rtl/sockets/index-5.html
      // https://www.freepascal.org/docs-html/rtl/sockets/fpsend.html
      if not send_all(socket, header, length(header)) then
        begin
          writeln('failed');
          exit;
        end;
      //dataLength := length(header);
      //while dataLength > 0 do
      //  begin
      //    byteCount := send(socket, header, dataLength, 0);
      //    if byteCount > 0 then
      //      begin
      //        // TODO: inc buffer index
      //        //Inc(header, byteCount);
      //        Dec(dataLength, byteCount);
      //        writeln('sent ', byteCount, ' left ', dataLength);
      //        if dataLength > 0 then
      //          fatal('bytes left to send!');
      //      end
      //    else
      //      begin
      //        writeln('failed');
      //        exit;
      //      end;
      //  end;

      byteCount := high(byteCount);
      response := '';
      offset := 0;
      writeln('reading.... ');
      while byteCount > 0 do
        begin
          byteCount := recv(socket, @buffer[0], BUFFER_SIZE, 0);
          //writeln('byteCount:',byteCount);
          {
            HTTP/1.1 200 OK
            Cache-Control: max-age=604800
            Content-Type: text/html; charset=UTF-8
            Date: Sun, 21 Jul 2019 20:06:18 GMT
            Etag: "1541025663+ident"
            Expires: Sun, 28 Jul 2019 20:06:18 GMT
            Last-Modified: Fri, 09 Aug 2013 23:54:35 GMT
            Server: ECS (agb/A438)
            Vary: Accept-Encoding
            X-Cache: HIT
            Content-Length: 1270
          }
          if byteCount > 0 then
            begin
              response.AppendBytes(@buffer[0], byteCount);
              repeat
                pos := strpos(response, CRLF, offset);
                if pos > -1 then
                  begin
                    { this is the end of header with a single empty line }
                    if offset = pos then
                      writeln('got header');
                    line := StrCopy(response, offset + 1, (pos - offset) + 0);
                    writeln(offset,',',pos,':',line);
                    // TODO: this can be -1 in which case there is no value
                    //key := Substr(line, 0, strpos(line, ':'));
                    // TODO: read until CRLF not 0 
                    //value := Substr(line, length(key) + 1, 0);
                    //write(line);
                    //writeln(key,'->',value);
                    offset := pos + length(CRLF);
                  end;
              until pos = -1;
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
    writeln('failed');

  halt;
end;

{$endif}