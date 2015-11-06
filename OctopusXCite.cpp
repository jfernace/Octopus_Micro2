
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusXCite.h"
#include "OctopusGlobals.h"
#include "cport.h"
#include <bitset>
extern COctopusGlobals B;

COctopusXCite::COctopusXCite(CWnd* pParent)
	: CDialog(COctopusXCite::IDD, pParent)
	, m_LEDIntensity(0)
	, m_IntensitySlider(0)
{    
	position_now      = 0;
	stepsize_10nm     = 10;
	pPortScope        = NULL;
	Scope_initialized = false;
	old_position      = 0;
	slampIntensity = "Starting...";
	lampIntensity = 0;

	VERIFY(m_bmp_on.LoadBitmap(IDB_ON));
	VERIFY(m_bmp_off.LoadBitmap(IDB_OFF));

	if( Create(COctopusXCite::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	first_tick  = true;
	working     = false;
	Path_camera = false;

	//m_Slider.SetRange( 0, 100 );
	//m_Slider.SetPos( 0 );
	//m_Slider.SetTicFreq( 10 );
	B.XCite_loaded = true;

	Init();


	m_islider.SetRange(0, 100);
	//m_islider.SetPos( (int) ND_value );
	m_islider.SetTicFreq(10);

}

BOOL COctopusXCite::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetTimer( TIMER_SCOPE, 250, NULL );
	return TRUE;
}

void COctopusXCite::OnTimer( UINT nIDEvent ) 
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

bool COctopusXCite::Init( void )
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

	//right now LED lamp light source for Fl. imaging is on COM4 on USB3.0
	pPortScope->mPort.Format(_T("COM4"));

	if( pPortScope->OpenCPort() )//open and setup the comm port
	{
		Scope_initialized = true;

		//connect to the PC -- enables control of the unit
		WriteScope("tt\r\n"); 

		Sleep(30);

		WriteScope("jj\r");//we want to be able to use the advanced commands
		Sleep(30);
		
		WriteScope("cc\r");//we want to be able to use the advanced commands
		Sleep(30);

		GetIntensityLevel();
		GetLEDStatus();
		getTemperature();

		return true;

	}
	else 
	{
		Close();
		return false;
	}
}

void COctopusXCite::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	//DDX_Radio(pDX,		IDC_SCOPE_SSIZE_1,			        m_Radio_S);
	//DDX_Radio(pDX,		IDC_SCOPE_OBJ_1,			        m_Radio_OBJ);
	//DDX_Radio(pDX,		IDC_SCOPE_FW_1,			            m_Radio_FW);
	//DDX_Radio(pDX,		IDC_SCOPE_BFW_1,		            m_Radio_BFW);
	//DDX_Control(pDX,	IDC_SCOPE_POS,				        m_Pos);
	//DDX_Control(pDX,	IDC_SCOPE_INTENSITY_SLIDER,		    m_Slider);
	//DDX_Control(pDX,	IDC_SCOPE_INTENSITY_SLIDER_SETTING, m_Slider_Setting);

	DDX_Text( pDX, IDC_DOUT1 , slampIntensity);
	DDX_Text( pDX, IDC_LED_TEMPERATURE , sLEDTemperature);

	//DDX_Control(pDX, IDC_MANUAL_ADJUST_EDIT, m_lampIntensity);

	DDX_Text(pDX, IDC_MANUAL_ADJUST_EDIT, m_LEDIntensity);
	DDV_MinMaxInt(pDX, m_LEDIntensity, 0, 100);
	DDX_Slider(pDX, IDC_INTENSITY_SLIDER, m_IntensitySlider);
	DDX_Control(pDX, IDC_INTENSITY_SLIDER, m_islider);
	DDX_Control(pDX,  IDC_XCITE_ONOFF, m_status_LED);
}  

