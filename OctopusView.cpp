/**********************************************************************************
***************************   OctopusView           *******************************
**********************************************************************************/

#include "stdafx.h"
#include "MainFrm.h"
#include "Octopus.h"
#include "OctopusView.h"
#include "OctopusClock.h"
#include "OctopusCameraDlg.h"
#include "OctopusShutter.h"
#include "OctopusGlobals.h"
#include "OctopusScript.h"
#include "OctopusStage545.h"
//#include "OctopusStage686.h"
//#include "OctopusAOTF.h"
#include "Octopus_LED.h"
#include "OctopusLog.h"
#include "OctopusObjectivePiezo.h"
//#include "OctopusGrating.h"
#include "OctopusLaser.h"
#include "OctopusXCite.h"
#include "OctopusFocus.h"
#include "OctopusScope.h"

COctopusCamera*            glob_m_pCamera          = NULL;
COctopusShutterAndWheel*   glob_m_pShutterAndWheel = NULL;
COctopusScript*            glob_m_pScript          = NULL;
Octopus_LED*               glob_m_pLED             = NULL;
//COctopusAOTF*              glob_m_pAOTF            = NULL;
COctopusStage545*          glob_m_pStage545        = NULL;
//COctopusStage686*          glob_m_pStage686		   = NULL;
COctopusObjectivePiezo*    glob_m_pObjPiezo        = NULL;
//COctopusGrating*           glob_m_pGrating         = NULL;
COctopusFocus*             glob_m_pFocus           = NULL;
OctopusLaser*             glob_m_pLasers          = NULL;
COctopusXCite*         glob_m_pXCite       = NULL;
COctopusScope*             glob_m_pScope		   = NULL;

COctopusGoodClock          GoodClock;
COctopusGoodClock*         glob_m_pGoodClock = &GoodClock;

COctopusLog                Log;
COctopusLog*               glob_m_pLog = &Log;

COctopusGlobals            B;

IMPLEMENT_DYNCREATE(COctopusView, CFormView)

BEGIN_MESSAGE_MAP(COctopusView, CFormView)
	ON_BN_CLICKED(IDC_STATUS_OPEN_ANDOR,   OnOpenAndor)
	ON_BN_CLICKED(IDC_STATUS_OPEN_SCRIPT,  OnOpenScript)
	ON_BN_CLICKED(IDC_STATUS_OPEN_WHEEL,   OnOpenWheel)
	ON_BN_CLICKED(IDC_STATUS_OPEN_LED,     OnOpenLED)
	ON_BN_CLICKED(IDC_STATUS_OPEN_PIEZO,   OnOpenPiezo)
	//ON_BN_CLICKED(IDC_STATUS_OPEN_GRAT,    OnOpenGrating)
	ON_BN_CLICKED(IDC_STATUS_OPEN_LASERS,  OnOpenLasers)
	ON_BN_CLICKED(IDC_STATUS_OPEN_SCOPE,   OnOpenScope)
	ON_BN_CLICKED(IDC_STATUS_OPEN_STAGE,   OnOpenStage545)
	//ON_BN_CLICKED(IDC_STATUS_OPEN_POL,	   OnOpenPolarizer)
	ON_BN_CLICKED(IDC_STATUS_OPEN_FOCUS, &COctopusView::OnBnClickedStatusOpenFocus)
	ON_BN_CLICKED(IDC_STATUS_OPEN_XCITE, &COctopusView::OnBnClickedStatusOpenXcite)
END_MESSAGE_MAP()

void COctopusView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange( pDX );
}  

COctopusView::COctopusView():CFormView(COctopusView::IDD) 
{
}

