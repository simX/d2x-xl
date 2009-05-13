// ComCube.cpp : Implementation of CComCube
#include "stdafx.h"
#include "dlc.h"
#include "mine.h"
#include "ComCube.h"

/////////////////////////////////////////////////////////////////////////////
// CComCube


STDMETHODIMP CComCube::get_StaticLight(long* pRetVal)
{
	*pRetVal = _pCube->static_light;
	return S_OK;
}

STDMETHODIMP CComCube::put_StaticLight(long val)
{
	_pCube->static_light = val;
	return S_OK;
}
