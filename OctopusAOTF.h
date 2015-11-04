///////////////////////////////////////////////////////////////////////////
// OctopusAOTF.h : header file
///////////////////////////////////////////////////////////////////////////

#if !defined(AFX_H_OctopusAOTF)
#define AFX_H_OctopusAOTF

#include "stdafx.h"
#include "olerrors.h"
//#include "olmem.h"
#include "oldadefs.h"
//#include "oldaapi.h"
#include <vector>

class COctopusAOTF : public CDialog {

public:

	COctopusAOTF(CWnd* pParent = NULL);
	virtual ~COctopusAOTF();
	enum { IDD = IDC_AOTF };

	double  freq;
	double  volts_CH1;
	double  volts_CH2;

protected:
	
	u16 outbuffersize;

	CBitmap m_bmp_yes;
	CBitmap m_bmp_no;
	CStatic m_status_AOTF;

	void Start( void );
	void Stop( void );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	void GenerateSignal( void );
	afx_msg void StartStop( void );
	void Configure( void );
	u16 ConvertVoltToDAC( double volt ); 

	HDEV   hdrvr_9834;   // device handle
	HDASS  hdass_9834;   // sub system handle
	ECODE  status;       // board error status
	HBUF   hBuf_DAC;     // sub system buffer handle
	PWORD  lpbuf;        // buffer pointer

	afx_msg void OnKillfocus();

};

#endif
