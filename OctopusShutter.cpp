
#include "stdafx.h"
#include "ftd2xx.h"
#include "Octopus.h"
#include "OctopusCameraDlg.h"
#include "OctopusDoc.h"
#include "OctopusView.h"
#include "OctopusGlobals.h"
#include "OctopusShutter.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
//#include <tiffio.h>



#include "windows.h"

extern COctopusGlobals B;

extern COctopusCamera*          glob_m_pCamera;

COctopusShutterAndWheel::COctopusShutterAndWheel(CWnd* pParent)
: CDialog(COctopusShutterAndWheel::IDD, pParent)
{    

	USB_ready               =  false;
	m_ShutterOpen			=  2; // 2 means that the shutter is closed
	bus_busy				=  0;
	board_present			=  0;
	B.filter_wheel          =  0;
	ND_value                =  0;
	m_snap_power_percent    = 100;
	open_imagej_on_snap     = true;

	if( Create(COctopusShutterAndWheel::IDD, pParent) ) {
		ShowWindow( SW_SHOW );
	}
}


void COctopusShutterAndWheel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,	IDC_SHUTTER_SLIDER,				m_Slider);
	DDX_Control(pDX,	IDC_SHUTTER_SLIDER_SETTING,		m_Slider_Setting);
	//DDX_Radio(pDX,		IDC_RADIO_FILTER_0,				m_Filter);
	DDX_Text( pDX, IDC_SNAP_POWER, m_snap_power_percent);
	DDX_Control(pDX, IDC_CHECK_IMAGEJ,  m_ImageJ_CheckBox);
	DDV_MinMaxInt(pDX, m_snap_power_percent, 1, 100);
}  


BEGIN_MESSAGE_MAP(COctopusShutterAndWheel, CDialog)
	/*
	ON_BN_CLICKED(IDC_RADIO_FILTER_0,			OnRadioFilter0)
	ON_BN_CLICKED(IDC_RADIO_FILTER_1,			OnRadioFilter1)
	ON_BN_CLICKED(IDC_RADIO_FILTER_2,			OnRadioFilter2)
	ON_BN_CLICKED(IDC_RADIO_FILTER_3,			OnRadioFilter3)
	ON_BN_CLICKED(IDC_RADIO_FILTER_4,			OnRadioFilter4)
	ON_BN_CLICKED(IDC_RADIO_FILTER_5,			OnRadioFilter5)
	ON_BN_CLICKED(IDC_RADIO_FILTER_6,			OnRadioFilter6)
	ON_BN_CLICKED(IDC_RADIO_FILTER_7,			OnRadioFilter7)
	ON_BN_CLICKED(IDC_RADIO_FILTER_8,			OnRadioFilter8)
	ON_BN_CLICKED(IDC_RADIO_FILTER_9,			OnRadioFilter9)
	*/	
	ON_WM_TIMER()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SHUTTER_SLIDER, OnNMCustomdrawExecute)
	ON_EN_KILLFOCUS(IDC_SNAP_POWER,   OnKillFocusGeneral)
	ON_BN_CLICKED(IDC_CHECK_IMAGEJ,    OnBnClickedImageJ)
	ON_BN_CLICKED(IDC_SNAP_BUTTON,	         OnSnap)

	ON_BN_CLICKED(IDC_ReturnFilterControl, &COctopusShutterAndWheel::OnBnClickedReturnfiltercontrol)
END_MESSAGE_MAP()


BOOL COctopusShutterAndWheel::OnInitDialog() 
{

	CDialog::OnInitDialog();

	m_nTimer = SetTimer(TIMER_LAMBDA, 200, NULL); // check for new USB data in the buffer every 10 ms

	Loadem();									  // load DLL and test for presence of DLP-USB1 and DLP-USB2
	ShutterClose();								  // close the shutter

	m_Slider.SetRange(0, 100);
	m_Slider.SetPos( (int) ND_value );
	m_Slider.SetTicFreq(10);
	if (open_imagej_on_snap) 
		CheckDlgButton(IDC_CHECK_IMAGEJ, BST_CHECKED);
	else
		CheckDlgButton(IDC_CHECK_IMAGEJ, BST_UNCHECKED);
	//m_snap_power_percent = default_snap_percent;

	return TRUE;
}

