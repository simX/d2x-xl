/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Thu Oct 01 13:01:48 1998
 */
/* Compiler settings for test.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __test_h__
#define __test_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IScriptError_FWD_DEFINED__
#define __IScriptError_FWD_DEFINED__
typedef interface IScriptError IScriptError;
#endif 	/* __IScriptError_FWD_DEFINED__ */


#ifndef __IScriptProcedure_FWD_DEFINED__
#define __IScriptProcedure_FWD_DEFINED__
typedef interface IScriptProcedure IScriptProcedure;
#endif 	/* __IScriptProcedure_FWD_DEFINED__ */


#ifndef __IScriptProcedureCollection_FWD_DEFINED__
#define __IScriptProcedureCollection_FWD_DEFINED__
typedef interface IScriptProcedureCollection IScriptProcedureCollection;
#endif 	/* __IScriptProcedureCollection_FWD_DEFINED__ */


#ifndef __IScriptModule_FWD_DEFINED__
#define __IScriptModule_FWD_DEFINED__
typedef interface IScriptModule IScriptModule;
#endif 	/* __IScriptModule_FWD_DEFINED__ */


#ifndef __IScriptModuleCollection_FWD_DEFINED__
#define __IScriptModuleCollection_FWD_DEFINED__
typedef interface IScriptModuleCollection IScriptModuleCollection;
#endif 	/* __IScriptModuleCollection_FWD_DEFINED__ */


#ifndef __IScriptControl_FWD_DEFINED__
#define __IScriptControl_FWD_DEFINED__
typedef interface IScriptControl IScriptControl;
#endif 	/* __IScriptControl_FWD_DEFINED__ */


#ifndef __ScriptControl_FWD_DEFINED__
#define __ScriptControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class ScriptControl ScriptControl;
#else
typedef struct ScriptControl ScriptControl;
#endif /* __cplusplus */

#endif 	/* __ScriptControl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_test_0000
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 


typedef /* [public][public][public][helpcontext][helpstring] */ 
enum __MIDL___MIDL_itf_test_0000_0001
    {	Initialized	= 0,
	Connected	= 1
    }	ScriptControlStates;



extern RPC_IF_HANDLE __MIDL_itf_test_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_test_0000_v0_0_s_ifspec;

#ifndef __IScriptError_INTERFACE_DEFINED__
#define __IScriptError_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptError
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("70841C78-067D-11D0-95D8-00A02463AB28")
    IScriptError : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Number( 
            /* [retval][out] */ long __RPC_FAR *plNumber) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Source( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrSource) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Description( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_HelpFile( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrHelpFile) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_HelpContext( 
            /* [retval][out] */ long __RPC_FAR *plHelpContext) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Text( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrText) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Line( 
            /* [retval][out] */ long __RPC_FAR *plLine) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Column( 
            /* [retval][out] */ long __RPC_FAR *plColumn) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall Clear( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptError __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptError __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptError __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Number )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plNumber);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Source )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrSource);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Description )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_HelpFile )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrHelpFile);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_HelpContext )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plHelpContext);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Text )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrText);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Line )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plLine);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Column )( 
            IScriptError __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plColumn);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *Clear )( 
            IScriptError __RPC_FAR * This);
        
        END_INTERFACE
    } IScriptErrorVtbl;

    interface IScriptError
    {
        CONST_VTBL struct IScriptErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptError_get_Number(This,plNumber)	\
    (This)->lpVtbl -> get_Number(This,plNumber)

#define IScriptError_get_Source(This,pbstrSource)	\
    (This)->lpVtbl -> get_Source(This,pbstrSource)

#define IScriptError_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IScriptError_get_HelpFile(This,pbstrHelpFile)	\
    (This)->lpVtbl -> get_HelpFile(This,pbstrHelpFile)

#define IScriptError_get_HelpContext(This,plHelpContext)	\
    (This)->lpVtbl -> get_HelpContext(This,plHelpContext)

#define IScriptError_get_Text(This,pbstrText)	\
    (This)->lpVtbl -> get_Text(This,pbstrText)

#define IScriptError_get_Line(This,plLine)	\
    (This)->lpVtbl -> get_Line(This,plLine)

#define IScriptError_get_Column(This,plColumn)	\
    (This)->lpVtbl -> get_Column(This,plColumn)

#define IScriptError_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Number_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plNumber);


void __RPC_STUB IScriptError_get_Number_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Source_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrSource);


