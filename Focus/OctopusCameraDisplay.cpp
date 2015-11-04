#include "stdafx.h"
#include "Octopus.h"
#include "OctopusCameraDisplay.h"
#include "OctopusGlobals.h"
#include "OctopusCameraDlg.h"
#include "OctopusClock.h"
#include "OctopusLM.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <CSLScope.h>

#define COL_BLUE 0x00FF0000
#define COL_RED  0x000000FF

extern COctopusGlobals	  B;
extern COctopusCamera*    glob_m_pCamera;
extern COctopusGoodClock* glob_m_pGoodClock;

COctopusPictureDisplay::COctopusPictureDisplay(CWnd* pParent, u16 x, u16 y)
	: CDialog(COctopusPictureDisplay::IDD, pParent) 
{
	Width			 = x; 
	Height			 = y;

	Length           = x * y;

	roi_x_begin      = 0;
	roi_x_end        = 0;
	roi_y_begin      = 0;
	roi_y_end        = 0;
	roi_Width        = 0;
	roi_Height       = 0;
	roi_bead         = 20;

	plottime         = 0;
	FullFrame        = B.CCD_x_phys_e * B.CCD_y_phys_e;

	g_mp		     = 0.05;
	g_max		     = 0;
	g_min		     = 65535;
	g_mean           = 0.0;
	
	time_dx			 = 0.0;
	time_old		 = 0.0;
	camera_freq      = 1.0;
	display_freq     = 1.0;
	
	pictures_written = 0;

	pPic_zoom        = NULL;
	pPic_bead        = NULL;
	pPic_main        = NULL;

	pLM              = NULL;

	MemoryFramesOld  = 1;
	g_NumFT          = 1;

	//allocate some memory
	first_picture		= new u16[ Length ];
	first_picture_flip	= new u16[ Length ];
	tp					= NULL;
	tb					= new u16[ roi_bead * roi_bead ];
	
	//may need to reallocate for very fast runs
	frames_to_save   = new u16[ Length * MemoryFramesOld ];

	pFileData        = NULL;
	pFileHead		 = NULL;

	Marking          = false;

	picture_size     = 700;
	zoom             = 1.0;	
	control_height   = 50;
	save_halflength  = 60;

	//focus
	B.focus_score = 0.0;
	B.focus_beadX = 0.0;
	B.focus_beadY = 0.0;
	B.Focus_ROI_Set = false;

	pic_x1 = 0;
	pic_y1 = 0;
	pic_x2 = 0;
	pic_y2 = 0;

	pos_y_old = 0;
	pos_x_old = 0;
}

BOOL COctopusPictureDisplay::OnInitDialog() 
{
	CDialog::OnInitDialog();

	VCL_InitControls( m_hWnd );
	
	Scope.Open( m_Scope.m_hWnd );
	Scope.Channels.Add(3);
	Scope.Channels[ 0 ].Name = "X Position";
	Scope.Channels[ 1 ].Name = "Y Position";
	Scope.Channels[ 2 ].Name = "FWHM";
	Scope.Channels[ 3 ].Name = "Focus";
	
	return TRUE;
}

BEGIN_MESSAGE_MAP(COctopusPictureDisplay, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void COctopusPictureDisplay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIME, m_info);
	DDX_Control(pDX, IDC_CAM_SCOPE, m_Scope);
}

COctopusPictureDisplay::~COctopusPictureDisplay() 
{
	Bitmap_main.DeleteObject();

	delete [] first_picture;		first_picture			= NULL;
	delete [] frames_to_save;		frames_to_save			= NULL;
	delete [] first_picture_flip;	first_picture_flip		= NULL;
	delete [] pPic_main;			pPic_main				= NULL;
	delete [] tb;					tb						= NULL;		   

	if ( B.Focus_ROI_Set ) 
	{
		Bitmap_zoom.DeleteObject();
		delete [] pPic_zoom;		   
		pPic_zoom = NULL;

		Bitmap_bead.DeleteObject();
		delete [] pPic_bead;		   
		pPic_bead = NULL;

		delete [] tp;		   
		tp = NULL;

		if (pLM != NULL) {
			delete pLM;
			pLM = NULL;
		}
	}
	if ( pFileData != NULL ) 
	{
		fclose( pFileData );
		pFileData = NULL;
	}
	if ( pFileHead != NULL ) 
	{
		fclose( pFileHead );
		pFileHead = NULL;
	}
}

