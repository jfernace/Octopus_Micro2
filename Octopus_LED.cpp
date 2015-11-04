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
#include "Octopus_LED.h"

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <Mmsystem.h>
//#include "f2c.h"
//#include "clapack.h"

DWORD WINAPI FocusThread(LPVOID lpParameter);

extern COctopusGlobals    B;
extern COctopusLog*       glob_m_pLog;


//Initialization
Octopus_LED::Octopus_LED(CWnd* pParent)
	: CDialog(Octopus_LED::IDD, pParent)
	, i_LED(0)
	, i_Laser(0)
	, s_Laser(100)
	, s_LED(100)
	, d_out1(FALSE)
	, d_out2(FALSE)
	, d_out3(FALSE)
	, d_out4(FALSE)
	, d_out5(FALSE)
	, d_out6(FALSE)
	, d_out7(FALSE)
	, d_out8(FALSE)
{    
	if( Create(Octopus_LED::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	led_intensity = 0;
	laser_intensity = 0;
	d_val = 0;

	//if ( glob_m_pLog != NULL ) 
	//	glob_m_pLog->Write(_T("Octopus_LED(CWnd* pParent) "));

	

}

void Octopus_LED::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	//ledIntensity = 0;
	//laserIntensity = 0;
	//DDX_Text( pDX, LED_INTENSITY_VIEW, ledIntensity);
	//DDX_Control(pDX,	IDC_LED_INTENSITY_SLIDER,				led_slider);
	//DDV_MinMaxInt(pDX, ledIntensity, 0, 100);
	//DDX_Control(pDX, LED_INTENSITY_VIEW, m_edit);

	//DDX_Text( pDX, LASER_INTENSITY_VIEW, laserIntensity);
	//DDX_Control(pDX,	IDC_LASER_INTENSITY_SLIDER,				laser_slider);
	//DDV_MinMaxInt(pDX, laserIntensity, 0, 100);
	//DDX_Control(pDX, LASER_INTENSITY_VIEW, m_edit);
	DDX_Text(pDX, LED_INTENSITY_VIEW, i_LED);
	DDV_MinMaxInt(pDX, i_LED, 0, 100);
	DDX_Text(pDX, LASER_INTENSITY_VIEW, i_Laser);
	DDV_MinMaxInt(pDX, i_Laser, 0, 100);
	DDX_Slider(pDX, IDC_LASER_INTENSITY_SLIDER, s_Laser);
	DDV_MinMaxInt(pDX, s_Laser, 0, 100);
	DDX_Slider(pDX, IDC_LED_INTENSITY_SLIDER, s_LED);
	DDV_MinMaxInt(pDX, s_LED, 0, 100);
	DDX_Check(pDX, IDC_DOUT1, d_out1);
	DDX_Check(pDX, IDC_DOUT2, d_out2);
	DDX_Check(pDX, IDC_DOUT3, d_out3);
	DDX_Check(pDX, IDC_DOUT4, d_out4);
	DDX_Check(pDX, IDC_DOUT5, d_out5);
	DDX_Check(pDX, IDC_DOUT6, d_out6);
	DDX_Check(pDX, IDC_DOUT7, d_out7);
	DDX_Check(pDX, IDC_DOUT8, d_out8);
}  

BEGIN_MESSAGE_MAP(Octopus_LED, CDialog)

	ON_WM_TIMER()
	//ON_NOTIFY(NM_CUSTOMDRAW, IDC_LED_INTENSITY_SLIDER, OnNMCustomdrawLedIntensitySlider)
	//ON_EN_KILLFOCUS(LED_INTENSITY_VIEW, &Octopus_LED::OnEnKillfocusIntensityView)
	//ON_NOTIFY(NM_CUSTOMDRAW, IDC_LASER_INTENSITY_SLIDER, &Octopus_LED::OnNMCustomdrawLedIntensitySlider2)
	//ON_EN_KILLFOCUS(LASER_INTENSITY_VIEW, &Octopus_LED::OnEnKillfocusIntensityView2)
	ON_EN_CHANGE(LED_INTENSITY_VIEW, &Octopus_LED::OnEnChangeLED)
	ON_EN_CHANGE(LASER_INTENSITY_VIEW, &Octopus_LED::OnEnChangeLaser)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LASER_INTENSITY_SLIDER, &Octopus_LED::OnNMCustomdrawLaserIntensitySlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LED_INTENSITY_SLIDER, &Octopus_LED::OnNMCustomdrawLEDIntensitySlider)
	ON_BN_CLICKED(IDC_DOUT1, &Octopus_LED::OnBnClickedDout1)
	ON_BN_CLICKED(IDC_DOUT2, &Octopus_LED::OnBnClickedDout2)
	ON_BN_CLICKED(IDC_DOUT3, &Octopus_LED::OnBnClickedDout3)
	ON_BN_CLICKED(IDC_DOUT4, &Octopus_LED::OnBnClickedDout4)
	ON_BN_CLICKED(IDC_DOUT5, &Octopus_LED::OnBnClickedDout5)
	ON_BN_CLICKED(IDC_DOUT6, &Octopus_LED::OnBnClickedDout6)
	ON_BN_CLICKED(IDC_DOUT7, &Octopus_LED::OnBnClickedDout7)
	ON_BN_CLICKED(IDC_DOUT8, &Octopus_LED::OnBnClickedDout8)