void COctopusView::OnInitialUpdate() 
{
	//toyscope
	// scope = 1 Phil's toyscope
	// scope = 3 Pol scope
	scope = 2;
	
	if ( scope == 1 ) //Phil's toyscope
	{
		//set this to true if you have a iXon Plus.
		B.Andor_new = true;

		GetDlgItem( IDC_STATUS_OPEN_PIEZO )->EnableWindow( false );
		GetDlgItem( IDC_STATUS_OPEN_PIEZO )->ShowWindow(   false );

		GetDlgItem( IDC_STATUS_OPEN_WHEEL )->EnableWindow( false );
		GetDlgItem( IDC_STATUS_OPEN_WHEEL )->ShowWindow(   false );
	}
	else if ( scope == 2 ) //Rebecca's scope
	{
		B.Andor_new = true;
		GetDlgItem( IDC_STATUS_OPEN_LASERS )->EnableWindow( false );
		GetDlgItem( IDC_STATUS_OPEN_LASERS )->ShowWindow(   false );
		GetDlgItem( IDC_STATUS_OPEN_WHEEL )->EnableWindow( true );
		GetDlgItem( IDC_STATUS_OPEN_WHEEL )->ShowWindow(   true );
	}
	else if ( scope == 3 ) //Pol scope
	{

		B.Andor_new = false;

		GetDlgItem( IDC_STATUS_OPEN_LASERS )->EnableWindow( false );
		GetDlgItem( IDC_STATUS_OPEN_LASERS )->ShowWindow(   false );
		//GetDlgItem( IDC_STATUS_OPEN_STAGE )->EnableWindow( true );
		//GetDlgItem( IDC_STATUS_OPEN_STAGE )->ShowWindow(   true );
		
	}
	else
	{
		B.Andor_new = false;
	}

	Log = COctopusLog();
	CFormView::OnInitialUpdate();
}

COctopusView::~COctopusView() 
{
	char command[256]; 
	sprintf (command, "cd C:\\Program Files\\Octopus\\Josh's Octopus Files\\MTTTY\\Release & MITTY.exe %d", 7); 
	system(command);

	if( glob_m_pCamera != NULL ) 
	{
		if ( B.Camera_Thread_running ) 
			glob_m_pCamera->StopCameraThread();
		glob_m_pCamera->DestroyWindow();
		delete glob_m_pCamera;
		glob_m_pCamera = NULL;
		TRACE(_T("Stopping Camera\n"));
	}
	if( glob_m_pShutterAndWheel != NULL ) 
	{
		glob_m_pShutterAndWheel->DestroyWindow();
		delete glob_m_pShutterAndWheel;
		glob_m_pShutterAndWheel = NULL;
	}
	
	if( glob_m_pLED != NULL ) 
	{
		glob_m_pLED->DestroyWindow();
		delete glob_m_pLED;
		glob_m_pLED = NULL;
	}
	
	if( glob_m_pScript != NULL ) 
	{
		glob_m_pScript->DestroyWindow();
	}
	
	if( glob_m_pStage545 != NULL ) 
	{
		glob_m_pStage545->DestroyWindow();
		delete glob_m_pStage545;
		glob_m_pStage545 = NULL;
	}
	/*
	if( glob_m_pStage686 != NULL ) 
	{
		glob_m_pStage686->DestroyWindow();
		delete glob_m_pStage686;
		glob_m_pStage686 = NULL;
	}
	*/
	if( glob_m_pObjPiezo != NULL ) 
	{
		glob_m_pObjPiezo->DestroyWindow();
		delete glob_m_pObjPiezo;
		glob_m_pObjPiezo = NULL;
	}
	if( glob_m_pFocus != NULL ) 
	{
		glob_m_pFocus->DestroyWindow();
		delete glob_m_pFocus;
		glob_m_pFocus = NULL;
	}
	/*if( glob_m_pGrating != NULL ) 
	{
		glob_m_pGrating->DestroyWindow();
		delete glob_m_pGrating;
		glob_m_pGrating = NULL;
	}*/
	if( glob_m_pLasers != NULL ) 
	{
		glob_m_pLasers->DestroyWindow();
		delete glob_m_pLasers;
		glob_m_pLasers = NULL;
	}
	/*
	if( glob_m_pAOTF != NULL ) 
	{
		glob_m_pAOTF->DestroyWindow();
		delete glob_m_pAOTF;
		glob_m_pAOTF = NULL;
	}
	*/
	/*if( glob_m_pPolarizer != NULL ) 
	{
		glob_m_pPolarizer->DestroyWindow();
		delete glob_m_pPolarizer;
		glob_m_pPolarizer = NULL;
	}*/
	if( glob_m_pScope != NULL ) 
	{
		glob_m_pScope->DestroyWindow();
		delete glob_m_pScope;
		glob_m_pScope = NULL;
	}
}

