/////////////////////////////////////////////////////////////////////////////
// OctopusView.h
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_H_OctopusView)
#define AFX_H_OctopusView

#include "stdafx.h"
#include "OctopusDoc.h"

class COctopusView : public CFormView {

protected:	
	
	COctopusView();
	DECLARE_DYNCREATE(COctopusView)

public:

	enum { IDD = IDC_BACKGROUND };
	virtual ~COctopusView();

protected:

	afx_msg void OnOpenAndor();
	afx_msg void OnOpenWheel();
	afx_msg void OnOpenScript();
	afx_msg void OnOpenLED();
	afx_msg void OnOpenStage545();
	afx_msg void OnOpenStage686();
	afx_msg void OnOpenPiezo();
	afx_msg void OnOpenXCite();
	//afx_msg void OnOpenGrating();
	//afx_msg void OnOpenPolarizer();
	afx_msg void OnOpenScope();

	CTime systemtime;

	virtual void OnInitialUpdate();
	virtual void DoDataExchange(CDataExchange* pDX);

	u16 scope;

	DECLARE_MESSAGE_MAP()

	CString debug;

	
public:
	afx_msg void OnBnClickedStatusOpenFocus();
	afx_msg void OnBnClickedStatusOpenXcite();
};

#endif