COctopusShutterAndWheel::~COctopusShutterAndWheel() 
{
	Close();
	FreeLibrary(m_hmodule);
}

/***************************************************************************/
/***************************************************************************/


/*
void COctopusShutterAndWheel::OpenImageJ()
{
SHELLEXECUTEINFO ShExecInfo;
TIFF *img;
CString tiff_full_pathname;
int i;
bool tiff_write_error;
char *filename_text;



ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
// NULL = use process defaults
ShExecInfo.fMask = NULL;
// hwnd = handle to parent window in case errors arise
ShExecInfo.hwnd = NULL;
// The kind of call to use NULL gives open default
ShExecInfo.lpVerb = "open";
// The file argument
// ShExecInfo.lpFile = "C:\\Program Files\\ImageJ\\imagej.exe";
ShExecInfo.lpFile = "C:\\Program Files\\ImageJ\\jre\\bin\\java.exe";
ShExecInfo.lpParameters = "-jar ij.jar -macro OpenFromOctopus blobs.tif";
// The current working directory
ShExecInfo.lpDirectory = "C:\\Program Files\\ImageJ";
// Required Flags that specify how an application is to be shown when it is opened
ShExecInfo.nShow = SW_MAXIMIZE;
// Where the output is stored
ShExecInfo.hInstApp = NULL;

// Now prepare the tiff file parameters
tiff_full_pathname.Format(_T("%s-%d-imagej.tif"), B.pathname, B.files_written );
// (LPCTSTR} casts Cstring to const char *
//filename_text = tiff_full_pathname.GetBuffer(tiff_full_pathname.GetLength());
filename_text = "C:\\Program\ Files\\Octopus\\Octopus_051812\\Debug\\test-for-imagej.tif";
img = TIFFOpen(filename_text, "w"); 
TIFFSetField (img, TIFFTAG_IMAGEWIDTH, B.W);  // set the width of the image
TIFFSetField(img, TIFFTAG_IMAGELENGTH, B.H);    // set the height of the image	
TIFFSetField(img, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
TIFFSetField(img, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
TIFFSetField(img, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
//   Got these from instructions on this page: http://research.cs.wisc.edu/graphics/Courses/638-f1999/libtiff_tutorial.htm
TIFFSetField(img, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
TIFFSetField(img, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); 

tiff_write_error = false;
for (i=0; i < B.H; i++) {
if ( TIFFWriteScanline(img,&B.memory[i*B.W], i, 0) < 0 ) {
tiff_write_error = true;
break;
}
}
if ( tiff_write_error ) {
AfxMessageBox(_T("Could not write tiff file."));
}
TIFFClose ( img );

ShellExecuteEx(&ShExecInfo);

// If the call succeeds, the output is greater than 32
if ((int) ShExecInfo.hInstApp <= 32) 
AfxMessageBox(_T("Could not open imagej"));
else
AfxMessageBox(_T("Opened imagej"));

}
*/

void COctopusShutterAndWheel::OnSnap()
{
	int original_shutter_intensity = m_Slider.GetPos();
	if( m_snap_power_percent == 0 ) 
		ShutterClose();
	else if ( m_snap_power_percent == 100 )
		ShutterOpen();
	else
		ShutterPartial( m_snap_power_percent ); 
	if (glob_m_pCamera != NULL)  {
		glob_m_pCamera->TakePicture();
		if (open_imagej_on_snap) {
			//OpenImageJ();

		}
	}
	if( original_shutter_intensity == 0 ) 
		ShutterClose();
	else if ( original_shutter_intensity == 100 )
		ShutterOpen();
	else
		ShutterPartial( original_shutter_intensity );
	m_Slider.SetPos( original_shutter_intensity );
}