void COctopusView::OnOpenAndor( void ) 
{	
	GetDlgItem( IDC_STATUS_OPEN_ANDOR )->EnableWindow( false );

	if( glob_m_pCamera == NULL ) 
	{
		glob_m_pCamera = new COctopusCamera( this );
	} 
	else 
	{
		if ( B.Camera_Thread_running )
		{
			AfxMessageBox(_T("The camera is running in movie mode,\n and you just tried to close it!\nPlease turn it off first!"));
		}
		else if( glob_m_pCamera->IsWindowVisible() ) 
		{
			glob_m_pCamera->DestroyWindow();
			delete glob_m_pCamera;
			glob_m_pCamera = NULL;
		} 
		else 
		{
			glob_m_pCamera->ShowWindow( SW_RESTORE );
		}
	}
	GetDlgItem( IDC_STATUS_OPEN_ANDOR )->EnableWindow( true );
}

void COctopusView::OnOpenScript( void ) 
{	
	GetDlgItem( IDC_STATUS_OPEN_SCRIPT )->EnableWindow( false );

	if( glob_m_pScript == NULL ) 
	{
		glob_m_pScript = new COctopusScript( this );
	} 
	else 
	{
		if( glob_m_pScript->IsWindowVisible() ) 
		{
			glob_m_pScript->DestroyWindow();
			delete glob_m_pScript;
			glob_m_pScript = NULL;
		} 
		else 
		{
			glob_m_pScript->ShowWindow( SW_RESTORE );
		}
	}
	GetDlgItem( IDC_STATUS_OPEN_SCRIPT )->EnableWindow( true );
}

