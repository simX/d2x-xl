// ComMine.h : Declaration of the CComMine

#ifndef __COMMINE_H_
#define __COMMINE_H_

#include "dlcres.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CComMine
class ATL_NO_VTABLE CComMine : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComMine, &CLSID_ComMine>,
	public IDispatchImpl<IComMine, &IID_IComMine, &LIBID_dlc>
{
public:
	CComMine()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMMINE)

BEGIN_COM_MAP(CComMine)
	COM_INTERFACE_ENTRY(IComMine)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IComMine
public:
	STDMETHOD(InfoMessage)(BSTR strValue);
	STDMETHOD(get_NumberOfCubes)(long* pRetVal);
	STDMETHOD(get_NumberOfPoints)(long* pRetVal);
	STDMETHOD(get_NumberOfWalls)(long* pRetVal);
	STDMETHOD(get_NumberOfTriggers)(long* pRetVal);
	STDMETHOD(get_NumberOfObjects)(long* pRetVal);
	STDMETHOD(get_Cube)(VARIANT index, LPDISPATCH *pVal);
	STDMETHOD(get_Object)(VARIANT index, LPDISPATCH* pVal);


// Data members
public:
	CMine* _pMine;
};

#endif //__COMMINE_H_
