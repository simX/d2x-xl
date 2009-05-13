// Copyright (C) 1997 Bryan Aamot
//==============================================================================
//
// FILENAME: eregreg.h
//
// DESCRIPTION: 
//		Contains the definitions and prototypes for the EREGREG DLLs.
//
// CHANGE LOG:	
//		11-21-94	BHJ		Created
//
//==============================================================================

#ifndef __EREGREG_H__
#define __EREGREG_H__

#if __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#ifndef FAR
#define FAR _far
#endif
#endif

typedef struct tagCOUNTERS
	{
	WORD	wTimesRegistered;			// Times the program has been registered.
	WORD	wTimesBypassed;				// Times the user has bypassed register.
	WORD	wTimesRun;					// Times the program has been run.
	}
	COUNTERS, 
	FAR* LPCOUNTERS;

#if _WIN32
extern int __declspec(dllexport) ERegCard_Register
#else
extern int CALLBACK _export ERegCard_Register
#endif
	(
	HWND 		hwndParent,				// Parent window.
	BOOL		fUserRequested,			// Did the user request registration?
	BOOL		fApplicationRequested,	// Is the function called by the app?
	WORD		wMaxBypass,				// Maximum number of allowed bypasses.
	LPCOUNTERS	lpRegCounter,           // Counter structure.
	BOOL FAR*	lpfRegistered			// Was the program registered?
	);           

#if _WIN32
extern int __declspec(dllexport) ERegCard_Show3D();
#else
extern int CALLBACK _export ERegCard_Show3D();
#endif


#if __cplusplus
}
#endif

#endif // #ifndef !__EREGREG_H__

