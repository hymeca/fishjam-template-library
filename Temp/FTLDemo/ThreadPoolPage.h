#pragma once

// CThreadPoolPage 对话框

#define TEST_ATL_THREAD_POOL    0
#include <ftlThreadPool.h>
#include "afxcmn.h"


#if TEST_ATL_THREAD_POOL
#  include <atlutil.h>		//ATL ThreadPool CThreadPool
#endif

//定义是否测试 Vista 后的 Thread Pool API 的宏，设置为 1 则测试(需要Vista以后的运行环境)
#define CHECK_VISTA_THREAD_POOL	0

class CATLPoolJob
{
public:
	typedef DWORD_PTR RequestType;

	virtual BOOL Initialize(void *pvParam)
	{
		UNREFERENCED_PARAMETER(pvParam);
		return TRUE; 
	}
	virtual void Terminate(void* /*pvParam*/){}
	void Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw()
	{
		UNREFERENCED_PARAMETER(pOverlapped);
		UNREFERENCED_PARAMETER(pOverlapped);

		CATLPoolJob* pThis = reinterpret_cast<CATLPoolJob*>(dw);
		//pvParam 是 Initialize 传入的参数

		FTLTRACE(TEXT("CATLPoolJob Execute, pThis=%p, Thread id = %d, pvParam=%d\n"),
			pThis, GetCurrentThreadId(), pvParam);
		int nCount = 3;
		while(nCount > 0)
		{
			Sleep(300);
			nCount--;
		}
		delete pThis;
	}
};

class CThreadPoolPage : public CPropertyPage
	, public FTL::IFThreadPoolCallBack<CThreadPoolPage*>
{
	DECLARE_DYNAMIC(CThreadPoolPage)

public:
	CThreadPoolPage();
	virtual ~CThreadPoolPage();

// 对话框数据
	enum { IDD = IDD_PAGE_THREADPOOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
protected:
protected:
	virtual void OnJobProgress(LONG nJobIndex , CFJobBase<CThreadPoolPage*>* pJob, LONGLONG nCurPos, LONGLONG nTotalSize)
	{
		UNREFERENCED_PARAMETER(pJob);

		FTLTRACE(TEXT("OnJobProgress, nJobIndex=%d, CurPos=%lld, TotalSize=%lld\n"),
			nJobIndex, nCurPos, nTotalSize);
	}

    //CProgressCtrl m_ProgressOfThread;
	FTL::CFThreadPool<CThreadPoolPage*>*	m_pFtlThreadPool;

#if TEST_ATL_THREAD_POOL
	CThreadPool<CATLPoolJob>				m_AtlThreadPool;
#endif

	LONG	m_nFtlThreadPoolMaxWaitingJobs;
	LONG    m_nFtlThreadPoolMinThreads;
	LONG	m_nFtlThreadPoolMaxThreads;
	
	LONG	m_nFtlCurJobIndex;
    LONG	m_nHighJobPriority;
    LONG	m_nLowJobPriority;
	//BOOL	m_bHadRequestFtlThreadPoolPause;
public:
    afx_msg void OnDestroy();

	void SetFtlThreadPoolButtonStatus(BOOL bStarted, BOOL bPaused);
	afx_msg void OnBnClickedBtnFtlThreadPoolStart();
	afx_msg void OnBnClickedBtnFtlThreadPoolSetThreadCount();
	afx_msg void OnBnClickedBtnFtlThreadPoolSetJobPriorityHighest();
	afx_msg void OnBnClickedBtnFtlThreadPoolAddJobLow();
	afx_msg void OnBnClickedBtnFtlThreadPoolAddJobHigh();
	afx_msg void OnBnClickedBtnFtlThreadPoolAddJobSuspend();
	afx_msg void OnBnClickedBtnFtlThreadPoolResumeJob();
	afx_msg void OnBnClickedBtnFtlThreadPoolCancelJob();
	afx_msg void OnBnClickedBtnFtlThreadPoolStop();
	afx_msg void OnBnClickedBtnFtlThreadPoolPauseAll();
	afx_msg void OnBnClickedBtnFtlThreadPoolResumeAll();

	void SetAtlThreadPoolButtonStatus(BOOL bStarted, BOOL bPaused);
	afx_msg void OnBnClickedBtnAtlThreadPoolStart();
	afx_msg void OnBnClickedBtnAtlThreadPoolAddJob();
	afx_msg void OnBnClickedBtnAtlThreadPoolStop();


	static DWORD  __stdcall QueueUserWorkItemDemoProc( LPVOID lpThreadParameter );
	FTL::CFCriticalSection	m_csLock;
	LONG m_CurWorkItemCount;
	typedef std::set<DWORD>	ThreadIdSet;
	ThreadIdSet	m_ThreadIdSetOfQueueUserWorkItem;
	afx_msg void OnBnClickedBtnThreadpoolQueueUserWorkItem();


protected:
#if CHECK_VISTA_THREAD_POOL
    //Vista 后 CreateThreadPool 的线程池
    TP_CALLBACK_ENVIRON m_CallBackEnviron;
    PTP_POOL            m_pVistaThreadPool;
    PTP_CLEANUP_GROUP   m_pVistaCleanupGroup;

    void SetVistaThreadPoolButtonStatus(BOOL bStarted, BOOL bPaused);
    afx_msg void OnBnClickedBtnVistaCreateThreadPool();
    afx_msg void OnBnClickedBtnVistaCloseThreadPool();
	afx_msg void OnBnClickedBtnVistaCreateThreadpoolWork();
	afx_msg void OnBnClickedBtnVistaCreateThreadpoolTimer();

#endif //CHECK_VISTA_THREAD_POOL
};
