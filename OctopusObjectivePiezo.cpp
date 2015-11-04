#include "stdafx.h"
#include "Octopus.h"
#include "OctopusObjectivePiezo.h"
#include "OctopusLog.h"
//#include "Pi_gcs2_dll.h"

//#include "atmcd32d.h" // Andor Camera library
#include "OctopusDoc.h"
#include "OctopusView.h"
#include "OctopusClock.h"
#include "OctopusCameraDlg.h"
#include "OctopusCameraDisplay.h"
//#include "OctopusGrating.h"
#include "OctopusGlobals.h"
#include "Octopus_LED.h"

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <Mmsystem.h>

extern COctopusGlobals B;
extern COctopusLog*             glob_m_pLog;

COctopusObjectivePiezo::COctopusObjectivePiezo(CWnd* pParent)
	: CDialog(COctopusObjectivePiezo::IDD, pParent)
{    
	if( Create(COctopusObjectivePiezo::IDD, pParent) )
	{
		TRACE(_T("Piezo window created\n"));
		ShowWindow( SW_SHOW );
	}
	CString debug;
	debug.Format(_T("IDD: %d\n"),IDD);
	TRACE(debug);
	B.position_z                = 42.0;
	stepsize_z_microns          = 1.0;
	target_z_microns			= 100.0;
	save_z_microns				= target_z_microns;
	middle_z_microns            = 100.0;
	max_z_microns               = 200.0;
	first_tick                  = true;
	initialized					= false;
	connected					= false;

	SetTimer( TIMER_PIEZO, 100, NULL );

}

BOOL COctopusObjectivePiezo::OnInitDialog()
{
     CDialog::OnInitDialog();
     //SetWindowPos(NULL, 9, 775, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
     return TRUE;
}

COctopusObjectivePiezo::~COctopusObjectivePiezo() { 
	Close();
}
int COctopusObjectivePiezo::Initialize( void ) 
{
	//initalize the z stage
	const char* piezo_string = "0111173017";
	ID = -1;//ADDED THIS FOR STRING BELOW JF 11/4/15
	//ID = PI_ConnectUSB( piezo_string );
	//const char* piezo_string = "0112001542";
	//ID = PI_ConnectUSBWithBaudRate( piezo_string, 38400 );
		//ID = PI_ConnectRS232(5,57600);
		CString str;
	str.Format(_T("ID:%d\n"), ID);
	TRACE(str);

	char *buffer = new char[1024];
	char* filter = "";

	//PI_EnumerateUSB (buffer, 1024, filter);
	TRACE(buffer);

	if (ID == -1)
	{
		AfxMessageBox(_T("PI 709 stage not connected"));
		TRACE(str);
	} 
	else
	{
		if ( glob_m_pLog != NULL ) 
			glob_m_pLog->Write(debug);
		
		connected = true;
	}

   // Get the name of the connected axis 
   /*if ( !PI_qSAI(ID, axis, 16) )
   {
		AfxMessageBox(_T("PI 709 stage could not get name"));
   }*/

	/////////////////////////////////////////
	// close the servo loop (closed-loop). //
	/////////////////////////////////////////
	BOOL bFlags[1];

	// Switch on the Servo for all axes
	bFlags[0] = TRUE; // servo on for first axis in the string 'axes'.

	// call the SerVO mode command.
	/*if(!PI_SVO(ID, axis, bFlags))
	{
		debug.Format(_T(" PI 709 stage SVO failed "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		initialized = false;
		PI_CloseConnection(ID);
		AfxMessageBox(_T("PI 709 stage SVO failed"));
		return 1;
	}*/

	initialized = true;
	return 0;

}

void COctopusObjectivePiezo::Close( void ) 
{ 
    //KillTimer( TIMER_PIEZO );
	//Sleep(500);

	initialized = false; 

	if ( connected )
	{
		TRACE(_T("Closing true"));
		//PI_CloseConnection( ID );
		connected = false;
	}

	debug.Format(_T(" PI 709 stage closed "));

	if ( glob_m_pLog != NULL ) 
		glob_m_pLog->Write(debug);

}

void COctopusObjectivePiezo::MoveRelZ( double dist )
{
	/*BOOL bIsMoving[1]; //COMMENTED OUT ENTIRE BLOCK FOR NOW UNTIL REMOVE ALL OBJ PIEZO FILE JF 11/4/15
	bIsMoving[0] = TRUE;
	
	if ( connected )
	{
		PI_IsMoving(ID, NULL, bIsMoving);

		if ( bIsMoving[0] == FALSE ) 
		{
			double dPos[1];

			double new_target = target_z_microns + dist;

			if( new_target > 0.0 && new_target < max_z_microns ) 
			{
				target_z_microns = new_target;
				dPos[0]  = target_z_microns;
			
				if(!PI_MOV(ID, axis, dPos))
				{
					debug.Format(_T(" PI 709 stage MOV Z failed "));
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
				else
				{
					debug.Format(_T(" PI 709 stage MOV Z to %.3f"), target_z_microns);
					if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
				}
			}
		}
	UpdatePosition();
	}		*/
}

