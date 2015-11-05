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
//include "OctopusGrating.h"
#include "OctopusGlobals.h"
#include "OctopusLog.h"
#include "OctopusObjectivePiezo.h"
#include "OctopusStage545.h"
#include "OctopusLED.h"

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <Mmsystem.h>

DWORD WINAPI FocusThread(LPVOID lpParameter);

extern COctopusGlobals    B;
extern COctopusLog*       glob_m_pLog;
COctopusPictureDisplay*   glob_m_pPictureDisplay = NULL;
extern COctopusGoodClock* glob_m_pGoodClock;
//extern COctopusGrating*   glob_m_pGrating;
extern COctopusObjectivePiezo*  glob_m_pObjPiezo;
extern COctopusStage545*	glob_m_pStage545;



//extern COctopusLED*       glob_m_pLED;

COctopusCamera::COctopusCamera(CWnd* pParent)
: CDialog(COctopusCamera::IDD, pParent)
, m_focusType(2)
, m_focusInterval(1)
, m_focusResult(_T(""))
{    
	B.files_written			= 0;
	B.savetofile			= false;
	B.W						= 0;
	B.H						= 0;
	B.automatic_gain		= true;
	B.manual_gain			= 0.05;
	B.pics_per_file			= 1;
	B.Camera_Thread_running = false;
	B.bin                   = 1;
	B.memory				= NULL;
	B.savetime				= 0;
	B.nd_setting            = 0;
	B.expt_frame            = 0;
	B.expt_time             = 0;
	B.ROI_changed           = true;
	B.SetFocus              = false;
	FrameTransfer           = false;
	TTL                     = false;
	B.moveG                 = false;
	B.mask_now              = 2;
	B.mask_next             = 3;
	B.pictype               = 0;
	B.lines_mm              = 20;
	B.Acq_Timings			= 0; 
	//allocate lots of memory and be done with it...
	//this is way more than we should ever need.

	B.memory = new u16 [710 * 512 * 512]; // Does this mean 200 images can be saved to single file?

	u32_Exposure_Time_Single_ms = 100;
	u32_Exposure_Time_Movie_ms  = 100;
	//u16_Kinetic_Series_Length_frames = 1;



	u16_Gain_Mult_Factor_Single = 1;
	u16_Gain_Mult_Factor_Movie  = 1;
	m_IDC_MANUAL_GAIN           = B.manual_gain;
	m_DISPLAY_GAIN_CHOICE       = 1;
	m_DISPLAY_VSSPEED           = 3;
	m_DISPLAY_GL                = 0;
	//m_DISPLAY_LMM               = 1; //set elsewhere
	m_DISPLAY_BIN_CHOICE        = B.bin - 1;
	m_IDC_PICTURES_PER_FILE     = B.pics_per_file;
	m_CCD_target_temp		    = 10;
	m_CCD_current_temp          = 0;
	B.CameraExpTime_ms          = u32_Exposure_Time_Single_ms;
	B.EM_GAIN					= u16_Gain_Mult_Factor_Single;
	//B.KSeriesLength_frames = u16_Kinetic_Series_Length_frames;

	//default the converter to 10 MHZ 14 bit conventional
	m_DISPLAY_HSSPEED           = 0;
	B.Ampl_Conv					= false; //does not really matter. will be set correctly by OnHSSPEED_4() anyway

	VERIFY(m_bmp_no.LoadBitmap(IDB_NO));
	VERIFY(m_bmp_yes.LoadBitmap(IDB_YES));

	CString currentDir;
	GetCurrentDirectory( MAX_PATH, currentDir.GetBufferSetLength(MAX_PATH) );
	currentDir.ReleaseBuffer();

	currentDir.Append(_T("\\fallback"));
	pathname_display = currentDir;
	B.pathname       = pathname_display;

	if( Create(COctopusCamera::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(_T(" COctopusCamera(CWnd* pParent) "));

	//this if for old versus new andor iXon 
	em_limit = 300;

	if ( B.Andor_new ) 
		em_limit = 950; //we are dealing with a new camera

}

void COctopusCamera::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control( pDX, IDC_CAS_TEMP_CURRENT, m_Temperature_Now);
	DDX_Control(pDX, IDC_STATIC_GAIN1,      m_g1_text1);
	DDX_Control(pDX, IDC_STATIC_GAIN2,      m_g1_text2);

	DDX_Text( pDX, IDC_CAS_EXPOSURE_TIME_SINGLE, u32_Exposure_Time_Single_ms);
	DDV_MinMaxLong(pDX, u32_Exposure_Time_Single_ms, 1, 10000);
	DDX_Text( pDX, IDC_CAS_EXPOSURE_TIME_MOVIE, u32_Exposure_Time_Movie_ms);
	DDV_MinMaxLong(pDX, u32_Exposure_Time_Movie_ms, 1, 10000);

	DDX_Text( pDX, IDC_CAS_GAIN_SINGLE, u16_Gain_Mult_Factor_Single);
	DDV_MinMaxInt(pDX, u16_Gain_Mult_Factor_Single, 1, em_limit);
	DDX_Text( pDX, IDC_CAS_GAIN_MOVIE, u16_Gain_Mult_Factor_Movie);
	DDV_MinMaxInt(pDX, u16_Gain_Mult_Factor_Movie, 1, em_limit);

	DDX_Text( pDX, IDC_CAS_MANUAL_GAIN,	m_IDC_MANUAL_GAIN);
	DDV_MinMaxDouble(pDX, m_IDC_MANUAL_GAIN, 0.0001, 10.0);

	DDX_Text( pDX, IDC_CAS_PICTURES_PER_FILE, m_IDC_PICTURES_PER_FILE);
	DDV_MinMaxInt(pDX, m_IDC_PICTURES_PER_FILE, 1, 10000);

	DDX_Text( pDX, IDC_CAS_TEMP_TARGET, m_CCD_target_temp);
	DDV_MinMaxInt(pDX, m_CCD_target_temp, -80, 25);

	DDX_Radio( pDX, IDC_CAS_DISPLAY_GAIN_MANUAL, m_DISPLAY_GAIN_CHOICE);
	DDX_Radio( pDX, IDC_CAS_DISPLAY_BIN_1, m_DISPLAY_BIN_CHOICE);

	DDX_Radio( pDX, IDC_CAS_VSSPEED_0, m_DISPLAY_VSSPEED);

	DDX_Text(pDX, IDC_CAS_PATHNAME,   pathname_display);

	DDX_Control(pDX, IDC_CAS_SAVEONOFF_BMP,  m_status_save);

	DDX_Radio(pDX, IDC_GENERAL_FOCUS, m_focusType);
	DDX_Text(pDX, IDC_FOCUS_TEXT, m_focusResult);
	DDX_Text(pDX, IDC_FOCUS_EDIT, m_focusInterval);
	DDV_MinMaxDouble(pDX, m_focusInterval, 0.1, 5);
}  

BEGIN_MESSAGE_MAP(COctopusCamera, CDialog)

	ON_BN_CLICKED(IDC_CAS_STARTSTOP_PICTURE,  StartPicture)
	ON_BN_CLICKED(IDC_CAS_START_MOVIE,	      StartMovie)
	ON_BN_CLICKED(IDC_CAS_STOP_MOVIE,	      StopCameraThread)

	ON_BN_CLICKED(IDC_CAS_SETPATH,			  OnSetPath)

	ON_EN_KILLFOCUS(IDC_CAS_EXPOSURE_TIME_SINGLE,   OnKillfocusGeneral)
	ON_EN_KILLFOCUS(IDC_CAS_EXPOSURE_TIME_MOVIE,    OnKillfocusGeneral)

	ON_EN_KILLFOCUS(IDC_CAS_GAIN_SINGLE,			OnKillfocusGeneral)
	ON_EN_KILLFOCUS(IDC_CAS_GAIN_MOVIE,				OnKillfocusGeneral)

	ON_EN_KILLFOCUS(IDC_CAS_TEMP_TARGET,			OnKillfocusTempTarget)
	ON_EN_KILLFOCUS(IDC_CAS_MANUAL_GAIN,			OnKillfocusManualGain)

	ON_EN_KILLFOCUS(IDC_CAS_PICTURES_PER_FILE,		OnKillfocusPicturesPerFile)

	ON_BN_CLICKED(IDC_CAS_DISPLAY_GAIN_MANUAL,		OnDisplayGainManual)
	ON_BN_CLICKED(IDC_CAS_DISPLAY_GAIN_AUTOMATIC,	OnDisplayGainAutomatic)

	ON_BN_CLICKED(IDC_CAS_DISPLAY_BIN_1,			OnBinning1x1)
	ON_BN_CLICKED(IDC_CAS_DISPLAY_BIN_2,			OnBinning2x2)
	ON_BN_CLICKED(IDC_CAS_DISPLAY_BIN_4,			OnBinning4x4)
	ON_BN_CLICKED(IDC_CAS_DISPLAY_BIN_8,			OnBinning8x8)

	ON_BN_CLICKED(IDC_CAS_VSSPEED_0, OnVSSPEED_0)
	ON_BN_CLICKED(IDC_CAS_VSSPEED_1, OnVSSPEED_1)
	ON_BN_CLICKED(IDC_CAS_VSSPEED_2, OnVSSPEED_2)
	ON_BN_CLICKED(IDC_CAS_VSSPEED_3, OnVSSPEED_3)
	ON_BN_CLICKED(IDC_CAS_VSSPEED_4, OnVSSPEED_4)



	ON_BN_CLICKED(IDC_CAS_SAVEONOFF,  OnFileChange)


	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_GENERAL_FOCUS, &COctopusCamera::OnBnClickedGeneralFocus)
	ON_BN_CLICKED(IDC_BEAD_FOCUS, &COctopusCamera::OnBnClickedGeneralFocus)
	ON_BN_CLICKED(IDC_NOFOCUS, &COctopusCamera::OnBnClickedGeneralFocus)
	ON_EN_CHANGE(IDC_FOCUS_EDIT, &COctopusCamera::OnEnChangeFocusEdit)
	ON_EN_CHANGE(IDC_CAS_PICTURES_PER_FILE, &COctopusCamera::OnEnChangeCasPicturesPerFile)
	ON_STN_CLICKED(IDC_CAS_TEMP_CURRENT, &COctopusCamera::OnStnClickedCasTempCurrent)
