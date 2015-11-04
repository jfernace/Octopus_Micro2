
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusScope.h"
#include "OctopusGlobals.h"
#include "cport.h"

extern COctopusGlobals B;

COctopusScope::COctopusScope(CWnd* pParent)
	: CDialog(COctopusScope::IDD, pParent)
{    
	position_now      = 0;
	stepsize_10nm     = 10;
	pPortScope        = NULL;
	Scope_initialized = false;
	old_position      = 0;

	if( Create(COctopusScope::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	first_tick  = true;
	working     = false;
	Path_camera = false;

	m_Slider.SetRange( 0, 60 );
	m_Slider.SetPos( 0 );
	m_Slider.SetTicFreq( 10 );

}

BOOL COctopusScope::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetTimer( TIMER_SCOPE, 250, NULL );
	return TRUE;
}

void COctopusScope::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == TIMER_SCOPE ) 
	{
		if( first_tick ) 
		{
			first_tick = false;

			Init(); //fire up the scope
			OnObjectiveStepSize2();
		} 
		else
		{
			if ( working == false )
			{
				GetPosition();
				DisplayPosition();
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}

bool COctopusScope::Init( void )
{
	if( Scope_initialized ) 
	{
		AfxMessageBox(_T("Microscope already init()ed..."));
		return false;
	}

	pPortScope = new CPort;
	
	if( pPortScope == NULL ) 
	{
		AfxMessageBox(_T("pPortScope is a Null pointer..."));
		return false;
	}

	pPortScope->mPort.Format(_T("COM6"));
	
	if( pPortScope->OpenCPort() )//open and setup the comm port
	{
		Scope_initialized = true;

		//get rid of garbage in channel
		WriteScope("1UNIT?\r\n"); //anyone out there?	
		Sleep(30);
		while ( ReadScope(1) != "\0" ) 
			{ ; } //clear the buffer

		WriteScope("1UNIT?\r\n"); //anyone out there?
		Sleep(30);
		while ( ReadScope(1) != "\0" ) 
			{ ; } //clear the buffer
		
		WriteScope("1UNIT?\r\n"); //anyone out there?
		Sleep(30);

		if ( ReadScope(9).Find("1UNIT IX2") >=0 ) 
		{
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("1LOG IN\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2LOG IN\r\n"));
			Sleep(30);

			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("1SW ON\r\n"));
			Sleep(30);

			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("1SNDOB ON\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2maxspd 70000,300000,250\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2JOG ON\r\n"));
			Sleep(30);

			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2NEARLMT 3000000\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2FARLMT 100\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope("2JOGSNS 7\r\n"); //something like 100 microns per turn
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer
			WriteScope(_T("2joglmt ON\r\n"));
			Sleep(30);
			
			while ( ReadScope( 1 ) != "\0" ) { ; } //clear the buffer

			Sleep(100);

			int ob  = GetObj();
			int fw  = GetFW();
			int bfw = GetBFW();

			m_Radio_OBJ  = ob  - 1; 
			m_Radio_FW   = fw  - 1; 
			m_Radio_BFW  = bfw - 1; 

			UpdateData( false );

			return true;
		} 
		else 
		{
			Close();
			return false;
		}
	}
	else 
	{
		Close();
		return false;
	}
}

void COctopusScope::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Radio(pDX,		IDC_SCOPE_SSIZE_1,			        m_Radio_S);
	DDX_Radio(pDX,		IDC_SCOPE_OBJ_1,			        m_Radio_OBJ);
	DDX_Radio(pDX,		IDC_SCOPE_FW_1,			            m_Radio_FW);
	DDX_Radio(pDX,		IDC_SCOPE_BFW_1,		            m_Radio_BFW);
	DDX_Control(pDX,	IDC_SCOPE_POS,				        m_Pos);
	DDX_Control(pDX,	IDC_SCOPE_INTENSITY_SLIDER,		    m_Slider);
	DDX_Control(pDX,	IDC_SCOPE_INTENSITY_SLIDER_SETTING, m_Slider_Setting);
}  