void COctopusObjectivePiezo::MoveUp(   void ) { MoveRelZ( +stepsize_z_microns ); }
void COctopusObjectivePiezo::MoveDown( void ) { MoveRelZ( -stepsize_z_microns ); }

void COctopusObjectivePiezo::OnStepSize1() { m_Radio_S = 0; stepsize_z_microns = 0.005; UpdateData (false); };
void COctopusObjectivePiezo::OnStepSize2() { m_Radio_S = 1; stepsize_z_microns = 0.050; UpdateData (false); };
void COctopusObjectivePiezo::OnStepSize3() { m_Radio_S = 2; stepsize_z_microns = 0.500; UpdateData (false); };
void COctopusObjectivePiezo::OnStepSize4() { m_Radio_S = 3; stepsize_z_microns = 1.000; UpdateData (false); };
void COctopusObjectivePiezo::OnStepSize5() { m_Radio_S = 4; stepsize_z_microns = 5.000; UpdateData (false); };

double COctopusObjectivePiezo::GetPosition( void ) 
{ 
	//this function is just for completeness, if you do not like globals!
	UpdatePosition(); 
	return B.position_z;
}

void COctopusObjectivePiezo::UpdatePosition( void ) 
{ 
	double dPos[1];

	dPos[0] = 0.0; 
	
	BOOL bIsMoving[1];
	bIsMoving[0] = TRUE;

	/*if ( connected )//Commented this section out too JF 11/4/15
	{
		PI_IsMoving(ID, NULL, bIsMoving);
		
		if ( bIsMoving[0] == FALSE ) 
		{
			PI_qPOS(ID, axis, dPos);
		}
	}*/

	//this gets saved to file
	B.position_z = dPos[0];
}

void COctopusObjectivePiezo::ShowPosition( void )
{
	CString str;

	if( IsWindowVisible() ) 
	{
	    str.Format(_T("Z:%.3f\nSave Z:%.3f"), B.position_z, save_z_microns);
		m_Pos.SetWindowText( str );
	}
}

void COctopusObjectivePiezo::Center( void ) 
{	
	target_z_microns = middle_z_microns;

	/*if ( connected ) //HERE TOO JF 11/4/15
	{
		double dPos[1];
		dPos[0] = target_z_microns;

		if(!PI_MOV(ID, axis, dPos))
		{
			debug.Format(_T(" PI 709 stage MOV centering failed "));

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(debug);

		}

		debug.Format(_T(" PI 709 stage centered "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}*/
}

void COctopusObjectivePiezo::OnSave( void ) 
{	
	save_z_microns = GetPosition();

	debug.Format(_T(" COctopusObjectivePiezo::OnSave( void ) "));
	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
}

void COctopusObjectivePiezo::OnSaveGoTo( void ) 
{	
	target_z_microns = save_z_microns;
	//HERE TOO JF 11/4/15
	/*if ( connected ) {

		double dPos[1];
		dPos[0] = target_z_microns;

		if(!PI_MOV(ID, axis, dPos)) {
			debug.Format(_T(" PI 709 stage MOV savegoto failed "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		}

		debug.Format(_T(" PI 709 stage savegoto "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	}*/
}

void COctopusObjectivePiezo::OnTimer( UINT nIDEvent ) 
{

	if( nIDEvent == TIMER_PIEZO ) 
	{
		if( first_tick ) 
		{
			OnStepSize3();
			first_tick = false;
			TRACE("In first tick");
			Initialize(); 
			Center();

			if ( glob_m_pLog != NULL ) 
				glob_m_pLog->Write(" PI 709 stage initialized ");
		}
		if( initialized ) 
		{
			UpdatePosition();
			ShowPosition();
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void COctopusObjectivePiezo::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio(pDX,	 IDC_PIEZO_SSIZE_1,			  m_Radio_S);
	DDX_Control(pDX, IDC_PIEZO_POS,				  m_Pos);
}  

BEGIN_MESSAGE_MAP(COctopusObjectivePiezo, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PIEZO_UP,	         MoveUp)
	ON_BN_CLICKED(IDC_PIEZO_DOWN,        MoveDown)
	ON_BN_CLICKED(IDC_PIEZO_SAVE,	     OnSave)
	ON_BN_CLICKED(IDC_PIEZO_SAVE_GOTO,   OnSaveGoTo)
	ON_BN_CLICKED(IDC_PIEZO_CENTER,		 Center)
	ON_BN_CLICKED(IDC_PIEZO_SSIZE_1,     OnStepSize1)
	ON_BN_CLICKED(IDC_PIEZO_SSIZE_2,     OnStepSize2)
	ON_BN_CLICKED(IDC_PIEZO_SSIZE_3,     OnStepSize3)
	ON_BN_CLICKED(IDC_PIEZO_SSIZE_4,     OnStepSize4)
	ON_BN_CLICKED(IDC_PIEZO_SSIZE_5,     OnStepSize5)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void COctopusObjectivePiezo::OnKillfocusGeneral() { UpdateData( true ); }

BOOL COctopusObjectivePiezo::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}