END_MESSAGE_MAP()

BOOL COctopusCamera::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char aBuffer[256];

	if ( FrameTransfer )
		CheckDlgButton(IDC_CAS_FT, BST_CHECKED);
	else
		CheckDlgButton(IDC_CAS_FT, BST_UNCHECKED);

	if ( TTL )
		CheckDlgButton(IDC_CAS_TTL, BST_CHECKED);
	else
		CheckDlgButton(IDC_CAS_TTL, BST_UNCHECKED);

	if ( B.moveG )
		CheckDlgButton(IDC_CAS_ALIGN, BST_CHECKED);
	else
		CheckDlgButton(IDC_CAS_ALIGN, BST_UNCHECKED);

	//GetCurrentDirectory(256, (LPWSTR)aBuffer);
	GetCurrentDirectory(256, aBuffer);
	// Look in current working directory
	// for driver files

	//if( Initialize(aBuffer) != DRV_SUCCESS ) 
	//	AfxMessageBox(_T("Failure Opening Andor Camera"));

	Opencam(); 

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(_T(" COctopusCamera::OnInitDialog() "));

	return TRUE;
}

COctopusCamera::~COctopusCamera() 
{
	if( B.Camera_Thread_running ) 
	{

		StopCameraThread();
	}
	//AndorCamera//
	/*if( CoolerOFF() != DRV_SUCCESS ) 
	{
		AfxMessageBox(_T("Failure turning Cooler Off"));
	}
	else {
		AfxMessageBox(_T("Turning camera cooler off."));
	}


	do{ GetTemperature(&m_CCD_current_temp); } while ( m_CCD_current_temp < 0 );

	if ( B.memory != NULL ) 
	{
		delete [] B.memory;
		B.memory = NULL;
	}

	ShutDown();*/
	AfxMessageBox(_T("Camera cooler is now shut down."));	
	if( glob_m_pPictureDisplay != NULL ) 
	{
		glob_m_pPictureDisplay->DestroyWindow();
		delete glob_m_pPictureDisplay;
		glob_m_pPictureDisplay = NULL;
	}

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" ~COctopusCamera() ");
}

