//////////////////////////////////////////////////////////////////////////////////
// OctopusAOTF.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Octopus.h"
#include "OctopusAOTF.h"
#include "OctopusGlobals.h"

#include <math.h>
#include <fstream>
#include <string>
#include <vector>

#include "olerrors.h"
//#include "olmem.h"
#include "oldadefs.h"
//#include "oldaapi.h"

extern COctopusGlobals B;

COctopusAOTF::COctopusAOTF(CWnd* pParent)
	: CDialog(COctopusAOTF::IDD, pParent)
{
	freq           =  10.0;
	volts_CH1      =   4.0;
	volts_CH2      =   8.0;
	B.AOTF_running = false;
	outbuffersize  =   400;

	VERIFY(m_bmp_no.LoadBitmap(IDB_NO));
	VERIFY(m_bmp_yes.LoadBitmap(IDB_YES));

	if( Create(COctopusAOTF::IDD, pParent) )
	{ 
		ShowWindow( SW_SHOW );
		m_status_AOTF.SetBitmap( m_bmp_no );
	}

	Configure();
}

void COctopusAOTF::Configure() 
{
	/*
	hdrvr_9834 = NULL;
   
	olDaInitialize( PTSTR("DT9834(01)"), &hdrvr_9834 );

	if ( hdrvr_9834 == NULL ) 
	{
		AfxMessageBox(_T("DT9834(01): connect to board failed.\n"));
		B.AOTF_loaded = false;
		return;
	}
	
	olDaGetDASS( hdrvr_9834, OLSS_DA, 0, &hdass_9834 );
	olDaSetDataFlow( hdass_9834, OL_DF_CONTINUOUS );
	
	olDaSetChannelListSize( hdass_9834, 4 );
	
	olDaSetChannelListEntry( hdass_9834, 0 , 0);
	olDaSetChannelListEntry( hdass_9834, 1 , 1);
	olDaSetChannelListEntry( hdass_9834, 2 , 2);
	olDaSetChannelListEntry( hdass_9834, 3 , 3);
	
	olDaSetDmaUsage( hdass_9834, 0 );						// don't use DMA channel
	olDaSetChannelType( hdass_9834, OL_CHNT_DIFFERENTIAL );	// differential input
	olDaSetWrapMode( hdass_9834, OL_WRP_SINGLE );
	olDaSetClockSource( hdass_9834, OL_CLK_INTERNAL );		// select internal clock source
	olDaSetEncoding( hdass_9834, OL_ENC_BINARY );			// binary encoding
	olDaSetRange( hdass_9834, 10.0, -10.0 );				// 10 to -10 V
	olDaSetTrigger( hdass_9834, OL_TRG_SOFT );

	B.AOTF_loaded = true;
	*/
}

COctopusAOTF::~COctopusAOTF()
{	

	B.AOTF_loaded = false;

	HBUF hBuf = NULL;
	olDaAbort( hdass_9834 );
	olDaFlushBuffers( hdass_9834 ); 
	
	do {
		olDaGetBuffer( hdass_9834, &hBuf );           // get the buffer from the done queue
		if( hBuf != NULL ) olDmFreeBuffer( hBuf );    // and free it
	} while( hBuf != NULL );                          // until the done queue is empty	

	olDaReleaseDASS( hdass_9834 );					  // release the subsystem and the board
	olDaTerminate( hdrvr_9834 );
}

/*****************************************************************************
****************** dialog box ************************************************
*****************************************************************************/

void COctopusAOTF::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,	IDC_AOTF_ONOFF_BMP,		  m_status_AOTF);
	DDX_Text(pDX,       IDC_AOTF_FREQ,	          freq);
	DDV_MinMaxDouble(pDX, freq, 0.1, 100.0);
	DDX_Text(pDX,       IDC_AOTF_CH_1,	          volts_CH1);
	DDV_MinMaxDouble(pDX, volts_CH1, 0.0, 10.0);
	DDX_Text(pDX,       IDC_AOTF_CH_2,	          volts_CH2);
	DDV_MinMaxDouble(pDX, volts_CH2, 0.0, 10.0);
}