void COctopusShutterAndWheel::Loadem()
{
	unsigned char txbuf[25], rxbuf[25];
	DWORD ret_bytes;
	FT_STATUS status;

	//CString	m_NameNmbr = _T("Sutter Instrument Lambda SC");

	m_hmodule = LoadLibrary(_T("Ftd2xx.dll"));	

	m_pWrite            = (PtrToWrite)GetProcAddress(m_hmodule, "FT_Write");
	m_pRead             = (PtrToRead)GetProcAddress(m_hmodule, "FT_Read");
	m_pOpen             = (PtrToOpen)GetProcAddress(m_hmodule, "FT_Open");
	m_pOpenEx           = (PtrToOpenEx)GetProcAddress(m_hmodule, "FT_OpenEx");
	m_pListDevices      = (PtrToListDevices)GetProcAddress(m_hmodule, "FT_ListDevices");
	m_pClose            = (PtrToClose)GetProcAddress(m_hmodule, "FT_Close");
	m_pResetDevice      = (PtrToResetDevice)GetProcAddress(m_hmodule, "FT_ResetDevice");
	m_pPurge            = (PtrToPurge)GetProcAddress(m_hmodule, "FT_Purge");
	m_pSetTimeouts      = (PtrToSetTimeouts)GetProcAddress(m_hmodule, "FT_SetTimeouts");
	m_pGetQueueStatus   = (PtrToGetQueueStatus)GetProcAddress(m_hmodule, "FT_GetQueueStatus");
	m_pSetBaudRate      = (PtrToSetBaudRate)GetProcAddress(m_hmodule, "FT_SetBaudRate");
	m_pSetLatencyTimer  = (PtrToSetLatencyTimer)GetProcAddress(m_hmodule, "FT_SetLatencyTimer");
	m_pSetUSBParameters = (PtrToSetUSBParameters)GetProcAddress(m_hmodule, "FT_SetUSBParameters");

	OpenEx((PVOID)LPCTSTR("Sutter Instrument Lambda SC"), FT_OPEN_BY_DESCRIPTION);

	SetTimeouts(100, 100);

	Close();

	status = OpenEx((PVOID)LPCTSTR("Sutter Instrument Lambda SC"), FT_OPEN_BY_DESCRIPTION);

	if(status > 0)
	{
		TRACE(_T("Shutter not opened\n"));
		USB_ready = false;
		board_present = 0;
	}
	else
	{
		TRACE(_T("Shutter opened\n"));
		ResetDevice();
		SetTimeouts(1000, 1000);
		Purge(FT_PURGE_RX || FT_PURGE_TX);
		SetLatencyTimer(2);
		SetBaudRate(128000);
		SetUSBParameters(64,0);
		Sleep(150);
		txbuf[0] = 0xEE;
		Write(txbuf, 1, &ret_bytes);
		Read(rxbuf, 2, &ret_bytes);
		Sleep(150);

		if(ret_bytes==0) 
		{
			Read(rxbuf, 2, &ret_bytes);
			TRACE(_T("ret_bytes==0\n"));
		}

		if(rxbuf[0] != 0xEE)
		{
			CString str;
			str.Format(_T("rxbuf:%04x"),rxbuf[0]);
			TRACE(str);
			TRACE(_T("Shutter not opened\n"));
			USB_ready = false;
			board_present = 0;
			Close();
		}	
		else
		{
			USB_ready = true;	
			board_present = 1;
		}
	}

	B.load_wheel_failed = false;

	if ( board_present == 0 ) 
		B.load_wheel_failed = true;

	SetTimeouts(100, 100);
	UpdateData(FALSE);
}

/***************************************************************************/
/***************************************************************************/

