

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __dlc_i_h__
#define __dlc_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __Idlc_FWD_DEFINED__
#define __Idlc_FWD_DEFINED__
typedef interface Idlc Idlc;
#endif 	/* __Idlc_FWD_DEFINED__ */


#ifndef __IComMine_FWD_DEFINED__
#define __IComMine_FWD_DEFINED__
typedef interface IComMine IComMine;
#endif 	/* __IComMine_FWD_DEFINED__ */


#ifndef __IComCube_FWD_DEFINED__
#define __IComCube_FWD_DEFINED__
typedef interface IComCube IComCube;
#endif 	/* __IComCube_FWD_DEFINED__ */


#ifndef __IComObj_FWD_DEFINED__
#define __IComObj_FWD_DEFINED__
typedef interface IComObj IComObj;
#endif 	/* __IComObj_FWD_DEFINED__ */


#ifndef __Document_FWD_DEFINED__
#define __Document_FWD_DEFINED__

#ifdef __cplusplus
typedef class Document Document;
#else
typedef struct Document Document;
#endif /* __cplusplus */

#endif 	/* __Document_FWD_DEFINED__ */


#ifndef __ComMine_FWD_DEFINED__
#define __ComMine_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComMine ComMine;
#else
typedef struct ComMine ComMine;
#endif /* __cplusplus */

#endif 	/* __ComMine_FWD_DEFINED__ */


#ifndef __ComCube_FWD_DEFINED__
#define __ComCube_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComCube ComCube;
#else
typedef struct ComCube ComCube;
#endif /* __cplusplus */

#endif 	/* __ComCube_FWD_DEFINED__ */


#ifndef __ComObj_FWD_DEFINED__
#define __ComObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComObj ComObj;
#else
typedef struct ComObj ComObj;
#endif /* __cplusplus */

#endif 	/* __ComObj_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __dlc_LIBRARY_DEFINED__
#define __dlc_LIBRARY_DEFINED__

/* library dlc */
/* [version][uuid] */ 


