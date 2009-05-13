// Parser.h: interface for the CParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSER_H__6530B927_69FD_11D2_AE2A_00C0F03014A5__INCLUDED_)
#define AFX_PARSER_H__6530B927_69FD_11D2_AE2A_00C0F03014A5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CParser  
{
public:
	CParser(CMine* pMine);
	virtual ~CParser();
	
	void RunScript();

public:
	CMine* _pMine;

};

#endif // !defined(AFX_PARSER_H__6530B927_69FD_11D2_AE2A_00C0F03014A5__INCLUDED_)
