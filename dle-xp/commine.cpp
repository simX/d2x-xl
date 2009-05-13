// ComMine.cpp : Implementation of CComMine
#include "stdafx.h"
#include "dlc.h"
#include "mine.h"
#include "ComMine.h"
#include "ComCube.h"
#include "ComObj.h"

/////////////////////////////////////////////////////////////////////////////
// CComMine


STDMETHODIMP CComMine::InfoMessage(BSTR strValue)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	CString str(strValue);
	::MessageBox(NULL, str,"Information Message",  MB_OK);

	return S_OK;
}


STDMETHODIMP CComMine::get_NumberOfCubes(long* pRetVal)
{
	*pRetVal = _pMine->SegCount ();
	return S_OK;
}

STDMETHODIMP CComMine::get_NumberOfPoints(long* pRetVal)
{
	*pRetVal = _pMine->VertCount ();
	return S_OK;
}

STDMETHODIMP CComMine::get_NumberOfWalls(long* pRetVal)
{
	*pRetVal = _pMine->GameInfo ().walls.count;
	return S_OK;
}

STDMETHODIMP CComMine::get_NumberOfTriggers(long* pRetVal)
{
	*pRetVal = _pMine->GameInfo ().triggers.count;
	return S_OK;
}


STDMETHODIMP CComMine::get_NumberOfObjects(long* pRetVal)
{
	*pRetVal = _pMine->GameInfo ().objects.count;
	return S_OK;
}

STDMETHODIMP CComMine::get_Cube(VARIANT index, LPDISPATCH *pVal)
{
	if (_pMine && pVal)
	{
		CComVariant var(index);
		if (SUCCEEDED(var.ChangeType(VT_I4)))
		{
			if (var.lVal < _pMine->SegCount ())
			{
				CComObject<CComCube>* pCube = new CComObject<CComCube>();
				pCube->_pCube = &_pMine->Segments ()[var.lVal];
				
				if (SUCCEEDED(pCube->QueryInterface(IID_IDispatch,(void**)pVal)))
				{
					return S_OK;
				}
				delete pCube;
			}
		}
	}
	return E_FAIL;
}

STDMETHODIMP CComMine::get_Object(VARIANT index, LPDISPATCH* pVal)
{
	if (_pMine && pVal)
	{
		CComVariant var(index);
		if (SUCCEEDED(var.ChangeType(VT_I4)))
		{
			if (var.lVal < _pMine->GameInfo ().objects.count)
			{
				CComObject<CComObj>* pObject = new CComObject<CComObj>();
				pObject->_pObject = &_pMine->Objects ()[var.lVal];
				
				if (SUCCEEDED(pObject->QueryInterface(IID_IDispatch,(void**)pVal)))
				{
					return S_OK;
				}
				delete pObject;
			}
		}
	}
	return E_FAIL;
}
