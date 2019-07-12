{$ifdef INTERFACE}

function Rand(min, max: longint): longint;

{$endif}

{$ifdef IMPLEMENTATION}

// https://www.php.net/manual/en/function.rand.php
function Rand(min, max: longint): longint;
var
  zero: boolean = false;
begin
  if min = 0 then 
    begin
      //Fatal('GetRandomNumber 0 min value is invalid.');
      min += 1;
      max += 1;
      zero := true;
    end;
    
  if (min < 0) and (max > 0) then
    max += abs(min);
  
  result := System.Random(max) mod ((max - min) + 1);
  
  if result < 0 then
    result := abs(result);
    
  if zero then
    min -= 1;
  result += min;
end;
{$endif}

{$ifdef CODE}
System.Randomize;
{$endif}