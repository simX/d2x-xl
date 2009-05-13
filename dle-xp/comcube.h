// ComCube.h : Declaration of the CComCube

#ifndef __COMCUBE_H_
#define __COMCUBE_H_

#include "dlcres.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CComCube
class ATL_NO_VTABLE CComCube : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComCube, &CLSID_ComCube>,
	public IDispatchImpl<IComCube, &IID_IComCube, &LIBID_dlc>
{
public:
	CComCube()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMCUBE)

BEGIN_COM_MAP(CComCube)
	COM_INTERFACE_ENTRY(IComCube)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IComCube
public:
	STDMETHOD(get_StaticLight)(long* pRetVal);
	STDMETHOD(put_StaticLight)(long val);

// Data members
public:
	CDSegment* _pCube;
};

#endif //__COMCUBE_H_
