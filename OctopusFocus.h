#if !defined(AFX_H_COctopusFocus)
#define AFX_H_OctopusFocus

#include "stdafx.h"
#include "OctopusGlobals.h"

class COctopusFocus : public CDialog
{

public:
	
	COctopusFocus(CWnd* pParent = NULL);
	virtual ~COctopusFocus();
	enum { IDD = IDC_FOCUS };

	void AutoFocus( double fsi );
	void EndFocus( void );

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdrawExecute( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()

	CStatic m_Score;
	
    bool first_tick;

	int	m_Radio_S;

	CBitmap m_bmp_yes;
	CBitmap m_bmp_no;
	CStatic m_status;

	CString debug;
	CString result;

	double  focus_score_best;
	u8      focus_step_best;
	u8      focus_step_current;
	double  focus_step_size;

public:
	void ROIFocus( double );
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusGeneral();

	afx_msg void OnTimer(UINT nIDEvent);

	void Initialize( void );
	void Close( void );
	afx_msg void OnFocus();

	BOOL m_focusType;
};

#endif