DEFINE_GUID(LIBID_dlc,0x3F315843,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#ifndef __Idlc_DISPINTERFACE_DEFINED__
#define __Idlc_DISPINTERFACE_DEFINED__

/* dispinterface Idlc */
/* [uuid] */ 


DEFINE_GUID(DIID_Idlc,0x3F315844,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3F315844-67AC-11D2-AE2A-00C0F03014A5")
    Idlc : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IdlcVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Idlc * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Idlc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Idlc * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Idlc * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Idlc * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Idlc * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Idlc * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IdlcVtbl;

    interface Idlc
    {
        CONST_VTBL struct IdlcVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Idlc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Idlc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Idlc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Idlc_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Idlc_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Idlc_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Idlc_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __Idlc_DISPINTERFACE_DEFINED__ */


#ifndef __IComMine_INTERFACE_DEFINED__
#define __IComMine_INTERFACE_DEFINED__

/* interface IComMine */
/* [unique][helpstring][dual][uuid][object] */ 


DEFINE_GUID(IID_IComMine,0x8B31FC81,0x69E2,0x11D2,0x8F,0x0F,0x00,0x60,0xB0,0xB4,0x15,0x34);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B31FC81-69E2-11D2-8F0F-0060B0B41534")
    IComMine : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InfoMessage( 
            /* [in] */ BSTR strValue) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumberOfCubes( 
            /* [retval][out] */ long *pRetVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumberOfPoints( 
            /* [retval][out] */ long *pRetVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumberOfWalls( 
            /* [retval][out] */ long *pRetVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumberOfTriggers( 
            /* [retval][out] */ long *pRetVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumberOfObjects( 
            /* [retval][out] */ long *pRetVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Cube( 
            VARIANT index,
            /* [retval][out] */ LPDISPATCH *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Object( 
            VARIANT index,
            /* [retval][out] */ LPDISPATCH *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComMineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComMine * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComMine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComMine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComMine * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComMine * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComMine * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComMine * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InfoMessage )( 
            IComMine * This,
            /* [in] */ BSTR strValue);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumberOfCubes )( 
            IComMine * This,
            /* [retval][out] */ long *pRetVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumberOfPoints )( 
            IComMine * This,
            /* [retval][out] */ long *pRetVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumberOfWalls )( 
            IComMine * This,
            /* [retval][out] */ long *pRetVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumberOfTriggers )( 
            IComMine * This,
            /* [retval][out] */ long *pRetVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumberOfObjects )( 
            IComMine * This,
            /* [retval][out] */ long *pRetVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Cube )( 
            IComMine * This,
            VARIANT index,
            /* [retval][out] */ LPDISPATCH *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Object )( 
            IComMine * This,
            VARIANT index,
            /* [retval][out] */ LPDISPATCH *pVal);
        
        END_INTERFACE
    } IComMineVtbl;

    interface IComMine
    {
        CONST_VTBL struct IComMineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComMine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComMine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComMine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComMine_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComMine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComMine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComMine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComMine_InfoMessage(This,strValue)	\
    (This)->lpVtbl -> InfoMessage(This,strValue)

#define IComMine_get_NumberOfCubes(This,pRetVal)	\
    (This)->lpVtbl -> get_NumberOfCubes(This,pRetVal)

#define IComMine_get_NumberOfPoints(This,pRetVal)	\
    (This)->lpVtbl -> get_NumberOfPoints(This,pRetVal)

#define IComMine_get_NumberOfWalls(This,pRetVal)	\
    (This)->lpVtbl -> get_NumberOfWalls(This,pRetVal)

#define IComMine_get_NumberOfTriggers(This,pRetVal)	\
    (This)->lpVtbl -> get_NumberOfTriggers(This,pRetVal)

#define IComMine_get_NumberOfObjects(This,pRetVal)	\
    (This)->lpVtbl -> get_NumberOfObjects(This,pRetVal)

#define IComMine_get_Cube(This,index,pVal)	\
    (This)->lpVtbl -> get_Cube(This,index,pVal)

#define IComMine_get_Object(This,index,pVal)	\
    (This)->lpVtbl -> get_Object(This,index,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IComMine_InfoMessage_Proxy( 
    IComMine * This,
    /* [in] */ BSTR strValue);


void __RPC_STUB IComMine_InfoMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_NumberOfCubes_Proxy( 
    IComMine * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComMine_get_NumberOfCubes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_NumberOfPoints_Proxy( 
    IComMine * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComMine_get_NumberOfPoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_NumberOfWalls_Proxy( 
    IComMine * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComMine_get_NumberOfWalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_NumberOfTriggers_Proxy( 
    IComMine * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComMine_get_NumberOfTriggers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_NumberOfObjects_Proxy( 
    IComMine * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComMine_get_NumberOfObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_Cube_Proxy( 
    IComMine * This,
    VARIANT index,
    /* [retval][out] */ LPDISPATCH *pVal);


void __RPC_STUB IComMine_get_Cube_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComMine_get_Object_Proxy( 
    IComMine * This,
    VARIANT index,
    /* [retval][out] */ LPDISPATCH *pVal);


void __RPC_STUB IComMine_get_Object_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComMine_INTERFACE_DEFINED__ */


#ifndef __IComCube_INTERFACE_DEFINED__
#define __IComCube_INTERFACE_DEFINED__

/* interface IComCube */
/* [unique][helpstring][dual][uuid][object] */ 


DEFINE_GUID(IID_IComCube,0x5DF6BE60,0x6A08,0x11d2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5DF6BE60-6A08-11d2-AE2A-00C0F03014A5")
    IComCube : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StaticLight( 
            /* [in] */ long val) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StaticLight( 
            /* [retval][out] */ long *pRetVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComCubeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComCube * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComCube * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComCube * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComCube * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComCube * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComCube * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComCube * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StaticLight )( 
            IComCube * This,
            /* [in] */ long val);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StaticLight )( 
            IComCube * This,
            /* [retval][out] */ long *pRetVal);
        
        END_INTERFACE
    } IComCubeVtbl;

    interface IComCube
    {
        CONST_VTBL struct IComCubeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComCube_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComCube_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComCube_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComCube_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComCube_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComCube_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComCube_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComCube_put_StaticLight(This,val)	\
    (This)->lpVtbl -> put_StaticLight(This,val)

#define IComCube_get_StaticLight(This,pRetVal)	\
    (This)->lpVtbl -> get_StaticLight(This,pRetVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IComCube_put_StaticLight_Proxy( 
    IComCube * This,
    /* [in] */ long val);


void __RPC_STUB IComCube_put_StaticLight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComCube_get_StaticLight_Proxy( 
    IComCube * This,
    /* [retval][out] */ long *pRetVal);


void __RPC_STUB IComCube_get_StaticLight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComCube_INTERFACE_DEFINED__ */


#ifndef __IComObj_INTERFACE_DEFINED__
#define __IComObj_INTERFACE_DEFINED__

/* interface IComObj */
/* [unique][helpstring][dual][uuid][object] */ 


DEFINE_GUID(IID_IComObj,0xA9C67591,0x6DF0,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A9C67591-6DF0-11D2-AE2A-00C0F03014A5")
    IComObj : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Id( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Id( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComObjVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObj * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObj * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObj * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComObj * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComObj * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComObj * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComObj * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IComObj * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IComObj * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IComObj * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Id )( 
            IComObj * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IComObjVtbl;

    interface IComObj
    {
        CONST_VTBL struct IComObjVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObj_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObj_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObj_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObj_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComObj_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComObj_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComObj_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComObj_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IComObj_put_Type(This,newVal)	\
    (This)->lpVtbl -> put_Type(This,newVal)

#define IComObj_get_Id(This,pVal)	\
    (This)->lpVtbl -> get_Id(This,pVal)

#define IComObj_put_Id(This,newVal)	\
    (This)->lpVtbl -> put_Id(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComObj_get_Type_Proxy( 
    IComObj * This,
    /* [retval][out] */ long *pVal);


void __RPC_STUB IComObj_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IComObj_put_Type_Proxy( 
    IComObj * This,
    /* [in] */ long newVal);


void __RPC_STUB IComObj_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IComObj_get_Id_Proxy( 
    IComObj * This,
    /* [retval][out] */ long *pVal);


void __RPC_STUB IComObj_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IComObj_put_Id_Proxy( 
    IComObj * This,
    /* [in] */ long newVal);


void __RPC_STUB IComObj_put_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComObj_INTERFACE_DEFINED__ */


DEFINE_GUID(CLSID_Document,0x3F315842,0x67AC,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#ifdef __cplusplus

class DECLSPEC_UUID("3F315842-67AC-11D2-AE2A-00C0F03014A5")
Document;
#endif

DEFINE_GUID(CLSID_ComMine,0x8B31FC82,0x69E2,0x11D2,0x8F,0x0F,0x00,0x60,0xB0,0xB4,0x15,0x34);

#ifdef __cplusplus

class DECLSPEC_UUID("8B31FC82-69E2-11D2-8F0F-0060B0B41534")
ComMine;
#endif

DEFINE_GUID(CLSID_ComCube,0x5DF6BE61,0x6A08,0x11d2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#ifdef __cplusplus

class DECLSPEC_UUID("5DF6BE61-6A08-11d2-AE2A-00C0F03014A5")
ComCube;
#endif

DEFINE_GUID(CLSID_ComObj,0xA9C67592,0x6DF0,0x11D2,0xAE,0x2A,0x00,0xC0,0xF0,0x30,0x14,0xA5);

#ifdef __cplusplus

class DECLSPEC_UUID("A9C67592-6DF0-11D2-AE2A-00C0F03014A5")
ComObj;
#endif
#endif /* __dlc_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


