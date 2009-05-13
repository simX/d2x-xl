// Parser.cpp: implementation of the CParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dlc.h"
#include "mine.h"

#include "Parser.h"
#include "msscript_i.h"
#include "msscript_i.c"


#include "ComMine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParser::CParser(CMine* pMine)
{
	_pMine = pMine;
}

CParser::~CParser()
{

}


void CParser::RunScript()
{
	CString strScriptText = 
		"Rem This is a test\r\n"
		"Mine.InfoMessage(\"Hello out there\")\r\n"
		"n = Mine.NumberOfCubes\r\n"
		"msg = \"There are \" & n & \" cubes in this level\"\r\n"
		"MsgBox msg\r\n"
		"Set MyCube = Cube(0)\r\n"
		"MyCube.staticLight = 12345\r\n"
		"msg = \"Cube 0's static light = \" & MyCube.StaticLight\r\n"
		"MsgBox msg\r\n"
		;

	CComObject<CComMine>* mineObject;
	if (SUCCEEDED(CComObject<CComMine>::CreateInstance(&mineObject)))
	{
		mineObject->_pMine = _pMine;
		CComQIPtr<IDispatch, &IID_IDispatch> dispIntfc = mineObject;

		CString strObjName("Mine");
		USES_CONVERSION;

		BOOL bSuccess = FALSE;

		// delimit the script text properly
		CComBSTR bstrScriptText = "Sub Main :";
		bstrScriptText += CComBSTR(strScriptText);
		bstrScriptText += " : End Sub";
			
		CComPtr<IScriptControl> scriptControl;
		CComPtr<IScriptError> error;

		if (dispIntfc && SUCCEEDED(::CoCreateInstance(CLSID_ScriptControl,NULL,CLSCTX_INPROC_SERVER,IID_IScriptControl,(void**)&scriptControl)) )
		{
			HRESULT hr;
			
			if (SUCCEEDED(scriptControl->put_AllowUI(TRUE)))
			{
				if (SUCCEEDED(scriptControl->put_Language(CComBSTR("VBScript"))))
				{
					if (SUCCEEDED(scriptControl->AddCode (bstrScriptText)))
					{
						hr = scriptControl->AddObject(CComBSTR(strObjName),dispIntfc,TRUE);
						
						if (SUCCEEDED(hr))
						{
							SAFEARRAYBOUND bound;
							bound.cElements = 0;
							bound.lLbound = 0;
							SAFEARRAY* array = ::SafeArrayCreate(VT_UI1, 1, &bound);

							CComVariant varResult;
							hr = scriptControl->Run(CComBSTR("Main"),(SAFEARRAY**)&array,&varResult);
							if (SUCCEEDED(hr))
							{
								bSuccess = TRUE;
							}
							::SafeArrayDestroy(array);
						}
					}
				}
			}
			

			if (!bSuccess)
			{
				if (SUCCEEDED(scriptControl->get_Error(&error)))
				{
					CComBSTR bstr;
					long line;
					error->get_Line(&line);

					
					if (SUCCEEDED(error->get_Description(&bstr)))
					{
						if (bstr.Length())
						{
							::MessageBox(NULL,OLE2T(bstr),"Error",MB_OK);
						}
					}
				}
			}

		}
	}
}