void COctopusPictureDisplay::Create_Bitmap() 
{

	MoveWindow( 848, 2, picture_size + 120, picture_size + control_height + 390 );
	
	if ( Height > Width )
		zoom = (double)picture_size / (double)Height;
	else 
		zoom = (double)picture_size / (double)Width;
 
	screen_wp = (u16)( (double)Width  * zoom );
	screen_hp = (u16)( (double)Height * zoom );
	
	//bitmap placement control
	pic_x1 = 6;
	pic_y1 = 6 + control_height;
	pic_x2 = pic_x1 + screen_wp;
	pic_y2 = pic_y1 + screen_hp;

	CClientDC aDC( this );
	CDC* pDC = &aDC;
	
	//main bitmap
	BITMAP bm;
	Bitmap_main.CreateCompatibleBitmap( pDC, Width, Height );	
	Bitmap_main.GetObject( sizeof(BITMAP), &bm );
	pPic_main = new uC [ bm.bmHeight * bm.bmWidthBytes ];

	int i;
	
	for( i = 0; i < 1000; i ++ ) 
	{
		DataArrayX[ i ] = 0.0;
		DataArrayY[ i ] = 0.0;
		DataArrayW[ i ] = 0.0;
		DataArrayF[ i ] = 0.0;
	}
}