BEGIN_MESSAGE_MAP(COctopusScope, CDialog)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SCOPE_INTENSITY_SLIDER, OnNMCustomdrawExecute)
	ON_BN_CLICKED(IDC_SCOPE_DOWN,	     Z_StepDown)
	ON_BN_CLICKED(IDC_SCOPE_UP,		     Z_StepUp)
	ON_BN_CLICKED(IDC_SCOPE_ESC,		 ObjESC)
	ON_BN_CLICKED(IDC_SCOPE_RTN,		 ObjRTN)
	ON_BN_CLICKED(IDC_SCOPE_SSIZE_1,     OnObjectiveStepSize1)
	ON_BN_CLICKED(IDC_SCOPE_SSIZE_2,     OnObjectiveStepSize2)
	ON_BN_CLICKED(IDC_SCOPE_SSIZE_3,     OnObjectiveStepSize3)
	ON_BN_CLICKED(IDC_SCOPE_SSIZE_4,     OnObjectiveStepSize4)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_1,       OnObj_1)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_2,       OnObj_2)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_3,       OnObj_3)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_4,       OnObj_4)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_5,       OnObj_5)
	ON_BN_CLICKED(IDC_SCOPE_OBJ_6,       OnObj_6)
	ON_BN_CLICKED(IDC_SCOPE_FW_1,        OnFW_1)
	ON_BN_CLICKED(IDC_SCOPE_FW_2,        OnFW_2)
	ON_BN_CLICKED(IDC_SCOPE_FW_3,        OnFW_3)
	ON_BN_CLICKED(IDC_SCOPE_FW_4,        OnFW_4)
	ON_BN_CLICKED(IDC_SCOPE_FW_5,        OnFW_5)
	ON_BN_CLICKED(IDC_SCOPE_FW_6,        OnFW_6)
	ON_BN_CLICKED(IDC_SCOPE_BFW_1,       OnBFW_1)
	ON_BN_CLICKED(IDC_SCOPE_BFW_2,       OnBFW_2)
	ON_BN_CLICKED(IDC_SCOPE_BFW_3,       OnBFW_3)
	ON_BN_CLICKED(IDC_SCOPE_BFW_4,       OnBFW_4)
	ON_BN_CLICKED(IDC_SCOPE_BFW_5,       OnBFW_5)
	ON_BN_CLICKED(IDC_SCOPE_BFW_6,       OnBFW_6)
	ON_BN_CLICKED(IDC_SCOPE_PATH,        ChangePath)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/**************************************************************************************
SHUTDOWN
**************************************************************************************/

void COctopusScope::Close() 
{  
	
	WriteScope(_T("1LOG OUT\r\n"));
	Sleep(30);

	WriteScope(_T("2LOG OUT\r\n"));
	Sleep(30);

	Scope_initialized = false;

	if ( pPortScope != NULL ) 
	{
		delete pPortScope;
		pPortScope = NULL;
	}
	
}

COctopusScope::~COctopusScope() 
{  
	Close();
}

/**************************************************************************************
OBJECTIVE Z SETTING
**************************************************************************************/

void COctopusScope::Z_StepDown( void ) 
{ 
	working = true;
	CString temp;
	temp.Format(_T("2MOV F,%d\r\n"), stepsize_10nm );
	WriteScope( temp );
	Sleep(30);
	WriteScope(_T("2JOG ON\r\n"));
	Sleep(30);
	working = false;
}
void COctopusScope::Z_StepUp( void ) 
{ 
    working = true;
	CString temp;
	temp.Format(_T("2MOV N,%d\r\n"), stepsize_10nm );
	WriteScope( temp );	
	WriteScope(_T("2JOG ON\r\n"));
	Sleep(30);
	working = false;
}