/*********************************************************************************
* Open the Camera
*********************************************************************************/
void COctopusCamera::SetExposureTime_Single_ms( int exptime )
{
	u32_Exposure_Time_Single_ms = exptime;
	UpdateData(false);
}
void COctopusCamera::SetExposureTime_Movie_ms( int exptime )
{
	u32_Exposure_Time_Movie_ms = exptime;
	UpdateData(false);
}
void COctopusCamera::SetCameraGain( int u32_EMCCDGain )
{
	u16_Gain_Mult_Factor_Single = u32_EMCCDGain;
	if (u32_EMCCDGain < 1)
		u16_Gain_Mult_Factor_Single = 1;
	if (u32_EMCCDGain > 300)
		u16_Gain_Mult_Factor_Single = 300;

	//AndorCamera//SetEMCCDGain( u16_Gain_Mult_Factor_Single );
	UpdateData(false);
}
void COctopusCamera::SetNKinetics( int kslength )
{
	u16_kinetic_series_length_frames  = kslength;
	B.pics_per_file = kslength;
	//AndorCamera//SetNumberKinetics( u16_kinetic_series_length_frames );
	UpdateData(false);
}
void COctopusCamera::SetMovieGain( int u32_EMCCDGain )
{
	u16_Gain_Mult_Factor_Movie = u32_EMCCDGain;

	if (u32_EMCCDGain < 1)
		u16_Gain_Mult_Factor_Movie = 1;
	if (u32_EMCCDGain > 300)
		u16_Gain_Mult_Factor_Movie = 300;

	//AndorCamera//SetEMCCDGain( u16_Gain_Mult_Factor_Movie );
	UpdateData(false);
}

void COctopusCamera::Opencam() 
{
	//AndorCamera//SetTemperature( m_CCD_target_temp );

	/*if( CoolerON()!= DRV_SUCCESS ) 
		AfxMessageBox(_T("Failure turning Cooler On"));*/

	int xpixels;
	int ypixels;

	//AndorCamera//GetDetector( &xpixels, &ypixels );

	B.CCD_x_phys_e = xpixels;
	B.CCD_y_phys_e = ypixels;

	B.ROI_target.x1 = 1;
	B.ROI_target.y1 = 1;
	B.ROI_target.x2 = B.CCD_x_phys_e;
	B.ROI_target.y2 = B.CCD_y_phys_e;

	SetROI_To_Default();

	//keep track of temperature
	SetTimer( TIMER_TEMP, 2000, NULL ); 

	//enable access to gains > 300
	u16 setEM;//AndorCamera//
	/*if ( B.Andor_new )
		setEM = SetEMAdvanced(1);
	SetOutputAmplifier( 0 );*/
	/*setEM = SetEMAdvanced(1);*/ 
	OnVSSPEED_3();

	OnPIC_0();

	OnLMM_1(); //10 L/mm

	//OnHSSPEED_4();	//set to conv. amplifier

	// 0 = 0-255
	// 1 = 0-4096
	// 2 = linear
	// 3 = real

	//if ( B.Andor_new )
	//{
	//	TRACE(_T("ANDOR NEW\n"));
	//	OnGL_2();
	//}
	//else
	//{
	//	TRACE(_T("ANDOR not new\n"));
	//	OnGL_0();
	//}

	CString str;

	if ( B.Andor_new )
	{
		str.Format(_T("EMCCD Gain (1 to 300):"));
		//GetDlgItem( IDC_CAS_GL_0 )->EnableWindow( false );
		//GetDlgItem( IDC_CAS_GL_1 )->EnableWindow( false );
	}
	else
	{
		str.Format(_T("EMCCD Gain (1 to 300):"));
		GetDlgItem( IDC_CAS_GL_1 )->EnableWindow( false );
		GetDlgItem( IDC_CAS_GL_2 )->EnableWindow( false );
	}

	m_g1_text1.SetWindowText( str );
	m_g1_text2.SetWindowText( str );

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::OpenCam() ");

}

/*********************************************************************************
*********************************************************************************/
void COctopusCamera::StartPicture()
{
	CString temp;
	if (m_focusType == 2)
	{
		TakePicture();
	}
	else
	{
		if( glob_m_pObjPiezo == NULL ) 
		{
			AfxMessageBox(_T("Z-Piezo not initialized"));
			return;
		}
		if( glob_m_pStage545 == NULL ) 
		{
			AfxMessageBox(_T("Stage not initialized"));
			return;
		}
		numStep = 0;
		m_focusResult = "";
		totalSteps = 10;
		int prevPos = B.position_z;
		glob_m_pObjPiezo->MoveRelZ( -5 * m_focusInterval );
		int curPos = B.position_z;
		bestFocusScoreStep = 0;
		focus = true;

		if (m_focusType == 0)
		{
			B.CurrFocusScore = 0;
			B.focus_score = 0;
			SetTimer( TIMER_GEN_FOCUS, 1000, NULL );
		}
		else
		{
			B.CurrFocusScore = 100;
			B.focus_score = 100;
			SetTimer( TIMER_ROI_FOCUS, 1000, NULL );
		}



	}


}
void COctopusCamera::TakePicture() 
{
	if( B.Camera_Thread_running ) 
		return;

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::TakePicture() ");

	//AndorCamera//SetReadMode(4);             //set read out mode to image
	//AndorCamera//SetAcquisitionMode(1);      //set mode for single scan
	//AndorCamera//SetExposureTime( float(u32_Exposure_Time_Single_ms) * 0.001 );
	//AndorCamera//SetTriggerMode( 0 );		// internal trigger



	//AndorCamera//SetEMCCDGain( u16_Gain_Mult_Factor_Single );
	B.CameraExpTime_ms = u32_Exposure_Time_Single_ms;
	B.EM_GAIN =  u16_Gain_Mult_Factor_Single;
	//if ( B.Andor_new )//AndorCamera//
	//	SetShutter( 0, 1, 0, 0 ); // Always open
	//else
	//	SetShutter( 1, 1, 0, 0 ); // Always Open


	//AndorCamera//SetImage( 1, 1, 1,  512, 1,  512 );


	if ( glob_m_pPictureDisplay == NULL || B.ROI_changed ) 
		OpenWindow();

	//AndorCamera//StartAcquisition();

	int status = 0;
	//AndorCamera//do{ GetStatus( &status ); } while ( status != DRV_IDLE );

	//AndorCamera//GetAcquiredData16( B.memory, B.H * B.W );

	if ( glob_m_pPictureDisplay != NULL ) 
		glob_m_pPictureDisplay->Update_Bitmap( B.memory, 1 );


}
void COctopusCamera::TakeKineticPicture() 
{
if( B.Camera_Thread_running ) 
	return;

if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::TakeKineticPicture) ");
//AndorCamera//
	//SetReadMode(4);             //set read out mode to image
	//SetAcquisitionMode(3);      //set mode for kinetic scan
	//SetExposureTime( float(u32_Exposure_Time_Single_ms) * 0.001 );
	//SetTriggerMode( 0 );		// internal trigger 
	//SetNumberKinetics( u16_kinetic_series_length_frames ); // should be good
	//int NumAccums = 1;
	//SetNumberAccumulations( NumAccums );
	//SetAccumulationCycleTime( float(u32_Exposure_Time_Single_ms) * 0.001);
	//SetKineticCycleTime( float(u32_Exposure_Time_Single_ms) * 0.001 );
 //   SetEMCCDGain( u16_Gain_Mult_Factor_Single );
	B.CameraExpTime_ms = u32_Exposure_Time_Single_ms;
	B.EM_GAIN =  u16_Gain_Mult_Factor_Single;
	//if ( B.Andor_new )//AndorCamera//
	//	SetShutter( 0, 1, 0, 0 ); // Always open
	//else
	//	SetShutter( 1, 1, 0, 0 ); // Always Open
	//

