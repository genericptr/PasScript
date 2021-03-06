{$mode objfpc}
{$modeswitch advancedrecords}

unit Scanner;
interface
uses
	SysUtils;

type 
	TToken = (kTokenID, 
						kTokenInteger, 
						kTokenRealNumber,
						kTokenStringDoubleQuote,
						kTokenStringSingleQuote,
						kTokenSquareBracketOpen,
						kTokenSquareBracketClosed,
						kTokenParenthasisOpen,
						kTokenParenthasisClosed,
						kTokenCurlyBracketOpen,
						kTokenCurlyBracketClosed,
						kTokenAngleBracketOpen,
						kTokenAngleBracketClosed,
						kTokenEquals,
						kTokenColon,
						kTokenComma,
						kTokenSemicolon,
						kTokenQuestionMark,
						kTokenForwardSlash,
						kTokenExclamationMark,
						kTokenBackSlash,
						kTokenAmpersand,
						kTokenDash,
						kTokenEOF
						);

type
	TFileInfo = record
		line: integer;
		column: integer;
	end;

type
	TScanner = class
		public
			constructor Create(str: ansistring);
			procedure Parse; virtual; abstract;
			destructor Destroy; override;
		protected
			contents: ansistring;
			pattern: ansistring;
			token: TToken;
			c: char;
		protected
			function ReadToken: TToken;
			procedure ParserError (messageString: string = '');

			procedure  Consume; inline;
			procedure Consume(t: TToken);
			function TryConsume(t: TToken): boolean;
			function TryConsume(t: TToken; out s: shortstring): boolean; inline;
			function TryConsume(t: TToken; out s: ansistring): boolean; inline;

			function ReadWord: ansistring;
			function ReadChar: char;
			function ReadNumber: string;
			
			function Peek(str: string): boolean;
		private
			procedure SkipLine;
			procedure SkipSpace;
			procedure AdvancePattern;
			function Peek (advance: integer = 1): char;
		private
			currentIndex: integer;
			fileInfo: TFileInfo;
	end;

function TokenToStr(t: TToken): string;

implementation

