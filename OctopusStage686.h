
#if !defined(AFX_H_OctopusStage686)
#define AFX_H_OctopusStage686

#include "stdafx.h"
#include "OctopusGlobals.h"
#include "afxwin.h"
#include <process.h>
#include "Pi_gcs2_dll.h"

class COctopusStage686 : public CDialog
{

public:
	
	COctopusStage686(CWnd* pParent = NULL);
	virtual ~COctopusStage686();
	enum { IDD = IDC_STAGE_686 };

	void MoveRelX( double dist );
	void MoveRelY( double dist );

	void MoveLeft( void );
	void MoveRight( void );
	void MoveFwd( void );
	void MoveBack( void );

	void OnJoyStickOnOff( void );

protected:

	virtual BOOL OnInitDialog();

	CButton m_btn_JOY_ON_OFF;
	CButton m_btn_GoToPos;
	CButton m_btn_SavePos;

	CFont m_Font;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdrawExecute( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()

	CStatic m_Pos;
	CStatic m_Pos_Save;
	CStatic m_Step;

	double middle;
	double range;
	double save_xy[2];
	double target_xy[2];
	double stepsize;

	int	m_Radio_S;

	void GetPosition( void );
	void ShowPosition( void );
	void UpdatePosition( void );
	bool initialized;
	bool first_tick;

	char    axis_xy[6];
	
	int		ID_XY;
	bool	XY_connected;
	double	dPos[2];
	BOOL	bIsMoving[2];
	BOOL	bJONState[2];

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

	void InitializeStage( void );
	void Close( void );
	afx_msg void StageCenter();

};

#endif