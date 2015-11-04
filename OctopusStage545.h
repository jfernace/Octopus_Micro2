
#if !defined(AFX_H_OctopusStage545)
#define AFX_H_OctopusStage545

#include "stdafx.h"
#include "OctopusGlobals.h"
#include "afxwin.h"
#include <process.h>
//#include "Pi_gcs2_dll.h"

class COctopusStage545 : public CDialog
{

public:
	
	COctopusStage545(CWnd* pParent = NULL);
	virtual ~COctopusStage545();
	enum { IDD = IDC_STAGE_545 };

	void MoveRelX( double dist );
	void MoveRelY( double dist );
	void MoveRelZ( double dist );

	
	void MoveLeft( void );
	void MoveRight( void );
	void MoveFwd( void );
	void MoveBack( void );
	void MoveUp( void );
	void MoveDown( void );
	void GetPosition( void );

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdrawExecute( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()

	CStatic m_Pos;
	CStatic m_Pos_Save;
	CStatic m_Step;

	double middlex;
	double middley;
	double middlez;

	double range;
	
	double savex;
	double savey; 
	double savez;
	
	double target_x;
	double target_y;
	double target_z;

	int	m_Radio_S;

	int m_gotoy;
	int m_gotox;

	
	void ShowPosition( void );
	void UpdatePosition( void );

	bool first_tick;

	char axis[17];

	int ID;
	int IDZ;
	bool connected;
	bool initialized;
	double stepsize;

	CString debug; 

public:



	afx_msg void OnStepSize1();
	afx_msg void OnStepSize2();
	afx_msg void OnStepSize3();
	afx_msg void OnStepSize4();
    afx_msg void OnStepSize5();

	afx_msg void OnSave();
	afx_msg void OnSaveGoTo();

	afx_msg void OnKillfocusGeneral();

	afx_msg void OnTimer(UINT nIDEvent);

	int InitializeStage( void );
	void Close( void );
	afx_msg void StageCenter();

	afx_msg void OnStnClickedStagePos();
	afx_msg void OnStnClickedStagePosSave();
	afx_msg void OnBnClickedButtonGotoxy();
	afx_msg void OnEnChangeGotoy();
	afx_msg void OnEnChangeGotox();
};

#endif