void COctopusShutterAndWheel::ShutterOpen() 
{
	DWORD ret_bytes;
	unsigned char txbuf[25];

	if( USB_ready )
	{
		txbuf[0] = 0xDC;
		Write(txbuf, 1, &ret_bytes);
		txbuf[0] = 0xAA;
		Write(txbuf, 1, &ret_bytes);
		B.nd_setting = 100;
		UpdateShutterVal();
	}

	SetTimeouts(100, 100);
}




void COctopusShutterAndWheel::ShutterPartial( u8 val ) 
{
	DWORD ret_bytes;
	char txbuf[3];

	double temp = (double)val * 1.44;

	u8 out = 0;

	if( temp < 0.0 ) 
		out = 0;
	else if ( temp > 144.0) 
		out = 144;
	else 
		out = (u8) temp;

	if( USB_ready )
	{
		txbuf[0] = 0xDE;			// ND mode
		txbuf[1] = (char) out;		// value
		txbuf[2] = 0xAA;			// open
		Write(txbuf, 3, &ret_bytes);
		B.nd_setting = val;
		UpdateShutterVal();
	}

	SetTimeouts(100, 100);
}

void COctopusShutterAndWheel::ShutterClose() 
{
	DWORD ret_bytes;
	unsigned char txbuf[25];

	if( USB_ready )
	{
		txbuf[0] = 0xDC;
		Write(txbuf, 1, &ret_bytes);
		txbuf[0] = 0xAC;
		Write(txbuf, 1, &ret_bytes);
		m_ShutterOpen = 2;
		B.nd_setting = 0;
		UpdateShutterVal();
	}

	SetTimeouts(100, 100);
}

void COctopusShutterAndWheel::Filter( u8 filter ) 
{

	unsigned char txbuf[25];
	DWORD ret_bytes;

	if( USB_ready )
	{
		B.filter_wheel = filter;
		m_Filter = filter;
		u8 speed = 6;
		txbuf[0] = speed * 16 + filter;
		Write(txbuf, 1, &ret_bytes);
	}

	SetTimeouts(100, 100);
	UpdateData(FALSE);
}

/*
void COctopusShutterAndWheel::OnRadioFilter0() { Filter ( 0 ); }
void COctopusShutterAndWheel::OnRadioFilter1() { Filter ( 1 ); }
void COctopusShutterAndWheel::OnRadioFilter2() { Filter ( 2 ); }
void COctopusShutterAndWheel::OnRadioFilter3() { Filter ( 3 ); }
void COctopusShutterAndWheel::OnRadioFilter4() { Filter ( 4 ); }
void COctopusShutterAndWheel::OnRadioFilter5() { Filter ( 5 ); }
void COctopusShutterAndWheel::OnRadioFilter6() { Filter ( 6 ); }
void COctopusShutterAndWheel::OnRadioFilter7() { Filter ( 7 ); }
void COctopusShutterAndWheel::OnRadioFilter8() { Filter ( 8 ); }
void COctopusShutterAndWheel::OnRadioFilter9() { Filter ( 9 ); }
*/

bool COctopusShutterAndWheel::ShutterReady( void ) {

	DWORD bytes_in_buf;
	FT_STATUS status;

	status = GetQueueStatus(&bytes_in_buf);

	if( status == FT_OK ) 
		return true;
	else 
		return false;
}

/***************************************************************************/
/***************************************************************************/

FT_STATUS COctopusShutterAndWheel::Read(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytesRead)
{
	return (*m_pRead)(m_ftHandle, lpvBuffer, dwBuffSize, lpdwBytesRead);
}	

FT_STATUS COctopusShutterAndWheel::SetBaudRate(ULONG nRate)
{
	return (*m_pSetBaudRate)(m_ftHandle, nRate);
}

FT_STATUS COctopusShutterAndWheel::SetLatencyTimer(UCHAR nTimer)
{
	return (*m_pSetLatencyTimer)(m_ftHandle, nTimer);
}

