// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__47022C0C_6BB7_44FF_B8E6_3C8ADDC7E3FC__INCLUDED_)
#define AFX_STDAFX_H__47022C0C_6BB7_44FF_B8E6_3C8ADDC7E3FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define _WIN32_IE 0x0501

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

//
// These are needed for IDeskBand
//

#include <shlguid.h>
#include <shlobj.h>

// needed for IInputObject and _bstr_t
#include <comdef.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#include <ftlbase.h>
#include <ftlCom.h>
#include <ftlComDetect.h>
#include <ftlIEHelper.h>
#include <ftlWindow.h>

#endif // !defined(AFX_STDAFX_H__47022C0C_6BB7_44FF_B8E6_3C8ADDC7E3FC__INCLUDED)