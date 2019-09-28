{$ifdef INTERFACE}

type
  TXMLNodeName = string;
  TXMLNodeContents = ansistring;
  TXMLNode = class;
  TXMLNodeEnumerator = record
    private
      node: TXMLNode;
      currentValue: TXMLNode;
      currentIndex: TListIndex;
    public
      constructor Create(source: TXMLNode);
      function MoveNext: boolean;
      property Current: TXMLNode read currentValue;
  end;
  TXMLNode = class (TValue)
    public
      constructor Create(name: TXMLNodeName; text: TXMLNodeContents = ''); overload;
      function Str: ansistring; override;
      function FullName: TXMLNodeName; inline;
      function Sibling: TXMLNode; inline;
      function Add(name: string; text: TXMLNodeContents = ''): TXMLNode;
      function Add(name: string; text: TXMLNodeContents; attributes: array of string): TXMLNode;
      procedure AddComment(contents: TXMLNodeContents);
      function FindAll(name: string): TList;
      function Copy: TValue; override;
    private
      function FindChildAtIndex(name: string; index: integer): TXMLNode;
      function FindChild (name: string): TXMLNode;
      function GetChildAtIndex (index: integer): TXMLNode; inline;
      function GetValue(key: string): TXMLNode; overload; inline;
      function GetValue(index: integer): TXMLNode; overload; inline;
      function GetValue(name: string; index: integer): TXMLNode; overload; inline;
    public
      property Values[key: string]: TXMLNode read GetValue; default;
      function GetEnumerator: TXMLNodeEnumerator;
    public
      text: TXMLNodeContents;
      name: TXMLNodeName;
      prefix: TXMLNodeName;
      attributes: TDict;
      parent: TXMLNode;
      index: integer;
    private
      children: TList;
      comment: boolean;
      function MakeAttributeString: ansistring;
      function MakeFormatedString (indent: string): ansistring; virtual;
      procedure AddChild(node: TXMLNode);
  end;
  TXMLCommentNode = class (TXMLNode)
    public
      constructor Create(contents: TXMLNodeContents);
    private
      function MakeFormatedString (indent: string): ansistring; override;
  end;

type
  TXMLDocument = record
    private
      root: TXMLNode;
      function GetValue(key: variant): TXMLNode; overload; inline;
      function GetValue(name: string; index: TListIndex): TXMLNode; overload; inline;
    public
      property Values[key: variant]: TXMLNode read GetValue; default;
      function GetEnumerator: TXMLNodeEnumerator;

      function Add(name: string; contents: TXMLNodeContents = ''): TXMLNode;

      procedure Show;
      //function Str(formatted: boolean = true): ansistring;
      //function MakeFormattedString(indent: string = ''; formatting: TStringFormatting = []): ansistring;
    private
      class operator Initialize(var doc: TXMLDocument);
      class operator Finalize(var doc: TXMLDocument);
      class operator AddRef(var doc: TXMLDocument);
      class operator Copy(constref src: TXMLDocument; var dest: TXMLDocument);
  end;

// TODO: xml_encode - just call toString on the TXMLDocument root node
function xml_decode (contents: LongString): TXMLDocument;
function xml_decode_file (path: LongString): TXMLDocument; inline;

function DecodeHTMLEntity (name: string): string;
function EncodeHTMLEntity (name: string): string;

{$endif}

{$ifdef IMPLEMENTATION}

//$bookmark -
//$bookmark UTILITIES
//$bookmark -

function DecodeHTMLEntity (name: string): string;
begin
  if name = 'lt' then
    result := '<'
  else if name = 'gt' then
    result := '>'
  else if name = 'amp' then
    result := '&'
  else if name = 'apos' then
    result := ''''
  else if name = 'quot' then
    result := '"'
  else
    result := '';
end;

function EncodeHTMLEntity (name: string): string;
begin
  if name = '<' then
    result := 'lt'
  else if name = '>' then
    result := 'gt'
  else if name = '&' then
    result := 'amp'
  else if name = '''' then
    result := 'apos'
  else if name = '"' then
    result := 'quot'
  else
    result := '';
end;

//$bookmark -
//$bookmark XML RECORD
//$bookmark -