//AndorCamera//SetImage( 1, 1, 1,  512, 1,  512 );


if ( glob_m_pPictureDisplay == NULL || B.ROI_changed ) 
		OpenWindow();

	//AndorCamera//StartAcquisition();
	StartWaitThread(); //necessary to image multiple times in a row

	int status = 0;
	//AndorCamera//do{ GetStatus( &status ); } while ( status != DRV_IDLE ); // necessary as well to get the right number of pictures in a file
    
	//AndorCamera//GetAcquiredData16( B.memory, u16_kinetic_series_length_frames * B.H * B.W); //Necessary to get correct number of images/save only one file

	//if ( glob_m_pPictureDisplay != NULL ) 
	//glob_m_pPictureDisplay->Update_Bitmap( B.memory, u16_kinetic_series_length_frames ); //this saves multiple copies of possibly (?) the same data (3 x number of images in 3 separate files)
	//float etime = 4.0;
	//etime = u32_Exposure_Time_Single_ms * u16_kinetic_series_length_frames;
	//SetTimer( TIMER_SCRIPT, (u32)etime * 1000, NULL );
	//StopCameraThread();

}
/*********************************************************************************
* Start/stop continuous data aquisition
*********************************************************************************/

void COctopusCamera::StartMovie( void )
{	

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::StartMovie( void ) ");

	if( B.Camera_Thread_running ) 
		return;

	DisableDlgMovie();

	//AndorCamera//SetAcquisitionMode( 5 );		// set to run till abort
	//AndorCamera//SetReadMode( 4 );				// image mode

	//AndorCamera//SetExposureTime( float(u32_Exposure_Time_Movie_ms) * 0.001 );

	B.CameraExpTime_ms = u32_Exposure_Time_Movie_ms;

	//AndorCamera//SetKineticCycleTime( 0 );

	//if ( B.Ampl_Conv )
	//	SetEMCCDGain( 0 );
	//else
	//AndorCamera//SetEMCCDGain( int(u16_Gain_Mult_Factor_Movie) );
	B.EM_GAIN =  u16_Gain_Mult_Factor_Movie;
	if( TTL )
	{	
		//AndorCamera//SetTriggerMode( 1 );		// base external trigger
		//AndorCamera//SetFastExtTrigger( 1 );     // fast external trigger
	}
	else
		//AndorCamera//SetTriggerMode( 0 );		// internal trigger
//AndorCamera//
	//if ( FrameTransfer )
	//	SetFrameTransferMode( 1 );	// FT mode
	//else
	//	SetFrameTransferMode( 0 );	// non FT mode
	//AndorCamera//
	//if ( B.Andor_new )
	//	SetShutter( 0, 1, 0, 0 ); // Always open
	//else
	//	SetShutter( 1, 1, 0, 0 ); // Always Open

	if ( B.ROI_changed ) 
		SetROI();

	//SetImage( B.ROI_actual.bin, B.ROI_actual.bin, \
	B.ROI_actual.x1,  B.ROI_actual.x2,  \
	B.ROI_actual.y1,  B.ROI_actual.y2 );
	//AndorCamera//SetImage( 1, 1, 1,  512, 1,  512 );
	if ( glob_m_pPictureDisplay == NULL || B.ROI_changed ) 
		OpenWindow();

	//AndorCamera//StartAcquisition();

	StartWaitThread();

	if( TTL )
		SetTimer( TIMER_TRIG, B.CameraExpTime_ms + 100, NULL ); 
}

void COctopusCamera::StopCameraThread() 
{
	KillWaitThread();

	//AndorCamera//AbortAcquisition();

	KillTimer( TIMER_TRIG ); 

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::StopCameraThread() ");
}

/*********************************************************************************
********************	Utility functions ****************************************
*********************************************************************************/

void COctopusCamera::SetROI_To_Default( void )  // set ROI to default
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::SetROI_To_Default( void ) ");

	B.ROI_target.bin = 1;
	B.ROI_target.x1  = 0;
	B.ROI_target.y1  = 0;
	//B.ROI_target.x2  = B.CCD_x_phys_e;
	//B.ROI_target.y2  = B.CCD_y_phys_e;
	B.ROI_target.x2  = 0;
	B.ROI_target.y2  = 0;


	SetROI();
}

