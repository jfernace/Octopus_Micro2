#if !defined(AFX_H_OctopusLaser)
#define AFX_H_OctopusLaser


#include "stdafx.h"
#include "OctopusGlobals.h"
#include "oldaapi.h"
#include "stdlib.h"
#include "afxwin.h"

class OctopusLaser : public CDialog
{

public:
	enum { IDD = IDC_LED };
	OctopusLaser(CWnd* pParent = NULL);
	virtual ~OctopusLaser();

protected:

	float Intensity;
	HDEV   hdrvr_9812;
	HDASS  hdass_9812_DAC;
	int displayedIntensity;


	CSliderCtrl m_Slider;
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	BOOL CALLBACK GetDriver( LPSTR lpszName, LPSTR lpszEntry, LPARAM lParam );
	void PrintIntensity();
	void SetIntensity(float volts);


public:
	void OctopusLaser::LED_On(float intensity);
	void OctopusLaser::LED_Off();
	afx_msg void OnNMCustomdrawLedIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnClickedLedIntensity();
	int LED_Intensity;
	afx_msg void OnEnChangeIntensityView();
	afx_msg void OnBnClickedSubmitIntensity();
	afx_msg void OnEnKillfocusIntensityView();
	CEdit m_edit;
	afx_msg void OnNMCustomdrawLasersSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeLaserInput();
	afx_msg void OnEnKillfocusLaserInput();
};

#endif