void __RPC_STUB IScriptError_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Description_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IScriptError_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_HelpFile_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrHelpFile);


void __RPC_STUB IScriptError_get_HelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_HelpContext_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plHelpContext);


void __RPC_STUB IScriptError_get_HelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Text_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrText);


void __RPC_STUB IScriptError_get_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Line_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plLine);


void __RPC_STUB IScriptError_get_Line_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptError_get_Column_Proxy( 
    IScriptError __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plColumn);


void __RPC_STUB IScriptError_get_Column_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptError_Clear_Proxy( 
    IScriptError __RPC_FAR * This);


void __RPC_STUB IScriptError_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptError_INTERFACE_DEFINED__ */


#ifndef __IScriptProcedure_INTERFACE_DEFINED__
#define __IScriptProcedure_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptProcedure
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptProcedure;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("70841C73-067D-11D0-95D8-00A02463AB28")
    IScriptProcedure : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_NumArgs( 
            /* [retval][out] */ long __RPC_FAR *pcArgs) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_HasReturnValue( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfHasReturnValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptProcedureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptProcedure __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptProcedure __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptProcedure __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptProcedure __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptProcedure __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptProcedure __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptProcedure __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Name )( 
            IScriptProcedure __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_NumArgs )( 
            IScriptProcedure __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pcArgs);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_HasReturnValue )( 
            IScriptProcedure __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfHasReturnValue);
        
        END_INTERFACE
    } IScriptProcedureVtbl;

    interface IScriptProcedure
    {
        CONST_VTBL struct IScriptProcedureVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptProcedure_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptProcedure_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptProcedure_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptProcedure_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptProcedure_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptProcedure_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptProcedure_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptProcedure_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IScriptProcedure_get_NumArgs(This,pcArgs)	\
    (This)->lpVtbl -> get_NumArgs(This,pcArgs)

#define IScriptProcedure_get_HasReturnValue(This,pfHasReturnValue)	\
    (This)->lpVtbl -> get_HasReturnValue(This,pfHasReturnValue)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptProcedure_get_Name_Proxy( 
    IScriptProcedure __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IScriptProcedure_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptProcedure_get_NumArgs_Proxy( 
    IScriptProcedure __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pcArgs);


void __RPC_STUB IScriptProcedure_get_NumArgs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptProcedure_get_HasReturnValue_Proxy( 
    IScriptProcedure __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfHasReturnValue);


void __RPC_STUB IScriptProcedure_get_HasReturnValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptProcedure_INTERFACE_DEFINED__ */


#ifndef __IScriptProcedureCollection_INTERFACE_DEFINED__
#define __IScriptProcedureCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptProcedureCollection
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptProcedureCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("70841C71-067D-11D0-95D8-00A02463AB28")
    IScriptProcedureCollection : public IDispatch
    {
    public:
        virtual /* [hidden][propget][id] */ HRESULT __stdcall get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumProcedures) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Item( 
            /* [in] */ VARIANT Index,
            /* [retval][out] */ IScriptProcedure __RPC_FAR *__RPC_FAR *ppdispProcedure) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Count( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptProcedureCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptProcedureCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptProcedureCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [hidden][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumProcedures);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [in] */ VARIANT Index,
            /* [retval][out] */ IScriptProcedure __RPC_FAR *__RPC_FAR *ppdispProcedure);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Count )( 
            IScriptProcedureCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        END_INTERFACE
    } IScriptProcedureCollectionVtbl;

    interface IScriptProcedureCollection
    {
        CONST_VTBL struct IScriptProcedureCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptProcedureCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptProcedureCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptProcedureCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptProcedureCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptProcedureCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptProcedureCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptProcedureCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptProcedureCollection_get__NewEnum(This,ppenumProcedures)	\
    (This)->lpVtbl -> get__NewEnum(This,ppenumProcedures)

#define IScriptProcedureCollection_get_Item(This,Index,ppdispProcedure)	\
    (This)->lpVtbl -> get_Item(This,Index,ppdispProcedure)

#define IScriptProcedureCollection_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [hidden][propget][id] */ HRESULT __stdcall IScriptProcedureCollection_get__NewEnum_Proxy( 
    IScriptProcedureCollection __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumProcedures);


void __RPC_STUB IScriptProcedureCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptProcedureCollection_get_Item_Proxy( 
    IScriptProcedureCollection __RPC_FAR * This,
    /* [in] */ VARIANT Index,
    /* [retval][out] */ IScriptProcedure __RPC_FAR *__RPC_FAR *ppdispProcedure);


void __RPC_STUB IScriptProcedureCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptProcedureCollection_get_Count_Proxy( 
    IScriptProcedureCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB IScriptProcedureCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptProcedureCollection_INTERFACE_DEFINED__ */


#ifndef __IScriptModule_INTERFACE_DEFINED__
#define __IScriptModule_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptModule
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptModule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("70841C70-067D-11D0-95D8-00A02463AB28")
    IScriptModule : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Name( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_CodeObject( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Procedures( 
            /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall AddCode( 
            /* [in] */ BSTR Code) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall Eval( 
            /* [in] */ BSTR Expression,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall ExecuteStatement( 
            /* [in] */ BSTR Statement) = 0;
        
        virtual /* [helpcontext][helpstring][vararg][id] */ HRESULT __stdcall Run( 
            /* [in] */ BSTR ProcedureName,
            /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptModuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptModule __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptModule __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptModule __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Name )( 
            IScriptModule __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_CodeObject )( 
            IScriptModule __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Procedures )( 
            IScriptModule __RPC_FAR * This,
            /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *AddCode )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ BSTR Code);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *Eval )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ BSTR Expression,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *ExecuteStatement )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ BSTR Statement);
        
        /* [helpcontext][helpstring][vararg][id] */ HRESULT ( __stdcall __RPC_FAR *Run )( 
            IScriptModule __RPC_FAR * This,
            /* [in] */ BSTR ProcedureName,
            /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);
        
        END_INTERFACE
    } IScriptModuleVtbl;

    interface IScriptModule
    {
        CONST_VTBL struct IScriptModuleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptModule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptModule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptModule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptModule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptModule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptModule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptModule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptModule_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IScriptModule_get_CodeObject(This,ppdispObject)	\
    (This)->lpVtbl -> get_CodeObject(This,ppdispObject)

#define IScriptModule_get_Procedures(This,ppdispProcedures)	\
    (This)->lpVtbl -> get_Procedures(This,ppdispProcedures)

#define IScriptModule_AddCode(This,Code)	\
    (This)->lpVtbl -> AddCode(This,Code)

#define IScriptModule_Eval(This,Expression,pvarResult)	\
    (This)->lpVtbl -> Eval(This,Expression,pvarResult)

#define IScriptModule_ExecuteStatement(This,Statement)	\
    (This)->lpVtbl -> ExecuteStatement(This,Statement)

#define IScriptModule_Run(This,ProcedureName,Parameters,pvarResult)	\
    (This)->lpVtbl -> Run(This,ProcedureName,Parameters,pvarResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptModule_get_Name_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IScriptModule_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptModule_get_CodeObject_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject);


void __RPC_STUB IScriptModule_get_CodeObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptModule_get_Procedures_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures);


void __RPC_STUB IScriptModule_get_Procedures_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptModule_AddCode_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [in] */ BSTR Code);


