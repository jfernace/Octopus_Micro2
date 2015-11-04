/**********************************************************************************
******************** COctopusCamera.cpp : implementation file
**********************************************************************************/

#include "stdafx.h"
//#include "atmcd32d.h" // Andor Camera library
#include "Octopus.h"
#include "OctopusDoc.h"
#include "OctopusView.h"
#include "OctopusClock.h"
#include "OctopusCameraDlg.h"
#include "OctopusCameraDisplay.h"
//#include "OctopusGrating.h"
#include "OctopusGlobals.h"
#include "OctopusLog.h"
#include "OctopusLaser.h"

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <Mmsystem.h>

DWORD WINAPI FocusThread(LPVOID lpParameter);

extern COctopusGlobals    B;
extern COctopusLog*       glob_m_pLog;


//Initialization
OctopusLaser::OctopusLaser(CWnd* pParent)
	: CDialog(OctopusLaser::IDD, pParent)
	, LED_Intensity(0)
{    
	if( Create(OctopusLaser::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	displayedIntensity = 0;
	m_Slider.SetRange( 0, 100 );
	m_Slider.SetPos( 100 );
	m_Slider.SetTicFreq( 20 );
	m_edit.SetDlgItemInt(LED_INTENSITY_VIEW, 0, false);
	
	Intensity = 0;
	LED_Intensity = 0;

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(_T("OctopusLaser(CWnd* pParent) "));

	

}

void OctopusLaser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	displayedIntensity = 0;
	DDX_Text( pDX, IDC_LASER_INPUT, displayedIntensity);
	DDX_Control(pDX,	IDC_LASER_SLIDER,				m_Slider);
	DDV_MinMaxInt(pDX, displayedIntensity, 0, 100);
	DDX_Control(pDX, IDC_LASER_INPUT, m_edit);
}  

BEGIN_MESSAGE_MAP(OctopusLaser, CDialog)

	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LASERS_SLIDER, &OctopusLaser::OnNMCustomdrawLasersSlider)
	ON_EN_KILLFOCUS(IDC_LASER_INPUT, &OctopusLaser::OnEnKillfocusLaserInput)
END_MESSAGE_MAP()

BOOL OctopusLaser::OnInitDialog() 
{
	CDialog::OnInitDialog();
	ECODE status = NULL;
	TRACE("Initializing OctopusLaser\n");
 
	hdrvr_9812 = NULL;
	hdass_9812_DAC = NULL;
    status = olDaInitialize(PTSTR("DT9812(00)"), &hdrvr_9812 );	
    if (status != OLNOERROR)
	{
	  TRACE("Error: %lu\n ",status);
      AfxMessageBox(_T("Connect to DT9812 board failed.\nHave you plugged it in and turned it on?"));
	}
	status = olDaGetDASS( hdrvr_9812, OLSS_DA, 1, &hdass_9812_DAC );
	status = olDaSetDataFlow( hdass_9812_DAC, OL_DF_SINGLEVALUE );
	status = olDaConfig( hdass_9812_DAC );
	if( status != OLNOERROR  ) 
	{
		TRACE("Error: %lu\n ",status);
		AfxMessageBox(_T("Error at Subsystem DAC"));
	}
	return TRUE;
}

OctopusLaser::~OctopusLaser() 
{
	olDaReleaseDASS( hdass_9812_DAC );
	olDaTerminate( hdrvr_9812 );
	TRACE("Ending OctopusLaser\n");
}

BOOL OctopusLaser::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}

/*********************************************************************************
* Start/stop continuous data aquisition
*********************************************************************************/


/*********************************************************************************
********************	Utility functions ****************************************
*********************************************************************************/
void OctopusLaser::LED_On(float intensity)
{
	TRACE("LED On called\n");
	SetIntensity(intensity);
}
void OctopusLaser::LED_Off()
{
	SetIntensity(0);
}
void OctopusLaser::SetIntensity( float intensity )
{
	ECODE status = NULL;
	float volts = 0;

	volts = (intensity / 100) * 1024;
	volts = min(volts, 1024);
	volts = max(0, volts);
	//convert to DAC units
	status = olDaPutSingleValue( hdass_9812_DAC, volts, 2, 0 );
	CString str;
	str.Format(_T("Volts:%f\n"), volts);
	TRACE(str);
}

/************************************
 User interface etc
 ***********************************/

void OctopusLaser::OnNMCustomdrawLasersSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	int CurPos = m_Slider.GetPos();
	CString str;
	Intensity = 100 - CurPos;
	displayedIntensity = Intensity;
	SetIntensity(Intensity);
	str.Format(_T("Intensity:%f\n"), Intensity);
	TRACE(str);
	LED_Intensity = Intensity;
	*pResult = 0;
	*pResult = 0;


}



void OctopusLaser::OnEnKillfocusLaserInput()
{
	UpdateData(true);
	CString str;
	str.Format("From shutter: %d\n",displayedIntensity);
	TRACE(str);
	m_Slider.SetPos(100-displayedIntensity);
	// TODO: Add your control notification handler code here
}
