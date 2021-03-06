///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlCom.h
/// @brief  Fishjam Template Library Com Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
/// @defgroup ftlcom ftl com function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_COM_H
#define FTL_COM_H
#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
//COM对象 .rgs --  val DllSurrogate = s ''
//COM接口 .rgs -- 
/*
	  Elevation
	  {
		  val Enabled = d 1
	  }
	  val LocalizedString = s  '@%MODULE%,-101'
*/

//调试: COM中各种函数签名可以通过MSDN或 OleView.exe 查看对应的 tlb(如 mshtml.tlb) 等文件
//.NET 框架带来了高效的开发，同时也带来了COM和.NET框架发生冲突的问题

/*************************************************************************************************************
* INITGUID -- 包含 guiddef.h ？ 中定义的所有 GUID(使用库编译已经编译好的),更好的方式是直接包含 InitGuid.h
*
* 各编程语言的字符串表示
*   C++ 字符串是以null结束的ASCII或Unicode字符数组
*   Visual Basic字符串是一个ASCII字符数组加上表示长度的前缀。
*   Java字符串是以0结束的Unicode字符数组
*
* 
* COM的基本字符数据类型是OLECHAR，与平台无关的字符表示法，其C++体现即为BSTR
*   BSTR(Basic STRing) <== 带长度(BSTR指针的前4个字节,不包括结束NULL字符的字符串的字节数，如 SysAllocString(L"Hello")长度为10)的宽字符(16bit)数组，
*     可以存贮NULL字节进行分段，并且 解释型环境 和 进行Marshal时更高效 -- 不需要扫描来确定字符串长度；
*     注意使用BSTR时，如果包含内嵌NULL字符的字符串时很有可能出错 -- BSTR 会被编译器视为 OLECHAR*(即以NULL结尾的字符串)，
*       但实际上两者不同（前面是否有长度、是否可以内嵌NULL字符 等）
*       即：CComBSTR::Append(BSTR) 错误 -- 正确的方法是 CComBSTR::AppendBSTR(BSTR).
*           CComBSTR::Append(LPCOLESTR 或 LPCSTR) 正确
*           CComBSTR bstrTest(BSTR); 错误 -- 如果BSTR中包含NULL字符，会被认为是参数 LPCOLESTR 的重载，从而丢失数据，
*           CComBSTR bstrTest(SysStringLen(BSTR), BSTR); 正确 -- 指定长度
*           CComBSTR bstrTest = BSTR; 错误 -- 吧一个包含内嵌NULL字符的BSTR赋值给一个CComBSTR对象永远无法正常工作
*           CComBSTR bstrTest; bstrTest.AppendBSTR(BSTR); 错误, AppendBSTR 以前可以先 Empty
*     SysAllocString返回的指针指向BSTR的第一个字符(而不是BSTR在内存的第一个字节 -- 长度位置)，因此可直接用于大部分需要 LPCWSTR 或 const OLECHAR*  的地方，
*       但是不能用于 非const 的地方，否则可能照成错误
*     所有的BSTR都必须使用SysFreeString()释放,否则会造成内存泄漏。
*     有一个构造函数，可以接收REFGUID，将其转换为易读的字符串格式
*     使用规则（COM接口中推荐使用，但其他时候尽量避免）
*       1.COM接口定义(推荐) -- 可以不用自定义Marshal
*       2.不要在在IDL结构体中定义BSTR成员(避免使用) -- 会给结构体的复制和释放带来麻烦，最好直接使用最大长度的TCHAR数组
*       3.类的成员变量和非COM接口的函数参数不使用BSTR,局部变量要尽快释放
*       4.在对BSTR进行修改（包括创建和释放时），必须使用BSTR的专用函数 -- SysAllocString / SysAllocStringByteLen / SysFreeString / SysStringLen 等
*       5.在VB等中NULL指针是一个空BSTR字符串的合法值，但SysStringLen等函数认为NULL不合法，因此要正确获取BSTR长度的话，需要：
*         UINT nLength = bstrInput ? SysStringLen(bstrInput) : 0;
*     转换(对编译器来说， BSTR 是 OLECHAR* 或 LPCOLESTR )
*       1.LPSTR   => BSTR  -- a.WCHAR wstr[MAX_WSTR_LEN]; MultiByteToWideChar(CP_ACP, 0, str, strlen(str)+1, wstr, xxx); BSTR bstr1 = ::SysAllocString(wstr);
*                             b._bstr_t("xxxx");
*       2.BSTR    => LPSTR -- a.WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)bstr,xxx); 
*                             b._bstr_t bstr1(bstr); const char* str = static_cast<const char*> bstr1;
*       3.BSTR    => LPWSTR(需要修改) -- UINT unLen = ::SysStringLen(bstr1); LPWSTR psz1 = new WCHAR[len+1]; ::_tcsncpy(psz1, bstr1, unLen);
*       4.BSTR    => LPCWSTR(不修改，只读)  -- LPCWSTR psz2 = static_cast<LPCWSTR>bstr2; 但注意：如果Free bstr2 后就不能再使用 psz2
*       5.CString => BSTR  -- BSTR bstrTest = str1.AllocSysString();  xxxxxx;  ::SysFreeString(bstrTest);  
*                          或 CComBSTR bstr1(static_cast<LPCTSTR>(str1));  不用手工释放
*       6.BSTR    => CString -- BSTR bstrTest = SysAllocString("XXX"); CString str1(bstrTest); SysFreeString(bstrTest); 
*                               问题：CString 是哪种?(ATL/WTL/MFC)
*       
*     辅助类: 
*       CComBSTR -- 只有一个m_str成员，常用于服务器程序，默认构造器将值设置为NULL，析构自动回收BSTR，也可用Empty手动释放，
*         通常需要把值设置为空串，避免客户程序使用未赋值的字串抛出异常，能接受ANSI，但只能输出UNICODE
*         CComBSTR bstrBad(NULL) -- 调用CComBSTR(int nSize)生成长度为0的字符串，等价于CComBSTR bstr2(L(""))， CComBSTR::m_str 需要释放
*         CComBSTR bstrGood / CComBSTR bstrGood(static_cast< LPCOLESTR> (NULL))才调用 CComBSTR(LPCOLESTR pSrc) 把CComBSTR::m_str 生成NULL
*       _bstr_t -- 有引用计数，要求异常处理，适合于客户程序，提供了到wchar_t和char的转换输出，可以完成简单的BSTR字符串比较等操作
*       std::string/wstring -- 功能最完善，可移植性最好。推荐使用
*       comutil.h 在 _com_util 名称空间中定义了 ConvertStringToBSTR 和 ConvertBSTRToString 转换函数
*     注意：
*       1.Automation会cache BSTR使用的空间，会给发现问题带来困难 -- 因此，应该尽量减小BSTR的生存范围
*       2.把一个已经初始化好的CComBSTR的地址传给一个函数作为[out]参数会导致内存泄漏 -- 必须先调用Empty方法清空字符串的内容，
*         但对于相同的代码，如果参数类型是[in, out]，就不会有泄漏。因为被调函数应该会在复制之前，Free原有的串
*       3.把一个CComBSTR类型的类成员变量返回给传出[out]参数，需要用 Copy 或 Detach
*       4.避免用CComBSTR在内循环进行频繁字符串修改操作，BSTR的效率较低
* 
* CComVariant(VARIANT)
*   VariantInit -- 初始化,会设置成 (vt = VT_EMPTY )
*   VariantCopy -- 根据类型正确的进行浅拷贝或深拷贝
*   VariantCopyInd -- VARIANT可以有选择地表示最多一层的间接性(VT_BYREF 还是 VT_ARRAY?)， 这个函数可以去除VARIANT的这一层间接性
*   VariantClear -- 根据类型正确的释放，如包含BSTR的SAFEARRAY会先释放数组中每个BSTR，然后释放数组本身
*   注意：CComVariant 针对字符串有几个重载构造，稍一不注意，可能就会出错(深入解析ATL.pdf中写了还有 BSTR 参数的构造，但VS2008中没有)
*     1.CComVariant(LPCOLESTR lpszSrc) 
*     2.CComVariant(LPCSTR lpszSrc)
* 
* 程序中创建GUID: CoCreateGuid, 然后可通过其 Data1~Data4 访问具体的数据(可当成随机数来用)
*
* COM连接点客户端调试(参见CFBhoObject::Invoke)：
*   重载 Invoke 函数，然后可以打印相关信息，从而知道发生的所有连接点事件
*
* 通过COM进行动态权限提升(运行在UAC下的普通用户，可动态通过权利提升激活COM类 -- COM Elevation Moniker)
*   http://blogs.msdn.com/b/vistacompatteam/archive/2006/09/28/cocreateinstanceasadmin-or-createelevatedcomobject-sample.aspx
*   COM必须在注册表中有权限提升支持标识：
*     Lib的rgs文件，
*        APPID { val DllSurrogate = s '' }			-- COM对象会运行在新创建的 dllhost.exe 进程中
*        可以设置二进制的 AccessPermission ?
*     Object的rgs 文件中，
*        val LocalizedString = s'@%MODULE%,-101'	-- 在提升对话框中显示的字符串，否则会报 CO_E_MISSING_DISPLAYNAME
*        Elevation { 
*          val Enabled = d 1						-- 声明 LUA-Enabled，否则会返回 CO_E_ELEVATION_DISABLED
*          val IconReference= -s'@%MODULE%,-201'	-- 可选的程序图标(实测无效? 必须签名才会生效?)
*        } 
*   客户端：通过 CreateElevatedComObject 函数调用
*   允许标准用户访问(如果不设置的话，管理员访问没有问题，其他用户访问会 E_ACCESSDENIED)
*     1.dcomcnfg -> Component Services\Computers\My Computer\DCOM Config -> 找到对象 -> Properties -> Security
*       Access Permissions 中 Add "Interactive"
*       TODO:可以用代码直接设置吗?
* 
*   Over-The-Shoulder (OTS) Elevation -- 这是什么类型的提升？
*   
* 
//http://blogs.msdn.com/b/vistacompatteam/archive/2006/09/28/cocreateinstanceasadmin-or-createelevatedcomobject-sample.aspx
#pragma TODO(The COM Elevation Moniker)
*     COM类( AppID/DllSurrogate, Elevation/Enabled=1, LocalizedString="xxx", )

*************************************************************************************************************/

