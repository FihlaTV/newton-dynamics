/* Copyright (c) <2003-2016> <Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dNewtonLuaStdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <tchar.h>

#include "dNewtonLuaLex.h"
#include "dNewtonLuaCompiler.h"
//#include <dVirtualMachine.h>


#ifdef _MSC_VER
#include <windows.h>
#include <crtdbg.h>
#endif

void *operator new(size_t s) 
{
	static int installmemmoryLeaksTracker = 1;
	if (installmemmoryLeaksTracker) {
		installmemmoryLeaksTracker = 0;
		#ifdef _MSC_VER
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		#endif
	}
	void* const mem = malloc (s);
//	unsigned long xxx = unsigned long (mem);
//	if (((xxx & 0xffff) == 0x8598) && (s == 16))
//		dAssert(0);
	return mem;
}

void operator delete (void* ptr) 
{
//	unsigned long xxx = unsigned long (ptr);
//	if (((xxx & 0xffff) == 0x3DF0))
//		dAssert(0);
	free (ptr);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		fprintf (stdout, "usage: dNewtonLuaCompiler filename\n");
		exit (0);
	}

	const char* const sourceFileName = argv[1];
	FILE* const sourceFile = fopen (sourceFileName, "r");
	if (!sourceFile) {
		fprintf (stdout, "script source file: \"%s\" not found\n", sourceFileName);
		exit (0);
	}

	dString source;
	source.LoadFile(sourceFile);
	fclose (sourceFile);

//	const char* const packacgeName = "demos";
//	dNewtonLuaCompiler compiler(packacgeName);
//	compiler.CompileSource (source.GetStr());

	dNewtonLuaCompiler compiler;
	compiler.CompileSource (source.GetStr());

//	dNewtonLuaLex scanner(source.GetStr());
//	dNewtonLuaParcer parcel;
//	bool status = parcel.Parse(scanner);

	return 0;
}

