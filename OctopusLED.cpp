
#include "stdafx.h"
#include "Octopus.h"
#include "OctopusLED.h"

extern COctopusGlobals B;

COctopusLED::COctopusLED(CWnd* pParent)
	: CDialog(COctopusLED::IDD, pParent)
{    

	LED_intensity_setpoint = 0;
	LED_intensity_current  = 0;
	first_tick             = true;
    B.ADC_1                = 0;

	if( Create(COctopusLED::IDD, pParent) ) 
		ShowWindow( SW_SHOW );

	m_Slider.SetRange( 0, 100 );
	m_Slider.SetPos( 100 );
	m_Slider.SetTicFreq( 5 );

	SetTimer( TIMER_ADC, 500, NULL );
}

BOOL COctopusLED::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowPos(NULL, 493, 775, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	hdrvr_9812 = NULL;
	olDaInitialize(PTSTR("DT9812-10V(00)"), &hdrvr_9812 );	
	
	olDaGetDASS( hdrvr_9812, OLSS_DA, 0, &hdass_9812_DAC );
    olDaSetDataFlow( hdass_9812_DAC, OL_DF_SINGLEVALUE );
    olDaConfig( hdass_9812_DAC );
	
	olDaGetDASS( hdrvr_9812, OLSS_AD, 0, &hdass_9812_ADC );
	olDaSetDataFlow( hdass_9812_ADC, OL_DF_SINGLEVALUE );
	olDaConfig( hdass_9812_ADC );

	olDaGetDASS( hdrvr_9812, OLSS_DOUT, 0, &hdass_9812_DOUT );
	olDaSetDataFlow( hdass_9812_DOUT, OL_DF_SINGLEVALUE );
	olDaConfig( hdass_9812_DOUT );

	olDaGetRange(hdass_9812_ADC,&ADCmax,&ADCmin);
	olDaGetEncoding(hdass_9812_ADC,&encoding);
	olDaGetResolution(hdass_9812_ADC,&resolution);

	if ( hdrvr_9812 == NULL )
		AfxMessageBox(_T("Connect to DT9812 board failed.\nHave you plugged it in and turned it on?"));

	return TRUE;
}

void COctopusLED::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange( pDX );
	DDX_Control(pDX,	IDC_LED_INTENSITY_SLIDER, m_Slider);
	DDX_Control(pDX,	IDC_LED_INTENSITY,        m_Slider_Text);
	DDX_Control(pDX,	IDC_LED_ADC,              m_ADC_Text);
}  

BEGIN_MESSAGE_MAP(COctopusLED, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LED_INTENSITY_SLIDER, OnNMCustomdrawLedIntensitySlider)
END_MESSAGE_MAP()

COctopusLED::~COctopusLED() 
{  
	olDaReleaseDASS( hdass_9812_ADC );
	olDaReleaseDASS( hdass_9812_DAC );
	olDaTerminate( hdrvr_9812 );
}

void COctopusLED::TTL_Pulse_Up( void )
{
	long value = (1L<<8)-1;
	//TTL is now high
	olDaPutSingleValue(hdass_9812_DOUT,value,0,1.0);
	Sleep(10);
	//and let it fall
	olDaPutSingleValue(hdass_9812_DOUT,0,0,1.0);
}

void COctopusLED::LED_On( void )
{
	LED_SetIntensity( LED_intensity_setpoint );
}

void COctopusLED::LED_On( u16 intensity )
{
	LED_intensity_setpoint = intensity;
	LED_SetIntensity( LED_intensity_setpoint );
}

void COctopusLED::LED_Off( void )
{
	LED_SetIntensity( 0 );
}

void COctopusLED::LED_SetIntensity( u16 intensity ) 
{

	if ( intensity > 100 ) intensity = 100;
	if ( intensity <   0 ) intensity =   0;

	LED_intensity_current = intensity;

	u32 u32_intensity = 1024; //Ann's scope

	//0    = 0  volts = max brightness
	//1024 = 5  volts = off

	if ( intensity == 0 )
		u32_intensity = 1024;
	else
		u32_intensity = 827 - (intensity * 5);

	if ( u32_intensity > 1024 ) 
		u32_intensity = 1024;
	if ( u32_intensity <  327 ) 
		u32_intensity =  327;

	olDaPutSingleValue(hdass_9812_DAC, u32_intensity, 0, 0);

	UpdateLEDIntensity();
}

void COctopusLED::UpdateLEDIntensity( void )
{
	if( IsWindowVisible() && !B.focus_in_progress ) 
	{
		CString str;
		
		str.Format(_T("LED Intensity:\n%d %%"), LED_intensity_current );
		m_Slider_Text.SetWindowText( str );
		m_Slider.SetPos( 100 - LED_intensity_current );

		str.Format(_T("ADC signal:\n%.2f C"), B.ADC_1 );
		m_ADC_Text.SetWindowText( str );

		UpdateData( false );
	}
}

BOOL COctopusLED::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
    return CDialog::OnCommand(wParam, lParam);
}

void COctopusLED::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == TIMER_ADC ) 
	{
		if( first_tick ) 
		{
			LED_On( 50 );
			first_tick = false;
		}

		olDaGetSingleValue(hdass_9812_ADC,&ADCvalue,0,1);

		if ( encoding != OL_ENC_BINARY ) 
		{
		    //convert to offset binary by inverting the sign bit
			ADCvalue ^=  1L << (resolution - 1);
			ADCvalue &= (1L <<  resolution)- 1; //zero upper bits
		}
   
		double val = float(ADCmax-ADCmin)/(1L<<resolution) * ADCvalue + (float)ADCmin;

		B.ADC_1 = val * 20.0;

		UpdateLEDIntensity();

		TTL_Pulse_Up();
	}
	CDialog::OnTimer(nIDEvent);
}
void COctopusLED::OnNMCustomdrawLedIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	u16 CurPos = m_Slider.GetPos();
	
	if ( CurPos > 100 ) CurPos = 100;
	if ( CurPos <   0 ) CurPos =   0;
	
	LED_intensity_setpoint = 100 - CurPos;

	if ( LED_intensity_setpoint > 0 )
		LED_On( LED_intensity_setpoint );
	else
		LED_Off();
	
	*pResult = 0;
}