function TXMLNodeEnumerator.MoveNext: boolean;
begin
  if currentIndex < node.children.Count then
    begin
      currentValue := TXMLNode(node.children[currentIndex]);
      inc(currentIndex);
      result := true;
    end
  else
    result := false;
end;

constructor TXMLNodeEnumerator.Create(source: TXMLNode);
begin
  node := source;
  currentValue := nil;
  currentIndex := 0;
end;

function TXMLDocument.Add(name: string; contents: ansistring = ''): TXMLNode;
begin
  if root = nil then
    root := TXMLNode.Create(name, contents)
  else
    root.Add(name, contents);
end;

procedure TXMLDocument.Show;
begin
  if assigned(root) then
    root.show;
end;

function TXMLDocument.GetEnumerator: TXMLNodeEnumerator;
begin
  result := TXMLNodeEnumerator.Create(root);
end;

function TXMLDocument.GetValue(key: variant): TXMLNode;
begin
  case VarType(key) of
    varShortInt, varSmallint, varInteger:
      result := root.GetChildAtIndex(integer(key));
    varString:
      begin
        if root.name = string(key) then
          result := root
        else
          result := root.FindChild(string(key))
      end;
    otherwise
      fatal('invalid index type for list');
  end;
end;

function TXMLDocument.GetValue(name: string; index: TListIndex): TXMLNode;
begin
  result := root.FindChildAtIndex(name, index);
end;

class operator TXMLDocument.Initialize(var doc: TXMLDocument);
begin
  doc.root := nil;
end;

class operator TXMLDocument.Finalize(var doc: TXMLDocument);
begin
end;

class operator TXMLDocument.AddRef(var doc: TXMLDocument);
begin
end;

class operator TXMLDocument.Copy(constref src: TXMLDocument; var dest: TXMLDocument);
begin
  // TODO: copy will kill us here!
  dest.root := src.root;
end;

//$bookmark -
//$bookmark XML NODE
//$bookmark -

constructor TXMLNode.Create(name: TXMLNodeName; text: TXMLNodeContents = '');
begin
  self.name := name;
  self.text := text;
end;

function TXMLNode.GetEnumerator: TXMLNodeEnumerator;
begin
  result := TXMLNodeEnumerator.Create(self);
end;

function TXMLNode.GetValue(index: integer): TXMLNode; overload; inline;
begin
  result := GetChildAtIndex(index);
end;

function TXMLNode.GetValue(key: string): TXMLNode;
begin
  if name = key then
    result := self
  else
    result := FindChild(key);
end;

function TXMLNode.GetValue(name: string; index: integer): TXMLNode;
begin
  result := FindChildAtIndex(name, index);
end;

function TXMLNode.MakeAttributeString: ansistring;
var
  key: TDictionaryKey;
begin
  if attributes.Count > 0 then
    begin
      result := ' ';
      for key in attributes.Keys do
        result += key+'='+wrap(attributes[key])+' ';
      result := rtrim(result);
    end
  else
    result := '';
end;

function TXMLNode.MakeFormatedString (indent: string): ansistring;
var
  node: TXMLNode;
begin
  if text <> '' then
    result := indent+'<'+FullName+MakeAttributeString+'>'+text+'</'+FullName+'>'+#10
  else
    begin
      if children.Count > 0 then
        result := indent+'<'+FullName+MakeAttributeString+'>'+#10
      else
        result := indent+'<'+FullName+MakeAttributeString+'>';
      for TValue(node) in children do
        result += node.MakeFormatedString(indent+'  ');
      result += indent+'</'+FullName+'>'+#10;
    end;
end;

function TXMLNode.Str: ansistring;
begin
  result := MakeFormatedString('');
end;

function TXMLNode.Sibling: TXMLNode;
begin
  result := parent[index + 1];
end;

function TXMLNode.FullName: TXMLNodeName;
begin
  if prefix = '' then
    result := name
  else
    result := prefix+':'+name;
end;

function TXMLNode.Add(name: string; text: TXMLNodeContents = ''): TXMLNode;
var
  child: TXMLNode;