void COctopusCamera::SetROI( void ) 
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::SetROI( void ) ");

	// ROInew setting quality control
	if ((B.ROI_target.x2 > B.CCD_x_phys_e ) || (B.ROI_target.x2 < B.ROI_target.x1 )) B.ROI_target.x2 = B.CCD_x_phys_e;
	if ((B.ROI_target.y2 > B.CCD_y_phys_e ) || (B.ROI_target.y2 < B.ROI_target.y1 )) B.ROI_target.y2 = B.CCD_y_phys_e;
	if ((B.ROI_target.x1 > B.ROI_target.x2) || (B.ROI_target.x1 < 0)) B.ROI_target.x1 = 0;
	if ((B.ROI_target.y1 > B.ROI_target.y2) || (B.ROI_target.y1 < 0)) B.ROI_target.y1 = 0;

	////write the new
	B.ROI_actual.x1 = B.ROI_target.x1;	// serial start
	B.ROI_actual.x2 = B.ROI_target.x2;	// serial end
	B.ROI_actual.y1 = B.ROI_target.y1;	// parallel start
	B.ROI_actual.y2 = B.ROI_target.y2;	// parallel end

	BinChange();
}

void COctopusCamera::BinChange( void )
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::BinChange( void ) ");

	B.ROI_actual.bin = B.bin;

	//while (((B.ROI_actual.x2 - B.ROI_actual.x1 + 1) % B.ROI_actual.bin) != 0) B.ROI_actual.x2--;
	//while (((B.ROI_actual.y2 - B.ROI_actual.y1 + 1) % B.ROI_actual.bin) != 0) B.ROI_actual.y2--;
	//
	//B.W = (B.ROI_actual.x2 - B.ROI_actual.x1 + 1) / B.ROI_actual.bin;
	//B.H = (B.ROI_actual.y2 - B.ROI_actual.y1 + 1) / B.ROI_actual.bin;
	B.W = 512;
	B.H = 512;

	//should be true anyway, but just in case.....
	B.ROI_changed = true;
}

void COctopusCamera::OpenWindow( void ) 
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::OpenWindow( void ) ");

	if( glob_m_pPictureDisplay != NULL ) 
	{
		glob_m_pPictureDisplay->DestroyWindow();
		delete glob_m_pPictureDisplay;
		glob_m_pPictureDisplay = NULL;
		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(" glob_m_pPictureDisplay != NULL ");
	}
	if( glob_m_pPictureDisplay == NULL ) 
	{
		B.ROI_changed = false;
		glob_m_pPictureDisplay = new COctopusPictureDisplay( this, B.W, B.H );
		glob_m_pPictureDisplay->Create( IDC_CAS_DISP );
		glob_m_pPictureDisplay->ShowWindow( SW_SHOW );
		glob_m_pPictureDisplay->Create_Bitmap(); 
		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(" glob_m_pPictureDisplay == NULL ");
	}
}

void COctopusCamera::OnKillfocusGeneral() { UpdateData( true ); }

void COctopusCamera::OnKillfocusTempTarget() 
{
	UpdateData( true );
	//AndorCamera//SetTemperature ( m_CCD_target_temp );
}

void COctopusCamera::OnKillfocusManualGain() 
{
	UpdateData( true );
	B.manual_gain = m_IDC_MANUAL_GAIN;
}

void COctopusCamera::OnKillfocusPicturesPerFile() 
{
	UpdateData( true );
	B.pics_per_file = m_IDC_PICTURES_PER_FILE;
}

void COctopusCamera::OnBinning1x1() { m_DISPLAY_BIN_CHOICE = 0; B.bin = 1; BinChange(); }
void COctopusCamera::OnBinning2x2() { m_DISPLAY_BIN_CHOICE = 1; B.bin = 2; BinChange(); }
void COctopusCamera::OnBinning4x4() { m_DISPLAY_BIN_CHOICE = 2; B.bin = 4; BinChange(); }
void COctopusCamera::OnBinning8x8() { m_DISPLAY_BIN_CHOICE = 3; B.bin = 8; BinChange(); }

void COctopusCamera::OnVSSPEED_0() { m_DISPLAY_VSSPEED = 0; OnVSSPEED_Report(); }
void COctopusCamera::OnVSSPEED_1() { m_DISPLAY_VSSPEED = 1; OnVSSPEED_Report(); }
void COctopusCamera::OnVSSPEED_2() { m_DISPLAY_VSSPEED = 2; OnVSSPEED_Report(); }
void COctopusCamera::OnVSSPEED_3() { m_DISPLAY_VSSPEED = 3; OnVSSPEED_Report(); }
void COctopusCamera::OnVSSPEED_4() { m_DISPLAY_VSSPEED = 4; OnVSSPEED_Report(); }

void COctopusCamera::OnHSSPEED_0() { m_DISPLAY_HSSPEED = 0; OnHSSPEED_Report(); }
void COctopusCamera::OnHSSPEED_1() { m_DISPLAY_HSSPEED = 1; OnHSSPEED_Report(); }
void COctopusCamera::OnHSSPEED_2() { m_DISPLAY_HSSPEED = 2; OnHSSPEED_Report(); }
void COctopusCamera::OnHSSPEED_3() { m_DISPLAY_HSSPEED = 3; OnHSSPEED_Report(); }
void COctopusCamera::OnHSSPEED_4() { m_DISPLAY_HSSPEED = 4; OnHSSPEED_Report(); }
void COctopusCamera::OnHSSPEED_5() { m_DISPLAY_HSSPEED = 5; OnHSSPEED_Report(); }

void COctopusCamera::OnLMM_0() { m_DISPLAY_LMM = 0; B.lines_mm =  5; UpdateData( false ); }
void COctopusCamera::OnLMM_1() { m_DISPLAY_LMM = 1; B.lines_mm = 10; UpdateData( false ); }
void COctopusCamera::OnLMM_2() { m_DISPLAY_LMM = 2; B.lines_mm = 20; UpdateData( false ); }
void COctopusCamera::OnLMM_3() { m_DISPLAY_LMM = 3; B.lines_mm = 40; UpdateData( false ); }


