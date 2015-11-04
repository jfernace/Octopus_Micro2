#if !defined(AFX_H_Octopus_LED)
#define AFX_H_Octopus_LED


#include "OctopusGlobals.h"
#include "oldaapi.h"
#include "stdlib.h"
#include "stdafx.h"
#include "afxwin.h"

class Octopus_LED : public CDialog
{

public:
	enum { IDD = IDC_LED };
	Octopus_LED(CWnd* pParent = NULL);
	virtual ~Octopus_LED();

protected:

	float Intensity;
	HDEV   hdrvr_9812;
	HDASS  hdass_9812_DAC;
	HDASS  hdass_9812_DOUT;
	int ledIntensity;
	int laserIntensity;


	CSliderCtrl led_slider;
	CSliderCtrl laser_slider;
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	BOOL CALLBACK GetDriver( LPSTR lpszName, LPSTR lpszEntry, LPARAM lParam );
	void SetLEDIntensity( float intensity );
	void SetLaserIntensity( float intensity );
	


public:
	void Octopus_LED::LED_On(float intensity);
	void Octopus_LED::LED_Off();
	void Octopus_LED::Laser_On(float intensity);
	void Octopus_LED::Laser_Off();
	void SetFlow( int val );
	afx_msg void OnEnChangeLED();
	afx_msg void OnEnChangeLaser();
	afx_msg void OnNMCustomdrawLaserIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawLEDIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult);
	int i_LED;
	int i_Laser;
	int s_Laser;
	int s_LED;
	float led_intensity;
	float laser_intensity;
	BOOL d_out1;
	BOOL d_out2;
	BOOL d_out3;
	BOOL d_out4;
	BOOL d_out5;
	BOOL d_out6;
	BOOL d_out7;
	BOOL d_out8;
	afx_msg void OnBnClickedDout1();
	afx_msg void OnBnClickedDout2();
	afx_msg void OnBnClickedDout3();
	afx_msg void OnBnClickedDout4();
	afx_msg void OnBnClickedDout5();
	afx_msg void OnBnClickedDout6();
	afx_msg void OnBnClickedDout7();
	afx_msg void OnBnClickedDout8();
	int d_val;
};

#endif