FT_STATUS COctopusShutterAndWheel::SetUSBParameters(DWORD nInTransferSize, DWORD nOutTransferSize)
{
	return (*m_pSetUSBParameters)(m_ftHandle, nInTransferSize, nOutTransferSize);
}

FT_STATUS COctopusShutterAndWheel::Write(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytes)
{
	return (*m_pWrite)(m_ftHandle, lpvBuffer, dwBuffSize, lpdwBytes);
}	

FT_STATUS COctopusShutterAndWheel::Open(PVOID pvDevice)
{
	return (*m_pOpen)(pvDevice, &m_ftHandle );
}	

FT_STATUS COctopusShutterAndWheel::OpenEx(PVOID pArg1, DWORD dwFlags)
{
	return (*m_pOpenEx)(pArg1, dwFlags, &m_ftHandle);
}	

FT_STATUS COctopusShutterAndWheel::ListDevices(PVOID pArg1, PVOID pArg2, DWORD dwFlags)
{
	return (*m_pListDevices)(pArg1, pArg2, dwFlags);
}	

FT_STATUS COctopusShutterAndWheel::Close()
{
	return (*m_pClose)(m_ftHandle);
}	

FT_STATUS COctopusShutterAndWheel::ResetDevice()
{
	return (*m_pResetDevice)(m_ftHandle);
}	

FT_STATUS COctopusShutterAndWheel::Purge(ULONG dwMask)
{
	return (*m_pPurge)(m_ftHandle, dwMask);
}	

FT_STATUS COctopusShutterAndWheel::SetTimeouts(ULONG dwReadTimeout, ULONG dwWriteTimeout)
{
	return (*m_pSetTimeouts)(m_ftHandle, dwReadTimeout, dwWriteTimeout);
}	

FT_STATUS COctopusShutterAndWheel::GetQueueStatus(LPDWORD lpdwAmountInRxQueue)
{
	return (*m_pGetQueueStatus)(m_ftHandle, lpdwAmountInRxQueue);
}	


BOOL COctopusShutterAndWheel::OnCommand(WPARAM wParam, LPARAM lParam) {
	int id = LOWORD(wParam);     // Notification code
	if( id == 2 ) return FALSE;  // Trap ESC key
	if( id == 1 ) return FALSE;  // Trap RTN key
	return CDialog::OnCommand(wParam, lParam);
}

void COctopusShutterAndWheel::OnNMCustomdrawExecute( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	int CurPos = m_Slider.GetPos();

	if( CurPos == 0) 
		ShutterClose();
	else if ( CurPos == 100 )
		ShutterOpen();
	else
		ShutterPartial( CurPos );

	*pResult = 0;	
}

void COctopusShutterAndWheel::UpdateShutterVal( void ) {

	CString str;
	str.Format(_T("Shutter (0-100 percent): %d"), B.nd_setting);

	if( IsWindowVisible() ) {
		m_Slider_Setting.SetWindowText( str );
		m_Slider.SetPos( (int) B.nd_setting );
		UpdateData(FALSE);
	}
}

void COctopusShutterAndWheel::OnBnClickedImageJ()
{
	int l_ChkBox = m_ImageJ_CheckBox.GetCheck();

	if (l_ChkBox == 0) 
		open_imagej_on_snap = false;
	else if (l_ChkBox == 1) 
		open_imagej_on_snap = true;
}

void COctopusShutterAndWheel::OnKillFocusGeneral() 
{ 
	UpdateData( true ); 
	CString str;
	str.Format("From shutter: %d\n",m_snap_power_percent);
	TRACE(str);


}
void COctopusShutterAndWheel::OnBnClickedReturnfiltercontrol()
{
	char command[256]; 
	sprintf (command, "cd C:\\Program Files\\Octopus\\Josh's Octopus Files\\MTTTY\\Release & MITTY.exe %d", 7); 
	system(command);
}
