#include "stdafx.h"
#include "Octopus.h"
#include "OctopusFocus.h"
#include "OctopusCameraDlg.h"
//#include "OctopusLED.h"
#include "OctopusLog.h"
#include "OctopusObjectivePiezo.h"

extern COctopusGlobals B;
//extern COctopusLED*             glob_m_pLED;
extern COctopusCamera*          glob_m_pCamera;
extern COctopusLog*             glob_m_pLog;
extern COctopusObjectivePiezo*  glob_m_pObjPiezo;

COctopusFocus::COctopusFocus(CWnd* pParent)
	: CDialog(COctopusFocus::IDD, pParent)
	, m_focusType(FALSE)
{    
	if( Create(COctopusFocus::IDD, pParent) ) 
	{
		ShowWindow( SW_SHOW );
		TRACE("Focus Window Created\n");
	}

	first_tick                  = true;
	B.focus_in_progress         = false;
	focus_score_best            = 0.0;
	focus_step_best             = 0;
	focus_step_current          = 0;
	focus_step_size             = 5.0; //microns

	VERIFY(m_bmp_no.LoadBitmap(IDB_NO));
	VERIFY(m_bmp_yes.LoadBitmap(IDB_YES));

	//SetTimer( TIMER_FOCUS, 100, NULL );

	TRACE("Initializing Focus Constructor\n");

}

BOOL COctopusFocus::OnInitDialog()
{
     CDialog::OnInitDialog();
     //SetWindowPos(NULL, 185, 775, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
     return TRUE;
}

COctopusFocus::~COctopusFocus() 
{  
}

void COctopusFocus::Close( void ) 
{ 
    KillTimer( TIMER_FOCUS );
}

