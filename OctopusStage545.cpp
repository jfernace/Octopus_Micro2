
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusStage545.h"
#include "OctopusLog.h"
//#include "Pi_gcs2_dll.h"

extern COctopusGlobals B;
extern COctopusLog* glob_m_pLog;


int saved_joystick_states[1];
int joystick_ids[1];

COctopusStage545::COctopusStage545(CWnd* pParent)
: CDialog(COctopusStage545::IDD, pParent)
{    

	B.position_x = 0.0;
	B.position_y = 0.0;
	B.position_z = 0.0;

	m_gotox = 1;
	m_gotoy = 1;

	initialized  = false;
	connected    = false;

	if( Create(COctopusStage545::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	first_tick = true;

	SetTimer( TIMER_STAGE, 500, NULL );

}

void COctopusStage545::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == TIMER_STAGE ) 
	{
		if( first_tick ) 
		{
			OnStepSize3();

			first_tick = false;


			//Editing
			InitializeStage(); 
			GetPosition();

			//middlex	 = 100.0;
			//middley	 = 100.0;
			//middlez	 = 100.0;
			middlex	 = 12;//B.position_x;
			middley	 = 12;//B.position_y;
			middlez	 = 0;

			//m_gotox = 0;
			//m_gotoy = 0;


			range    = 24.975; //max is 25mm, but lets leave a little wiggle room

			savex    = 12;//middlex;
			savey    = 12;//middley;
			savez    = middlez;

			target_x = savex;
			target_y = savey;
			target_z = savez;
			//InitializeStage(); 


			StageCenter();

			OnSave(); 

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(" PI 545 stage initialized ");
		}
		if( initialized ) 
		{
			GetPosition();
			ShowPosition();
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void COctopusStage545::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio(pDX,		IDC_STAGE_SSIZE_1,			  m_Radio_S);
	DDX_Control(pDX,	IDC_STAGE_POS,				  m_Pos);
	DDX_Control(pDX,	IDC_STAGE_POS_SAVE,		      m_Pos_Save);

	DDX_Text( pDX, IDC_GOTOY, m_gotoy);
	DDV_MinMaxDouble(pDX, m_gotoy, 0, 25000);

	DDX_Text( pDX, IDC_GOTOX, m_gotox);
	DDV_MinMaxDouble(pDX, m_gotox, 0, 25000);
}  

BEGIN_MESSAGE_MAP(COctopusStage545, CDialog)
	ON_BN_CLICKED(IDC_STAGE_RIGHT,	     MoveRight)
	ON_BN_CLICKED(IDC_STAGE_LEFT,		 MoveLeft)
	ON_BN_CLICKED(IDC_STAGE_BACK,	     MoveFwd)
	ON_BN_CLICKED(IDC_STAGE_FWD,		 MoveBack)
	ON_BN_CLICKED(IDC_STAGE_UP,	         MoveUp)
	ON_BN_CLICKED(IDC_STAGE_DOWN,		 MoveDown)
	ON_BN_CLICKED(IDC_STAGE_SAVE,	     OnSave)
	ON_BN_CLICKED(IDC_STAGE_SAVE_GOTO,   OnSaveGoTo)
	ON_BN_CLICKED(IDC_STAGE_CENTER,		 StageCenter)
	ON_BN_CLICKED(IDC_STAGE_SSIZE_1,     OnStepSize1)
	ON_BN_CLICKED(IDC_STAGE_SSIZE_2,     OnStepSize2)
	ON_BN_CLICKED(IDC_STAGE_SSIZE_3,     OnStepSize3)
	ON_BN_CLICKED(IDC_STAGE_SSIZE_4,     OnStepSize4)
	ON_BN_CLICKED(IDC_STAGE_SSIZE_5,     OnStepSize5)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_GOTOXY, OnBnClickedButtonGotoxy)
END_MESSAGE_MAP()

void COctopusStage545::OnKillfocusGeneral() { UpdateData( true ); }

COctopusStage545::~COctopusStage545() 
{
	Close();
}

void COctopusStage545::MoveLeft(  void ) { MoveRelY( +1 * stepsize ); }
void COctopusStage545::MoveRight( void ) { MoveRelY( -1 * stepsize ); }
void COctopusStage545::MoveBack(  void ) { MoveRelX( -1 * stepsize ); }
void COctopusStage545::MoveFwd(   void ) { MoveRelX( +1 * stepsize ); }
void COctopusStage545::MoveUp(    void ) { MoveRelZ( -1 * stepsize ); }
void COctopusStage545::MoveDown(  void ) { MoveRelZ( +1 * stepsize ); }

void COctopusStage545::MoveRelX( double dist )
{
	BOOL bIsMoving[3];
	bIsMoving[0] = TRUE;

	//Box Comment out PI fxns JF 11/4/15
	/*if ( connected )
	{

		PI_IsMoving(ID, NULL, bIsMoving);

		if ( bIsMoving[0] == FALSE ) 
		{

			double dPos[3];

			double new_target = target_x + dist;
			CString str;
			str.Format("MoveRelX: %f\n", new_target);
			TRACE(str);

			if( new_target > 0 && new_target < (range) ) 
			{
				target_x = new_target;
				dPos[0]  = target_x;
				dPos[1]  = target_y;
				dPos[2]  = target_z;

				if(!PI_MOV(ID, axis, dPos))
				{
					debug.Format(_T(" PI 545 stage MOV X failed "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
				else
				{
					debug.Format(_T(" PI 545 stage MOV X to %.3f"), target_x);
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
			}
		}	
	}	*/

}

void COctopusStage545::MoveRelY( double dist )
{
	BOOL bIsMoving[3];
	bIsMoving[0] = TRUE;
	//Box Comment out PI fxns JF 11/4/15
	/*if ( connected )
	{
		PI_IsMoving(ID, NULL, bIsMoving);
		if ( bIsMoving[0] == FALSE ) 
		{

			double dPos[3];

			double new_target = target_y + dist;
			CString str;
			str.Format("MoveRelY: %f\n", new_target);
			TRACE(str);

			if( new_target > 0 && new_target < range ) 
			{
				target_y = new_target;

				dPos[0]  = target_x;
				dPos[1]  = target_y;
				dPos[2]  = target_z;

				if(!PI_MOV(ID, axis, dPos))
				{
					debug.Format(_T(" PI 545 stage MOV Y failed "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
				else
				{
					debug.Format(_T(" PI 545 stage MOV Y to %.3f"), target_y);
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
			}
		}	
	}	*/	
}


void COctopusStage545::MoveRelZ( double dist )
{
	BOOL bIsMoving[3];
	bIsMoving[0] = TRUE;
	//Box Comment out PI fxns JF 11/4/15
	/*if ( connected )
	{
		PI_IsMoving(ID, NULL, bIsMoving);

		if ( bIsMoving[0] == FALSE ) 
		{

			double dPos[3];

			double new_target = target_z + dist;

			if( new_target > (middlez - range) && new_target < (middlez + range) ) 
			{
				target_z = new_target;

				dPos[0]  = target_x;
				dPos[1]  = target_y;
				dPos[2]  = target_z;

				if(!PI_MOV(ID, axis, dPos))
				{
					debug.Format(_T(" PI 545 stage MOV Z failed "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
				else
				{
					debug.Format(_T(" PI 545 stage MOV Z to %.3f"), target_z);
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
			}
		}	
	}	*/	
}

//need to change indicators in window

void COctopusStage545::OnStepSize1() { m_Radio_S = 0; stepsize = 0.001; UpdateData (false); };
void COctopusStage545::OnStepSize2() { m_Radio_S = 1; stepsize = 0.010; UpdateData (false); };
void COctopusStage545::OnStepSize3() { m_Radio_S = 2; stepsize = 0.100; UpdateData (false); };
void COctopusStage545::OnStepSize4() { m_Radio_S = 3; stepsize = 0.250; UpdateData (false); };
void COctopusStage545::OnStepSize5() { m_Radio_S = 4; stepsize = 1.000; UpdateData (false); };

int COctopusStage545::InitializeStage( void ) 
{
	//const char* piezo_string = "0109018015";
	const char* piezo_string = "0111036763";

	// PI's default in their app didn't work, so setting this explicitly here.
	// If you change the pin settings on the controller, please reset this!
	ID = -1; //ID = PI_ConnectUSBWithBaudRate( piezo_string, 38400 );// THIS WAS ADDED to maintain global fxn JF 11/4/15 
	CString str;
	str.Format(_T("ID:%d\n"), ID);
	TRACE(str);
	if (ID < 0)
	{
		AfxMessageBox(_T("PI 545 stage not connected"));
	}
	else
	{
		debug.Format(_T(" PI 545 stage connected "));

		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(debug);

		connected = true;
	}

	// Get the name of the connected axis //Box Comment out PI fxns JF 11/4/15
	/*if ( !PI_qSAI(ID, axis, 16) )
	{
		AfxMessageBox(_T("PI 545 stage could not get name"));
	}*/



	int joystick_off_states[1];

	joystick_ids[0] = 1;
	joystick_off_states[0] = 0;

	// Get and save old joystick id//Box Comment out PI fxns JF 11/4/15
	/*if ( !PI_qJON(ID,joystick_ids,saved_joystick_states,1) )  {
		debug.Format(_T(" PI 545 joystick state query failed "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		initialized = false;
		PI_CloseConnection(ID);
		return 1;
	}*/

	//Box Comment out PI fxns JF 11/4/15
	// Turn off joystick mode
	/*if ( !PI_JON(ID,joystick_ids,joystick_off_states,1) ) {
		debug.Format(_T(" PI 545 joystick turn off failed "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		initialized = false;
		PI_CloseConnection(ID);
		return 1;
	}*/

	/*
	///////////////////////////////////////////////
	// Switch the piezo channels to online mode. //
	///////////////////////////////////////////////
	int iChnl[3];
	int iVal[3];

	// select the desired piezo channels to change.
	iChnl[0] = 1;
	iChnl[1] = 2;
	iChnl[2] = 3;

	// select the corresponding online mode (1 = online, 0 = offline).
	iVal[0] = 1;
	iVal[1] = 1;
	iVal[2] = 1;



	Call the ONLine mode command
	if(!PI_ONL(ID, iChnl, iVal, 3) )
	{
	debug.Format(_T(" PI 545 stage ONL failed "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	initialized = false;
	PI_CloseConnection(ID);
	return 1;
	}
	*/

	/////////////////////////////////////////
	// close the servo loop (closed-loop). //
	/////////////////////////////////////////
	BOOL bFlags[3];

	// Switch on the Servo for all axes
	bFlags[0] = TRUE; // servo on for first  axis in the string 'axes'.
	bFlags[1] = TRUE; // servo on for second axis in the string 'axes'.
	bFlags[2] = TRUE; // servo on for third  axis in the string 'axes'.
	//Box Comment out PI fxns JF 11/4/15
	// call the SerVO mode command.
	/*if(!PI_SVO(ID, axis, bFlags))
	{
		debug.Format(_T(" PI 545 stage SVO failed "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		initialized = false;
		PI_CloseConnection(ID);
		return 1;
	}*/

	initialized = true;
	return 0;
}



void COctopusStage545::GetPosition( void ) 
{ 
	double dPos[3];

	dPos[0] = 0.0; 
	dPos[1] = 0.0; 
	dPos[2] = 0.0; 

	BOOL bIsMoving[3];
	bIsMoving[0] = TRUE;
	//Box Comment out PI fxns JF 11/4/15
	/*if ( connected )
	{
		PI_IsMoving(ID, NULL, bIsMoving);

		if ( bIsMoving[0] == FALSE ) 
		{
			PI_qPOS(ID, axis, dPos);
		}
	}*/

	//this gets saved to file
	B.position_x = dPos[0];
	B.position_y = dPos[1];
	B.position_z = dPos[2];
}

void COctopusStage545::Close( void ) 
{ 
	initialized = false; 
	//Box Comment out PI fxns JF 11/4/15
	// Return joystick mode to previous state
	/*if ( !PI_JON(ID,joystick_ids,saved_joystick_states,1) ) {
		debug.Format(_T(" PI 545 joystick return to previous state failed "));
	}

	if ( connected )
	{
		PI_CloseConnection( ID );
		connected = false;
	}*/

	debug.Format(_T(" PI 545 stage closed "));

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(debug);
}

void COctopusStage545::StageCenter( void ) 
{	
	target_x = middlex;
	target_y = middley;
	target_z = middlez;

	if ( connected ) 
	{
		double dPos[3];

		dPos[0] = target_x;
		dPos[1] = target_y;
		dPos[2] = target_z;
		//Box Comment out PI fxns JF 11/4/15
		/*if(!PI_MOV(ID, axis, dPos))
		{
			debug.Format(_T(" PI 545 stage MOV centering failed "));

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(debug);

		}*/

		debug.Format(_T(" PI 545 stage centered "));

		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(debug);
	}
}

void COctopusStage545::OnSave( void ) 
{
	//This section here is different from JZ
	savex = 25 - B.position_y;
	savey = B.position_x;
	savez = B.position_z;
	//end note
	if( IsWindowVisible() ) 
	{
		CString str;
		str.Format(_T("SaveX:%.3f\nSaveY:%.3f\n"), savex, savey);		
		m_Pos_Save.SetWindowText( str );
	}

	debug.Format(_T(" PI 545 stage save position "));

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(debug);
}

void COctopusStage545::OnSaveGoTo( void ) 
{	
	target_x = savey;
	target_y = 25-savex;
	target_z = savez;

	if ( connected ) 
	{
		double dPos[3];

		dPos[0] = target_x;
		dPos[1] = target_y;
		dPos[2] = target_z;
		//Box Comment out PI fxns JF 11/4/15
		/*if(!PI_MOV(ID, axis, dPos))
		{
			debug.Format(_T(" PI 545 stage MOV savegoto failed "));

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(debug);

		}*/

		debug.Format(_T(" PI 545 stage savegoto "));

		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(debug);
	}
}

void COctopusStage545::ShowPosition( void )
{
	if( IsWindowVisible() ) 
	{
		CString str;
		str.Format(_T("X:%.3f\nY:%.3f"), \
			25-B.position_y, B.position_x);
		m_Pos.SetWindowText( str );
	}
}

BOOL COctopusStage545::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
	return CDialog::OnCommand(wParam, lParam);
}




void COctopusStage545::OnBnClickedButtonGotoxy( void )
{

	UpdateData(true);
	GetPosition();

	float change_x = (float(m_gotox))/1000 - (25 - B.position_y) ;


	float change_y = (float(m_gotoy))/1000 - (B.position_x) ;


	CString str;
	str.Format("Movetoxy: %d %d Position: %f %f Change: %f %f\n", m_gotox, m_gotoy, 25 - B.position_y, B.position_x, change_x, change_y);
	TRACE(str);
	TRACE("MOVING X1\n");


	MoveRelY( -change_x ); 
	TRACE("MOVING X2\n");
		Sleep( 3000 );
	GetPosition();

	str.Format("Movetoxy: %d %d Position: %f %f Change: %f %f\n", m_gotox, m_gotoy, 25 - B.position_y, B.position_x, change_x, change_y);
	TRACE(str);
	TRACE("MOVING Y1\n");
	MoveRelX( change_y );
		Sleep( 3000 );
	TRACE("MOVING Y2\n");

	GetPosition();

	str.Format("Movetoxy: %d %d Position: %f %f Change: %f %f\n", m_gotox, m_gotoy, 25 - B.position_y, B.position_x, change_x, change_y);
	TRACE(str);
	UpdateData(false);
}