void COctopusCamera::OnPIC_0() { m_DISPLAY_PIC = 0; B.pictype = 0; UpdateData( false ); }
void COctopusCamera::OnPIC_1() { m_DISPLAY_PIC = 1; B.pictype = 1; UpdateData( false ); }
void COctopusCamera::OnPIC_2() { m_DISPLAY_PIC = 2; B.pictype = 2; UpdateData( false ); }

void COctopusCamera::OnGL_0() 
{ 
	m_DISPLAY_GL = 0; 
	//AndorCamera//SetEMGainMode( 2 );
	UpdateData( false );
}

void COctopusCamera::OnGL_1() 
{ 
	if ( B.Andor_new ) 
	{
		m_DISPLAY_GL = 1; 
		//AndorCamera//SetEMGainMode( 2 );
	}
	else
	{
		m_DISPLAY_GL = 0; 
		//AndorCamera//SetEMGainMode( 0 );
	}
	UpdateData( false );
}

void COctopusCamera::OnGL_2() 
{ 
	if ( B.Andor_new ) 
	{
		m_DISPLAY_GL = 2; 
		//AndorCamera//SetEMGainMode( 2 );
	}
	else
	{
		m_DISPLAY_GL = 0; 
		//AndorCamera//SetEMGainMode( 0 );
	}
	UpdateData( false );
}

void COctopusCamera::OnVSSPEED_Report()
{
	//AndorCamera//SetVSSpeed( m_DISPLAY_VSSPEED ); 
}

void COctopusCamera::OnHSSPEED_Report()
{
	//if ( m_DISPLAY_HSSPEED == 0 )//AndorCamera//
	//{
	//	B.Ampl_Conv = false;
	//	SetOutputAmplifier( 0 ); // EMCCD
	//	SetADChannel( 0 );		 // 14 bit
	//	SetHSSpeed( 0, 0 );      // 10Mhz
	//	SetPreAmpGain( 1 );		 // 5x
	//} 
	//else if ( m_DISPLAY_HSSPEED == 1 )
	//{
	//	B.Ampl_Conv = false;
	//	SetOutputAmplifier( 0 ); // EMCCD
	//	SetADChannel( 0 );		 // 14 bit
	//	SetHSSpeed( 0, 1 );      // 5 Mhz
	//	SetPreAmpGain( 1 );		 // 5x
	//} 
	//else if ( m_DISPLAY_HSSPEED == 2 )
	//{
	//	B.Ampl_Conv = false;
	//	SetOutputAmplifier( 0 ); // EMCCD
	//	SetADChannel( 0 );		 // 14 bit
	//	SetHSSpeed( 0, 2 );      // 3 Mhz
	//	SetPreAmpGain( 1 );		 // 5x
	//} 
	//else if ( m_DISPLAY_HSSPEED == 3 )
	//{
	//	B.Ampl_Conv = false;
	//	SetOutputAmplifier( 0 ); // EMCCD
	//	SetADChannel( 1 );		 // 16 bit
	//	SetHSSpeed( 0, 3 );      // 1MHz
	//	SetPreAmpGain( 1 );		 // 5x
	//} 
	//else if ( m_DISPLAY_HSSPEED == 4 )
	//{
	//	B.Ampl_Conv = true;
	//	SetOutputAmplifier( 1 ); // Conv
	//	SetADChannel( 1 );       // 16 bit
	//	SetHSSpeed( 1, 1 );      // 1 Mhz
	//	SetPreAmpGain( 0 );		 // none
	//} 
	//else if ( m_DISPLAY_HSSPEED == 5 )
	//{
	//	B.Ampl_Conv = true;
	//	SetOutputAmplifier( 1 ); // Conv 
	//	SetADChannel( 0 );       // 14 bit
	//	SetHSSpeed( 1, 0 );      // 3 Mhz
	//	SetPreAmpGain( 0 );		 // none
	//}
}

void COctopusCamera::OnDisplayGainManual() 
{ 
	m_DISPLAY_GAIN_CHOICE = 0;
	UpdateData( false );
	B.automatic_gain = false;
}

void COctopusCamera::OnDisplayGainAutomatic() 
{ 
	m_DISPLAY_GAIN_CHOICE = 1;
	UpdateData( false );
	B.automatic_gain = true;  
}

//void COctopusCamera::StepGrating( void ) 
//{
//	if (glob_m_pGrating == NULL) return;
//
//	/*****************************************
//	* Show the next mask                     *
//	*****************************************/	
//	// which mask?
//	//  5 l/mm = 8
//	// 10 l/mm = 4
//	// 20 l/mm = 2
//	// 40 l/mm = 1
//
//	double msk = 0;
//
//	if(      B.lines_mm ==  5 )  msk = 8.0;
//	else if( B.lines_mm == 10 )  msk = 4.0;
//	else if( B.lines_mm == 20 )  msk = 2.0;
//	else if( B.lines_mm == 40 )  msk = 1.0;
//
//	double middle = glob_m_pGrating->GetMiddle();
//
//	if ( B.moveG == false ) 
//	{
//		//do nothing
//	} 
//	else if ( B.mask_next == 1 )
//	{	
//		glob_m_pGrating->MoveTo( middle - (msk * 8.33) );
//		B.mask_now  = 1;
//		B.mask_next = 2;
//	}
//	else if ( B.mask_next == 2 )
//	{	
//		glob_m_pGrating->MoveTo( middle );
//		B.mask_now  = 2;
//		B.mask_next = 3;
//	}
//	else if ( B.mask_next == 3 )
//	{	
//		glob_m_pGrating->MoveTo( middle + (msk * 8.33) );
//		B.mask_now  = 3;
//		B.mask_next = 1;
//	}
//}

void COctopusCamera::OnFileChange( void ) 
{
	if ( !B.savetofile ) 
		StartSaving();
	else 
		StopSaving();
}

void COctopusCamera::OnFocusChange( void ) 
{
	if ( B.Camera_Thread_running == false ) 
		B.SetFocus = true;
}

void COctopusCamera::StartSaving() 
{

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::StartSaving() ");

	if ( !B.savetofile ) 
	{	
		B.savetofile = true;
		m_status_save.SetBitmap( m_bmp_yes );
		if ( glob_m_pGoodClock == NULL ) return;
		B.savetime = (double)glob_m_pGoodClock->End();
		B.expt_frame = 0; //counter for saved frames
	}
}