void COctopusView::OnOpenWheel( void ) 
{	

	GetDlgItem( IDC_STATUS_OPEN_WHEEL )->EnableWindow( false );

	if( glob_m_pShutterAndWheel == NULL ) 
	{
		
		glob_m_pShutterAndWheel = new COctopusShutterAndWheel( this );
		TRACE(_T("New Shutter Object Created\n"));
		if ( B.load_wheel_failed ) 
		{ // close it
			TRACE(_T("New Shutter Object Deleted Immediately\n"));
			glob_m_pShutterAndWheel->DestroyWindow();
			delete glob_m_pShutterAndWheel;
			glob_m_pShutterAndWheel = NULL;
		}
	} 
	else 
	{
		TRACE(_T("New Shutter Object Deleted\n"));
		if( glob_m_pShutterAndWheel->IsWindowVisible() ) 
		{
			glob_m_pShutterAndWheel->DestroyWindow();
			delete glob_m_pShutterAndWheel;
			glob_m_pShutterAndWheel = NULL;
		} 
		else 
		{
			glob_m_pShutterAndWheel->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_WHEEL )->EnableWindow( true );

}


void COctopusView::OnOpenLED( void ) 
{	
	
	GetDlgItem( IDC_STATUS_OPEN_LED )->EnableWindow( false );

	if( glob_m_pLED == NULL ) 
	{
		glob_m_pLED = new Octopus_LED( this );
	} 
	else 
	{
		if( glob_m_pLED->IsWindowVisible() ) 
		{
			glob_m_pLED->DestroyWindow();
			delete glob_m_pLED;
			glob_m_pLED = NULL;
		} 
		else 
		{
			glob_m_pLED->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_LED )->EnableWindow( true );
	
}


void COctopusView::OnOpenScope( void ) 
{	
	
	GetDlgItem( IDC_STATUS_OPEN_SCOPE )->EnableWindow( false );

	if( glob_m_pScope == NULL ) 
	{
		glob_m_pScope = new COctopusScope( this );
	} 
	else 
	{
		if( glob_m_pScope->IsWindowVisible() ) 
		{
			glob_m_pScope->DestroyWindow();
			delete glob_m_pScope;
			glob_m_pScope = NULL;
		} 
		else 
		{
			glob_m_pScope->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_SCOPE )->EnableWindow( true );
	
}



void COctopusView::OnOpenStage545( void ) 
{	

	GetDlgItem( IDC_STATUS_OPEN_STAGE )->EnableWindow( false );

	if( glob_m_pStage545 == NULL ) 
	{
		glob_m_pStage545 = new COctopusStage545( this );

		debug.Format(_T(" PI 545 stage class opened "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	} 
	else {
		if( glob_m_pStage545->IsWindowVisible() ) {
			glob_m_pStage545->Close();
			glob_m_pStage545->DestroyWindow();
			delete glob_m_pStage545;
			glob_m_pStage545 = NULL;
			debug.Format(_T(" PI 545 stage class closed "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		} else {
			glob_m_pStage545->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_STAGE )->EnableWindow( true );
}


/*
void COctopusView::OnOpenStage686( void ) 
{	

	GetDlgItem( IDC_STATUS_OPEN_STAGE )->EnableWindow( false );

	if( glob_m_pStage686 == NULL ) 
	{
		glob_m_pStage686 = new COctopusStage686( this );

		debug.Format(_T(" PI 686 stage class opened "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	} 
	else {
		if( glob_m_pStage686->IsWindowVisible() ) {
			glob_m_pStage686->Close();
			glob_m_pStage686->DestroyWindow();
			delete glob_m_pStage686;
			glob_m_pStage686 = NULL;
			debug.Format(_T(" PI 686 stage class closed "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		} else {
			glob_m_pStage686->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_STAGE )->EnableWindow( true );
}
*/

void COctopusView::OnOpenPiezo( void ) 
{	

	GetDlgItem( IDC_STATUS_OPEN_PIEZO )->EnableWindow( false );

	if( glob_m_pObjPiezo == NULL ) 
	{
		TRACE("Will create objective piezo\n");
		glob_m_pObjPiezo = new COctopusObjectivePiezo( this );
		//TRACE("Created new objective piezo");
		debug.Format(_T(" Obj Piezo class opened "));
		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	} 
	else 
	{
		if( glob_m_pObjPiezo->IsWindowVisible() ) 
		{
			glob_m_pObjPiezo->Close();
			glob_m_pObjPiezo->DestroyWindow();
			delete glob_m_pObjPiezo;
			glob_m_pObjPiezo = NULL;
			debug.Format(_T(" Obj Piezo class closed "));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		} 
		else 
		{
			glob_m_pObjPiezo->ShowWindow( SW_RESTORE );
		}
	}


	//if( glob_m_pFocus == NULL ) 
	//{
	//	glob_m_pFocus = new COctopusFocus( this );
	//	debug.Format(_T(" Focus class opened "));
	//	if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	//} 
	//else 
	//{
	//	if( glob_m_pFocus->IsWindowVisible() ) 
	//	{
	//		glob_m_pFocus->Close();
	//		glob_m_pFocus->DestroyWindow();
	//		delete glob_m_pFocus;
	//		glob_m_pFocus = NULL;
	//		debug.Format(_T(" Focus class closed "));
	//		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	//	} 
	//	else 
	//	{
	//		glob_m_pFocus->ShowWindow( SW_RESTORE );
	//	}
	//}

	GetDlgItem( IDC_STATUS_OPEN_PIEZO )->EnableWindow( true );
}

//void COctopusView::OnOpenGrating( void ) 
//{	
//
//	GetDlgItem( IDC_STATUS_OPEN_GRAT )->EnableWindow( false );
//
//	if( glob_m_pGrating == NULL ) {
//		glob_m_pGrating = new COctopusGrating( this );
//		debug.Format(_T(" Grating class opened "));
//		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
//	} 
//	else {
//		if( glob_m_pGrating->IsWindowVisible() ) 
//		{
//			glob_m_pGrating->Close();
//			glob_m_pGrating->DestroyWindow();
//			delete glob_m_pGrating;
//			glob_m_pGrating = NULL;
//			debug.Format(_T(" Grating class closed "));
//			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
//		} else {
//			glob_m_pGrating->ShowWindow( SW_RESTORE );
//		}
//	}
//
//	GetDlgItem( IDC_STATUS_OPEN_GRAT )->EnableWindow( true );
//}


//void COctopusView::OnOpenPolarizer( void ) 
//{	
//
//	GetDlgItem( IDC_STATUS_OPEN_POL )->EnableWindow( false );
//
//	if( glob_m_pPolarizer == NULL ) 
//	{
//		if (glob_m_pObjPiezo != NULL)
//			AfxMessageBox(_T("Polarizer initialization will only work if the piezo is not yet running.\nClose the piezo, and then the Pol should fire up after a moment."));
//		
//		glob_m_pPolarizer = new COctopusPolarizer( this );
//		
//		if ( !B.POL_loaded ) 
//		{ // close it down
//			glob_m_pPolarizer->DestroyWindow();
//			delete glob_m_pPolarizer;
//			glob_m_pPolarizer = NULL;
//		}
//	} else {
//		if( glob_m_pPolarizer->IsWindowVisible() ) 
//		{
//			glob_m_pPolarizer->DestroyWindow();
//			delete glob_m_pPolarizer;
//			glob_m_pPolarizer = NULL;
//		} 
//		else 
//		{
//			glob_m_pPolarizer->ShowWindow( SW_RESTORE );
//		}
//	}
//
//	GetDlgItem( IDC_STATUS_OPEN_POL )->EnableWindow( true );
//	
//}

void COctopusView::OnBnClickedStatusOpenFocus()
{

	

	GetDlgItem( IDC_STATUS_OPEN_FOCUS )->EnableWindow( false );

		if( glob_m_pFocus == NULL ) 
	{
		glob_m_pFocus = new COctopusFocus( this );
		TRACE(" Focus class opened\n");

		if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
	} 
	else 
	{
		if( glob_m_pFocus->IsWindowVisible() ) 
		{
			glob_m_pFocus->Close();
			glob_m_pFocus->DestroyWindow();
			delete glob_m_pFocus;
			glob_m_pFocus = NULL;
			TRACE(_T(" Focus class closed\n"));
			if ( glob_m_pLog != NULL ) glob_m_pLog->Write(debug);
		} 
		else 
		{
			glob_m_pFocus->ShowWindow( SW_RESTORE );
		}
	}

		GetDlgItem( IDC_STATUS_OPEN_FOCUS )->EnableWindow( true );
	// TODO: Add your control notification handler code here
}


void COctopusView::OnBnClickedStatusOpenXcite()
{

	
	GetDlgItem( IDC_STATUS_OPEN_XCITE )->EnableWindow( false );

	if( glob_m_pXCite == NULL ) 
	{
		glob_m_pXCite = new COctopusXCite( this );
		
		if ( !B.XCite_loaded ) { // close it
			glob_m_pXCite->DestroyWindow();
			delete glob_m_pXCite;
			glob_m_pXCite = NULL;
		}
	} else {
		if( glob_m_pXCite->IsWindowVisible() ) {
			glob_m_pXCite->DestroyWindow();
			delete glob_m_pXCite;
			glob_m_pXCite = NULL;
			B.XCite_loaded = false;
		} 
		else {
			glob_m_pXCite->ShowWindow( SW_RESTORE );
		}
	}

	GetDlgItem( IDC_STATUS_OPEN_XCITE )->EnableWindow( true );
	
}

}