#ifndef FTL_BASE_H
#  error ftlcom.h requires ftlbase.h to be included first
#endif

#include <exdispid.h>
#include <atlcomcli.h>
#include <atlsafe.h>

namespace FTL
{
//Class
    
//Function
    class CFComUtility
    {
    public:
        FTLINLINE static BOOL IsComponentRegistered(REFCLSID clsid);
        FTLINLINE static BOOL RegisterComponent(LPCTSTR pszFilePath,BOOL bRegister);
        FTLINLINE static HRESULT DisplayObjectPropertyPage(HWND hWndOwner,LPCOLESTR lpszCaption, IUnknown *pUnk);

		//创建一个进行过权限提升的COM对象(该COM对象必须满足特定的要求?)
		FTLINLINE static HRESULT CreateElevatedComObject(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv);
    };

    FTLEXPORT class CFVariantInfo : public CFConvertInfoT<CFVariantInfo, const VARIANT&>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFVariantInfo);
    public:
		//bComPtrDetect 表明是否通过 ComDetect 检查COM接口支持的接口(如 IUnknown)
        FTLINLINE explicit CFVariantInfo(const VARIANT& info, BOOL bComPtrDetect = FALSE);
        FTLINLINE virtual LPCTSTR ConvertInfo();
	public:
		FTLINLINE VOID GetTypeInfo(CFStringFormater& formaterType);
		FTLINLINE VOID GetValueInfo(CFStringFormater& formaterValue);
	protected:
		BOOL m_bComPtrDetect;
    };

	FTLEXPORT class CFPropVariantInfo : public CFConvertInfoT<CFPropVariantInfo, const PROPVARIANT&>
	{
		DISABLE_COPY_AND_ASSIGNMENT(CFPropVariantInfo);
	public:
		FTLINLINE explicit CFPropVariantInfo(const PROPVARIANT& info);
		FTLINLINE virtual LPCTSTR ConvertInfo();
	public:
		FTLINLINE VOID GetTypeInfo(CFStringFormater& formaterType);
		FTLINLINE VOID GetValueInfo(CFStringFormater& formaterValue);
	};

    class CFSideBySide //从指定文件中产生COM对象(从sbs中拷贝)
    {
    public:
        FTLINLINE static HRESULT /*__stdcall*/ SbsCreateInstance(LPCTSTR szModule, REFCLSID rclsid, LPUNKNOWN pUnkOuter, 
            DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);
        FTLINLINE static HRESULT /*__stdcall*/ SbsCreateInstanceFromModuleHandle(HMODULE hModule, REFCLSID rclsid, LPUNKNOWN pUnkOuter,
            DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);
    };

	//Explorer 连接点事件转换：
	//可以重载 Explorer 客户端的 Invoke 函数，然后在里面转换得到 dispIdMember 对应的字符串, 参见 CFBhoObject::Invoke
    //  FTL::CFIExplorerDispidInfo  idspidInfo(dispIdMember, pDispParams);
    //  FTLTRACE(TEXT("Invoke, dispIdMember=%d(%s)\n"), dispIdMember, idspidInfo.GetConvertedInfo());
	//通常需要过滤掉 DISPID_STATUSTEXTCHANGE(102), DISPID_COMMANDSTATECHANGE(105, DISPID_PROGRESSCHANGE(108),
    //参考例子？： http://www.codeproject.com/Articles/7147/WTL-Browser
    FTLEXPORT class CFIExplorerDispidInfo : public CFConvertInfoT<CFIExplorerDispidInfo, DISPID>
    {
        DISABLE_COPY_AND_ASSIGNMENT(CFIExplorerDispidInfo);
    public:
        FTLINLINE explicit CFIExplorerDispidInfo(DISPID id, DISPPARAMS* pDispParams);
        FTLINLINE virtual LPCTSTR ConvertInfo();
	protected:
		DISPPARAMS*	m_pDispParams;
    };

    //多线程之间传递COM接口的辅助类
    template<typename T>
    class QueryInterfaceForBackgroundThread
    {
    public:
        QueryInterfaceForBackgroundThread(IUnknown* pUnk);
        ~QueryInterfaceForBackgroundThread();
		T* GetBackThreadInterface();
        //operator T*();
    private:
        IStream*	m_pStream;
        T*	        m_pInterface;
    };

    class IInformationOutput
    {
    public:
        virtual HRESULT SetIndent(int nIndent) = 0;
		virtual int GetIndent() = 0;
        virtual HRESULT OutputInfoName(LPCTSTR pszInfoName) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszPrompInfo) = 0;
        virtual HRESULT OnOutput(LPCTSTR pszKey, LPCTSTR pValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, BSTR* pValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, VARIANT_BOOL value) = 0;
        virtual HRESULT OnOutput(LPCTSTR pszKey, LONG nValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, DWORD dwValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, DOUBLE fValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, int iValue) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, HWND hWnd) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, GUID* pGuid) = 0;
		virtual HRESULT OnOutput(LPCTSTR pszKey, VARIANT* pValue) = 0;
        virtual HRESULT OnOutput(LPCTSTR pszKey, LONG nTotal, LONG nIndex, VARIANT* pValue) = 0;
    };

    class CFOutputWindowInfoOutput : public IInformationOutput
    {
    public:
		FTLINLINE static CFOutputWindowInfoOutput* Instance();
        FTLINLINE CFOutputWindowInfoOutput();
        FTLINLINE ~CFOutputWindowInfoOutput();
        FTLINLINE virtual HRESULT SetIndent(int nIndent);
		FTLINLINE virtual int GetIndent();
        FTLINLINE virtual HRESULT OutputInfoName(LPCTSTR pszInfoName);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszPrompInfo);
        FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, LPCTSTR pValue);
        FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, BSTR* pValue);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, VARIANT_BOOL value);
        FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, LONG nValue);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, DWORD dwValue);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, DOUBLE fValue);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, int iValue);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, HWND hWnd);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, GUID* pGuid);
		FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, VARIANT* pValue);
        FTLINLINE virtual HRESULT OnOutput(LPCTSTR pszKey, LONG nTotal, LONG nIndex, VARIANT* pValue);
    protected:
        int     m_nIndent;
        TCHAR*  m_pszIndentSpace;
        FTLINLINE virtual VOID    OutputIndentSpace();
    };

	template <typename T>
	class CFInterfaceDumperBase
	{
		DISABLE_COPY_AND_ASSIGNMENT(CFInterfaceDumperBase);
	public:
		enum {
			INVALID_INTERFACE_DUMPER_PARAM = -1
		};
		FTLINLINE explicit CFInterfaceDumperBase(IUnknown* pObj, IInformationOutput* pInfoOutput, 
			int nIndent, LONG_PTR param = INVALID_INTERFACE_DUMPER_PARAM );
		FTLINLINE HRESULT GetObjInfo(IInformationOutput* pInfoOutput);
        virtual ~CFInterfaceDumperBase();
	protected:
		IUnknown*           m_pObj;
		IInformationOutput*	m_pInfoOutput;
		int                 m_nIndent;
		LONG_PTR			m_nParam;
	};

    //参见 codeproject 上的 CImageDataObject -- 原来的实现似乎有不少Bug
    class CFImageDataObject 
        : public CComObjectRootEx<CComMultiThreadModel>
        , public IDataObjectImpl<CFImageDataObject>
    {
    public:
        BEGIN_COM_MAP(CFImageDataObject)
            COM_INTERFACE_ENTRY(IDataObject)
        END_COM_MAP()

        FTLINLINE HRESULT IDataObject_GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    public:
        FTLINLINE CFImageDataObject();
        FTLINLINE ~CFImageDataObject();
        IDataAdviseHolder* m_spDataAdviseHolder;
    };
}//namespace FTL

#endif //FTL_COM_H

#ifndef USE_EXPORT
#  include "ftlCom.hpp"
#endif