{$ifdef INTERFACE}

function plist_decode(xmlString: longstring): TDict;

{$endif}

{$ifdef IMPLEMENTATION}

function plist_decode(xmlString: longstring): TDict;

  procedure parse_array(root: TXMLNode; var list: TList); forward;
  procedure parse_dict(root: TXMLNode; var dict: TDict); forward;

  function parse_element(node: TXMLNode): TValue;
  begin
    if node.name = 'dict' then
      result := TDictValue.Create
    else if node.name = 'array' then
      result := TListValue.Create
    else if node.name = 'string' then
      result := ansistring(node.text)
    else if node.name = 'date' then
      result := string(node.text)
    else if node.name = 'real' then
      result := node.text.ToFloat
    else if node.name = 'integer' then
      result := node.text.ToInt
    else if node.name = 'boolean' then
      result := node.text.ToBool
    else if node.name = 'data' then
      result := ansistring(node.text)
    else
      fatal('dict value type '+node.name+' doesn''t exist');
  end;

  procedure parse_dict(root: TXMLNode; var dict: TDict);
  var
    child, value: TXMLNode;
    dictValue: TDictValue;
    arrayValue: TListValue;
    key: string;
  begin
    for child in root do
      begin
        if child.name = 'key' then
          begin
            value := child.sibling;
            key := child.text;
            if value.name = 'dict' then
              begin
                dictValue := TDictValue(parse_element(value));
                parse_dict(value, dictValue.value);
                dict[key] := dictValue;
              end
            else if value.name = 'array' then
              begin
                arrayValue := TListValue(parse_element(value));
                parse_array(value, arrayValue.value);
                dict[key] := arrayValue;
              end
            else
              dict[key] := parse_element(value);
          end;
      end;
  end;

  procedure parse_array(root: TXMLNode; var list: TList);
  var
    child: TXMLNode;
    dictValue: TDictValue;
    arrayValue: TListValue;
  begin
    for child in root do
      begin
        if child.name = 'dict' then
          begin
            dictValue := TDictValue(parse_element(child));
            parse_dict(child, dictValue.value);
            list.Add(dictValue);
          end
        else if child.name = 'array' then
          begin
            arrayValue := TListValue(parse_element(child));
            parse_array(child, arrayValue.value);
            list.Add(arrayValue);
          end
        else
          list.Add(parse_element(child));
      end;
  end;

var
  xml: TXMLDocument;
  dict: TDict;
begin
  xml := xml_decode(xmlString);
  parse_dict(xml['plist']['dict'], dict);
  result := dict;
end;

{$endif}

{$ifdef CODE}

{$endif}