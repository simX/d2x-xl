// ComObj.h : Declaration of the CComObj

#ifndef __COMOBJ_H_
#define __COMOBJ_H_

#include "dlcres.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CComObj
class ATL_NO_VTABLE CComObj : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComObj, &CLSID_ComObj>,
	public IDispatchImpl<IComObj, &IID_IComObj, &LIBID_dlc>
{
public:
	CComObj()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMOBJ)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CComObj)
	COM_INTERFACE_ENTRY(IComObj)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IComObj
public:
	STDMETHOD(get_Id)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Id)(/*[in]*/ long newVal);
	STDMETHOD(get_Type)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Type)(/*[in]*/ long newVal);

// Data members
public:
	CDObject* _pObject;
};

#endif //__COMOBJ_H_