begin
  fatal(self.text <> '', 'node <'+self.name+'> has text contents sp can not add children.');
  child := TXMLNode.Create(name, text);
  child.parent := self;
  child.index := children.Count;
  children.AddWithoutCopy(child);
  result := child;
end;

function TXMLNode.Add(name: string; text: TXMLNodeContents; attributes: array of string): TXMLNode;
var
  i: integer;
begin
  // TODO: set 
  result := Add(name, text);
  for i := 0 to high(attributes) div 2 do
    result.attributes[attributes[i * 2]] := attributes[(i * 2) + 1];
end;

procedure TXMLNode.AddComment(contents: TXMLNodeContents);
begin
  // TODO: not sure how to add these without breaking indexing
  //children.AddWithoutCopy(TXMLCommentNode.Create(contents));
end;

function TXMLNode.FindAll(name: string): TList;
var
  child: TXMLNode;
begin
  result.Clear;
  for TValue(child) in children do
    if child.name = name then
      result.AddWithoutCopy(child);
end;

function TXMLNode.Copy: TValue;
begin
  result := TXMLNode.Create;
  TXMLNode(result).text := self.text;
  TXMLNode(result).name := self.name;
  TXMLNode(result).prefix := self.prefix;
  TXMLNode(result).attributes := self.attributes;
  TXMLNode(result).parent := self.parent;
  TXMLNode(result).index := self.index;
end;

function TXMLNode.FindChild (name: string): TXMLNode;
var
  child: TXMLNode;
begin
  result := nil;
  for TValue(child) in children do
    if child.name = name then
      exit(child);
end;

function TXMLNode.FindChildAtIndex(name: string; index: integer): TXMLNode;
var
  child: TXMLNode;
  next: integer;
begin
  result := nil;
  next := 0;
  for TValue(child) in children do
    if child.name = name then
      begin
        if next = index then
          exit(child);
        next += 1;
      end;
end;

function TXMLNode.GetChildAtIndex (index: integer): TXMLNode;
begin
  result := TXMLNode(children[index]);
end;


procedure TXMLNode.AddChild(node: TXMLNode);
begin
  node.parent := self;
  node.index := children.Count;
  children.AddWithoutCopy(node);
end;

function TXMLCommentNode.MakeFormatedString (indent: string): ansistring;
begin
  result := indent+'<!-- '+text+' -->'+#10;
end;

constructor TXMLCommentNode.Create(contents: TXMLNodeContents);
begin
  text := contents;
end;

//$bookmark -
//$bookmark XML SCANNER
//$bookmark -

type
  TXMLScanner = class(TScanner)
    public
      procedure Parse; override;
    private
      root: TXMLNode;
      prologNode: TXMLNode;
      procedure ParseTag(node: TXMLNode);
      function ParseAttribute: ansistring;
      function ParseText: ansistring;
      function ParseComment: ansistring;
      function ParseTagName(out prefix: TXMLNodeName): TXMLNodeName;
  end;

function xml_decode (contents: ansistring): TXMLDocument;
var
  scanner: TXMLScanner;
begin
  scanner := TXMLScanner.Create(contents);
  scanner.Parse;
  result.root := scanner.root;
  scanner.Free;
end;

function xml_decode_file (path: LongString): TXMLDocument;
begin
  result := xml_decode(file_get_contents(path));
end;

function TXMLScanner.ParseAttribute: ansistring;
begin
  Consume(kTokenID);
  Consume(kTokenEquals);
  if token = kTokenStringDoubleQuote then
    begin
      //writeln('got attribute: ', wrap(pattern));
      result := pattern;
    end;
  Consume(kTokenStringDoubleQuote);
end;

function TXMLScanner.ParseText: ansistring;
var
  name: string;
begin
  result := pattern;
  while c <> '<' do
    begin
      if c = '&' then
        begin
          ReadChar;
          name := ReadWord;
          //writeln('got ', name);
          if c = ';' then
            begin
              // TODO: should we decode html entities?
              //name := DecodeHTMLEntity(name);
              //if name = '' then
              //  ParserError('invalid html entity '+wrap(name));
              result += '&'+name+';';
              ReadChar;
            end
          else
            Consume(kTokenSemicolon);
        end;
      result += c;
      ReadChar;
    end;
  ReadToken;
  //writeln('got text: ', result);
