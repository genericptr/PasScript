
{$ifdef INTERFACE}

{
  load png,jpeg (does chris have this?), tga (from metal framework), bmp (existing code?)

  make a tbitmap class
}

type
  TBitmap = class
    {
      get/set pixel
    }
  end;

function load_image(path: ansistring): TBitmap;

{$endif}

{$ifdef IMPLEMENTATION}

{$endif}