void __RPC_STUB IScriptModule_AddCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptModule_Eval_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [in] */ BSTR Expression,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);


void __RPC_STUB IScriptModule_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptModule_ExecuteStatement_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [in] */ BSTR Statement);


void __RPC_STUB IScriptModule_ExecuteStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][vararg][id] */ HRESULT __stdcall IScriptModule_Run_Proxy( 
    IScriptModule __RPC_FAR * This,
    /* [in] */ BSTR ProcedureName,
    /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);


void __RPC_STUB IScriptModule_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptModule_INTERFACE_DEFINED__ */


#ifndef __IScriptModuleCollection_INTERFACE_DEFINED__
#define __IScriptModuleCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptModuleCollection
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptModuleCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("70841C6F-067D-11D0-95D8-00A02463AB28")
    IScriptModuleCollection : public IDispatch
    {
    public:
        virtual /* [hidden][propget][id] */ HRESULT __stdcall get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumContexts) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Item( 
            /* [in] */ VARIANT Index,
            /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Count( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall Add( 
            /* [in] */ BSTR Name,
            /* [optional][in] */ VARIANT __RPC_FAR *Object,
            /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptModuleCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptModuleCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptModuleCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [hidden][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get__NewEnum )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumContexts);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Item )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ VARIANT Index,
            /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Count )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *Add )( 
            IScriptModuleCollection __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [optional][in] */ VARIANT __RPC_FAR *Object,
            /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod);
        
        END_INTERFACE
    } IScriptModuleCollectionVtbl;

    interface IScriptModuleCollection
    {
        CONST_VTBL struct IScriptModuleCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptModuleCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptModuleCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptModuleCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptModuleCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptModuleCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptModuleCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptModuleCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptModuleCollection_get__NewEnum(This,ppenumContexts)	\
    (This)->lpVtbl -> get__NewEnum(This,ppenumContexts)

#define IScriptModuleCollection_get_Item(This,Index,ppmod)	\
    (This)->lpVtbl -> get_Item(This,Index,ppmod)

#define IScriptModuleCollection_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IScriptModuleCollection_Add(This,Name,Object,ppmod)	\
    (This)->lpVtbl -> Add(This,Name,Object,ppmod)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [hidden][propget][id] */ HRESULT __stdcall IScriptModuleCollection_get__NewEnum_Proxy( 
    IScriptModuleCollection __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenumContexts);


void __RPC_STUB IScriptModuleCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptModuleCollection_get_Item_Proxy( 
    IScriptModuleCollection __RPC_FAR * This,
    /* [in] */ VARIANT Index,
    /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod);


void __RPC_STUB IScriptModuleCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptModuleCollection_get_Count_Proxy( 
    IScriptModuleCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB IScriptModuleCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptModuleCollection_Add_Proxy( 
    IScriptModuleCollection __RPC_FAR * This,
    /* [in] */ BSTR Name,
    /* [optional][in] */ VARIANT __RPC_FAR *Object,
    /* [retval][out] */ IScriptModule __RPC_FAR *__RPC_FAR *ppmod);


void __RPC_STUB IScriptModuleCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptModuleCollection_INTERFACE_DEFINED__ */


#ifndef __IScriptControl_INTERFACE_DEFINED__
#define __IScriptControl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IScriptControl
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][nonextensible][dual][hidden][helpcontext][helpstring][uuid] */ 



EXTERN_C const IID IID_IScriptControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0E59F1D3-1FBE-11D0-8FF2-00A0D10038BC")
    IScriptControl : public IDispatch
    {
    public:
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Language( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrLanguage) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_Language( 
            /* [in] */ BSTR pbstrLanguage) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_State( 
            /* [retval][out] */ ScriptControlStates __RPC_FAR *pssState) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_State( 
            /* [in] */ ScriptControlStates pssState) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_SitehWnd( 
            /* [in] */ long phwnd) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_SitehWnd( 
            /* [retval][out] */ long __RPC_FAR *phwnd) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Timeout( 
            /* [retval][out] */ long __RPC_FAR *plMilleseconds) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_Timeout( 
            /* [in] */ long plMilleseconds) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_AllowUI( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfAllowUI) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_AllowUI( 
            /* [in] */ VARIANT_BOOL pfAllowUI) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_UseSafeSubset( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfUseSafeSubset) = 0;
        
        virtual /* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall put_UseSafeSubset( 
            /* [in] */ VARIANT_BOOL pfUseSafeSubset) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Modules( 
            /* [retval][out] */ IScriptModuleCollection __RPC_FAR *__RPC_FAR *ppmods) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Error( 
            /* [retval][out] */ IScriptError __RPC_FAR *__RPC_FAR *ppse) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_CodeObject( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject) = 0;
        
        virtual /* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall get_Procedures( 
            /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall _AboutBox( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall AddObject( 
            /* [in] */ BSTR Name,
            /* [in] */ IDispatch __RPC_FAR *Object,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL AddMembers) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall Reset( void) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall AddCode( 
            /* [in] */ BSTR Code) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall Eval( 
            /* [in] */ BSTR Expression,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult) = 0;
        
        virtual /* [helpcontext][helpstring][id] */ HRESULT __stdcall ExecuteStatement( 
            /* [in] */ BSTR Statement) = 0;
        
        virtual /* [helpcontext][helpstring][vararg][id] */ HRESULT __stdcall Run( 
            /* [in] */ BSTR ProcedureName,
            /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IScriptControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IScriptControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IScriptControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IScriptControl __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Language )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrLanguage);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_Language )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR pbstrLanguage);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_State )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ ScriptControlStates __RPC_FAR *pssState);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_State )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ ScriptControlStates pssState);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_SitehWnd )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ long phwnd);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_SitehWnd )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *phwnd);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Timeout )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plMilleseconds);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_Timeout )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ long plMilleseconds);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_AllowUI )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfAllowUI);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_AllowUI )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pfAllowUI);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_UseSafeSubset )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfUseSafeSubset);
        
        /* [helpcontext][helpstring][propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_UseSafeSubset )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL pfUseSafeSubset);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Modules )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ IScriptModuleCollection __RPC_FAR *__RPC_FAR *ppmods);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Error )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ IScriptError __RPC_FAR *__RPC_FAR *ppse);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_CodeObject )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject);
        
        /* [helpcontext][helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Procedures )( 
            IScriptControl __RPC_FAR * This,
            /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *_AboutBox )( 
            IScriptControl __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *AddObject )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR Name,
            /* [in] */ IDispatch __RPC_FAR *Object,
            /* [defaultvalue][optional][in] */ VARIANT_BOOL AddMembers);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IScriptControl __RPC_FAR * This);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *AddCode )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR Code);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *Eval )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR Expression,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);
        
        /* [helpcontext][helpstring][id] */ HRESULT ( __stdcall __RPC_FAR *ExecuteStatement )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR Statement);
        
        /* [helpcontext][helpstring][vararg][id] */ HRESULT ( __stdcall __RPC_FAR *Run )( 
            IScriptControl __RPC_FAR * This,
            /* [in] */ BSTR ProcedureName,
            /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);
        
        END_INTERFACE
    } IScriptControlVtbl;

    interface IScriptControl
    {
        CONST_VTBL struct IScriptControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IScriptControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IScriptControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IScriptControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IScriptControl_get_Language(This,pbstrLanguage)	\
    (This)->lpVtbl -> get_Language(This,pbstrLanguage)

#define IScriptControl_put_Language(This,pbstrLanguage)	\
    (This)->lpVtbl -> put_Language(This,pbstrLanguage)

#define IScriptControl_get_State(This,pssState)	\
    (This)->lpVtbl -> get_State(This,pssState)

#define IScriptControl_put_State(This,pssState)	\
    (This)->lpVtbl -> put_State(This,pssState)

#define IScriptControl_put_SitehWnd(This,phwnd)	\
    (This)->lpVtbl -> put_SitehWnd(This,phwnd)

#define IScriptControl_get_SitehWnd(This,phwnd)	\
    (This)->lpVtbl -> get_SitehWnd(This,phwnd)

#define IScriptControl_get_Timeout(This,plMilleseconds)	\
    (This)->lpVtbl -> get_Timeout(This,plMilleseconds)

#define IScriptControl_put_Timeout(This,plMilleseconds)	\
    (This)->lpVtbl -> put_Timeout(This,plMilleseconds)

#define IScriptControl_get_AllowUI(This,pfAllowUI)	\
    (This)->lpVtbl -> get_AllowUI(This,pfAllowUI)

#define IScriptControl_put_AllowUI(This,pfAllowUI)	\
    (This)->lpVtbl -> put_AllowUI(This,pfAllowUI)

#define IScriptControl_get_UseSafeSubset(This,pfUseSafeSubset)	\
    (This)->lpVtbl -> get_UseSafeSubset(This,pfUseSafeSubset)

#define IScriptControl_put_UseSafeSubset(This,pfUseSafeSubset)	\
    (This)->lpVtbl -> put_UseSafeSubset(This,pfUseSafeSubset)

#define IScriptControl_get_Modules(This,ppmods)	\
    (This)->lpVtbl -> get_Modules(This,ppmods)

#define IScriptControl_get_Error(This,ppse)	\
    (This)->lpVtbl -> get_Error(This,ppse)

#define IScriptControl_get_CodeObject(This,ppdispObject)	\
    (This)->lpVtbl -> get_CodeObject(This,ppdispObject)

#define IScriptControl_get_Procedures(This,ppdispProcedures)	\
    (This)->lpVtbl -> get_Procedures(This,ppdispProcedures)

#define IScriptControl__AboutBox(This)	\
    (This)->lpVtbl -> _AboutBox(This)

#define IScriptControl_AddObject(This,Name,Object,AddMembers)	\
    (This)->lpVtbl -> AddObject(This,Name,Object,AddMembers)

#define IScriptControl_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IScriptControl_AddCode(This,Code)	\
    (This)->lpVtbl -> AddCode(This,Code)

#define IScriptControl_Eval(This,Expression,pvarResult)	\
    (This)->lpVtbl -> Eval(This,Expression,pvarResult)

#define IScriptControl_ExecuteStatement(This,Statement)	\
    (This)->lpVtbl -> ExecuteStatement(This,Statement)

#define IScriptControl_Run(This,ProcedureName,Parameters,pvarResult)	\
    (This)->lpVtbl -> Run(This,ProcedureName,Parameters,pvarResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_Language_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrLanguage);


void __RPC_STUB IScriptControl_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_Language_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR pbstrLanguage);