void COctopusCamera::StopSaving()
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::StopSaving() ");

	if ( B.savetofile ) 
	{ 
		B.savetofile = false;
		m_status_save.SetBitmap( m_bmp_no );
		if (glob_m_pPictureDisplay != NULL) 
			glob_m_pPictureDisplay->Close_The_File();
	}
}

void COctopusCamera::FileClose()
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::FileClose() ");

	if (glob_m_pPictureDisplay != NULL) 
		glob_m_pPictureDisplay->Close_The_File();
}

void COctopusCamera::OnSetPath( void ) 
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(_T(" COctopusCamera::OnSetPath( void ) "));

	CFileDialog FileDlg( FALSE, _T("dth"), NULL, OFN_HIDEREADONLY, \
		_T("Octopus Header (*.dth)|*.dth|"), this);

	CString pathname;

	if( FileDlg.DoModal() == IDOK ) 
	{
		pathname   = FileDlg.GetPathName();
		pathname   = pathname.Left(pathname.Find(_T(".dth")));
		pathname_display = pathname;
		B.pathname       = pathname;
		UpdateData( false );
	}
}

void COctopusCamera::OnTimer( UINT nIDEvent ) 
{

	if( nIDEvent == TIMER_TEMP ) 
	{
		int status = 0;

		//AndorCamera//GetStatus( &status );
		//AndorCamera//
		//if( status == DRV_IDLE ) 
		//{
		//	GetTemperature( &m_CCD_current_temp );
		//	CString str;
		//	str.Format(_T("Current T: %d C"), m_CCD_current_temp);
		//	if( IsWindowVisible() ) 
		//		m_Temperature_Now.SetWindowText( str );
		//}
	}

	else if( nIDEvent == TIMER_TRIG ) 
	{
		//move the grating
		//StepGrating(); 	
		//Sleep(100);
		/*
		if ( glob_m_pLED != NULL )
		glob_m_pLED->TTL_Pulse_Up(); //fire the camera
		*/
	}
	else if( nIDEvent == TIMER_GEN_FOCUS)
	{

		if (numStep <= totalSteps)
		{
			TakePicture();
			if (B.focus_score < B.CurrFocusScore)
			{
				B.focus_score = B.CurrFocusScore; //best focus score saved in focus_score
				bestFocusScoreStep = numStep;
			}
			CString timer;
			timer.Format("%.2f %.2f\n",B.position_z,B.CurrFocusScore);
			m_focusResult= m_focusResult + timer;
			UpdateData(false);
			numStep++;
			glob_m_pObjPiezo->MoveRelZ( m_focusInterval );
			Sleep(100);
			//glob_m_pStage545->MoveRelY(B.roi_xmove);
			//Sleep(100);
			//glob_m_pStage545->MoveRelX(B.roi_ymove);
		}
		else
		{

			KillTimer(TIMER_GEN_FOCUS);
			glob_m_pObjPiezo->MoveRelZ( (bestFocusScoreStep - totalSteps - 1 ) * m_focusInterval );

			CString timer;
			timer.Format("General: %.2f\n",B.focus_score);
			m_focusResult= m_focusResult + _T("--------\n") + timer;
			UpdateData(false);

			focus = false;
			TakePicture();
			//B.focus_in_progress = false;
		}

	}
	else if( nIDEvent == TIMER_ROI_FOCUS)
	{

		if (numStep <= totalSteps)
		{
			TakePicture();
			if (B.LM_fwhm < B.CurrFocusScore)
			{
				B.LM_fwhm = B.CurrFocusScore;
				bestFocusScoreStep = numStep;
			}
			CString timer;
			timer.Format("%.2f %.2f\n",B.position_z,B.LM_fwhm);
			m_focusResult= m_focusResult + timer;
			UpdateData(false);
			numStep++;
			glob_m_pObjPiezo->MoveRelZ( m_focusInterval );
			Sleep(100);
			glob_m_pStage545->MoveRelY(B.roi_xmove);
			Sleep( 3000 );
			glob_m_pStage545->MoveRelX(B.roi_ymove);
		}
		else
		{
			KillTimer(TIMER_ROI_FOCUS);
			glob_m_pObjPiezo->MoveRelZ( (bestFocusScoreStep - totalSteps - 1 ) * m_focusInterval );

			TakePicture();
			Sleep(100);
			CString timer;
			timer.Format("ROI: %.2f\n",B.LM_fwhm);
			m_focusResult= m_focusResult + _T("--------\n") + timer;
			UpdateData(false);

		}

	}

	CDialog::OnTimer(nIDEvent);
}

/************************************
Read info from camera
***********************************/

void COctopusCamera::StartWaitThread( void )
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::StartWaitThread( void ) ");
	DWORD WaitThreadID;

	B.Camera_Thread_running = true;

	WaitThread = CreateThread( 0, 4096, FocusThread, NULL, 0, &WaitThreadID );
}

void COctopusCamera::KillWaitThread( void )
{
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(" COctopusCamera::KillWaitThread( void ) ");

	if( !B.Camera_Thread_running ) return;

	B.Camera_Thread_running = false;


	WaitForSingleObject( WaitThread, 500 );

	Sleep( 500 );

	EnableDlg();

	if ( !CloseHandle( WaitThread ) ) 
		AfxMessageBox(_T("Error: could not close camera thread"));
}

DWORD WINAPI FocusThread(LPVOID lpParameter) 
{

	long first      = 0;
	long last       = 0;
	long validfirst = 0;
	long validlast  = 0;
	long retval     = 0;

	u16 NumberOfNewImages = 0;
	//AndorCamera//
	//while( B.Camera_Thread_running ) 
	//{		
	//	retval = GetNumberNewImages( &first, &last );
	//	// GetNumberNewImages is a wierd function 
	//	// when there is one image to transfer, 
	//	// last == first, and thus number of images = 0;
	//	// likewise, transfer two images = ... 1, 1+1 and so forth

	//	NumberOfNewImages = u16(last - first + 1);
	//	if ( retval == DRV_NO_NEW_DATA )
	//	{
	//		//do nothing
	//	} 
	//	else if ( retval == DRV_SUCCESS && NumberOfNewImages >= 1 )
	//	{
	//		//let's get cracking...
	//		retval = GetImages16( first, last,               \
	//			B.memory, B.W * B.H * NumberOfNewImages, \
	//			&validfirst, &validlast);
	//		//GetMostRecentImage(B.memory)

	//		if ( retval == DRV_SUCCESS )
	//		{
	//			if ( glob_m_pPictureDisplay != NULL )
	//				glob_m_pPictureDisplay->Update_Bitmap( B.memory, NumberOfNewImages );
	//		} 
	//	}
	//}
	return 0;	
}

