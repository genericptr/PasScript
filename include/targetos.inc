{$macro on}

// Compiler defines during compilation:
// http://www.freepascal.org/docs-html/prog/progap7.html

{$if defined(PLATFORM_OSX) or defined(PLATFORM_ARM)}
{$fatal Deprecated: PLATFORM_OSX and PLATFORM_ARM for TARGET_OS_MAC and TARGET_OS_IPHONE}
{$endif}

{$if defined(IOS_UNIVERSAL)}
{$fatal Deprecated: IOS_UNIVERSAL for iOS 6 headers}
{$endif}

{$if defined(darwin) and defined(cpupowerpc32)}
{$define TARGET_OS_MAC_PPC}
{$define TARGET_OS_MAC_PPC32}
{$define TARGET_OS_MAC}
{$define TARGET_CPU_PPC}
{$endif}

{$if defined(darwin) and defined(cpupowerpc64)}
{$define TARGET_OS_MAC_PPC}
{$define TARGET_OS_MAC_PPC64}
{$define TARGET_OS_MAC}
{$define TARGET_CPU_PPC}
{$endif}

{$if defined(darwin) and defined(cpui386) and not defined(iphonesim)}
{$define TARGET_OS_MAC_I386}
{$define TARGET_OS_MAC}
{$endif}

{$if defined(darwin) and defined(cpux86_64) and not defined(iphonesim)}
{$define TARGET_OS_MAC_X86}
{$define TARGET_OS_MAC}
{$endif}

{$if defined(cpuarm) or defined(cpuaarch64) or defined(iphonesim)}
{$define TARGET_OS_IPHONE}

{$if defined(cpuaarch64) or defined(cpux86_64)}
{$define TARGET_OS_IPHONE_X64}
{$define TARGET_OS_IPHONE_DEVICE}
{$else}
{$define TARGET_OS_IPHONE_I386}
{$define TARGET_OS_IPHONE_DEVICE}
{$endif}

{$if defined(iphonesim)}
{$define TARGET_OS_IPHONE_SIM}
{$undef TARGET_OS_IPHONE_DEVICE}
{$endif}

{$undef TARGET_OS_MAC}
{$undef TARGET_OS_MAC_X86}
{$undef TARGET_OS_MAC_I386}
{$undef TARGET_OS_MAC_PPC}
{$undef TARGET_OS_MAC_PPC32}
{$undef TARGET_OS_MAC_PPC64}
{$endif}

{$if defined(windowscpu)}
{$define TARGET_OS_WINDOWS}
{$undef TARGET_OS_IPHONE}
{$undef TARGET_OS_IPHONE_X64}
{$undef TARGET_OS_IPHONE_I386}
{$undef TARGET_OS_MAC}
{$undef TARGET_OS_MAC_X86}
{$undef TARGET_OS_MAC_I386}
{$undef TARGET_OS_MAC_PPC}
{$undef TARGET_OS_MAC_PPC32}
{$undef TARGET_OS_MAC_PPC64}
{$endif}

{$if defined(TARGET_OS_MAC) or defined(TARGET_OS_IPHONE)}
{$define LIBRARY_COREFOUNDATION}
{$define LIBRARY_COREGRAPHICS}
{$define OBJC}
{$endif}

{$if defined(TARGET_OS_IPHONE)}
{$define LIBRARY_OPENGLES}
{$endif}

{$if defined(TARGET_OS_MAC_I386) or defined(TARGET_CPU_PPC)}
{$define CARBON_GUI_AVAILABLE}
{$else}
{$define CARBON_GUI_UNAVAILABLE}
{$endif}

{$define MAC_OS_X_VERSION_10_8 := 1080}
{$define MAC_OS_X_VERSION_10_9 := 1090}
{$define MAC_OS_X_VERSION_10_10 := 10100}