END_MESSAGE_MAP()

BOOL Octopus_LED::OnInitDialog() 
{
	CDialog::OnInitDialog();
	ECODE status = NULL;
	TRACE("Initializing Octopus_LED\n");
 
	hdrvr_9812 = NULL;
	hdass_9812_DAC = NULL;
	hdass_9812_DOUT = NULL;
    status = olDaInitialize(PTSTR("DT9812(00)"), &hdrvr_9812 );	
    if (status != OLNOERROR)
	{
	  TRACE("Error: %lu\n ",status);
      AfxMessageBox(_T("Connect to DT9812 board failed.\nHave you plugged it in and turned it on?"));
	}
	//status = olDaGetDASS( hdrvr_9812, OLSS_DA, 0, &hdass_9812_DAC );

	//Digital//////////////////////
	//status = olDaGetDASS( hdrvr_9812, OLSS_DOUT, 0, &hdass_9812_DOUT );
	//CString device;
	//device.Format(_T("Status: %l \n"), status);
	//TRACE(device);
	//////////////////////////////

	//status = olDaSetDataFlow( hdass_9812_DAC, OL_DF_SINGLEVALUE );
	//status = olDaSetDataFlow( hdass_9812_DOUT, OL_DF_SINGLEVALUE );
	//device.Format(_T("Status: %l \n"), status);
	//TRACE(device);

	//status = olDaConfig( hdass_9812_DAC );
	//status = oldaconfig( hdass_9812_dout );	
	//device.format(_t("status: %l \n"), status);
	//trace(device);
 //   double A[9] = {76, 27, 18, 25, 89, 60, 11, 51, 32};
 //   double b[3] = {10, 7, 43};

	//int N = 3;
	//int nrhs = 1;
	//int lda = 3;
	//int ipiv[3];
	//int ldb = 3;
	//int info;

 //   dgesv_(&N, &nrhs, A, &lda, ipiv, b, &ldb, &info);

 //   if(info == 0) /* succeed */
	//{
	//	TRACE("SUCCESS");
	//	printf("The solution is %lf %lf %lf\n", b[0], b[1], b[2]);
	//}
 //   else
	//{
	//	TRACE("FAIL");
	//	fprintf(stderr, "dgesv_ fails %d\n", info);
	//}
	//CString str;
	//str.Format(_T("The solution is %lf %lf %lf\n"), b[0], b[1], b[2]);
	//TRACE(str);


	if( status != OLNOERROR  ) 
	{
		TRACE("Error: %lu\n ",status);
		AfxMessageBox(_T("Error at Subsystem DAC"));
	}

	return TRUE;

}

Octopus_LED::~Octopus_LED() 
{
	SetLEDIntensity(0);
	SetLaserIntensity(0);
	SetFlow(0);
	olDaTerminate( hdrvr_9812 );
	TRACE("Ending Octopus_LED\n");
}

BOOL Octopus_LED::OnCommand(WPARAM wParam, LPARAM lParam) 
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
void Octopus_LED::LED_On(float intensity)
{
	TRACE("LED On called\n");
	SetLEDIntensity(intensity);
}
void Octopus_LED::LED_Off()
{
	SetLEDIntensity(0);
}
void Octopus_LED::Laser_On(float intensity)
{
	TRACE("Laser On called\n");
	SetLaserIntensity(intensity);
}
void Octopus_LED::Laser_Off()
{
	SetLaserIntensity(0);
}

void Octopus_LED::SetLEDIntensity( float intensity )
{
	ECODE status = NULL;
	float volts = 0;
	status = olDaGetDASS( hdrvr_9812, OLSS_DA, 0, &hdass_9812_DAC );
	status = olDaSetDataFlow( hdass_9812_DAC, OL_DF_SINGLEVALUE );
	status = olDaConfig( hdass_9812_DAC );
	volts = (intensity / 100) * 1024;
	status = olDaPutSingleValue( hdass_9812_DAC, volts, 0, 0 );
	olDaReleaseDASS( hdass_9812_DAC );
}

