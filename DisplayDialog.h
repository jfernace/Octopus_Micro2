#pragma once


// DisplayDialog dialog

class DisplayDialog : public CDialog
{
	DECLARE_DYNAMIC(DisplayDialog)

public:
	DisplayDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~DisplayDialog();

// Dialog Data
	enum { IDD = IDC_CAS_DISP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