void COctopusScope::ObjESC( void ) 
{ 
	working = true;
	GetPosition();
	Sleep(100); 
	old_position = u32(position_now); //this is where we will go back to. 
	WriteScope(_T("2JOG OFF\r\n"));
	Sleep(30);
	CString temp;
	temp.Format(_T("2MOV F,%d\r\n"), old_position - 10000 );
	WriteScope( temp );
	Sleep(30);
	WriteScope(_T("2JOG ON\r\n"));
	Sleep(30);
	working = false;
}

void COctopusScope::ObjRTN( void ) 
{ 
	working = true;
	WriteScope(_T("2JOG OFF\r\n"));
	Sleep(30);
	CString temp;
	temp.Format(_T("2MOV N,%d\r\n"), old_position - 10000 );
	WriteScope( temp );
	Sleep(30);
	WriteScope(_T("2JOG ON\r\n"));
	Sleep(30);
	working = false;
}

void COctopusScope::OnObjectiveStepSize1() 
{ m_Radio_S = 0; stepsize_10nm =   1; UpdateData( false ); }; //0.01
void COctopusScope::OnObjectiveStepSize2() 
{ m_Radio_S = 1; stepsize_10nm =   5; UpdateData( false ); }; //0.05
void COctopusScope::OnObjectiveStepSize3() 
{ m_Radio_S = 2; stepsize_10nm =  10; UpdateData( false ); }; //0.10
void COctopusScope::OnObjectiveStepSize4() 
{ m_Radio_S = 3; stepsize_10nm = 100; UpdateData( false ); }; //1.00

void COctopusScope::GetPosition( void )
{
	CString position;
	CString temp;

	if( !Scope_initialized ) 
		return;

	while ( ReadScope( 1 ) != "\0" ) 
		{ ; } //clear the buffer

	WriteScope("2POS?\r\n");
	
	Sleep(50);

	if ( ReadScope(5).Find("2POS ") >= 0 ) 
	{
		while ( temp != '\n') 
		{
			temp = ReadScope(1);
			position.Append(temp);
		}
		position_now = _tstof(position);
	} 
	else 
	{
		while ( ReadScope( 1 ) != "\0" ) 
			{ ; } //clear the buffer
	}
}

void COctopusScope::DisplayPosition( void )
{
	if( IsWindowVisible() ) 
	{
		CString str;
		str.Format("%.2f", position_now / 100.0 );
		m_Pos.SetWindowText( str );
	}		
}

/**************************************************************************************
OBJECTIVE CHOICE
**************************************************************************************/

void COctopusScope::OnObj_1() { m_Radio_OBJ = 0; Objective( 1 ); UpdateData( false ); };
void COctopusScope::OnObj_2() { m_Radio_OBJ = 1; Objective( 2 ); UpdateData( false ); };
void COctopusScope::OnObj_3() { m_Radio_OBJ = 2; Objective( 3 ); UpdateData( false ); };
void COctopusScope::OnObj_4() { m_Radio_OBJ = 3; Objective( 4 ); UpdateData( false ); };
void COctopusScope::OnObj_5() { m_Radio_OBJ = 4; Objective( 5 ); UpdateData( false ); };
void COctopusScope::OnObj_6() { m_Radio_OBJ = 5; Objective( 6 ); UpdateData( false ); };

void COctopusScope::OnFW_1() { m_Radio_FW = 0; EpiFilterWheel( 1 ); UpdateData( false ); };
void COctopusScope::OnFW_2() { m_Radio_FW = 1; EpiFilterWheel( 2 ); UpdateData( false ); };
void COctopusScope::OnFW_3() { m_Radio_FW = 2; EpiFilterWheel( 3 ); UpdateData( false ); };
void COctopusScope::OnFW_4() { m_Radio_FW = 3; EpiFilterWheel( 4 ); UpdateData( false ); };
void COctopusScope::OnFW_5() { m_Radio_FW = 4; EpiFilterWheel( 5 ); UpdateData( false ); };
void COctopusScope::OnFW_6() { m_Radio_FW = 5; EpiFilterWheel( 6 ); UpdateData( false ); };

