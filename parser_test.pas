{$mode objfpc}

program UTest;

{
  parser_test.TVec2.x
  parser_test.TVec2.y
  parser_test.TVec2.m_z
  parser_test.TVec2.z
}
type
  TVec2 = record
    x, y: integer;
    m_z: integer;
    property z: integer read m_z write m_z;
  end;

procedure MyProcedure (var input: integer); 
{
  adds to scope parser_test.MyProcedure.x = integer
}
var
  x, y, z: integer;
  vec: TVec2;
  spez: TList<integer>;
begin
  {
    vec. queries = parser_test.TVec2 because we know "vec" is type TVec2
  }
  // case is only statement that has end of with no begin?
  case x of
  end;

end;

begin

end.  