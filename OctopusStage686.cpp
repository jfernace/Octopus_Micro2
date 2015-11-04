
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusStage686.h"
#include "OctopusLog.h"
#include "Pi_gcs2_dll.h"

extern COctopusGlobals B;
extern COctopusLog* glob_m_pLog;

#define IDC_STAGEm_SAVE						1870
#define IDC_STAGEm_SAVE_GOTO                1871
#define IDC_STAGEm_JON                      1872

COctopusStage686::COctopusStage686(CWnd* pParent)
	: CDialog(COctopusStage686::IDD, pParent)
{    

	B.position_x    = 0.0;
	B.position_y    = 0.0;

	initialized     = false;
	XY_connected	= false;
	
	bIsMoving[0]    = FALSE;
	bIsMoving[1]    = FALSE;
	
	middle			= 12.5;
	range			= 10.0;
	
	save_xy[0]		= middle;
	save_xy[1]		= middle;
	
	target_xy[0]	= middle;
	target_xy[1]	= middle;

	if( Create(COctopusStage686::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	first_tick = true;

	SetTimer( TIMER_STAGE, 500, NULL );
}

BOOL COctopusStage686::OnInitDialog()
{
    CDialog::OnInitDialog();

	m_Font.CreatePointFont(80, _T("MS Shell Dlg"));

    /*CRect(6,151,35,33) factor of 2*/
	//left top right bottom
	m_btn_JOY_ON_OFF.Create(_T("Joystick On/Off"),\
		WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_MULTILINE|BS_FLAT,\
		CRect(10,240,10+60,240+60),this,IDC_STAGEm_JON);

	m_btn_SavePos.Create(_T("Save this pos"),\
		WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_MULTILINE|BS_FLAT,\
		CRect(110,24,110+100,24+20),this,IDC_STAGEm_SAVE);

	m_btn_GoToPos.Create(_T("GoTo saved pos"),\
		WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_MULTILINE|BS_FLAT,\
		CRect(110,45,110+100,45+20),this,IDC_STAGEm_SAVE_GOTO);

	m_btn_JOY_ON_OFF.SetFont(&m_Font);
	m_btn_SavePos.SetFont(&m_Font);
    m_btn_GoToPos.SetFont(&m_Font);

	return TRUE;
}

void COctopusStage686::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == TIMER_STAGE ) 
	{
		if( first_tick ) 
		{
			OnStepSize3();
			
			first_tick = false;
			InitializeStage(); 
			StageCenter();

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(" PI 686 stage initialized ");
		}
		if( initialized ) 
		{
			GetPosition();
			ShowPosition();
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void COctopusStage686::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio(pDX,		IDC_STAGEm_SSIZE_1,	 m_Radio_S);
	DDX_Control(pDX,	IDC_STAGEm_POS,		 m_Pos);
	DDX_Control(pDX,	IDC_STAGEm_POS_SAVE, m_Pos_Save);
}  

BEGIN_MESSAGE_MAP(COctopusStage686, CDialog)
	ON_BN_CLICKED(IDC_STAGEm_RIGHT,	      MoveBack)
	ON_BN_CLICKED(IDC_STAGEm_LEFT,		  MoveFwd)
	ON_BN_CLICKED(IDC_STAGEm_BACK,	      MoveRight)
	ON_BN_CLICKED(IDC_STAGEm_FWD,		  MoveLeft)
	ON_BN_CLICKED(IDC_STAGEm_SAVE,	      OnSave)
	ON_BN_CLICKED(IDC_STAGEm_SAVE_GOTO,   OnSaveGoTo)
	ON_BN_CLICKED(IDC_STAGEm_CENTER,	  StageCenter)
	ON_BN_CLICKED(IDC_STAGEm_SSIZE_1,     OnStepSize1)
	ON_BN_CLICKED(IDC_STAGEm_SSIZE_2,     OnStepSize2)
	ON_BN_CLICKED(IDC_STAGEm_SSIZE_3,     OnStepSize3)
	ON_BN_CLICKED(IDC_STAGEm_SSIZE_4,     OnStepSize4)
	ON_BN_CLICKED(IDC_STAGEm_SSIZE_5,     OnStepSize5)
	ON_BN_CLICKED(IDC_STAGEm_JON,		  OnJoyStickOnOff)
	ON_WM_TIMER()
END_MESSAGE_MAP()

COctopusStage686::~COctopusStage686() {}

void COctopusStage686::InitializeStage( void ) 
{

   const char* XY_string = "0111017919"; 
   
   ID_XY = PI_ConnectUSB( XY_string );

   if (ID_XY < 0)
   {
		AfxMessageBox(_T("PI stage 0111017919 not connected"));
   } 
   else
   {
		debug.Format(_T(" PI coarse stage XY connected "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		XY_connected = true;
   }

   // Get the name of the connected axis 
   if ( !PI_qSAI(ID_XY, axis_xy, 6) )
   {
		AfxMessageBox(_T("PI stage XY could not get name"));
   }

   // Switch on the Servo
   BOOL bFlag = TRUE;
   if( !PI_SVO(ID_XY, axis_xy, &bFlag ) )
   {
		AfxMessageBox(_T("PI stage XY closed loop could not switch on"));
   }

   // Wait until done.
   bFlag = FALSE;
   
   while( bFlag != TRUE )
   {
	   PI_IsControllerReady(ID_XY, &bFlag);
   }

   // Reference the axis using the reference switch
   if( !PI_FRF(ID_XY, axis_xy) )
   {
		AfxMessageBox(_T("PI stage XY reference problem"));
   }
   
   // Wait until done.
   bFlag = FALSE;
   
   while( bFlag != TRUE )
   {
		PI_IsControllerReady(ID_XY, &bFlag);
   }
   /*
   if( !PI_VEL(ID_X, axis_x, &vel_x) )
   {
		AfxMessageBox(_T("PI stage X velocity set problem"));
   }
   */
	initialized = true;
}


void COctopusStage686::Close( void ) 
{ 
	initialized = false; 
	
	if ( XY_connected )
	{
		PI_CloseConnection( ID_XY );
		XY_connected = false;
	}

	debug.Format(_T(" PI 686 stage closed "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
}

void COctopusStage686::MoveRelX( double dist )
{
	double new_target = target_xy[0] + dist;

	if( new_target > (middle - range) && new_target < (middle + range) ) 
	{
		target_xy[0] = new_target;
		if (XY_connected) PI_MOV(ID_XY, axis_xy, target_xy);
		debug.Format(_T(" PI 686 stage X move to %.3f"), target_xy[0]);
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}
}

void COctopusStage686::MoveRelY( double dist )
{
	double new_target = target_xy[1] + dist;

	if( new_target > (middle - range) && new_target < (middle + range) ) 
	{
		target_xy[1] = new_target;
	    if (XY_connected) PI_MOV(ID_XY, axis_xy, target_xy);
		debug.Format(_T(" PI 686 stage Y move to %.3f"), target_xy[1]);
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}
}

void COctopusStage686::MoveLeft(  void ) { MoveRelY( +1 * stepsize ); }
void COctopusStage686::MoveRight( void ) { MoveRelY( -1 * stepsize ); }
void COctopusStage686::MoveBack(  void ) { MoveRelX( -1 * stepsize ); }
void COctopusStage686::MoveFwd(   void ) { MoveRelX( +1 * stepsize ); }

void COctopusStage686::OnStepSize1() { m_Radio_S = 0; stepsize = 0.0005; UpdateData (false); };
void COctopusStage686::OnStepSize2() { m_Radio_S = 1; stepsize = 0.0050; UpdateData (false); };
void COctopusStage686::OnStepSize3() { m_Radio_S = 2; stepsize = 0.0200; UpdateData (false); };
void COctopusStage686::OnStepSize4() { m_Radio_S = 3; stepsize = 0.2000; UpdateData (false); };
void COctopusStage686::OnStepSize5() { m_Radio_S = 4; stepsize = 1.0000; UpdateData (false); };


void COctopusStage686::StageCenter( void ) 
{	
	target_xy[0] = middle;
	target_xy[1] = middle;

	if (XY_connected) PI_MOV(ID_XY, axis_xy, target_xy);

	debug.Format(_T(" PI 686 stage centered "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);

}

void COctopusStage686::OnSave( void ) 
{
	save_xy[0] = target_xy[0];
	save_xy[1] = target_xy[1];

	if( IsWindowVisible() ) 
	{
	    CString str;
		str.Format(_T("SaveX:%.3f\nSaveY:%.3f"), save_xy[0], save_xy[1]);
		m_Pos_Save.SetWindowText( str );
	}

	debug.Format(_T(" PI 686 stage save position "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
}

void COctopusStage686::OnSaveGoTo( void ) 
{	
	target_xy[0] = save_xy[0];
	target_xy[1] = save_xy[1];
	
	if (XY_connected) PI_MOV(ID_XY, axis_xy, target_xy);

	debug.Format(_T(" PI 686 stage goto save "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
}
void COctopusStage686::OnJoyStickOnOff( void )
{
	bJONState[0] = TRUE;
	bJONState[1] = TRUE;

	int J_ID[2];
	
	J_ID[0] = 1;
	J_ID[1] = 2;
	
	PI_qJON(ID_XY, J_ID, bJONState, 2);
		
	if( bJONState[0] == TRUE )
	{
		//turn it off
		bJONState[0] = FALSE;
		bJONState[1] = FALSE;
		PI_JON(ID_XY, J_ID, bJONState, 2);

		GetPosition();
		target_xy[0] = B.position_x;
		target_xy[1] = B.position_y;
	} 
	else
	{
		bJONState[0] = TRUE;
		bJONState[1] = TRUE;
		PI_JON(ID_XY, J_ID, bJONState, 2);
	}
}

void COctopusStage686::GetPosition( void ) 
{ 

	if ( XY_connected )
	{
		bIsMoving[0] = TRUE;
		bIsMoving[1] = TRUE;

		dPos[0] = B.position_x;
		dPos[1] = B.position_y;

		PI_IsMoving(ID_XY, axis_xy, bIsMoving);
		
		if (!bIsMoving[0] && !bIsMoving[1])
		{
			PI_qPOS(ID_XY, axis_xy, dPos);
			B.position_x = dPos[0];
			B.position_y = dPos[1];
		}
	}
}

void COctopusStage686::ShowPosition( void )
{
	if( IsWindowVisible() ) 
	{
	    CString str;
		str.Format(_T("X:%.5f\nY:%.5f"), B.position_x, B.position_y);
		m_Pos.SetWindowText( str );
	}
}

BOOL COctopusStage686::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}

void COctopusStage686::OnKillfocusGeneral() { UpdateData( true ); }
