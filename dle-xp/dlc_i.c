

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Sat Oct 07 21:53:20 2006
 */
/* Compiler settings for .\dlc.odl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_dlc,0x3F315843,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(IID, DIID_Idlc,0x3F315844,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(IID, IID_IComMine,0x8B31FC81,0x69E2,0x11D2,0x8F,0x0F,0x00,0x60,0xB0,0xB4,0x15,0x34);


MIDL_DEFINE_GUID(IID, IID_IComCube,0x5DF6BE60,0x6A08,0x11d2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(IID, IID_IComObj,0xA9C67591,0x6DF0,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(CLSID, CLSID_Document,0x3F315842,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(CLSID, CLSID_ComMine,0x8B31FC82,0x69E2,0x11D2,0x8F,0x0F,0x00,0x60,0xB0,0xB4,0x15,0x34);


MIDL_DEFINE_GUID(CLSID, CLSID_ComCube,0x5DF6BE61,0x6A08,0x11d2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);


MIDL_DEFINE_GUID(CLSID, CLSID_ComObj,0xA9C67592,0x6DF0,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

