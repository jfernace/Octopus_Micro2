#if !defined(AFX_H_COctopusObjectivePiezo)
#define AFX_H_OctopusObjectivePiezo

#include "stdafx.h"
#include "OctopusGlobals.h"
////#include "Pi_gcs2_dll.h"

class COctopusObjectivePiezo : public CDialog
{

public:
	
	COctopusObjectivePiezo(CWnd* pParent = NULL);
	virtual ~COctopusObjectivePiezo();
	enum { IDD = IDC_PIEZO };

	void MoveUp( void );
	void MoveDown( void );
	void MoveRelZ( double dist );

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdrawExecute( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()

	CStatic m_Pos;
	
    bool first_tick;
	double save_z_microns; 
	double target_z_microns;
	double stepsize_z_microns;
	double middle_z_microns;
	double max_z_microns;

	int	m_Radio_S;

	CBitmap m_bmp_yes;
	CBitmap m_bmp_no;
	CStatic m_status;

	CString debug;
	CString result;

	void   ShowPosition( void );
	void   UpdatePosition( void );
	double GetPosition( void );

	char axis[17];
	int ID;
	bool connected;
	bool initialized;

public:

	afx_msg void OnStepSize1();
	afx_msg void OnStepSize2();
	afx_msg void OnStepSize3();
	afx_msg void OnStepSize4();
	afx_msg void OnStepSize5();

	afx_msg void OnSave();
	afx_msg void OnSaveGoTo();

	virtual BOOL OnInitDialog();

	afx_msg void OnKillfocusGeneral();
	afx_msg void OnTimer(UINT nIDEvent);

	int Initialize( void );
	void Close( void );
	afx_msg void Center();

};

#endif