/************************************
User interface etc
***********************************/

void COctopusCamera::OnBnClickedCasFt()
{
	int l_ChkBox = m_ctlFTCheckBox.GetCheck();

	if (l_ChkBox == 0) 
		FrameTransfer = false;
	else if (l_ChkBox == 1) 
		FrameTransfer = true;
}

void COctopusCamera::OnBnClickedCasTTL()
{
	int l_ChkBox = m_ctlTTLCheckBox.GetCheck();

	if (l_ChkBox == 0) 
		TTL = false;
	else if (l_ChkBox == 1) 
		TTL = true;
}

void COctopusCamera::OnBnClickedCasAL()
{
	int l_ChkBox = m_ctlALCheckBox.GetCheck();

	if (l_ChkBox == 0) 
		B.moveG = false;
	else if (l_ChkBox == 1) 
		B.moveG = true;
}

BOOL COctopusCamera::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int id    = LOWORD(wParam); // Notification code
	if( id == 2 ) return FALSE; // Trap ESC key
	if( id == 1 ) return FALSE; // Trap RTN key
	return CDialog::OnCommand(wParam, lParam);
}

void COctopusCamera::EnableDlg( void ) 
{
	GetDlgItem( IDC_CAS_STARTSTOP_PICTURE		)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_START_MOVIE				)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_STOP_MOVIE				)->EnableWindow( false );
	GetDlgItem( IDC_CAS_EXPOSURE_TIME_SINGLE	)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_EXPOSURE_TIME_MOVIE		)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_PICTURES_PER_FILE		)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_TEMP_TARGET				)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_GAIN_SINGLE				)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_GAIN_MOVIE				)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_1			)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_2			)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_4			)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_8			)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_SETPATH					)->EnableWindow( true  );
	//GetDlgItem( IDC_CAS_HSSPEED_0	)->EnableWindow( true );
	//GetDlgItem( IDC_CAS_HSSPEED_1	)->EnableWindow( true );
	//GetDlgItem( IDC_CAS_HSSPEED_2	)->EnableWindow( true );
	//GetDlgItem( IDC_CAS_HSSPEED_3	)->EnableWindow( true );
	//GetDlgItem( IDC_CAS_HSSPEED_4	)->EnableWindow( true );
	//GetDlgItem( IDC_CAS_HSSPEED_5	)->EnableWindow( true );
	GetDlgItem( IDC_CAS_VSSPEED_0	)->EnableWindow( true );
	GetDlgItem( IDC_CAS_VSSPEED_1	)->EnableWindow( true );
	GetDlgItem( IDC_CAS_VSSPEED_2	)->EnableWindow( true );
	GetDlgItem( IDC_CAS_VSSPEED_3	)->EnableWindow( true );
	GetDlgItem( IDC_CAS_VSSPEED_4	)->EnableWindow( true );
}

void COctopusCamera::DisableDlgMovie( void ) 
{
	GetDlgItem( IDC_CAS_STARTSTOP_PICTURE	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_STOP_MOVIE			)->EnableWindow( true  );
	GetDlgItem( IDC_CAS_START_MOVIE			)->EnableWindow( false );
	DisableDlg();
}

void COctopusCamera::DisableDlg( void ) 
{
	//GetDlgItem( IDC_CAS_HSSPEED_0	)->EnableWindow( false );
	//GetDlgItem( IDC_CAS_HSSPEED_1	)->EnableWindow( false );
	//GetDlgItem( IDC_CAS_HSSPEED_2	)->EnableWindow( false );
	//GetDlgItem( IDC_CAS_HSSPEED_3	)->EnableWindow( false );
	//GetDlgItem( IDC_CAS_HSSPEED_4	)->EnableWindow( false );
	//GetDlgItem( IDC_CAS_HSSPEED_5	)->EnableWindow( false );

	GetDlgItem( IDC_CAS_VSSPEED_0	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_VSSPEED_1	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_VSSPEED_2	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_VSSPEED_3	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_VSSPEED_4	)->EnableWindow( false );

	GetDlgItem( IDC_CAS_EXPOSURE_TIME_SINGLE	)->EnableWindow( false );
	GetDlgItem( IDC_CAS_EXPOSURE_TIME_MOVIE		)->EnableWindow( false );
	GetDlgItem( IDC_CAS_PICTURES_PER_FILE		)->EnableWindow( false );
	GetDlgItem( IDC_CAS_TEMP_TARGET				)->EnableWindow( false );
	GetDlgItem( IDC_CAS_GAIN_SINGLE				)->EnableWindow( false );
	GetDlgItem( IDC_CAS_GAIN_MOVIE				)->EnableWindow( false );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_1			)->EnableWindow( false );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_2			)->EnableWindow( false );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_4			)->EnableWindow( false );
	GetDlgItem( IDC_CAS_DISPLAY_BIN_8			)->EnableWindow( false );
	GetDlgItem( IDC_CAS_SETPATH					)->EnableWindow( false );
}

void COctopusCamera::OnBnClickedGeneralFocus()
{

	UpdateData(true);
	B.focusType = m_focusType;
	if (m_focusType == 2)
	{
		GetDlgItem(IDC_FOCUS_EDIT)->EnableWindow(false);
	}
	else
	{
		GetDlgItem(IDC_FOCUS_EDIT)->EnableWindow(true);
	}
	// TODO: Add your control notification handler code here
}

void COctopusCamera::OnEnChangeFocusEdit()
{
	UpdateData(true);
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void COctopusCamera::OnEnChangeCasPicturesPerFile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void COctopusCamera::OnStnClickedCasTempCurrent()
{
	// TODO: Add your control notification handler code here
}