BEGIN_MESSAGE_MAP(COctopusXCite, CDialog)
	//ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SCOPE_INTENSITY_SLIDER, OnNMCustomdrawExecute)
	//ON_BN_CLICKED(IDC_SCOPE_DOWN,	     Z_StepDown)
	//ON_BN_CLICKED(IDC_SCOPE_UP,		     Z_StepUp)
	//ON_BN_CLICKED(IDC_SCOPE_ESC,		 ObjESC)
	//ON_BN_CLICKED(IDC_SCOPE_RTN,		 ObjRTN)
	//ON_BN_CLICKED(IDC_SCOPE_SSIZE_1,     OnObjectiveStepSize1)
	//ON_BN_CLICKED(IDC_SCOPE_SSIZE_2,     OnObjectiveStepSize2)
	//ON_BN_CLICKED(IDC_SCOPE_SSIZE_3,     OnObjectiveStepSize3)
	//ON_BN_CLICKED(IDC_SCOPE_SSIZE_4,     OnObjectiveStepSize4)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_1,       OnObj_1)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_2,       OnObj_2)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_3,       OnObj_3)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_4,       OnObj_4)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_5,       OnObj_5)
	//ON_BN_CLICKED(IDC_SCOPE_OBJ_6,       OnObj_6)
	//ON_BN_CLICKED(IDC_SCOPE_FW_1,        OnFW_1)
	//ON_BN_CLICKED(IDC_SCOPE_FW_2,        OnFW_2)
	//ON_BN_CLICKED(IDC_SCOPE_FW_3,        OnFW_3)
	//ON_BN_CLICKED(IDC_SCOPE_FW_4,        OnFW_4)
	//ON_BN_CLICKED(IDC_SCOPE_FW_5,        OnFW_5)
	//ON_BN_CLICKED(IDC_SCOPE_FW_6,        OnFW_6)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_1,       OnBFW_1)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_2,       OnBFW_2)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_3,       OnBFW_3)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_4,       OnBFW_4)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_5,       OnBFW_5)
	//ON_BN_CLICKED(IDC_SCOPE_BFW_6,       OnBFW_6)
	//ON_BN_CLICKED(IDC_SCOPE_PATH,        ToggleXCiteLEDStatus)
	//ON_WM_TIMER()
	//ON_STN_CLICKED(IDC_SCOPE_INTENSITY_SLIDER_SETTING, &COctopusXCite::OnStnClickedScopeIntensitySliderSetting)
	//ON_STN_CLICKED(IDC_SCOPE_POS, &COctopusXCite::OnStnClickedScopePos)
	//ON_STN_CLICKED(IDC_XCITE_ONOFF, &COctopusXCite::OnStnClickedXciteOnoff)
	//ON_STN_CLICKED(IDC_SHUTTER_IMAGE, &COctopusXCite::OnStnClickedShutterImage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_XCITE_INTENSITY_SLIDER2, &COctopusXCite::OnNMCustomdrawXciteIntensitySlider2)
	//	ON_EN_CHANGE(IDC_MANUAL_INTENSITY, &COctopusXCite::OnEnChangeManualIntensity)
	//	ON_EN_CHANGE(IDC_MG17LOGGERCTRL1, &COctopusXCite::OnEnChangeMg17loggerctrl1)
	//	ON_EN_CHANGE(IDC_MANUAL_ADJUST_EDIT, &COctopusXCite::OnEnChangeManualAdjustEdit)
	ON_BN_CLICKED(IDC_ADJUST_INTENSITY, &COctopusXCite::OnBnClickedAdjustIntensity)
	//ON_BN_CLICKED(IDC_SCOPE_PATH, &COctopusXCite::OnBnClickedScopePath)
	ON_BN_CLICKED(IDC_LED_ONOFF, &COctopusXCite::OnBnClickedLedOnoff)
	ON_STN_CLICKED(IDC_LED_TEMPERATURE, &COctopusXCite::OnStnClickedLedTemperature)
END_MESSAGE_MAP()

/**************************************************************************************
SHUTDOWN
**************************************************************************************/

void COctopusXCite::Close() 
{  
	//turn off lamp 
			WriteScope(_T("zz\r\n"));
	Sleep(30);

			WriteScope(_T("ss\r\n"));
	Sleep(30);

			WriteScope(_T("yy\r\n"));
	Sleep(30);
//Disconnect PC, disconnects all control from the PC for the X-Cite exacte.
	WriteScope(_T("xx\r\n"));
	Sleep(30);




	////	WriteScope(_T("2LOG OUT\r\n"));
	////	Sleep(30);

	Scope_initialized = false;

	if ( pPortScope != NULL ) 
	{
		delete pPortScope;
		pPortScope = NULL;
	}

}

COctopusXCite::~COctopusXCite() 
{  
	Close();
}

/**************************************************************************************
OBJECTIVE Z SETTING
**************************************************************************************/

void COctopusXCite::Z_StepDown( void ) 
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
void COctopusXCite::Z_StepUp( void ) 
{ 
	working = true;
	CString temp;
	temp.Format(_T("2MOV N,%d\r\n"), stepsize_10nm );
	WriteScope( temp );	
	WriteScope(_T("2JOG ON\r\n"));
	Sleep(30);
	working = false;
}

void COctopusXCite::ObjESC( void ) 
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

void COctopusXCite::ObjRTN( void ) 
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

void COctopusXCite::OnObjectiveStepSize1() 
{ m_Radio_S = 0; stepsize_10nm =   1; UpdateData( false ); }; //0.01
void COctopusXCite::OnObjectiveStepSize2() 
{ m_Radio_S = 1; stepsize_10nm =   5; UpdateData( false ); }; //0.05
void COctopusXCite::OnObjectiveStepSize3() 
{ m_Radio_S = 2; stepsize_10nm =  10; UpdateData( false ); }; //0.10
void COctopusXCite::OnObjectiveStepSize4() 
{ m_Radio_S = 3; stepsize_10nm = 100; UpdateData( false ); }; //1.00