void COctopusScope::OnBFW_1() { m_Radio_BFW = 0; BrightFieldFilterWheel( 1 ); UpdateData( false ); };
void COctopusScope::OnBFW_2() { m_Radio_BFW = 1; BrightFieldFilterWheel( 2 ); UpdateData( false ); };
void COctopusScope::OnBFW_3() { m_Radio_BFW = 2; BrightFieldFilterWheel( 3 ); UpdateData( false ); };
void COctopusScope::OnBFW_4() { m_Radio_BFW = 3; BrightFieldFilterWheel( 4 ); UpdateData( false ); };
void COctopusScope::OnBFW_5() { m_Radio_BFW = 4; BrightFieldFilterWheel( 5 ); UpdateData( false ); };
void COctopusScope::OnBFW_6() { m_Radio_BFW = 5; BrightFieldFilterWheel( 6 ); UpdateData( false ); };


void COctopusScope::Objective( int obj )
{
	KillTimer( TIMER_SCOPE );
		
	GetPosition();
	
	int old_obj = GetObj();


	Sleep(100);

	old_position = u32(position_now); //this is where we will go back to. 

	//from pos 1 (10 x)
	//10x is too low relative to standard ref frame
	if ( old_obj == 1 ) 
		old_position = old_position + 39398;

	//from pos 3 (5 x)
	//5x is too high relative to standard ref frame
	if ( old_obj == 3 ) 
		old_position = old_position - 81380;
	
	//now we are in stanrd frame
	//add the correct offsets back
		
	//to pos 1 (10 x)
	//10x needs to be lowered relative to standard ref frame
	if ( obj == 1 ) 
		old_position = old_position - 39398;

	//to pos 3 (5 x)
	//5x needs to be augmented relative to standard ref frame
	if ( obj == 3 ) 
		old_position = old_position + 81380;

	while ( ReadScope( 1 ) != "\0" ) 
		{ ; } //clear the buffer

	//move to the bottom
	CString temp;
	temp.Format("2MOV d,%d\r\n", 10000 );
	
	//temp.Format("2MOV F,%d\r\n", old_position - 10000 );
	WriteScope( temp );

	while ( ReadScope( 6 ).Find(_T("2MOV +" )) == 0 )
	{
		Sleep( 200 );
	}

	ReadScope( 2 );

	//at bottom?
	GetPosition();
	GetPosition();

	if ( position_now < 15000 )
	{
		temp.Format(_T("1OB %d\r\n"), obj);
		WriteScope( temp );
		
		while ( ReadScope( 5 ).Find(_T("1OB +")) == 0 )
		{
			Sleep( 500 );
		}
		
		while ( GetObj() != obj )
		{
			Sleep( 500 );
		}

		temp.Format(_T("2MOV d,%d\r\n"), old_position );
		WriteScope( temp );
		Sleep(100);
	}

    SetTimer( TIMER_SCOPE, 250, NULL );
}

int COctopusScope::GetObj( void )
{
	CString obj;

	if( !Scope_initialized ) 
		return 0;

	while ( ReadScope(1) != "\0" ) 
		{ ; } //clear the buffer

	WriteScope("1OB?\r\n");
	
	Sleep(100);

	if ( ReadScope(4).Find("1OB ") >= 0 ) 
	{
		return atol( ReadScope( 1 ) ); //_wtol( ReadScope( 1 ) );
	} 
	else 
	{
		while ( ReadScope(1) != "\0" ) 
			{ ; } //clear the buffer
	}
	return 0;
}

int COctopusScope::GetFW( void )
{
	CString FW;

	if( !Scope_initialized ) 
		return 0;

	while ( ReadScope( 1 ) != "\0" ) 
		{ ; } //clear the buffer

	WriteScope("1MU?\r\n");
	
	Sleep(30);

	if ( ReadScope( 4 ).Find("1MU ") >= 0 ) 
	{
		return atol( ReadScope( 1 ) );//_wtol( ReadScope( 1 ) );
	} 
	else 
	{
		while ( ReadScope( 1 ) != "\0" ) 
			{ ; } //clear the buffer
	}
	return 0;
}

