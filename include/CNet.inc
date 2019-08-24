
const
  LIB_C = clib; // use initc's idea of what libc is called. In the future overrides might be necessary here.
                // (older BSD convention is to have a separate libresolve that contains these functions)

// Error return codes from gethostbyname() and gethostbyaddr()
// (left in h_errno).

const

  NETDB_INTERNAL  = -(1);    { see errno }
  NETDB_SUCCESS   = 0;       { no problem  }
  HOST_NOT_FOUND  = 1;       { Authoritative Answer Host not found  }
  TRY_AGAIN       = 2;       { Non-Authoritative Host not found, or SERVERFAIL  }
  NO_RECOVERY     = 3;       { Non recoverable errors, FORMERR, REFUSED, NOTIMP  }
  NO_DATA         = 4;       { Valid name, no data record of requested type  }
  NO_ADDRESS      = NO_DATA; { no address, look for MX record  }

  {$IF DEFINED(FREEBSD)}

    AI_PASSIVE     = $00000001;
    AI_CANONNAME   = $00000002;
    AI_NUMERICHOST = $00000004;
    AI_V4MAPPED    = $00000008;
    AI_ALL         = $00000010;
    AI_ADDRCONFIG  = $00000020;
    AI_DEFAULT     = (AI_V4MAPPED OR AI_ADDRCONFIG);

    EAI_ADDRFAMILY = 1; (* address family for hostname not supported *)
    EAI_AGAIN      = 2; (* temporary failure in name resolution *)
    EAI_BADFLAGS   = 3; (* invalid value for ai_flags *)
    EAI_FAIL       = 4; (* non-recoverable failure in name resolution *)
    EAI_FAMILY     = 5; (* ai_family not supported *)
    EAI_MEMORY     = 6; (* memory allocation failure *)
    EAI_NODATA     = 7; (* no address associated with hostname *)
    EAI_NONAME     = 8; (* hostname nor servname provided, or not known *)
    EAI_SERVICE    = 9; (* servname not supported for ai_socktype *)
    EAI_SOCKTYPE   = 10; (* ai_socktype not supported *)
    EAI_SYSTEM     = 11; (* system error returned in errno *)
    EAI_BADHINTS   = 12;
    EAI_PROTOCOL   = 13;
    EAI_MAX        = 14;

  {$ELSE}

    (* Possible values for `ai_flags' field in `addrinfo' structure.  *)

    AI_PASSIVE                  = $0001; (* Socket address is intended for `bind'.  *)
    AI_CANONNAME                = $0002; (* Request for canonical name.  *)
    AI_NUMERICHOST              = $0004; (* Don't use name resolution.  *)
    AI_V4MAPPED                 = $0008; (* IPv4 mapped addresses are acceptable.  *)
    AI_ALL                      = $0010; (* Return IPv4 mapped and IPv6 addresses.  *)
    AI_ADDRCONFIG               = $0020; (* Use configuration of this host to choose returned address type..  *)
    AI_IDN                      = $0040; (* IDN encode input (assuming it is encoded in the current locale's character set) before looking it up.  *)
    AI_CANONIDN                 = $0080; (* Translate canonical name from IDN format.  *)
    AI_IDN_ALLOW_UNASSIGNED     = $0100; (* Don't reject unassigned Unicode code points.  *)
    AI_IDN_USE_STD3_ASCII_RULES = $0200; (* Validate strings according to STD3 rules.  *)
    AI_NUMERICSERV              = $0400; (* Don't use name resolution.  *)

    (* Error values for `getaddrinfo' function.  *)

    EAI_BADFLAGS    = -1;   (* Invalid value for `ai_flags' field.  *)
    EAI_NONAME      = -2;   (* NAME or SERVICE is unknown.  *)
    EAI_AGAIN       = -3;   (* Temporary failure in name resolution.  *)
    EAI_FAIL        = -4;   (* Non-recoverable failure in name res.  *)
    EAI_NODATA      = -5;   (* No address associated with NAME.  *)
    EAI_FAMILY      = -6;   (* `ai_family' not supported.  *)
    EAI_SOCKTYPE    = -7;   (* `ai_socktype' not supported.  *)
    EAI_SERVICE     = -8;   (* SERVICE not supported for `ai_socktype'.  *)
    EAI_ADDRFAMILY  = -9;   (* Address family for NAME not supported.  *)
    EAI_MEMORY      = -10;  (* Memory allocation failure.  *)
    EAI_SYSTEM      = -11;  (* System error returned in `errno'.  *)
    EAI_OVERFLOW    = -12;  (* Argument buffer overflow.  *)
    EAI_INPROGRESS  = -100; (* Processing request in progress.  *)
    EAI_CANCELED    = -101; (* Request canceled.  *)
    EAI_NOTCANCELED = -102; (* Request not canceled.  *)
    EAI_ALLDONE     = -103; (* All requests done.  *)
    EAI_INTR        = -104; (* Interrupted by a signal.  *)
    EAI_IDN_ENCODE  = -105; (* IDN encoding failed.  *)

  {$ENDIF}

  (* Constants for getnameinfo() *)

  NI_MAXHOST = 1025;
  NI_MAXSERV = 32;

  (* Flag values for getnameinfo() *)

  {$IF DEFINED(FREEBSD)}

    NI_NOFQDN       = $00000001;
    NI_NUMERICHOST  = $00000002;
    NI_NAMEREQD     = $00000004;
    NI_NUMERICSERV  = $00000008;
    NI_DGRAM        = $00000010;
    NI_NUMERICSCOPE = $00000020;

  {$ELSE}

    NI_NUMERICHOST = 1;
    NI_NUMERICSERV = 2;
    NI_NOFQDN      = 4;
    NI_NAMEREQD    = 8;
    NI_DGRAM       = 16;

  {$ENDIF}

{
 * Scope delimit character
  }

const
   SCOPE_DELIMITER = '%';
   
//#define	h_addr	h_addr_list[0]	/* address, for backward compatibility */

type
{*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 *}
  hostent = record
    h_name: PChar;      {/* official name of host *}
    h_aliases: PPChar;  {* alias list *}
    h_addrtype: cint;   {* host address type *}
    h_length: cint;     {* length of address *}
    h_addr_list: PPChar;{* list of addresses from name server *}
  end;

  THostEnt = hostent;
  PHostEnt = ^THostEnt;
  PPHostEnt = ^PHostEnt;

  netent = record
    n_name: PChar;      {* official name of net *}
    n_aliases: PPChar;  {* alias list *}
    n_addrtype: cint;   {* net address type *}
    n_net: cuInt32;     {* network # *}
  end;
  TNetEnt = netent;
  PNetEnt = ^TNetEnt;
  PPNetEnt = ^PNetEnt;

  servent = record
    s_name: PChar;    {* official service name *}
    s_aliases: PPChar;{* alias list *}
    s_port: cint;     {* port # *}
    s_proto: PChar;   {* protocol to use *}
  end;
  TServEnt = servent;
  PServEnt = ^TServEnt;
  PPServEnt = ^PServEnt;

  protoent = record
    p_name: PChar; {* official protocol name *}
    p_aliases: PPChar;  {* alias list *}
    p_proto: cint;      {* protocol # *}
  end;
  TProtoEnt = protoent;
  PProtoEnt = ^TProtoEnt;
  PPProtoEnt = ^PProtoEnt;

{$if defined(LINUX) or defined(OPENBSD)}
{$define FIRST_ADDR_THEN_CANONNAME}
{$endif}
{$if defined(FREEBSD) or defined(NETBSD) or defined(DRAGONFLY) or defined(SOLARIS) or defined(DARWIN)}
{$define FIRST_CANONNAME_THEN_ADDR}
{$endif}
{$if not defined(FIRST_CANONNAME_THEN_ADDR) and not defined(FIRST_ADDR_THEN_CANONNAME)}
{$error fatal 'Please consult the netdb.h file for your system to determine the order of ai_addr and ai_canonname'}
{$endif} 

  PAddrInfo = ^addrinfo;
  addrinfo = record
    ai_flags: cint;     {* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST *}
    ai_family: cint;    {* PF_xxx *}
    ai_socktype: cint;  {* SOCK_xxx *}
    ai_protocol: cint;  {* 0 or IPPROTO_xxx for IPv4 and IPv6 *}
    ai_addrlen: TSocklen;  {* length of ai_addr *}
{$ifdef FIRST_CANONNAME_THEN_ADDR}
    ai_canonname: PChar;   {* canonical name for hostname *}
    ai_addr: psockaddr;	   {* binary address *}
{$endif}
{$ifdef FIRST_ADDR_THEN_CANONNAME}
    ai_addr: psockaddr;	   {* binary address *}
    ai_canonname: PChar;   {* canonical name for hostname *}
{$endif}
    ai_next: PAddrInfo;	   {* next structure in linked list *}
  end;
  TAddrInfo = addrinfo;
  PPAddrInfo = ^PAddrInfo;
   
{ netdb }
procedure endhostent; cdecl; external LIB_C name 'endhostent';
procedure endnetent; cdecl; external LIB_C name 'endnetent';
procedure endnetgrent; cdecl; external LIB_C name 'endnetgrent';
procedure endprotoent; cdecl; external LIB_C name 'endprotoent';
procedure endservent; cdecl; external LIB_C name 'endservent';
procedure freehostent(ptr: PHostEnt); cdecl; external LIB_C name 'freehostent';
function  gethostbyaddr(addr: PChar; len, typ: cint): PHostEnt; cdecl; external LIB_C name 'gethostbyaddr';
function  gethostbyname(name: PChar): PHostEnt; cdecl; external LIB_C name 'gethostbyname';
function  gethostbyname2(name: PChar; af: cint): PHostEnt; cdecl; external LIB_C name 'gethostbyname2';
function  gethostent: PHostEnt; cdecl; external LIB_C name 'gethostent';
function getipnodebyaddr(src: Pointer; len: size_t; af: cint; error_num: Pcint): PHostEnt; cdecl; external LIB_C name 'getipnodebyaddr';
function getipnodebyname(name: PChar; af, flags: cint; error_num: Pcint): PHostEnt; cdecl; external LIB_C name 'getipnodebyname';
function getnetbyaddr(net: cint32; typ: cint): PNetEnt; cdecl; external LIB_C name 'getnetbyaddr';
function getnetbyname(name: PChar): PNetEnt; cdecl; external LIB_C name 'getnetbyname';
function getnetent: PNetEnt; cdecl; external LIB_C name 'getnetent';
function getnetgrent(host, user, domain: PPChar): cint; cdecl; external LIB_C name 'getnetgrent';
function getprotobyname(name: PChar): PProtoEnt; cdecl; external LIB_C name 'getprotobyname';
function getprotobynumber(proto: cint): PProtoEnt; cdecl; external LIB_C name 'getprotobynumber';
function getprotoent: PProtoEnt; cdecl; external LIB_C name 'getprotoent';
function getservbyname(name, proto: PChar): PServEnt; cdecl; external LIB_C name 'getservbyname';
function getservbyport(port: cint; proto: PChar): PServEnt; cdecl; external LIB_C name 'getservbyport';
function  getservent: PServEnt; cdecl; external LIB_C name 'getservent';
procedure herror(erstring: PChar); cdecl; external LIB_C name 'herror';
function  hstrerror(err: cint): PChar; cdecl; external LIB_C name 'hstrerror';
function  innetgr(netgroup, host, user, domain: PChar): cint; cdecl; external LIB_C name 'innetgr';
procedure sethostent(i: cint); cdecl; external LIB_C name 'sethostent';
procedure setnetent(stayopen: cint); cdecl; external LIB_C name 'setnetent';
procedure setprotoent(stayopen: cint); cdecl; external LIB_C name 'setprotoent';
function  getaddrinfo(name, service: PChar; hints: PAddrInfo;
                      res: PPAddrInfo): cint; cdecl; external LIB_C name 'getaddrinfo';
function  getnameinfo(sa: PSockAddr; salen: TSockLen; host: PChar; hostlen: TSize;
                      serv: PChar; servlen: TSize; flags: cint): cint; cdecl; external LIB_C name 'getnameinfo';
procedure freeaddrinfo(ai: PAddrInfo); cdecl; external LIB_C name 'freeaddrinfo';
function  gai_strerror(ecode: cint): PChar; cdecl; external LIB_C name 'gai_strerror';
procedure setnetgrent(netgroup: PChar); cdecl; external LIB_C name 'setnetgrent';
procedure setservent(stayopen: cint); cdecl; external LIB_C name 'setservent';

{ in.h }
const
  INET_ADDRSTRLEN = 16;

{ inet }

type
  in_addr_t = cuint32;//uint32_t;
  Pin_addr_t = ^in_addr_t;

function inet_addr(const Param1: PChar): cuint32; cdecl; external LIB_C name 'inet_addr';
function inet_aton(const Param1: PChar; Param2: Pin_addr): cint; cdecl; external LIB_C name 'inet_aton';
function inet_ntoa(Param1: Tin_addr): PChar; cdecl; external LIB_C name 'inet_ntoa';
function inet_pton(Param1: cint; const Param2: PChar; Param3: Pointer): cint; cdecl; external LIB_C name 'inet_pton';
function inet_ntop(Param1: cint; const Param2: Pointer; Param3: PChar; Param4: csize_t): PChar; cdecl; external LIB_C name 'inet_ntop';
function inet_nsap_addr(const Param1: PChar; Param2: PByte; Param3: cint): cuint; cdecl; external LIB_C name 'inet_nsap_addr';
function inet_nsap_ntoa(Param1: cint; const Param2: PChar; Param3: PChar): PChar; cdecl; external LIB_C name 'inet_nsap_ntoa';

//uint32_t htonl(uint32_t hostlong);
//uint16_t htons(uint16_t hostshort);
//uint32_t ntohl(uint32_t netlong);
//uint16_t ntohs(uint16_t netshort);

{ sockets }

Type
  Pmsghdr  = pointer;
  Psocklen_t  = ^socklen_t;
  socket_pair = array[0..1] of longint;

function accept(socket:longint; address:Psockaddr; address_len:Psocklen_t):longint;cdecl;external;
function bind(socket:longint; address:Psockaddr; address_len:socklen_t):longint;cdecl;external;
function connect(socket:longint; address:Psockaddr; address_len:socklen_t):longint;cdecl;external;
function getpeername(socket:longint; address:Psockaddr; address_len:Psocklen_t):longint;cdecl;external;
function getsockname(socket:longint; address:Psockaddr; address_len:Psocklen_t):longint;cdecl;external;
function getsockopt(socket:longint; level:longint; option_name:longint; option_value:pointer; option_len:Psocklen_t):longint;cdecl;external;
function listen(socket:longint; backlog:longint):longint;cdecl;external;
function recv(socket:longint; buffer:pointer; length:size_t; flags:longint):ssize_t;cdecl;external;
function recvfrom(socket:longint; buffer:pointer; length:size_t; flags:longint; address:Psockaddr; address_len:Psocklen_t):ssize_t;cdecl;external;
function recvmsg(socket:longint; msg:Pmsghdr; flags:longint):ssize_t;cdecl;external;
function send(socket:longint; msg:pointer; len:size_t; flags:longint):ssize_t;cdecl;external;
function sendmsg(socket:longint; msg:Pmsghdr; flags:longint):ssize_t;cdecl;external;
function sendto(socket:longint; msg:pointer; length:size_t; flags:longint; dest_addr:Psockaddr; dest_len:socklen_t):ssize_t;cdecl;external;
function setsockopt(socket:longint; level:longint; option_name:longint; option_value:pointer; option_len:socklen_t):longint;cdecl;external;
function shutdown(socket:longint; how:longint):longint;cdecl;external;
function socket(domain:longint; _type:longint; protocol:longint):longint;cdecl;external;
function socketpair(domain:longint; _type:longint; protocol:longint; socket_vector: socket_pair):longint;cdecl;external;