void Octopus_LED::SetLaserIntensity( float intensity )
{
	ECODE status = NULL;
	float volts = 0;
	status = olDaGetDASS( hdrvr_9812, OLSS_DA, 0, &hdass_9812_DAC );
	status = olDaSetDataFlow( hdass_9812_DAC, OL_DF_SINGLEVALUE );
	status = olDaConfig( hdass_9812_DAC );
	volts = (intensity / 100) * 1024;
	status = olDaPutSingleValue( hdass_9812_DAC, volts, 1, 0 );
	olDaReleaseDASS( hdass_9812_DAC );
}

void Octopus_LED::SetFlow( int val )
{
	olDaGetDASS( hdrvr_9812, OLSS_DOUT, 0, &hdass_9812_DOUT );
	olDaSetDataFlow( hdass_9812_DOUT, OL_DF_SINGLEVALUE );
	olDaConfig( hdass_9812_DOUT );
	olDaPutSingleValue( hdass_9812_DOUT, val, 0, 0 );
	olDaReleaseDASS( hdass_9812_DOUT );
}
/************************************
 User interface etc
 ***********************************/

//
//void Octopus_LED::OnNMCustomdrawLedIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
//	
//	int CurPos = led_slider.GetPos();
//	CString str;
//	Intensity = 100 - CurPos;
//	ledIntensity = Intensity;
//	SetLEDIntensity(Intensity);
//	str.Format(_T("Intensity:%f\n"), Intensity);
//	TRACE(str);
//	LED_Intensity = Intensity;
//	*pResult = 0;
//}
//
//
//void Octopus_LED::OnEnKillfocusIntensityView()
//{
//	// TODO: Add your control notification handler code here
//	UpdateData(true);
//	CString str;
//	str.Format("From shutter: %d\n",ledIntensity);
//	TRACE(str);
//	led_slider.SetPos(100-ledIntensity);
//
//}
//
//void Octopus_LED::OnNMCustomdrawLedIntensitySlider2(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
//	// TODO: Add your control notification handler code here
//	int CurPos = laser_slider.GetPos();
//	laserIntensity = 100 - CurPos;
//	SetLaserIntensity(100 - CurPos);
//	CString str;
//	str.Format(_T("Laser Intensity:%d\n"), 100 - CurPos);
//	TRACE(str);
//	*pResult = 0;
//}
//
//void Octopus_LED::OnEnKillfocusIntensityView2()
//{
//	UpdateData(true);
//	CString str;
//	str.Format("From shutter: %d\n",laserIntensity);
//	TRACE(str);
//	laser_slider.SetPos(100-laserIntensity);
//	// TODO: Add your control notification handler code here
//}


void Octopus_LED::OnEnChangeLED()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(true);
	led_intensity = i_LED;
	s_LED = 100 - i_LED;
	UpdateData(false);
	SetLEDIntensity(led_intensity);
}

void Octopus_LED::OnEnChangeLaser()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(true);
	laser_intensity = i_Laser;
	s_Laser = 100 - i_Laser;
	UpdateData(false);
	SetLaserIntensity(laser_intensity);
}

void Octopus_LED::OnNMCustomdrawLaserIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here

	UpdateData(true);
	laser_intensity = 100 - s_Laser;
	i_Laser = 100 - s_Laser;
	UpdateData(false);
	SetLaserIntensity(laser_intensity);
		*pResult = 0;

}

void Octopus_LED::OnNMCustomdrawLEDIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(true);
	led_intensity = 100 - s_LED;
	i_LED = 100 - s_LED;
	UpdateData(false);
	SetLEDIntensity(led_intensity);
		*pResult = 0;

}

void Octopus_LED::OnBnClickedDout1()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out1)
		d_val = d_val + 1;
	else
		d_val = d_val - 1;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout2()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out2)
		d_val = d_val + 2;
	else
		d_val = d_val - 2;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout3()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out3)
		d_val = d_val + 4;
	else
		d_val = d_val - 4;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout4()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out4)
		d_val = d_val + 8;
	else
		d_val = d_val - 8;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout5()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out5)
		d_val = d_val + 16;
	else
		d_val = d_val - 16;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout6()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out6)
		d_val = d_val + 32;
	else
		d_val = d_val - 32;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout7()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out7)
		d_val = d_val + 64;
	else
		d_val = d_val - 64;
	SetFlow(d_val);
}

void Octopus_LED::OnBnClickedDout8()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (d_out8)
		d_val = d_val + 128;
	else
		d_val = d_val - 128;
	SetFlow(d_val);
}
