#pragma once

#if defined(_WINDOWS_)
#	error "Only include windows.h through this file!!"
#endif

#if defined(_WIN32)

#	define WIN32_LEAN_AND_MEAN 

//#	define NOGDICAPMASKS
//#	define NOVIRTUALKEYCODES
//#	define NOWINMESSAGES
//#	define NOWINSTYLES
//#	define NOSYSMETRICS
//#	define NOMENUS
//#	define NOICONS
//#	define NOKEYSTATES
//#	define NOSYSCOMMANDS
//#	define NORASTEROPS
//#	define NOSHOWWINDOW
//#	define NOATOM
//#	define NOCLIPBOARD
//#	define NOCOLOR
//#	define NOCTLMGR
//#	define NODRAWTEXT
//#	define NOGDI
//#	define NOMB
//#	define NOMEMMGR
//#	define NOMETAFILE
//#	define NOMINMAX
//#	define NOOPENFILE
//#	define NOSCROLL
//#	define NOSERVICE
//#	define NOSOUND
//#	define NOTEXTMETRIC
//#	define NOWH
//#	define NOWINOFFSETS
//#	define NOCOMM
//#	define NOKANJI
//#	define NOHELP
//#	define NOPROFILER
//#	define NODEFERWINDOWPOS
//#	define NOMCX
//#	define NOUSER
//#	define NONLS
//#	define NOMSG

#	include <SDKDDKVer.h>
#	include <windows.h>

#endif // _WIN32