{$macro on}
{$define TCharSetWhiteSpace:=' ', '	', #10, #12}
{$define TCharSetLineEnding:=#10, #12}
{$define TCharSetWord:='a'..'z','A'..'Z','_'}
{$define TCharSetInteger:='0'..'9'}
{$define TCharSetQuotes:='"', ''''}

function TokenToStr(t: TToken): string;
begin
	case t of
		kTokenID:
			result := 'ID';
		kTokenInteger:
			result := 'Integer';
		kTokenRealNumber:
			result := 'Real';
		kTokenStringDoubleQuote:
			result := '"';
		kTokenStringSingleQuote:
			result := '''';
		kTokenSquareBracketOpen:
			result := '[';
		kTokenSquareBracketClosed:
			result := ']';
		kTokenParenthasisOpen:
			result := '(';
		kTokenParenthasisClosed:
			result := ')';
		kTokenCurlyBracketOpen:
			result := '{';
		kTokenCurlyBracketClosed:
			result := '}';
		kTokenAngleBracketOpen:
			result := '<';
		kTokenAngleBracketClosed:
			result := '>';
		kTokenEquals:
			result := '=';
		kTokenColon:
			result := ':';
		kTokenComma:
			result := ',';
		kTokenSemicolon:
			result := ';';
		kTokenQuestionMark:
			result := '?';
		kTokenExclamationMark:
			result := '!';
		kTokenForwardSlash:
			result := '/';
		kTokenBackSlash:
			result := '\';
		kTokenAmpersand:
			result := '&';
		kTokenDash:
			result := '-';
		kTokenEOF:
			result := 'EOF';
		otherwise
			raise exception.create('invalid token');
	end;
end;

procedure TScanner.Consume;
begin
	Consume(token);
end;

procedure TScanner.Consume(t: TToken);
begin
	if token = t then
	  ReadToken
	else
		ParserError('Got "'+TokenToStr(token)+'", expected "'+TokenToStr(t)+'"');
end;

function TScanner.TryConsume(t: TToken; out s: shortstring): boolean; inline;
begin
	s := pattern;
	result := TryConsume(t);
end;

function TScanner.TryConsume(t: TToken; out s: ansistring): boolean;
begin
	s := pattern;
	result := TryConsume(t);
end;

function TScanner.TryConsume(t: TToken): boolean;
begin
	if token = t then
	  begin
	  	ReadToken;
	  	result := true;
	  end
	else
		result := false;
end;

procedure TScanner.SkipLine;
begin
	repeat
		ReadChar;
	until c in [TCharSetLineEnding];
end;

procedure TScanner.SkipSpace;
begin
	while c in [TCharSetWhiteSpace] do
		ReadChar;
end;

function TScanner.Peek(advance: integer = 1): char;
begin
	if currentIndex + advance < length(contents) then
		result := contents[currentIndex + advance]
	else
		result := #0;
end;

function TScanner.Peek(str: string): boolean;
var
	i: integer;
begin
	result := true;
	for i := 1 to length(str) do
		if contents[currentIndex + (i - 1)] <> str[i] then
			exit(false);
end;

function TScanner.ReadChar: char;
begin
	currentIndex += 1;
	c := contents[currentIndex];
	fileInfo.column += 1;
	result := c;
end;

function TScanner.ReadWord: ansistring;
begin
	pattern := '';
	while c in [TCharSetWord, TCharSetInteger] do
		begin
			pattern += c;
			ReadChar;
		end;
	result := pattern;
end;

procedure TScanner.AdvancePattern;
begin
	pattern += c;
	ReadChar;
end;

function TScanner.ReadNumber: string;
var
	negative: boolean = false;
begin
	pattern := '';
	token := kTokenInteger;

	if c = '-' then
		begin
			negative := true;
			AdvancePattern;
		end;

	while c in [TCharSetInteger, '.', 'e'] do
		begin
			// TODO: must be followed by a number!
			if c = 'e' then
				begin
					AdvancePattern;
					if c = '-' then
						begin
							AdvancePattern;
							while c in [TCharSetInteger] do
							  AdvancePattern;
							 break;
						end;
				end
			else if c = '.' then
				token := kTokenRealNumber;
			AdvancePattern;
		end;

	result := pattern;
end;

procedure TScanner.ParserError(messageString: string = '');
begin
	writeln('Error at ', fileInfo.line, ':', fileInfo.column, ': ', messageString);
	halt(1);
end;

function TScanner.ReadToken: TToken;
label
	TokenRead;
var
	start: char;
begin
	while currentIndex < length(contents) do 
		begin
			//writeln('  ', currentIndex, ':', c);
			case c of
				'-':
					begin
						if Peek in [TCharSetInteger] then
							begin
								ReadNumber;
								goto TokenRead;
							end
						else
							begin
								token := kTokenDash;
								ReadChar;
								goto TokenRead;
							end;
					end;
				TCharSetInteger:
					begin
						ReadNumber;
						goto TokenRead;
					end;
				TCharSetWord:
					begin
						token := kTokenID;
						ReadWord;
						goto TokenRead;
					end;
				'[':
					begin
						token := kTokenSquareBracketOpen;
						ReadChar;
						goto TokenRead;
					end;
				']':
					begin
						token := kTokenSquareBracketClosed;
						ReadChar;
						goto TokenRead;
					end;
				'(':
					begin
						token := kTokenParenthasisOpen;
						ReadChar;
						goto TokenRead;
					end;
				')':
					begin
						token := kTokenParenthasisClosed;
						ReadChar;
						goto TokenRead;
					end;
				'{':
					begin
						token := kTokenCurlyBracketOpen;
						ReadChar;
						goto TokenRead;
					end;
				'}':
					begin
						token := kTokenCurlyBracketClosed;
						ReadChar;
						goto TokenRead;
					end;
				'<':
					begin
						token := kTokenAngleBracketOpen;
						ReadChar;
						goto TokenRead;
					end;
				'>':
					begin
						token := kTokenAngleBracketClosed;
						ReadChar;
						goto TokenRead;
					end;
				'=':
					begin
						token := kTokenEquals;
						ReadChar;
						goto TokenRead;
					end;
				':':
					begin
						token := kTokenColon;
						ReadChar;
						goto TokenRead;
					end;
				',':
					begin
						token := kTokenComma;
						ReadChar;
						goto TokenRead;
					end;
				';':
					begin
						token := kTokenSemicolon;
						ReadChar;
						goto TokenRead;
					end;
				'?':
					begin
						token := kTokenQuestionMark;
						ReadChar;
						goto TokenRead;
					end;
				'!':
					begin
						token := kTokenExclamationMark;
						ReadChar;
						goto TokenRead;
					end;
				TCharSetQuotes:
					begin
						pattern := '';
						start := c;
						while true do
							begin
								ReadChar;
								// escaped quotes
								if (c = '\') and (Peek = start) then
									begin
										pattern += c;
										ReadChar;
										pattern += c;
									end
								// join any character that isn't a quote
								else if c <> start then
									pattern += c
								else if c = '"' then
									begin
										token := kTokenStringDoubleQuote;
										ReadChar;
										break;
									end
								else if c = '''' then
									begin
										token := kTokenStringSingleQuote;
										ReadChar;
										break;
									end;
							end;
						goto TokenRead;
					end;
				// NOTE: ignore comments - make a way to override ReadToken so 
				// we can add this logic to parser subclasses
				'/':
					begin
						if Peek = '/' then
							begin
								SkipLine;
								continue;
							end
						else if Peek = '*' then
							begin
								while true do
									begin
										ReadChar;
										if (c = '*') and (Peek = '/') then
											begin
												ReadChar; // *
												ReadChar; // /
												break;
											end;
									end;
								continue;
							end
						else
							begin
								ReadChar;
								token := kTokenForwardSlash;
								goto TokenRead;
							end;
					end;
				//'#':
				//	begin
				//		SkipLine;
				//    continue;	
				//	end;
				TCharSetWhiteSpace:
					begin
						if c in [TCharSetLineEnding] then
							begin
								fileInfo.line += 1;
								fileInfo.column := 0;
							end;
						SkipSpace;
					end;
				otherwise
					ParserError('unknown character "'+c+'"');
			end;
		end;

	// if we got here we reached the end
	token := kTokenEOF;

	TokenRead:
		result := token;
end;

constructor TScanner.Create(str: ansistring);
begin
	contents := str;
	contents += #0;
	currentIndex := 1;
	fileInfo.line := 1;
	fileInfo.column := 1;
	c := contents[currentIndex];
end;

destructor TScanner.Destroy;
begin
end;

end.