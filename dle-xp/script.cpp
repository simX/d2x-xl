// script.cpp

#include "stdafx.h"
#include "script.h"

CScript::CScript()
{
	m_cref		= 1;
	m_ps		= NULL;
	m_psp		= NULL;
//	m_clsidEngine = CLSID_VBScript;
	s_pszError	= NULL;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HRESULT CScript::CreateScriptEngine(LPCOLESTR pstrItemName)
{
	HRESULT hr;
	
	if (m_ps)
		return S_FALSE;   // Already created it
	
	// Create the ActiveX Scripting Engine
	MAKE_WIDEPTR_FROMANSI(pwszCLSID, szCLSID_VBScript);
	hr = CLSIDFromString(pwszCLSID, &m_clsidEngine);
	if (hr)
	{
		s_pszError = "Bad $ENGINE={xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx} syntax";
		return E_FAIL;
	}

	hr = CoCreateInstance(m_clsidEngine, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_ps);
	if (hr)
	{
		s_pszError = "Creating the ActiveX Scripting engine failed.  Scripting engine is probably not correctly registered or CLSID incorrect.";
		return E_FAIL;
	}
	// Script Engine must support IActiveScriptParse for us to use it
	hr = m_ps->QueryInterface(IID_IActiveScriptParse, (void **)&m_psp);
	if (hr)
	{
		s_pszError = "ActiveX Scripting engine does not support IActiveScriptParse";
		return hr;
	}
	hr = m_ps->SetScriptSite(this);
	if (hr)
		return hr;
	// InitNew the object:
	hr = m_psp->InitNew();
	if (hr)
		return hr;
	hr = m_ps->AddNamedItem(L"Level", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
	return hr;
}


//***************************************************************************
// IActiveScriptSite Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetLCID(LCID *plcid)
{
	return E_NOTIMPL;	  // Use system settings
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetItemInfo
(
 LPCOLESTR	 pstrName,
 DWORD		 dwReturnMask,
 IUnknown**  ppunkItemOut,
 ITypeInfo** pptinfoOut
 )
{
//	HRESULT hr;
	
	if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
	{
		if (!pptinfoOut)
			return E_INVALIDARG;
		*pptinfoOut = NULL;
	}
	
	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	{
		if (!ppunkItemOut)
			return E_INVALIDARG;
		*ppunkItemOut = NULL;
	}
	
//	if (!_wcsicmp(L"Level", pstrName))
//	{
//		if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
//		{
//			hr = ((COleAuto *)this)->CheckTypeInfo(0, 0x0409);
//			if (hr)
//				return hr;
//			*pptinfoOut = g_ptinfoClsGame;
//			(*pptinfoOut)->AddRef();	  // because returning
//		}
//		if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
//		{
//			*ppunkItemOut = this->GetUnknown();
//			(*ppunkItemOut)->AddRef();	  // because returning
//		}
//		return S_OK;
//	}
	
	return TYPE_E_ELEMENTNOTFOUND;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetDocVersionString
(
 BSTR *pbstrVersion
 )
{
	return E_NOTIMPL;	// UNDONE: Implement this method
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::RequestItems(void)
{
	return m_ps->AddNamedItem(L"Level", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::RequestTypeLibs(void)
{
//	return m_ps->AddTypeLib(LIBID_SPRUUIDS, 1, 0, 0);
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::OnScriptTerminate
(
 const VARIANT	 *pvarResult,
 const EXCEPINFO *pexcepinfo
 )
{
	// UNDONE: Put up error dlg here
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::OnStateChange
(
 SCRIPTSTATE ssScriptState
 )
{
	// Don't care about notification
	return S_OK;
}


//---------------------------------------------------------------------------
// Display the error
//---------------------------------------------------------------------------
STDMETHODIMP CScript::OnScriptError(IActiveScriptError *pse)
{
/*
	char	  szError[1024];
	char	 *pszArrow = NULL;
	BOOL	  fAlloc = FALSE;
	EXCEPINFO ei;
	DWORD	  dwSrcContext;
	ULONG	  ulLine;
	LONG	  ichError;
	BSTR	  bstrLine = NULL;
	HRESULT   hr;
	
	CHECK(pse->GetExceptionInfo(&ei));
	CHECK(pse->GetSourcePosition(&dwSrcContext, &ulLine, &ichError));
	ulLine += g_clineOffset;	// Adjust for $ENGINE/$OBJECT/etc. lines
	hr = pse->GetSourceLineText(&bstrLine);
	if (hr)
		hr = S_OK;	// Ignore this error, there may not be source available
	
	if (!hr)
	{
		MAKE_ANSIPTR_FROMWIDE(pszSrc,  ei.bstrSource);
		MAKE_ANSIPTR_FROMWIDE(pszDesc, ei.bstrDescription);
		MAKE_ANSIPTR_FROMWIDE(pszLine, bstrLine);
		if (ichError > 0 && ichError < 255)
		{
			pszArrow = new char[ichError+1];
			memset(pszArrow, '-', ichError);
			pszArrow[ichError-1] = 'v';
			pszArrow[ichError]	 = 0;
			fAlloc = TRUE;
		}
		else
			pszArrow = "";
		
		wsprintf(szError, "Source:'%s'\nFile:'%s'  Line:%d  Char:%d\nError:%d  '%s'\n%s\n%s",
			pszSrc, g_pszCodeFile, ulLine, ichError,
			(int)ei.wCode, pszDesc, pszArrow, pszLine);
		DisplayScriptError(g_hinst, m_hwndDlg, szError, ichError);
	}
	
CleanUp:
	if (fAlloc)
		delete [] pszArrow;
	if (bstrLine)
		SysFreeString(bstrLine);
	
	return hr;
*/
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::OnEnterScript
(
 void 
 )
{
	// No need to do anything
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::OnLeaveScript
(
 void 
 )
{
	// No need to do anything
	return S_OK;
}



//***************************************************************************
// IActiveScriptSiteWindow Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetWindow(HWND *phwndOut)
{
	if (!phwndOut)
		return E_INVALIDARG;
	*phwndOut = NULL;
//	*phwndOut = m_hWnd;
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::EnableModeless(BOOL fEnable)
{
//	if (fEnable)
//		m_pcm->FOnComponentExitState(g_papp->m_idcomp, cmgrstateModal, cmgrcontextAll, 0, NULL);
//	else
//		m_pcm->OnComponentEnterState(g_papp->m_idcomp, cmgrstateModal, cmgrcontextAll, 0, NULL, NULL);
	return S_OK;
}

//***************************************************************************
// IUnknown Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::QueryInterface
(
  REFIID  iid,
  LPVOID* ppvObjOut
)
{
  if (!ppvObjOut)
    return E_INVALIDARG;

  *ppvObjOut = NULL;

  if (iid == IID_IUnknown)
    *ppvObjOut = this->GetUnknown();
  else if (iid == IID_IDispatch)
    *ppvObjOut = this->GetDispatch();
//  else if (iid == IID_IGameSubObjects)
//    *ppvObjOut = this->GetDispatch();
  else if (iid == IID_IActiveScriptSite)
    *ppvObjOut = (IActiveScriptSite *)this;
//  else if (iid == IID_IActiveScriptSiteWindow)
//    *ppvObjOut = (IActiveScriptSiteWindow *)this;
//  else if (iid == IID_IProvideClassInfo)
//    *ppvObjOut = (IProvideClassInfo *)this;
//  else if (iid == IID_IProvideMultipleClassInfo)
//    *ppvObjOut = (IProvideMultipleClassInfo *)this;
//  else if (iid == IID_IGame)
//    *ppvObjOut = (IGame *)(this->m_pgameoa);
//  else if (iid == IID_IConnectionPointContainer)
//    *ppvObjOut = (IConnectionPointContainer *)(this->m_pgameoa);

  if (*ppvObjOut)
    {
    this->AddRef();
    return S_OK;
    }

  return E_NOINTERFACE;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CScript::AddRef
(
  void 
)
{
  return ++m_cref;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CScript::Release
(
  void 
)
{
//  ASSERT(m_cref, "bad m_cref");
	m_cref--;
	if (!m_cref)
	{
//		m_pgameoa->m_cp.Close();     // Make sure nobody's holding on to us because we're holding their sink
//		if (!m_cref && !m_pgameoa->m_cp.m_cref)
		{
			delete this;
			return 0;
		}
	}
	return m_cref;
}



//***************************************************************************
// IDispatch Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetTypeInfoCount(UINT* pctinfo)
{
	// Validate args
	if (!pctinfo)
		return E_INVALIDARG;
	
	// Return requested info
	*pctinfo = 1;
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfoOut)
{
	HRESULT hr = S_OK;
	
	// Validate args
	if (!pptinfoOut)
		return E_INVALIDARG;
	*pptinfoOut = NULL;
	
	// Make sure we have the TypeInfo
//	CHECK(this->CheckTypeInfo(itinfo, lcid));
	
	// Return requested info
//	*pptinfoOut = *this->GetTinfoIntAddr();
	(*pptinfoOut)->AddRef();	// For *pptiOut
	
//CleanUp:
	return hr;
}


//---------------------------------------------------------------------------
// Re-implement CScript's GetIDsOfNames(), so we can implement the dynamic
// properties we added for the $OBJECT=SpriteClassName lines.
//---------------------------------------------------------------------------
STDMETHODIMP CScript::GetIDsOfNames(
  REFIID    iid,
  OLECHAR** rgszNames,
  UINT      cNames,
  LCID      lcid,
  DISPID*   prgdispid
)
{
  // Validate Args
  if (iid != IID_NULL)
      return E_INVALIDARG;

/*
  // See if a sub-object matches the name passed in.
  for (int i=0; i<m_csubobj; i++)
    if (!wcscmp(*rgszNames, m_rgpwszSubObjs[i]))
      {
      if (cNames != 1)
        return DISP_E_NONAMEDARGS;
      *prgdispid = 0x80010000 | i;
      return S_OK;
      }
  return COleAuto::GetIDsOfNames(iid, rgszNames, cNames, lcid, prgdispid);
*/
  return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CScript::Invoke(
  DISPID      dispid,
  REFIID      riid,
  LCID        lcid,
  WORD        wFlags,
  DISPPARAMS* pdispparams,
  VARIANT*    pvarResult,
  EXCEPINFO*  pexcepinfo, 
  UINT*       puArgErr)
{

  IDispatch *pdisp = NULL;
/*
  // Validate Args
  if (pvarResult)
    VariantInit(pvarResult);
  if (puArgErr)
    *puArgErr = 0;
  SetErrorInfo(0L, NULL);

  // If dispid isn't a dynamic property, call COleAuto's dispatch,
  // and it will forward the request on to m_pdispBaseObject.
  if ((dispid & 0x80010000) != 0x80010000)
    return COleAuto::Invoke(dispid, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
  // It's for one of our property Objects ()...

  // Make sure we have either a PropGet or a Method
  if (!(wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET)))
    return DISP_E_MEMBERNOTFOUND;

  dispid &= 0xffff;
  if (dispid >= m_csubobj)
    return DISP_E_MEMBERNOTFOUND;

  if (pdispparams->cArgs)
    return DISP_E_BADPARAMCOUNT;

  if (pdispparams->cNamedArgs)
    return DISP_E_NONAMEDARGS;

  // Get an addref'd IDispatch to the property object
  pdisp = m_rgpdispSubObjs[dispid];
  ASSERT(pdisp, "NULL m_rgpdispSubObjs[]");

  if (pvarResult)	
    {
    pvarResult->vt = VT_DISPATCH;
    pvarResult->pdispVal = pdisp;
    pdisp->AddRef();		// We're returning this disp ptr
    }
*/
  return S_OK;
}


