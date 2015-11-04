
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusLasers.h"
#include "APTMotor.h"

extern COctopusGlobals B;

COctopusLasers::COctopusLasers(CWnd* pParent)
	: CDialog(COctopusLasers::IDD, pParent)
{    

	B.Laser_405_is_On = false;
	B.Laser_561_is_On = false;
	B.Laser_488_is_On = false;
	B.Laser_639_is_On = false;
	//
	Laser_405_good = false;
    Laser_488_good = false;
    Laser_561_good = false;
	Laser_639_good = false;

	VERIFY(m_bmp_no.LoadBitmap(IDB_NO));
	VERIFY(m_bmp_yes.LoadBitmap(IDB_YES));

	if( Create(COctopusLasers::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	Initialize(); 
}

void COctopusLasers::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Control(pDX, IDC_LASER_405ONOFF_BMP, m_status_405);
	DDX_Control(pDX, IDC_MGSHUTTERCTRL1,     Laser405Shutter);
	DDX_Control(pDX, IDC_LASER_488ONOFF_BMP, m_status_488);
	DDX_Control(pDX, IDC_MGSHUTTERCTRL2,     Laser488Shutter);
	DDX_Control(pDX, IDC_LASER_561ONOFF_BMP, m_status_561);
	DDX_Control(pDX, IDC_MGSHUTTERCTRL3,     Laser561Shutter);
	DDX_Control(pDX, IDC_LASER_639ONOFF_BMP, m_status_639);
	DDX_Control(pDX, IDC_MGSHUTTERCTRL4,     Laser639Shutter);
}  

BEGIN_MESSAGE_MAP(COctopusLasers, CDialog)
	ON_BN_CLICKED(IDC_LASER_405,	 OnClicked405)
	ON_BN_CLICKED(IDC_LASER_488,	 OnClicked488)
	ON_BN_CLICKED(IDC_LASER_561,	 OnClicked561)
	ON_BN_CLICKED(IDC_LASER_639,	 OnClicked639)
END_MESSAGE_MAP()

COctopusLasers::~COctopusLasers() 
{  
}

void COctopusLasers::OnClicked405()
{
	if ( B.Laser_405_is_On ) {
		Laser_405_Off();
	} else {
		Laser_405_On();
	}
}

void COctopusLasers::OnClicked488()
{
	if ( B.Laser_488_is_On ) {
		Laser_488_Off();
	} else {
		Laser_488_On();
	}
}

void COctopusLasers::OnClicked561()
{
	if ( B.Laser_561_is_On ) {
		Laser_561_Off();
	} else {
		Laser_561_On();
	}
}

void COctopusLasers::OnClicked639()
{
	if ( B.Laser_639_is_On ) {
		Laser_639_Off();
	} else {
		Laser_639_On();
	}
}

void COctopusLasers::Laser_639_On( void ) {		
	if( Laser_639_good ) {
		Laser639Shutter.SC_Enable( 0 );
		B.Laser_639_is_On = true;
		m_status_639.SetBitmap( m_bmp_yes );
	}
}

void COctopusLasers::Laser_639_Off( void ) {		
	if( Laser_639_good ) {
		Laser639Shutter.SC_Disable( 0 );
		B.Laser_639_is_On = false;	
		m_status_639.SetBitmap( m_bmp_no );
	}
}

void COctopusLasers::Laser_405_On( void ) {		
	if( Laser_405_good ) {
		Laser405Shutter.SC_Enable( 0 );
		B.Laser_405_is_On = true;
		m_status_405.SetBitmap( m_bmp_yes );
	}
}

void COctopusLasers::Laser_405_Off( void ) {		
	if( Laser_405_good ) {
		Laser405Shutter.SC_Disable( 0 );
		B.Laser_405_is_On = false;	
		m_status_405.SetBitmap( m_bmp_no );
	}
}

void COctopusLasers::Laser_488_On( void ) {		
	if( Laser_488_good ) {
		Laser488Shutter.SC_Enable( 0 );
		B.Laser_488_is_On = true;
		m_status_488.SetBitmap( m_bmp_yes );
	}
}

void COctopusLasers::Laser_488_Off( void ) {		
	if( Laser_488_good ) {
		Laser488Shutter.SC_Disable( 0 );
		B.Laser_488_is_On = false;	
		m_status_488.SetBitmap( m_bmp_no );
	}
}

void COctopusLasers::Laser_561_On( void ) {		
	if( Laser_561_good ) {
		Laser561Shutter.SC_Enable( 0 );
		B.Laser_561_is_On = true;
		m_status_561.SetBitmap( m_bmp_yes );
	}
}

void COctopusLasers::Laser_561_Off( void ) {		
	if( Laser_561_good ) {
		Laser561Shutter.SC_Disable( 0 );
		B.Laser_561_is_On = false;	
		m_status_561.SetBitmap( m_bmp_no );
	}
}

void COctopusLasers::Initialize( void ) 
{
    /*
	APT_sys.StartCtrl();

	long units = 0;

	APT_sys.GetNumHWUnits( 6, (long *)&units );

	if ( units != 3 )
	{
		AfxMessageBox("You forgot to turn on the\npower to the USB shutters!");
		return;
	}
	*/

	Laser405Shutter.StartCtrl();
	Laser488Shutter.StartCtrl();
	Laser561Shutter.StartCtrl();
	Laser639Shutter.StartCtrl();
	
	long error = 0;

	Laser405Shutter.SetHWSerialNum(85813949);
	error = Laser405Shutter.StartCtrl();
	if ( error == 0 ) {
		Laser_405_good = true;
		Laser405Shutter.SC_SetOperatingMode( 0 , 1 );
		Laser_405_Off();
	}

	Laser488Shutter.SetHWSerialNum(85813983);
	error = Laser488Shutter.StartCtrl();
	if ( error == 0 )
	{
		Laser_488_good = true;
		Laser488Shutter.SC_SetOperatingMode( 0 , 1 );
		Laser_488_Off();
	}
	
	Laser561Shutter.SetHWSerialNum(85813972);
	error = Laser561Shutter.StartCtrl();
	if ( error == 0 )
	{
		Laser_561_good = true;
		Laser561Shutter.SC_SetOperatingMode( 0 , 1 );
		Laser_561_Off();
	}

	Laser639Shutter.SetHWSerialNum(85819090);
	error = Laser639Shutter.StartCtrl();
	if ( error == 0 )
	{
		Laser_639_good = true;
		Laser639Shutter.SC_SetOperatingMode( 0 , 1 );
		Laser_639_Off();
	}

	Sleep( 3000 );

	B.Lasers_loaded = true;
}

BOOL COctopusLasers::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}