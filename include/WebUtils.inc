{$ifdef INTERFACE}

// TODO: use sockets to read webpage
procedure TestHTTP; 

{$endif}

{$ifdef IMPLEMENTATION}

//http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
//https://stackoverflow.com/questions/10673684/send-http-request-manually-via-socket
//http://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/

///Developer/ObjectivePascal/fpc/packages/fcl-net/src/cnetdb.pp 

//http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html
type
  PAddrInfo = ^Addrinfo;
  Addrinfo = record
    ai_flags: cInt;     {* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST *}
    ai_family: cInt;    {* PF_xxx *}
    ai_socktype: cInt;  {* SOCK_xxx *}
    ai_protocol: cInt;  {* 0 or IPPROTO_xxx for IPv4 and IPv6 *}
    ai_addrlen: TSocklen;  {* length of ai_addr *}
    ai_canonname: PChar;   {* canonical name for hostname *}
    ai_addr: psockaddr;    {* binary address *}
    ai_next: PAddrInfo;    {* next structure in linked list *}
  end;
  TAddrInfo = addrinfo;
  PPAddrInfo = ^PAddrInfo;

function getaddrinfo(name, service: PChar; hints: PAddrInfo; res: PPAddrInfo): cInt; cdecl; external 'getaddrinfo';
procedure freeaddrinfo(ai: PAddrInfo); cdecl; external name 'freeaddrinfo';

function HTTPSocket (url, port: pchar; out addr: TSockAddr): longint; 
var
  hints: TAddrInfo;
  error: integer;
  res: PAddrInfo;
begin
  FillChar(hints, Sizeof(hints), 0);
  hints.ai_family := PF_UNSPEC; // use AF_INET6 to force IPv6
  hints.ai_socktype := SOCK_STREAM;

  error := GetAddrInfo(url, port, @hints, @res);
  if error = 0 then
    begin
      addr := res^.ai_addr^;
      result := fpSocket(res^.ai_family, res^.ai_socktype, res^.ai_protocol);
      FreeAddrInfo(res);
    end
  else
    begin
      writeln('GetAddrInfo ', error);
      result := -1;
    end;
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

procedure TestHTTP; 
var
  address: TSockAddr;
  hints: TAddrInfo;
  error: integer;
  byteCount: integer;
  socket: longint;
  buffer: pointer;
  str: string;
  request: ansistring;
  host: pchar;
begin
  host := 'freepascal.org';
  socket := HTTPSocket(host, 'http', address);
  //address := MakeSocketAddress('172.217.19.206', 80);
  //socket := fpSocket(AF_INET, SOCK_STREAM, 0);
  if socket <> -1 then
    begin
      writeln('connecting ', socket, ' to ', NetAddrToStr(address.sin_addr), ' @ ', Ntohs(address.sin_port));
      if fpConnect(socket, @address, sizeof(address)) = -1 then
        writeln('failed to connect ', SocketError)
      else
        begin
          // same as MSG_DONTWAIT
          //fpFcntl(socket, F_SETFL, O_NONBLOCK);
          writeln('connected');

          request := 'GET / HTTP/1.1\r\n';
          request += 'Host: '+host+'\r\n\r\n';

          byteCount := fpSend(socket, pchar(request), length(request), 0);

          writeln('sent ', byteCount);
          if byteCount < 0 then
            exit;

          buffer := GetMem(1024);
          byteCount := -1;
          while byteCount <> 0 do
            begin
              byteCount := fpRecv(socket, buffer, MemSize(buffer), {MSG_DONTWAIT}0);
              if byteCount > 0 then
                begin
                  writeln('byteCount:', byteCount, ' err:', SocketError);
                  writeln(pchar(buffer));
                end;
              if (byteCount = -1) and (SocketError <> ESysEAGAIN) then
                begin
                  writeln('error ', SocketError);
                  break;
                end;
            end;
        end;
      
    end
  else
    writeln('failed');

  halt;
end;

{$endif}