void __RPC_STUB IScriptControl_put_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_State_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ ScriptControlStates __RPC_FAR *pssState);


void __RPC_STUB IScriptControl_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_State_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ ScriptControlStates pssState);


void __RPC_STUB IScriptControl_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_SitehWnd_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ long phwnd);


void __RPC_STUB IScriptControl_put_SitehWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_SitehWnd_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *phwnd);


void __RPC_STUB IScriptControl_get_SitehWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_Timeout_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plMilleseconds);


void __RPC_STUB IScriptControl_get_Timeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_Timeout_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ long plMilleseconds);


void __RPC_STUB IScriptControl_put_Timeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_AllowUI_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfAllowUI);


void __RPC_STUB IScriptControl_get_AllowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_AllowUI_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pfAllowUI);


void __RPC_STUB IScriptControl_put_AllowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_UseSafeSubset_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfUseSafeSubset);


void __RPC_STUB IScriptControl_get_UseSafeSubset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propput][id] */ HRESULT __stdcall IScriptControl_put_UseSafeSubset_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL pfUseSafeSubset);


void __RPC_STUB IScriptControl_put_UseSafeSubset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_Modules_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ IScriptModuleCollection __RPC_FAR *__RPC_FAR *ppmods);


void __RPC_STUB IScriptControl_get_Modules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_Error_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ IScriptError __RPC_FAR *__RPC_FAR *ppse);


