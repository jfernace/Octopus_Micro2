#if !defined(AFX_H_OctopusScript)
#define AFX_H_OctopusScript

#include "stdafx.h"
#include "OctopusGlobals.h"

class COctopusScript : public CDialog
{

public:
	COctopusScript(CWnd* pParent = NULL);
	virtual ~COctopusScript();
	enum { IDD = IDC_SCRIPT };

protected:
	
	UINT        m_nTimer;
	CListBox	m_SeqList;
	int			m_SeqListIndex;
	u16		    cycles_to_do;
	u16         cycles_to_do_in_program;
	u16			command_index;
	CBitmap     m_bmp_running;
	CBitmap     m_bmp_stopped;
	CStatic     m_status_scr;
	CStatic     m_cycle_count;
	CString		saved_path;

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnButtonSeqLoad();
	afx_msg void OnButtonSeqRun();
	afx_msg void OnButtonSeqStop();

	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};

#endif