int COctopusScope::GetBFW( void )
{
	CString BFFW;

	if( !Scope_initialized ) 
		return 0;

	while ( ReadScope( 1 ) != "\0" ) 
		{ ; } //clear the buffer

	WriteScope("1CD?\r\n");
	
	Sleep(30);

	if ( ReadScope( 4 ).Find("1CD ") >= 0 ) 
	{
		return atol( ReadScope( 1 ) );//_wtol( ReadScope( 1 ) );
	} 
	else 
	{
		while ( ReadScope( 1 ) != "\0" ) 
			{ ; } //clear the buffer
	}
	return 0;
}

void COctopusScope::EpiFilterWheel( int cube )
{
	KillTimer( TIMER_SCOPE );
		
	CString temp;
	temp.Format("1MU %d\r\n", cube );
	WriteScope( temp );

	m_Radio_FW = cube - 1; UpdateData( false );

    SetTimer( TIMER_SCOPE, 250, NULL );
}

void COctopusScope::BrightFieldFilterWheel( int filter )
{
	KillTimer( TIMER_SCOPE );
		
	CString temp;
	temp.Format("1CD %d\r\n", filter );
	WriteScope( temp );

	m_Radio_BFW = filter - 1; UpdateData( false );

    SetTimer( TIMER_SCOPE, 250, NULL );
}


/**************************************************************************************
LIGHT PATH
**************************************************************************************/

void COctopusScope::ChangePath( void )
{
	working = true;
	if( Path_camera )
	{
		WriteScope( "1PRISM 1\r\n" );
		Path_camera = false;
	} 
	else
	{
		WriteScope( "1PRISM 2\r\n" );
		Path_camera = true;
	}
	working = false;
}

/**************************************************************************************
LAMP
**************************************************************************************/

void COctopusScope::BrightField( int volt )
{
	working = true;

	if ( volt >  60 ) volt =  60;
	if ( volt <   0 ) volt =   0;

	CString temp;
	temp.Format("1LMP %d\r\n", volt);
	WriteScope( temp );

	DisplayBFIntensity( volt );

	working = false;
}

void COctopusScope::OnNMCustomdrawExecute( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	int CurPos = m_Slider.GetPos();
	
	if ( CurPos >  60 ) CurPos =  60;
	if ( CurPos <   0 ) CurPos =   0;
	
	BrightField( CurPos );

	*pResult = 0;	
}

void COctopusScope::DisplayBFIntensity( int val )
{
	if( IsWindowVisible() ) 
	{
		CString str;
		str.Format("BF Lamp (0-6V): %.1f", double(val)/10 );
		m_Slider_Setting.SetWindowText( str );
		m_Slider.SetPos( val );
		UpdateData( false );
	}
}

/**************************************************************************************
NITTY
**************************************************************************************/
bool COctopusScope::WriteScope( CString str )
{

	if( Scope_initialized )
	{
		sprintf(pPortScope->mOutBuf, _T("%s"), str.GetBuffer());

		str.ReleaseBuffer();
		
		if( pPortScope->WriteCPort() ) 
		{
			while( pPortScope->mResWrite != true ) 
			{
				pPortScope->CheckWrite();
			}
		}
		else//an error occurred during the write
			return false;
        
		return true;//the write was successful
	}
	else//the scope was not first initialized
		return false;
}

CString COctopusScope::ReadScope( u16 CharsToRead )
{
	
	if( pPortScope == NULL ) 
		return "Error";
	
	pPortScope->mBytesToRead = CharsToRead;

	if( pPortScope->ReadCPort() )//issue the comm port read command
	{
		while( pPortScope->mResRead != true ) 
		{
			pPortScope->CheckRead();
		}	
		return (CString)pPortScope->mInBuf;
	}
	else 
	{
		return "Error";
	}
}

BOOL COctopusScope::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}
