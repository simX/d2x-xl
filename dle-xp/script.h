// script.h
#include "ActivScp.h"

// The GUID used to identify the coclass of the VB Script engine
// {B54F3741-5B07-11cf-A4B0-00AA004A55E8}
#define szCLSID_VBScript "{B54F3741-5B07-11cf-A4B0-00AA004A55E8}"
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);

//---------------------------------------------------------------------------
// Allocates a temporary buffer that will disappear when it goes out of scope
// NOTE: Be careful of that-- make sure you use the string in the same or
// nested scope in which you created this buffer.  People should not use this
// class directly; use the macro(s) below.
//---------------------------------------------------------------------------
class TempBuffer
  {
  public:
    TempBuffer(ULONG cb)
      {
      m_fAlloc = (cb > 120);
      if (m_fAlloc)
        m_pbBuf = new char[cb];
      else
        m_pbBuf = &m_szBufT;
      }
    ~TempBuffer()
      { if (m_pbBuf && m_fAlloc) delete m_pbBuf; }
    void *GetBuffer(void)
      { return m_pbBuf; }

  private:
    void *m_pbBuf;
    char  m_szBufT[120];  // We'll use this temp buffer for small cases.
    int   m_fAlloc;
  };



#define MAKE_WIDEPTR_FROMANSI(ptrname, pszAnsi) \
    char *__psz##ptrname = pszAnsi?pszAnsi:""; \
    long __l##ptrname = (lstrlen(__psz##ptrname) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, __psz##ptrname, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

class CScript : public IDispatch, public IActiveScriptSite
{
public:

	// *** General Methods ***
	CScript();
	HRESULT CreateScriptEngine(LPCOLESTR pstrItemName);

	// *** Member Variables ***
	UINT					m_cref;
	IActiveScript*			m_ps;
	IActiveScriptParse*		m_psp;
	CLSID					m_clsidEngine;
	char*					s_pszError;


  // *** IUnknown methods ***
  STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
  STDMETHOD_(ULONG, AddRef)(void);
  STDMETHOD_(ULONG, Release)(void);
  inline IUnknown  *GetUnknown(void)  {return (IDispatch *)this;};
  inline IDispatch *GetDispatch(void) {return (IDispatch *)this;};

  // *** IDispatch methods ***
  STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
  STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfoOut);
  STDMETHOD(GetIDsOfNames)(REFIID iid, OLECHAR **rgszNames, UINT cnames, LCID lcid, DISPID *rgdispid);
  STDMETHOD(Invoke)(DISPID dispid, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);

  // *** IActiveScriptSite methods ***
  STDMETHOD(GetLCID)(LCID *plcid);
  STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
  STDMETHOD(GetDocVersionString)(BSTR *pszVersion);
  STDMETHOD(RequestItems)(void);
  STDMETHOD(RequestTypeLibs)(void);
  STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
  STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);
  STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);
  STDMETHOD(OnEnterScript)(void);
  STDMETHOD(OnLeaveScript)(void);

  // *** IActiveScriptSiteWindow methods ***
  STDMETHOD(GetWindow)(HWND *phwnd);
  STDMETHOD(EnableModeless)(BOOL fEnable);

};