end;

function TXMLScanner.ParseComment: ansistring;
begin
  result := pattern;
  while not Peek('-->') do
    begin
      result += c;
      ReadChar;
    end;
  ReadToken;
  Consume(kTokenDash);
  Consume(kTokenDash);
end;

function TXMLScanner.ParseTagName(out prefix: TXMLNodeName): TXMLNodeName;
var
  name: TXMLNodeName;
begin
  name := pattern;
  Consume(kTokenID);
  if TryConsume(kTokenColon) then
    begin
      result := pattern;
      prefix := name;
      Consume(kTokenID);
    end
  else
    begin
      result := name;
      prefix := '';
    end;
end;

procedure TXMLScanner.ParseTag(node: TXMLNode);
var
  name, prefix: TXMLNodeName;
  child: TXMLNode;
begin
  //writeln('parse tag');
  while true do
    begin
      //writeln('next: ', token);
      case token of
        kTokenEOF:
          break;
        // attributes
        kTokenID:
          begin
            // TODO: check if tag is mixing text with child tags
            if node.name = '' then
              begin
                node.name := ParseTagName(node.prefix);
                //writeln('got name ', wrap(node.name));
              end
            else
              begin
                name := pattern;
                node.attributes[name] := ParseAttribute;
              end;
          end;
        kTokenAngleBracketClosed:
          begin              
            //writeln('close tag ', wrap(node.name));
            Consume;
            if token <> kTokenAngleBracketOpen then
              node.text := ParseText;
          end;
        kTokenAngleBracketOpen:
          begin
            Consume;
            if TryConsume(kTokenExclamationMark) then
              begin
                Consume(kTokenDash);
                Consume(kTokenDash);
                node.AddComment(ParseComment);
                Consume(kTokenAngleBracketClosed);
              end
            else if TryConsume(kTokenForwardSlash) then
              begin
                name := ParseTagName(prefix);
                //writeln('end tag ', wrap(name));
                if (prefix <> node.prefix) or (name <> node.name) then
                  ParserError('end tag name doesn''t match');
                Consume(kTokenAngleBracketClosed);
                break;
              end
            else
              begin
                child := TXMLNode.Create;
                child.name := ParseTagName(child.prefix);
                //writeln('open tag ', wrap(child.name));
                node.AddChild(child);
                ParseTag(child);
              end;
          end;
        otherwise
          ParserError('invalid token '+TokenToStr(token));
      end;
    end;
end;

procedure TXMLScanner.Parse;
var
  name: TXMLNodeName;
begin
  ReadToken;
  root := TXMLNode.Create;
  Consume(kTokenAngleBracketOpen);

  // parse prolog
  if TryConsume(kTokenQuestionMark) then
    if TryConsume(kTokenID, name) and (StrCaseCmp(name, 'xml')) then
      begin
        prologNode := TXMLNode.Create(name);
        //writeln('got prolog');
        while true do
          begin
            if token = kTokenID then
              begin
                prologNode.attributes[name] := ParseAttribute;
              end
            else if token = kTokenQuestionMark then
              begin
                Consume(kTokenQuestionMark);
                Consume(kTokenAngleBracketClosed);
                //writeln('done prolog');
                //writeln('==============================');
                Consume(kTokenAngleBracketOpen);
                break;
              end
            else
              consume(kTokenID);
          end;
      end
    else
      ParserError('XML prolog name must be "xml", got "'+name+'"');

  {
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <!DOCTYPE note SYSTEM "Note.dtd">
  }
  // parse DTD
  if TryConsume(kTokenExclamationMark) then
    if TryConsume(kTokenID, name) and (StrCaseCmp(name, 'doctype')) then
      begin
        Consume(kTokenID);
        Consume(kTokenID);
        //writeln('got DTD');
        Consume(kTokenStringDoubleQuote);
        //writeln('got attribute: ', wrap(pattern));
        Consume(kTokenStringDoubleQuote);
        //writeln('done DTD');
        //writeln('==============================');
        Consume(kTokenAngleBracketClosed);
        Consume(kTokenAngleBracketOpen);
      end;

  ParseTag(root);
end;

{$endif}

{$ifdef CODE}

{$endif}