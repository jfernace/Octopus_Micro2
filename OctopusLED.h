
#if !defined(AFX_H_OctopusLED)
#define AFX_H_OctopusLED

#include "stdafx.h"
#include "OctopusGlobals.h"

#include "olerrors.h"
#include "olmem.h"
#include "oldadefs.h"
#include "oldaapi.h"

class COctopusLED : public CDialog
{

public:
	
	COctopusLED(CWnd* pParent = NULL);
	virtual ~COctopusLED();
	enum { IDD = IDC_LED };

	void LED_On( void );
	void LED_On( u16 intensity );
	void LED_Off( void );
	void TTL_Pulse_Up( void );

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	void LED_SetIntensity ( u16 intensity );

	DECLARE_MESSAGE_MAP()

	void UpdateLEDIntensity( void );

	CSliderCtrl m_Slider;
	CStatic     m_Slider_Text;
	CStatic     m_ADC_Text;
	CString     m_Slider_String;

	u16 LED_intensity_setpoint;
	u16 LED_intensity_current;

	bool first_tick;

	HDEV   hdrvr_9812;       // device handle
	HDASS  hdass_9812_DAC;   // sub system handle
	HDASS  hdass_9812_ADC;   // sub system handle
	HDASS  hdass_9812_DOUT;  // sub system handle
	ECODE  status;       // board error status
	HBUF   hBuf_DAC;     // sub system buffer handle
	HBUF   hBuf_ADC;     // sub system buffer handle
	PWORD  lpbuf;        // buffer pointer

	afx_msg void OnTimer(UINT nIDEvent);

	double ADCmin;
	double ADCmax;
	long   ADCvalue;
	UINT encoding;
	UINT resolution;

public:
	afx_msg void OnNMCustomdrawLedIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult);
};

#endif