void COctopusPictureDisplay::Draw_Bitmap( void ) 
{
	CClientDC aDC( this );
	CDC* pDC = &aDC;
	CDC hMemDC;
	hMemDC.CreateCompatibleDC( pDC );

	BITMAP bm;
	Bitmap_main.GetBitmap( &bm );
	hMemDC.SelectObject( &Bitmap_main );
	hMemDC.SetMapMode( MM_ANISOTROPIC );
	hMemDC.SetViewportOrg(bm.bmWidth - 1, 0);
	hMemDC.SetViewportExt(-1, 1);

    pDC->StretchBlt( pic_x1, pic_y1, \
		screen_wp, screen_hp, \
		&hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

	if ( B.SetFocus ) 
	{	
		//this tells the user that they need to select an ROI		
		CPen pen(PS_SOLID,5,COL_RED);
		pDC->SelectObject(&pen);
		pDC->MoveTo(       6 + 3, 6 + control_height + 3       ); 
		pDC->LineTo( 700 + 6 - 3, 6 + control_height + 3       ); 
		pDC->LineTo( 700 + 6 - 3, 6 + control_height + 700 - 3 ); 
		pDC->LineTo(       6 + 3, 6 + control_height + 700 - 3 ); 
		pDC->LineTo(       6 + 3, 6 + control_height + 3       );
	}

	if ( B.Focus_ROI_Set ) 
	{	
		BITMAP bmz;
		Bitmap_zoom.GetBitmap( &bmz );
		hMemDC.SelectObject( &Bitmap_zoom );
		hMemDC.SetMapMode( MM_ANISOTROPIC );
		hMemDC.SetViewportOrg(bmz.bmWidth - 1, 0);
		hMemDC.SetViewportExt(-1, 1);
		pDC->StretchBlt( pic_x2, pic_y1, 100, 100, \
			&hMemDC, 0, 0, bmz.bmWidth, bmz.bmHeight, SRCCOPY );	

		Bitmap_bead.GetBitmap( &bmz );
		hMemDC.SelectObject( &Bitmap_bead );
		hMemDC.SetMapMode( MM_ANISOTROPIC );
		hMemDC.SetViewportOrg(bmz.bmWidth - 1, 0);
		hMemDC.SetViewportExt(-1, 1);
		pDC->StretchBlt( pic_x2, pic_y1 + 100, 100, 100,  \
			&hMemDC, 0, 0, bmz.bmWidth, bmz.bmHeight, SRCCOPY );	
	}

}

void COctopusPictureDisplay::Update_Bitmap( u16 *pict, u16 FramesTransferred ) 
{

	u16 temp   = 0;
	u32 c	   = 0;
	u32 b      = 0;
	uC  val    = 0;
	u32 x      = 0;
	u32 y      = 0;
	
	g_mean     = 0.0;
	g_min      = 65535;
	g_max      = 0;
    g_NumFT    = FramesTransferred;

	if ( g_NumFT == MemoryFramesOld )
	{
		//all is well - stick with old memory
	} 
	else
	{
		//yipes need to change memory size
		delete [] frames_to_save;     
		frames_to_save  = NULL;
		frames_to_save  = new u16[ Length * FramesTransferred ];
		MemoryFramesOld = g_NumFT;
	}

	//there may be more frames in this vector, 
	//but let's only look at the first one. 
	for ( c = 0; c < Length; c++ ) 
	{
		temp = *( pict + c );
		*( first_picture + c ) = temp;
		if ( temp < g_min ) 
			g_min = temp; 
		else if ( temp > g_max ) 
			g_max = temp;	
		g_mean = g_mean + temp;
	}

	g_mean = g_mean / Length;

	// these are the data to be saved - there may be multiple 
	// frames in one vector, hence c < Length * g_NumFT

	for ( c = 0; c < Length * g_NumFT; c++ ) 
	{
		*( frames_to_save + c ) = *( pict + c );
	}

    //if CONV amplifier, need to flip image...	
	if ( B.Ampl_Conv ) 
	{
		u32 xf = 0;

		for ( y = 0; y < Height; y++ ) 
		{
			for ( x = 0; x < Width; x++ ) 
			{
				xf = Width - 1 - x;
				*( first_picture_flip + ( y * Width ) + xf ) = \
				*( first_picture      + ( y * Width ) + x  );
			}
		}
	} 

	u16 *good_pic = first_picture;

	if( B.Ampl_Conv )
		good_pic = first_picture_flip;

	if ( B.automatic_gain ) 
	{
		g_mp = 255.0 / double( g_max - g_min ); 
	} 
	else 
	{
		g_mp = B.manual_gain;
	}

	for ( c=0, b=0; c < Length; c++ ) 
	{	
		val = (uC)((double)( *( good_pic + c ) - g_min ) * g_mp);
		*(pPic_main + b++) = val;
		*(pPic_main + b++) = val;
		*(pPic_main + b++) = val;
		*(pPic_main + b++) = val;
	}

	B.time_now = (double)glob_m_pGoodClock->End();
	time_dx    = B.time_now - time_old;
	time_old   = B.time_now;

	if( time_dx > 0 ) 
	{
		camera_freq  = double(g_NumFT) / time_dx;
	    display_freq = 1.0             / time_dx;
	}

	BITMAP bm;
	Bitmap_main.GetBitmap( &bm );	
	Bitmap_main.SetBitmapBits( bm.bmWidthBytes * bm.bmHeight, pPic_main );

	if ( B.Focus_ROI_Set && (Length == FullFrame) )
	{	
		//get min, max, mean
		u16 min            = 65535;
		u16 max            = 0;
		double totalint    = 0;
		u16 temp           = 0;
		b = 0;
		
		for ( y = roi_y_begin; y < roi_y_end; y++ ) 
		{
			for ( x = roi_x_begin; x < roi_x_end; x++ ) 
			{
				*(tp + b++) = *( good_pic + ( y * Width ) + x );
			}
		}

		COctopusCentroid result;

		if ( pLM != NULL )
			result = pLM->GetCenterFast( tp );
		
		min               = result.min;
		max               = result.max;
		totalint          = result.totalint;
		g_mean            = result.mean;
		B.focus_beadX     = result.X;
		B.focus_beadY     = result.Y;
		B.focus_min       = result.min;
		B.focus_max       = result.max;

		double range = double(max - min);
		
		B.focus_score = range / 100.0;

		// let's scale the minipicture to 8 bits
		double mp = 255.0 / range; 

		for ( c = 0, b = 0; b < roi_Width * roi_Width; b++ ) 
		{	
			temp = *(tp + b);
			val  = uC( double(temp - min) * mp );
			
			if( temp < result.cut_high )
			{
				*(pPic_zoom + c++) = val;
				*(pPic_zoom + c++) = val;
				*(pPic_zoom + c++) = val;
				*(pPic_zoom + c++) = val;
			} 
			else
			{
				*(pPic_zoom + c++) = 0;
				*(pPic_zoom + c++) = 255;
				*(pPic_zoom + c++) = 0;
				*(pPic_zoom + c++) = 0;
			}
		}

		BITMAP bmz;
		Bitmap_zoom.GetBitmap( &bmz );	
		Bitmap_zoom.SetBitmapBits( bmz.bmWidthBytes * bmz.bmHeight, pPic_zoom );

		//bead tracking window

		b = 0;
		
		u16 offset = roi_bead / 2;

		u16 xm = (roi_x_end - roi_x_begin) / 2;
		u16 ym = (roi_y_end - roi_y_begin) / 2;

		u16 xbb = roi_x_begin + xm - offset;
		u16 ybb = roi_y_begin + ym - offset;

		u16 xbe = xbb + roi_bead;
		u16 ybe = ybb + roi_bead;

		for ( y = ybb; y < ybe; y++ ) 
		{
			for ( x = xbb; x < xbe; x++ ) 
			{
				*(tb + b++) = *( good_pic + ( y * Width ) + x );
			}
		}

		if ( pLM != NULL )
			result = pLM->GetCenterGood( tb );	
	
		B.focus_beadX_LM  = result.LM_X;
		B.focus_beadY_LM  = result.LM_Y;
		B.focus_bead_fwhm = result.LM_fwhm;
		range             = double(result.max - result.min);
		mp                = 255.0 / range; 

		PlotGraph();
		
		for ( c = 0,b = 0; b < roi_bead * roi_bead; b++ ) 
		{	

			val  = uC( double(*(tb + b) - result.min) * mp );
		
			*(pPic_bead + c++) = val;
			*(pPic_bead + c++) = val;
			*(pPic_bead + c++) = val;
			*(pPic_bead + c++) = val;
		}

		Bitmap_bead.GetBitmap( &bmz );	
		Bitmap_bead.SetBitmapBits( bmz.bmWidthBytes * bmz.bmHeight, pPic_bead );

	}

	Draw_Bitmap();

	UpdateTitle();

	if( B.savetofile ) WritePic();
}      

bool COctopusPictureDisplay::Open_A_File( void ) 
{
	CString temp;
	B.files_written++;

	temp.Format(_T("%ld.dat"), B.files_written );
	filename = B.pathname + temp;	
	pFileData = _wfopen ( filename , _T("wb"));
	
	temp.Format(_T("%ld.dth"), B.files_written );
	filename = B.pathname + temp;
	pFileHead = _wfopen ( filename , _T("wt"));

	if ( pFileData != NULL && pFileHead != NULL ) 
		return true;
	else 
		return false;
}

void COctopusPictureDisplay::Close_The_File( void )
{
	if( pFileHead == NULL || pFileData == NULL ) return;

	pictures_written = 0;

	fclose( pFileData );
	pFileData = NULL;

	fclose( pFileHead );
	pFileHead = NULL;
}

void COctopusPictureDisplay::WritePic( void ) 
{

	if( pictures_written >= B.pics_per_file ) 
	{
		Close_The_File();
		pictures_written = 0;
	}

	// if the file is closed - due to any number of reasons 
	// => open a new one
	
	if( pFileHead == NULL || pFileData == NULL ) 
	{ 
		if ( !Open_A_File() ) return; 
	}

	u16 h = Height;
	u16 w = Width;
	
	CString str;

	for ( u16 n = 0; n < g_NumFT; n++)
	{
		str.Format(_T("N:%ld H:%d W:%d T:%.3f Filter:%d ND:%d X:%.3f Y:%.3f Z:%.3f Ex1:%.3f\n"),
		        pictures_written, h, w, B.time_now, B.filter_wheel, B.nd_setting, \
				B.position_x, B.position_y, B.position_z, \
				B.ADC_1 );
		
		pictures_written++;

		fwprintf( pFileHead, str );	
	}
	fflush( pFileHead );

	fwrite( frames_to_save, sizeof(u16), w * h * g_NumFT, pFileData );
	fflush( pFileData );

	B.expt_time = B.time_now - B.savetime;
	B.expt_frame++;
}

void COctopusPictureDisplay::UpdateTitle( void ) 
{
	CString temp;
	
	temp.Format(_T("Min:%d Max:%d Cutoff:%.1f Mean:%.1f Focus:%.2f Cam_Freq(Hz):%.1f PicsTransfer:%d\n"), \
		         g_min, g_max, g_mp, g_mean, B.focus_score, camera_freq, g_NumFT);
	temp.AppendFormat(_T("Frames in current file:%d  Total saved frames:%d Time(s):%.1f SaveDuration(s):%.1f \n"), \
		         pictures_written, B.expt_frame, B.time_now, B.expt_time ); 
	temp.AppendFormat(_T("X:%.1f Y:%.1f Z:%.1f FX:%.1f FXLM:%.2f FY:%.1f FYLM:%.2f LMW:%.2f Fmin:%.1f Fmax:%.1f "), \
		B.position_x, B.position_y, B.position_z, \
		B.focus_beadX, B.focus_beadX_LM, \
		B.focus_beadY, B.focus_beadY_LM, \
		B.focus_bead_fwhm, \
		B.focus_min, B.focus_max);

	m_info.SetWindowText( temp );
}

/**************************************************************************
********************* MOUSE ***********************************************
**************************************************************************/

void COctopusPictureDisplay::ValidateMarkersAndSetROI( void ) 
{	
	//first, correct offset
	Marker1.x = Marker1.x - 6;
	Marker2.x = Marker2.x - 6;
	Marker1.y = Marker1.y - 6 - control_height;
	Marker2.y = Marker2.y - 6 - control_height;

	//second, correct scale
	Marker1.x = (u32)( (double)Marker1.x / zoom );
	Marker2.x = (u32)( (double)Marker2.x / zoom );
	Marker1.y = (u32)( (double)Marker1.y / zoom );
	Marker2.y = (u32)( (double)Marker2.y / zoom );

	//we are now in true camera pixels

	//correct for bottom to top dragging etc.
	if( Marker2.x < Marker1.x ) 
	{
		u32 temp = Marker1.x;
		Marker1.x = Marker2.x;	
		Marker2.x = temp;	
	} 

	if( Marker2.y < Marker1.y ) 
	{
		u32 temp = Marker1.y;
		Marker1.y = Marker2.y;	
		Marker2.y = temp;	
	} 

	u32 temp = Marker1.x;

	Marker1.x = B.ROI_actual.x2 - Marker2.x;
	Marker2.x = B.ROI_actual.x2 - temp;

	Marker1.y += B.ROI_actual.y1;
	Marker2.y += B.ROI_actual.y1;

	ROI.x1 = Marker1.x;
	ROI.x2 = Marker2.x;
	ROI.y1 = Marker1.y;
	ROI.y2 = Marker2.y;

	if( ( ROI.x2 - ROI.x1 < 14 ) ||  ( ROI.y2 - ROI.y1 < 14 ) ) 
	{
		ROI.x1 = 1;
		ROI.y1 = 1;
		ROI.x2 = ROI.x1 + 14;
		ROI.y2 = ROI.y1 + 14;
		AfxMessageBox(_T("ROI too small - making it a bit bigger."));
	} 

	if ( B.SetFocus )
	{
		B.SetFocus  = false;

		roi_x_begin = ROI.x1;
		roi_x_end   = ROI.x2;
		roi_y_begin = ROI.y1;
		roi_y_end   = ROI.y2;

		roi_Width	= roi_x_end - roi_x_begin;
		roi_Height	= roi_y_end - roi_y_begin;

		if ( roi_Width == roi_Height )
		{
			//do nothing
		}
		else if ( roi_Width > roi_Height ) 
		{
			int remove = roi_Width - roi_Height;
			roi_x_end  = roi_x_end - remove;
			roi_Width  = roi_x_end - roi_x_begin;
		}
		else if ( roi_Height > roi_Width )
		{
			int remove = roi_Height - roi_Width;
			roi_y_end  = roi_y_end - remove;
			roi_Height = roi_y_end - roi_y_begin;
		}

		if ( roi_Width != roi_Height ) 
		{
			AfxMessageBox(_T("Something went wrong in the focus ROI code"));
		}

		CClientDC aDC( this );
		CDC* pDC = &aDC;
		
		if ( B.Focus_ROI_Set ) //yep, this is not the first time - need to clear out old stuff
		{

			Bitmap_zoom.DeleteObject();
			delete [] pPic_zoom;		   
			pPic_zoom = NULL;
			
			if (pLM != NULL)
			{
				delete pLM;
				pLM = NULL;
			}
			if (tp != NULL)
			{
				delete [] tp;
				tp = NULL;
			}
		}

		//zoom bitmap
		BITMAP bmz;
		Bitmap_zoom.CreateCompatibleBitmap( pDC, roi_Width, roi_Height );	
		Bitmap_zoom.GetObject( sizeof(BITMAP), &bmz );

		pPic_zoom = new uC [ bmz.bmHeight * bmz.bmWidthBytes ];
		tp        = new u16[ roi_Width * roi_Height ];
		pLM       = new COctopusLM( roi_Width );
		
		Bitmap_bead.CreateCompatibleBitmap( pDC, roi_bead, roi_bead );	
		Bitmap_bead.GetObject( sizeof(BITMAP), &bmz );
		
		pPic_bead = new uC [ bmz.bmHeight * bmz.bmWidthBytes ];

		B.Focus_ROI_Set = true;

	} 
	else 
	{
		B.ROI_target  = ROI;
		B.ROI_changed = true;
	}
}

void COctopusPictureDisplay::MouseMove( CPoint point ) 
{
	
	if( point.x < pic_x1 ) return;
	if( point.x > pic_x2 ) return;
	if( point.y < pic_y1 ) return;
	if( point.y > pic_y2 ) return;
	
	Draw_Bitmap();

	CClientDC aDC( this );
	CDC* pDC = &aDC;
	
	if ( B.SetFocus )
	{
		CPen pen(PS_SOLID,2,COL_RED);
		pDC->SelectObject(&pen);
		pDC->MoveTo( Marker1.x , Marker1.y); 
		pDC->LineTo( point.x   , Marker1.y); 
		pDC->LineTo( point.x   , point.y  );
		pDC->LineTo( Marker1.x , point.y  );
		pDC->LineTo( Marker1.x , Marker1.y);
	} 
	else 
	{
		CPen pen(PS_SOLID,2,COL_BLUE);
		pDC->SelectObject(&pen);
		pDC->MoveTo( Marker1.x , Marker1.y); 
		pDC->LineTo( point.x   , Marker1.y); 
		pDC->LineTo( point.x   , point.y  );
		pDC->LineTo( Marker1.x , point.y  );
		pDC->LineTo( Marker1.x , Marker1.y);
	}
}

void COctopusPictureDisplay::LeftButtonDown( CPoint point ) 
{	
	if( point.x < pic_x1 ) return;
	if( point.x > pic_x2 ) return;
	if( point.y < pic_y1 ) return;
	if( point.y > pic_y2 ) return;
	
	Marker1 = point;
	Marking = true;
}

void COctopusPictureDisplay::LeftButtonUp( CPoint point ) 
{
	if( point.x < pic_x1 ) return;
	if( point.x > pic_x2 ) return;
	if( point.y < pic_y1 ) return;
	if( point.y > pic_y2 ) return;
	
	Marker2 = point;	
	Marking = false;

	ValidateMarkersAndSetROI();
}

void COctopusPictureDisplay::OnMouseMove( UINT nFlags, CPoint point ) 
{
	if ( B.Camera_Thread_running ) return;
	if ( Marking ) MouseMove( point );
}

void COctopusPictureDisplay::OnLButtonUp( UINT nFlags, CPoint point ) 
{
	if ( B.Camera_Thread_running ) return;
	LeftButtonUp( point );
}

void COctopusPictureDisplay::OnLButtonDown( UINT nFlags, CPoint point ) 
{
	if ( B.Camera_Thread_running ) return;
	LeftButtonDown( point );
}

void COctopusPictureDisplay::OnRButtonDown( UINT nFlags, CPoint point ) 
{	
	if ( B.Camera_Thread_running ) return;
	glob_m_pCamera->SetROI_To_Default();
}

BOOL COctopusPictureDisplay::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);    // Notification code
	if( id == 1 ) return FALSE; // Trap RTN key
	if( id == 2 ) return FALSE; // Trap ESC key
    return CDialog::OnCommand(wParam, lParam);
}

void COctopusPictureDisplay::PlotGraph( void )
{

	if ( plottime == 0 )
	{
		plotx0 = B.focus_beadX_LM;
		ploty0 = B.focus_beadY_LM;
		plotw0 = B.focus_bead_fwhm;
		plotf0 = B.focus_score;
	}

	double x = 0.1*(B.focus_beadX_LM - plotx0) + 0.9*pos_x_old;
	double y = 0.1*(B.focus_beadY_LM - ploty0) + 0.9*pos_y_old;

	DataArrayX[ plottime ] = x;
	DataArrayY[ plottime ] = y;
	DataArrayW[ plottime ] = B.focus_bead_fwhm - plotw0;
	DataArrayF[ plottime ] = B.focus_score     - plotf0;

	pos_x_old = x;
	pos_y_old = y;

	plottime++;

	if (plottime >= 1000 ) plottime = 0;

	Scope.Channels[ 0 ].Data.SetYData( DataArrayX, 1000 );
	Scope.Channels[ 1 ].Data.SetYData( DataArrayY, 1000 );
	Scope.Channels[ 2 ].Data.SetYData( DataArrayW, 1000 );
	Scope.Channels[ 3 ].Data.SetYData( DataArrayF, 1000 );
		
}