void COctopusXCite::GetPosition( void )
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

void COctopusXCite::DisplayPosition( void )
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

void COctopusXCite::OnObj_1() { m_Radio_OBJ = 0; Objective( 1 ); UpdateData( false ); };
void COctopusXCite::OnObj_2() { m_Radio_OBJ = 1; Objective( 2 ); UpdateData( false ); };
void COctopusXCite::OnObj_3() { m_Radio_OBJ = 2; Objective( 3 ); UpdateData( false ); };
void COctopusXCite::OnObj_4() { m_Radio_OBJ = 3; Objective( 4 ); UpdateData( false ); };
void COctopusXCite::OnObj_5() { m_Radio_OBJ = 4; Objective( 5 ); UpdateData( false ); };
void COctopusXCite::OnObj_6() { m_Radio_OBJ = 5; Objective( 6 ); UpdateData( false ); };

void COctopusXCite::OnFW_1() { m_Radio_FW = 0; EpiFilterWheel( 1 ); UpdateData( false ); };
void COctopusXCite::OnFW_2() { m_Radio_FW = 1; EpiFilterWheel( 2 ); UpdateData( false ); };
void COctopusXCite::OnFW_3() { m_Radio_FW = 2; EpiFilterWheel( 3 ); UpdateData( false ); };
void COctopusXCite::OnFW_4() { m_Radio_FW = 3; EpiFilterWheel( 4 ); UpdateData( false ); };
void COctopusXCite::OnFW_5() { m_Radio_FW = 4; EpiFilterWheel( 5 ); UpdateData( false ); };
void COctopusXCite::OnFW_6() { m_Radio_FW = 5; EpiFilterWheel( 6 ); UpdateData( false ); };

void COctopusXCite::OnBFW_1() { m_Radio_BFW = 0; BrightFieldFilterWheel( 1 ); UpdateData( false ); };
void COctopusXCite::OnBFW_2() { m_Radio_BFW = 1; BrightFieldFilterWheel( 2 ); UpdateData( false ); };
void COctopusXCite::OnBFW_3() { m_Radio_BFW = 2; BrightFieldFilterWheel( 3 ); UpdateData( false ); };
void COctopusXCite::OnBFW_4() { m_Radio_BFW = 3; BrightFieldFilterWheel( 4 ); UpdateData( false ); };
void COctopusXCite::OnBFW_5() { m_Radio_BFW = 4; BrightFieldFilterWheel( 5 ); UpdateData( false ); };
void COctopusXCite::OnBFW_6() { m_Radio_BFW = 5; BrightFieldFilterWheel( 6 ); UpdateData( false ); };


void COctopusXCite::Objective( int obj )
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

int COctopusXCite::GetObj( void )
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

int COctopusXCite::GetFW( void )
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

int COctopusXCite::GetBFW( void )
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

void COctopusXCite::EpiFilterWheel( int cube )
{
	KillTimer( TIMER_SCOPE );

	CString temp;
	temp.Format("1MU %d\r\n", cube );
	WriteScope( temp );

	m_Radio_FW = cube - 1; UpdateData( false );

	SetTimer( TIMER_SCOPE, 250, NULL );
}

void COctopusXCite::BrightFieldFilterWheel( int filter )
{
	KillTimer( TIMER_SCOPE );

	CString temp;
	temp.Format("1CD %d\r\n", filter );
	WriteScope( temp );

	m_Radio_BFW = filter - 1; UpdateData( false );

	SetTimer( TIMER_SCOPE, 250, NULL );
}


/**************************************************************************************
TURN THE LED ON/OFF!!!
**************************************************************************************/

void COctopusXCite::ToggleXCiteLEDStatus( void )
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

void COctopusXCite::BrightField( int volt )
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

void COctopusXCite::OnNMCustomdrawExecute( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	int CurPos = m_Slider.GetPos();

	if ( CurPos >  60 ) CurPos =  60;
	if ( CurPos <   0 ) CurPos =   0;

	BrightField( CurPos );

	*pResult = 0;	
}

void COctopusXCite::DisplayBFIntensity( int val )
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
bool COctopusXCite::WriteScope( CString str )
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

CString COctopusXCite::ReadScope( u16 CharsToRead )
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

BOOL COctopusXCite::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
	return CDialog::OnCommand(wParam, lParam);
}


void COctopusXCite::OnStnClickedScopeIntensitySliderSetting()
{
	// TODO: Add your control notification handler code here
}


void COctopusXCite::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void COctopusXCite::OnStnClickedScopePos()
{
	// TODO: Add your control notification handler code here
}