void COctopusAOTF::OnKillfocus() { UpdateData( true ); }

BEGIN_MESSAGE_MAP(COctopusAOTF, CDialog)
	ON_BN_CLICKED( IDC_AOTF_STARTSTOP,	StartStop)
	ON_EN_KILLFOCUS(IDC_AOTF_FREQ,		OnKillfocus)
	ON_EN_KILLFOCUS(IDC_AOTF_CH_1,		OnKillfocus)
	ON_EN_KILLFOCUS(IDC_AOTF_CH_2,		OnKillfocus)
END_MESSAGE_MAP()

/*****************************************************************************
****************** main functions ********************************************
*****************************************************************************/

void COctopusAOTF::StartStop( void ) 
{
	if( B.AOTF_running ) 
		Stop();
	else
		Start();
}

void COctopusAOTF::Start( void ) 
{ 
	olDaSetClockFrequency( hdass_9834, 0.25 * freq * outbuffersize );

	olDmAllocBuffer( 0, outbuffersize, &hBuf_DAC );

	if( hBuf_DAC == NULL ) 
	{
		AfxMessageBox(_T("Error Allocating buffer."));
		return;
	}
	
	GenerateSignal();
	
	olDaPutBuffer( hdass_9834, hBuf_DAC );	
	
	olDaConfig( hdass_9834 );
	
	olDaStart( hdass_9834 );

	B.AOTF_running = true;

	m_status_AOTF.SetBitmap( m_bmp_yes );
}

void COctopusAOTF::Stop( void ) 
{
	HBUF hBuf;
	
	olDaAbort( hdass_9834 );
	
	olDaFlushBuffers( hdass_9834 );                        // make sure flush all buffers to done queue
		
	do {
		olDaGetBuffer( hdass_9834, &hBuf );                // get the buffer from the done queue
		if( hBuf != NULL ) {
			olDmFreeBuffer( hBuf );
		}
	} while( hBuf != NULL );

	B.AOTF_running = false;

	m_status_AOTF.SetBitmap( m_bmp_no );

	Sleep( 50 );
}

/*****************************************************************************
****************** pattern generation ****************************************
*****************************************************************************/

void COctopusAOTF::GenerateSignal( void ) 
{  
	u16*      ptr;
	u16     i = 0;

	olDmGetBufferPtr( hBuf_DAC, (LPVOID FAR*)&ptr );

	for( i = 0; i < outbuffersize / 4; i++ ) 
	{		
		if ( i < 50 ) 
		{
			//on
			*(ptr++) = ConvertVoltToDAC( volts_CH1 ); //CH_1
			*(ptr++) = ConvertVoltToDAC( volts_CH2 ); //CH_2
			*(ptr++) = ConvertVoltToDAC(      10.0 ); //blanking channel?
			*(ptr++) = ConvertVoltToDAC(       4.5 ); //camera start exposure
		} 
		else
		{
			//off
			*(ptr++) = ConvertVoltToDAC(       0.0 ); //CH_1
			*(ptr++) = ConvertVoltToDAC(       0.0 ); //CH_2
			*(ptr++) = ConvertVoltToDAC(      10.0 ); //blanking channel?
			*(ptr++) = ConvertVoltToDAC(       0.0 ); //camera off
		}
	}
}

u16 COctopusAOTF::ConvertVoltToDAC( double volt ) 
{  
	u32    val = 0;
	u16 retval = 0;

	if ( volt < 0.0 ) 
		val = 32768;
	else if ( volt > 10.0)
		val = 65535;
	else
		val = u32( volt * 3276.8 ) + 32768;

	//final sanity check
    if ( val < 32768 ) 
		retval = 32768;
	else if ( val > 65535)
		retval = 65535;
	else
		retval = u16(val);

	return retval;
}