// COctopusShutterAndWheel.h : header file

#if !defined(AFX_H_OctopusXCite)
#define AFX_H_OctopusXCite

#include "stdafx.h"
#include "OctopusGlobals.h"
#include "ftd2xx.h"
#include "afxwin.h"
#include <process.h>
#include "cport.h"
#include "afxcmn.h"

class COctopusXCite : public CDialog
{

public:
	
	COctopusXCite(CWnd* pParent = NULL);
	virtual ~COctopusXCite();
	enum { IDD = IDC_XCITE };
	
	void Z_StepUp( void );
	void Z_StepDown( void );
	void BrightField( int volt );
	void ToggleXCiteLEDStatus( void );
	void Close( void );
	void EpiFilterWheel( int cube );
	void BrightFieldFilterWheel( int filter );
	void GetIntensityLevel(void);
	void GetLEDStatus(void);

	int lampIntensity;

	CString slampIntensity;//create a stringstream

protected:

	bool   Scope_initialized;
	bool   Path_camera;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMCustomdrawExecute( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()

	CPort* pPortScope;

	bool WriteScope( CString str );
	CString ReadScope( u16 CharsToRead );

	bool Init( void );
	bool IS_LED_ON;
	CStatic m_Pos;

	int	m_Radio_S;
	int m_Radio_OBJ;
	int m_Radio_FW;
	int m_Radio_BFW;
	
	bool working;
	double position_now;
  	u16 stepsize_10nm;
    u32 old_position;

	void DisplayBFIntensity( int volt );
	void DisplayPosition( void );

	int  GetObj( void );
	int  GetFW( void );
	int  GetBFW( void );

	
	CStatic     m_Slider_Setting;
	CSliderCtrl m_Slider;
	CString     m_Slider_Setting_String;
	CBitmap     m_bmp_on;
	CBitmap     m_bmp_off;
	CStatic      m_status_LED;
	bool first_tick;

public:

	afx_msg void OnObjectiveStepSize1();
	afx_msg void OnObjectiveStepSize2();
	afx_msg void OnObjectiveStepSize3();
	afx_msg void OnObjectiveStepSize4();

	afx_msg void OnObj_1();
	afx_msg void OnObj_2();
	afx_msg void OnObj_3();
	afx_msg void OnObj_4();
	afx_msg void OnObj_5();
	afx_msg void OnObj_6();

	afx_msg void OnFW_1();
	afx_msg void OnFW_2();
	afx_msg void OnFW_3();
	afx_msg void OnFW_4();
	afx_msg void OnFW_5();
	afx_msg void OnFW_6();

	afx_msg void OnBFW_1();
	afx_msg void OnBFW_2();
	afx_msg void OnBFW_3();
	afx_msg void OnBFW_4();
	afx_msg void OnBFW_5();
	afx_msg void OnBFW_6();

	void Objective( int obj );

	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg void ObjESC();
	afx_msg void ObjRTN();

	void GetPosition( void );

	afx_msg void OnStnClickedScopeIntensitySliderSetting();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnStnClickedScopePos();
	afx_msg void OnStnClickedXciteOnoff();
	afx_msg void OnStnClickedShutterImage();
	afx_msg void OnNMCustomdrawXciteIntensitySlider2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeManualIntensity();
	afx_msg void OnEnChangeMg17loggerctrl1();
	afx_msg void OnEnChangeManualAdjustEdit();
	afx_msg void OnBnClickedAdjustIntensity();
	CEdit m_lampIntensity;
	int m_LEDIntensity;
	int m_IntensitySlider;
	CSliderCtrl m_islider;
	//afx_msg void OnBnClickedScopePath();
	afx_msg void OnBnClickedLedOnoff();
};

#endif