void COctopusXCite::OnStnClickedShutterImage()
{
	// TODO: Add your control notification handler code here
}


void COctopusXCite::OnNMCustomdrawXciteIntensitySlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}



void COctopusXCite::GetIntensityLevel()
{
	CString temp;
	CString str;
	working = true;
	str = "dd\r\n";
	WriteScope(_T(str));//command to ask camera for lamp intensity
	Sleep(30);
	temp = ReadScope(16);


	slampIntensity=temp;
	slampIntensity.Append("%");
	slampIntensity.Replace("\r","");
	m_LEDIntensity = atoi(slampIntensity);
	UpdateData(false);
	working = false;

}








void COctopusXCite::OnBnClickedAdjustIntensity()
{
	CString str;
	CString temp;
	working = true;
	UpdateData(true);
	// get value from text box and update the LED to reflect
	//if (m_LEDIntensity > 4 ){
	//	m_LEDIntensity=4;

	//}else if( m_LEDIntensity < 0){
	//	m_LEDIntensity=0;
	//}
	str.Format("i%i\r", m_LEDIntensity); 

	temp = ReadScope(1);
	str.Format("d%03i\r", m_LEDIntensity); 
	WriteScope(_T(str));//command to ask camera for lamp intensity
	Sleep(30);
	temp = ReadScope(1);
	GetIntensityLevel();
	working = false;
}


void COctopusXCite::GetLEDStatus()

{
	//xxx\r” where xxx is the status of the unit. The number returned is bitwise and is decoded as follows:
	//Bit 15 – Iris Moving: 1 = Iris movement complete, 0 = Iris Moving.
	//bit 14 – CLF Engaged
	//bit 10 – Light guide inserted
	//bit 8 – X-Cite exacte communication mode.
	//Bit 7 – Power or Intensity Mode : 1 = Power mode, 0 = Intensity Mode
	//bit 5 - Lock Bit: 1 = front panel locked, 0 = front panel unlocked;
	//bit 4 - Lamp Ready Bit: 1 = lamp is ready, 0 = lamp is not ready;
	//bit 3 - Home Bit: 1 = fault, 0 = pass;
	//bit 2 - Shutter Bit: 1 = shutter is opened, 0 = shutter is closed;
	//bit 1 - Lamp Bit: 1 = lamp is ON, 0 = lamp is OFF;
	//bit 0 - Alarm Bit: 1 = alarm is ON, 0 = alarm is OFF.
	//Note: The 110LED and 120LED will respond to CLF commands and Calibration commands, however since these features are not enabled on these products, the responses are simulated for compatibility purposes only.


	///note to self -- they call the "shutter" turning the LED power on (ie the lamp xcite unit has to manually switched on, then according to the software the "lamp Bit" is on
	CString str;
	CString temp;
	int xxx;
	working = true;

	str="uu\r";  
	WriteScope(_T(str));//command to ask camera for lamp intensity
	Sleep(30);
	temp = ReadScope(16);
	std::string binary = std::bitset<16>(atoi(temp)).to_string(); //to binary
	IS_LED_ON =  (atoi(temp) & 4)!=0;

	if (IS_LED_ON) m_status_LED.SetBitmap( m_bmp_on );

	UpdateData(true);
	working = false;

}



void COctopusXCite::OnBnClickedLedOnoff()
{
		// TODO: Add your control notification handler code here
	CString temp;
	CString str;

	working = true;

	if (IS_LED_ON){
		//turn it off
		str = "zz\r\n";

		WriteScope(_T(str));//command to ask camera for lamp intensity
		Sleep(30);

		str="uu\r";  
		WriteScope(_T(str));//command to ask camera for lamp intensity
		Sleep(30);
		temp = ReadScope(16);
		IS_LED_ON =   (atoi(temp) & 4)!=0;

		m_status_LED.SetBitmap( m_bmp_off );

	}else {
		//turn it ON
		str = "mm\r\n";

		WriteScope(_T(str));
		Sleep(30);

		str="uu\r";  
		WriteScope(_T(str));
		Sleep(30);
		temp = ReadScope(16);
	
		IS_LED_ON = (atoi(temp) & 4)!=0;
		m_status_LED.SetBitmap( m_bmp_on );

	
	}
	getTemperature();
	UpdateData(true);
	working = false;
}

void COctopusXCite::getTemperature()

{
	working = true;
	CString temp;
	CString str;

		str="gt\r";  
		WriteScope(_T(str));
		Sleep(30);
		temp = ReadScope(16);
	
	sLEDTemperature=temp;
	
	sLEDTemperature.Replace("\r","");
	
	sLEDTemperature.Replace(","," and ");

	UpdateData(false);//update and refresh display
	working = false;

}

void COctopusXCite::OnStnClickedLedTemperature()
{
	// TODO: Add your control notification handler code here
}