void COctopusFocus::OnFocus( void ) 
{	
	UpdateData(true);
	if ( B.focus_in_progress == false )
	{
		if (m_focusType == false)
		{
			AutoFocus( focus_step_size );
		}
		else
		{
			ROIFocus( focus_step_size );
		}
		debug.Format(_T(" AutoFocus( focus_step_size ) "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}
	else
	{
		EndFocus();
		debug.Format(_T(" OnFocus:EndFocus() ")); 
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}
}

void COctopusFocus::AutoFocus( double fsi )
{
	debug.Format(_T(" COctopusFocus::AutoFocus( double fsi ) ")); 
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

	if( glob_m_pCamera == NULL ) 
	{
		AfxMessageBox(_T("Sorry - camera is not initialized.\nCan't focus without the camera."));
		return;
	}

	if ( B.Camera_Thread_running ) 
	{
		AfxMessageBox(_T("Please stop the camera and then try again."));
		return;
	}

	// out of range abort
	if ( B.position_z < 10.0 || B.position_z > 190.0 ) 
	{
		AfxMessageBox(_T("Z-piezo too close to end of range"));
		return;
	}

	if ( fsi < 0.10 ) 
		fsi = 0.10;
	else if ( fsi > 20.0 ) 
		fsi = 20.0;
	
	focus_step_size = fsi;
	
	//focus in progress?
	m_status.SetBitmap( m_bmp_yes );
		
	if( glob_m_pObjPiezo == NULL ) 
	{
		AfxMessageBox(_T("Sorry - piezo stage not on.\nCan't focus without the stage."));
		return;
	}

	//move to beginning - all the way down
	glob_m_pObjPiezo->MoveRelZ( -5 * fsi );

	// make sure we use fresh score info
	B.focus_score = 0.0; 

	glob_m_pCamera->TakePicture();

	Sleep( glob_m_pCamera->GetExposureTime_Single_ms() + 200 );

	focus_score_best     = B.focus_score;
	focus_step_best      = 0;
	focus_step_current   = 0;
	B.focus_in_progress  = true;

	SetTimer( TIMER_FOCUS, 100, NULL );

	result.Format(_T("%-3d: %-7.2f %-7.2f\n"),  \
					focus_step_current,         \
					B.position_z,               \
					B.focus_score);

	m_Score.SetWindowText( result );

}

void COctopusFocus::ROIFocus( double fsi )
{
	TRACE(_T("ROI Focus\n"));
	TRACE(_T("----------------------------------------------\n"));
		debug.Format(_T(" COctopusFocus::AutoFocus( double fsi ) ")); 
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

	if( glob_m_pCamera == NULL ) 
	{
		AfxMessageBox(_T("Sorry - camera is not initialized.\nCan't focus without the camera."));
		return;
	}

	if ( B.Camera_Thread_running ) 
	{
		AfxMessageBox(_T("Please stop the camera and then try again."));
		return;
	}

	// out of range abort
	if ( B.position_z < 10.0 || B.position_z > 190.0 ) 
	{
		AfxMessageBox(_T("Z-piezo too close to end of range"));
		return;
	}

	if ( fsi < 0.10 ) 
		fsi = 0.10;
	else if ( fsi > 20.0 ) 
		fsi = 20.0;
	
	focus_step_size = fsi;
	
	//focus in progress?
	m_status.SetBitmap( m_bmp_yes );
		
	if( glob_m_pObjPiezo == NULL ) 
	{
		AfxMessageBox(_T("Sorry - piezo stage not on.\nCan't focus without the stage."));
		return;
	}

		//move to beginning - all the way down
	glob_m_pObjPiezo->MoveRelZ( -5 * fsi );

	// make sure we use fresh score info
	B.focus_score = 0.0; 

	glob_m_pCamera->TakePicture();

	Sleep( glob_m_pCamera->GetExposureTime_Single_ms() + 200 );

	focus_score_best     = B.focus_score;
	focus_step_best      = 0;
	focus_step_current   = 0;
	B.focus_in_progress  = true;

	SetTimer( TIMER_ROI, 100, NULL );

	result.Format(_T("%-3d: %-7.2f %-7.2f\n"),  \
					focus_step_current,         \
					B.position_z,               \
					B.focus_score);

}

void COctopusFocus::EndFocus( void )
{
	debug.Format(_T(" COctopusFocus::EndFocus( void ) "));
	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(debug);

	m_status.SetBitmap( m_bmp_no );

	B.focus_in_progress = false;

	Close();
	if ( focus_step_best > 10) 
		focus_step_best = 10;
	
	if ( focus_step_best <  0) 
		focus_step_best =  0;

	glob_m_pObjPiezo->MoveRelZ( (focus_step_best - 10) * focus_step_size );	

	//make sure the "in focus" and centered frame is displayed
	glob_m_pCamera->TakePicture();

}

void COctopusFocus::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == TIMER_FOCUS ) 
	{
		TRACE(_T("Focus Timer is Running\n"));
		if ( B.focus_in_progress )
		{
			debug.Format(_T(" else if ( B.focus_in_progress ) "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
			
			if ( focus_step_current < 10 )
			{
				debug.Format(_T(" if ( focus_step_current < 10 ) "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				
				focus_step_current++;

				glob_m_pObjPiezo->MoveRelZ( focus_step_size );

				debug.Format(_T(" MoveRelZ( %.3f ) "), focus_step_size );
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

				glob_m_pCamera->TakePicture(); 	

				Sleep( glob_m_pCamera->GetExposureTime_Single_ms() + 200 );

				result.AppendFormat(_T("%-3d: %-7.2f %-7.4f\n"),    \
									focus_step_current,			    \
									B.position_z,		            \
									B.focus_score);

				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(result);

				m_Score.SetWindowText( result );

				debug.Format(_T(" m_Score.SetWindowText( result ) "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

				if ( B.focus_score > focus_score_best ) 
				{
					debug.Format(_T(" if ( B.focus_score > focus_score_best ) "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

					focus_score_best  = B.focus_score;
					focus_step_best   = focus_step_current;
				}
			} 
			else
			{
				EndFocus();
				debug.Format(_T(" EndFocus() "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
			}
		} 
	}
	if( nIDEvent == TIMER_ROI ) 
	{
		if ( B.focus_in_progress )
		{
			debug.Format(_T(" else if ( B.focus_in_progress ) "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
			
			if ( focus_step_current < 10 )
			{
				debug.Format(_T(" if ( focus_step_current < 10 ) "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				
				focus_step_current++;

				glob_m_pObjPiezo->MoveRelZ( focus_step_size );

				debug.Format(_T(" MoveRelZ( %.3f ) "), focus_step_size );
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

				glob_m_pCamera->TakePicture(); 	

				Sleep( glob_m_pCamera->GetExposureTime_Single_ms() + 200 );

				result.AppendFormat(_T("%-3d: %-7.2f %-7.4f\n"),    \
									focus_step_current,			    \
									B.position_z,		            \
									B.focus_score);

				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(result);

				m_Score.SetWindowText( result );

				debug.Format(_T(" m_Score.SetWindowText( result ) "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

				if ( B.focus_score > focus_score_best ) 
				{
					debug.Format(_T(" if ( B.focus_score > focus_score_best ) "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

					focus_score_best  = B.focus_score;
					focus_step_best   = focus_step_current;
				}
			} 
			else
			{
				EndFocus();
				debug.Format(_T(" EndFocus() "));
				if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
			}
		} 
	}
	CDialog::OnTimer(nIDEvent);
}

void COctopusFocus::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Control(pDX, IDC_PIEZO_FOCUS_SCORE,	      m_Score);
	DDX_Control(pDX, IDC_PIEZO_FOCUS_RUNNING_BMP, m_status);
	DDX_Check(pDX, IDC_FOCUSTYPE, m_focusType);
}  

BEGIN_MESSAGE_MAP(COctopusFocus, CDialog)
	ON_BN_CLICKED(IDC_PIEZO_FOCUS, OnFocus)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void COctopusFocus::OnKillfocusGeneral() { UpdateData( true ); }

BOOL COctopusFocus::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}