void __RPC_STUB IScriptControl_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_CodeObject_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispObject);


void __RPC_STUB IScriptControl_get_CodeObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][propget][id] */ HRESULT __stdcall IScriptControl_get_Procedures_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [retval][out] */ IScriptProcedureCollection __RPC_FAR *__RPC_FAR *ppdispProcedures);


void __RPC_STUB IScriptControl_get_Procedures_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IScriptControl__AboutBox_Proxy( 
    IScriptControl __RPC_FAR * This);


void __RPC_STUB IScriptControl__AboutBox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptControl_AddObject_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR Name,
    /* [in] */ IDispatch __RPC_FAR *Object,
    /* [defaultvalue][optional][in] */ VARIANT_BOOL AddMembers);


void __RPC_STUB IScriptControl_AddObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptControl_Reset_Proxy( 
    IScriptControl __RPC_FAR * This);


void __RPC_STUB IScriptControl_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptControl_AddCode_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR Code);


void __RPC_STUB IScriptControl_AddCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptControl_Eval_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR Expression,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);


void __RPC_STUB IScriptControl_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][id] */ HRESULT __stdcall IScriptControl_ExecuteStatement_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR Statement);


void __RPC_STUB IScriptControl_ExecuteStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpcontext][helpstring][vararg][id] */ HRESULT __stdcall IScriptControl_Run_Proxy( 
    IScriptControl __RPC_FAR * This,
    /* [in] */ BSTR ProcedureName,
    /* [in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *Parameters,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarResult);


void __RPC_STUB IScriptControl_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IScriptControl_INTERFACE_DEFINED__ */



#ifndef __MSScriptControl_LIBRARY_DEFINED__
#define __MSScriptControl_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: MSScriptControl
 * at Thu Oct 01 13:01:48 1998
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_MSScriptControl;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ScriptControl;

class DECLSPEC_UUID("0E59F1D5-1FBE-11D0-8FF2-00A0D10038BC")
ScriptControl;
#endif
#endif /* __MSScriptControl_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long __RPC_FAR *, unsigned long            , LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
