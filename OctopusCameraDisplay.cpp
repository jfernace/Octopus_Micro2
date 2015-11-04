#include "stdafx.h"
#include "Octopus.h"
#include "OctopusCameraDisplay.h"
#include "OctopusGlobals.h"
#include "OctopusCameraDlg.h"
#include "OctopusClock.h"
//#include "atmcd32d.h" // Andor Camera library
#include "levmar.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <iostream>
#include <fstream>
#include <string>


#define COL_BLUE 0x00FF0000
#define COL_RED  0x000000FF

#define DBL_RAND_MAX (double)(RAND_MAX)
#ifdef _MSC_VER // MSVC
#include <process.h>
#define GETPID  _getpid
#elif defined(__GNUC__) // GCC
#include <sys/types.h>
#include <unistd.h>
#define GETPID  getpid
#else
#warning Do not know the name of the function returning the process id for your OS/compiler combination
#define GETPID  0
#endif /* _MSC_VER */
#ifdef REPEATABLE_RANDOM
#define INIT_RANDOM(seed) srandom(seed)
#else
#define INIT_RANDOM(seed) srand((int)GETPID()) // seed unused
#endif

extern COctopusGlobals	  B;
extern COctopusCamera*    glob_m_pCamera;
extern COctopusGoodClock* glob_m_pGoodClock;

void __cdecl Gaussian_2D(double *p, double *x, int m, int n, void *data);
void __cdecl expfunc(double *p, double *x, int m, int n, void *data);

COctopusPictureDisplay::COctopusPictureDisplay(CWnd* pParent, u16 x, u16 y)
	: CDialog(COctopusPictureDisplay::IDD, pParent) 
{
	Width			 = x; 
	Height			 = y;
	Length           = x * y;
	FullFrame        = B.CCD_x_phys_e * B.CCD_y_phys_e;
	g_mp		     = 0.05;
	g_max		     = 0.0;
	g_min		     = 65535.0;
	g_mean           = 0.0;
	time_dx			 = 0.0;
	time_old		 = 0.0;
	camera_freq      = 1.0;
	display_freq     = 1.0;
	pictures_written = 0;
	pPic_main        = NULL;
	MemoryFramesOld  = 1;
	g_NumFT          = 1;

	//allocate some memory
	first_picture      = new u16[ Length ];
	picture_SR         = new double[ Length ];
	picture_1		   = new u16[ Length ];
	picture_2		   = new u16[ Length ];
	picture_3		   = new u16[ Length ];
	first_picture_flip = new u16[ Length ];
	
	//may need to reallocate for very fast runs
	frames_to_save   = new u16[ Length * MemoryFramesOld ];

	pFileData        = NULL;
	pFileHead		 = NULL;

	Marking          = false;

	picture_size     = 700;
	zoom             = 1.0;	
	control_height   = 60;
	save_halflength  = 60;

	//focus
	B.focus_score = 0.0;
	B.CurrFocusScore = 0.0;


	pic_x1 = 0;
	pic_y1 = 0;
	pic_x2 = 0;
	pic_y2 = 0;

}

BOOL COctopusPictureDisplay::OnInitDialog() 
{
	CDialog::OnInitDialog();
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
}

COctopusPictureDisplay::~COctopusPictureDisplay() 
{
	
	Bitmap_main.DeleteObject();

	delete [] first_picture;		first_picture		= NULL;
	delete [] frames_to_save;		frames_to_save		= NULL;
	delete [] first_picture_flip;	first_picture_flip	= NULL;
	delete [] pPic_main;			pPic_main			= NULL;
	delete [] picture_SR;		    picture_SR          = NULL;
	delete [] picture_1;		    picture_1			= NULL;
	delete [] picture_2;		    picture_2			= NULL;
	delete [] picture_3;		    picture_3			= NULL;

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
	//AndorCamera//FreeInternalMemory ();
}

void COctopusPictureDisplay::Create_Bitmap() 
{
	MoveWindow( 851, 2, picture_size + 20, picture_size + control_height + 45 );
	
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
	//hMemDC.SetViewportOrg(bm.bmWidth - 1, 0);
	hMemDC.SetViewportOrg(0, 0);
	//hMemDC.SetViewportExt(-1, 1);
	hMemDC.SetViewportExt(1, 1);

    pDC->StretchBlt( pic_x1, pic_y1, \
		screen_wp, screen_hp, \
		&hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

	double display_x1 = B.ROI_target.x1 * zoom + 6;
	double display_x2 = B.ROI_target.x2 * zoom + 6;
	double display_y1 = B.ROI_target.y1 * zoom + 6 + control_height;
	double display_y2 = B.ROI_target.y2 * zoom + 6 + control_height;

	CPen pen(PS_SOLID,2,COL_BLUE);
	pDC->SelectObject(&pen);
	pDC->MoveTo( display_x1 , display_y1); 
	pDC->LineTo( display_x2   , display_y1); 
	pDC->LineTo( display_x2   , display_y2  );
	pDC->LineTo( display_x1 , display_y2  );
	pDC->LineTo( display_x1 , display_y1);


	CPen bead_pen(PS_SOLID,2,COL_BLUE);
	pDC->SelectObject(&bead_pen);
	pDC->MoveTo( result_X - 15 , result_Y - 15); 
	pDC->LineTo( result_X - 15   , result_Y + 15); 
	pDC->LineTo( result_X + 15   , result_Y + 15  );
	pDC->LineTo( result_X + 15 , result_Y - 15  );
	pDC->LineTo( result_X - 15 , result_Y - 15);

	//for (int i = 6; i < 706; i += 100)
	//{
	//	pDC->MoveTo( i , 56); 
	//	pDC->LineTo( i , 756); 
	//}

	//for (int i = 56; i < 756; i += 100)
	//{
	//	pDC->MoveTo( 6 , i); 
	//	pDC->LineTo( 706 , i); 
	//}
}

void COctopusPictureDisplay::Update_Bitmap( u16 *pict, u16 FramesTransferred ) 
{

	u16 temp   = 0;
	u32 c	   = 0;
	u32 b      = 0;
	uC  val    = 0;
	u32 x      = 0;
	u32 y      = 0;
	double rj  = 0.0;
	g_mean     = 0.0;
	B.g_min      = 65535.0;
	B.g_max      = 0.0;
    g_NumFT    = FramesTransferred;
	double diff  = 0;
	double focus = 0;
	double totalint = 0;

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

	BasicAnalysis(pict);

	// these are the data to be saved - there may be multiple 
	// frames in one vector, hence c < Length * g_NumFT
	for ( c = 0; c < Length * g_NumFT; c++ )  // Length is Height*Width
	{
		*( frames_to_save + c ) = *( pict + c );
	}

	u16 *good_pic = pict;
    
	//
	//if( B.Ampl_Conv )
	//	good_pic = first_picture_flip;
    //
	
	if ( B.automatic_gain ) 
	{
		g_mp = 255.0 / ( B.g_max - B.g_min ); 
	} 
	else 
	{
		g_mp = B.manual_gain;
	}

	for ( c=0; c < Length; c++ ) 
	{
		if( B.mask_now == 1 )
			*( picture_1 + c ) = *( good_pic + c );
		else if( B.mask_now == 2 ) 
			*( picture_2 + c ) = *( good_pic + c );
		else if( B.mask_now == 3 )
			*( picture_3 + c ) = *( good_pic + c );
	}

	B.time_now = (double)glob_m_pGoodClock->End();
	time_dx    = B.time_now - time_old;
	time_old   = B.time_now;

	if( time_dx > 0 ) 
	{
		camera_freq  = double(g_NumFT) / time_dx;
		display_freq = 1.0             / time_dx;
	}

	/**********************************************************************/
	//ofstream ROIFile("ROI.txt");
	

	B.roi_width = B.ROI_target.x2 - B.ROI_target.x1;
	B.roi_height = B.ROI_target.y2 - B.ROI_target.y1;
	int q  = 0;
	roi        = new u16[ B.roi_width * B.roi_height ];
	CString roistr;
	for ( y = B.ROI_target.y1 ; y < B.ROI_target.y2 ; y++ ) 
	{
		for ( x = B.ROI_target.x1 ; x < B.ROI_target.x2 ; x++ ) 
		{	
			*(roi + q++) = *( pict + ( (y) * 512 ) + x );
			roistr.Format(_T("%u "),*( pict + ( (y) * 512 ) + x ));
			//ROIFile << roistr;
		}
		//ROIFile<< _T("\n");
	}


	//ROIFile.close();
	

	//ofstream CameraFile("Camera.txt");
	for ( y = 0 ; y < 512 ; y++ ) 
	{
		for ( x = 0 ; x < 512 ; x++ ) 
		{	
			roistr.Format(_T("%u "),*( pict + ( (y) * 512 ) + x ));
			//CameraFile << roistr;
		}
		//CameraFile << _T("\n") ;
	}
	//CameraFile.close();
	ROIAnalysis(roi);


	//Bead Location Display Conversion

	result_X  = B.roi_beadX + B.ROI_target.x1;// - lhalf;
	result_Y  = B.roi_beadY + B.ROI_target.y1;// - lhalf;

	result_X *= zoom;
	result_Y *= zoom;

	result_X = result_X + 6;
	result_Y = result_Y + 6 + control_height;


	/**********************************************************************/

	int beadsize = 30;
	B.bead_x1 = B.ROI_target.x1 + B.roi_beadX - beadsize / 2;
	B.bead_x2 = B.ROI_target.x1 + B.roi_beadX + beadsize / 2;
	B.bead_y1 = B.ROI_target.y1 + B.roi_beadY - beadsize / 2;
	B.bead_y2 = B.ROI_target.y1 + B.roi_beadY + beadsize / 2;
	if (B.bead_x1 < 0)
	{
		B.bead_x1 = 0;
		B.bead_x2 = 20;
	}
	if (B.bead_x2 > 512)
	{
		B.bead_x1 = 492;
		B.bead_x2 = 512;
	}
	if (B.bead_y1 < 0)
	{
		B.bead_y1 = 0;
		B.bead_y2 = 20;
	}
	if (B.bead_y2 > 512)
	{
		B.bead_y1 = 492;
		B.bead_y2 = 512;
	}


	q  = 0;
	bead        = new u16[ beadsize * beadsize ];
	//TRACE(_T("-------------------------------------------\n"));
	//ofstream BeadFile("Bead.txt");
	for ( y = B.bead_y1 ; y < B.bead_y2 ; y++ ) 
	{
		for ( x = B.bead_x1 ; x < B.bead_x2 ; x++ ) 
		{	
			*(bead + q++) = *( pict + ( (y) * 512 ) + x );
			roistr.Format(_T("%u "),*( pict + ( (y) * 512 ) + x ));
			//BeadFile << roistr;
			//TRACE(roistr);
		}
		//BeadFile << _T("\n");
		//TRACE("\n");
	}
	//BeadFile.close();
	//TRACE(_T("-------------------------------------------\n"));





	BeadAnalysis(bead);

	/**********************************Center the picture************************************/

	int x1_discrep = B.roi_beadX;
	int y1_discrep = B.roi_beadY;
	int x2_discrep = B.roi_width - B.roi_beadX;
	int y2_discrep = B.roi_height - B.roi_beadY;


	double x_move = (x1_discrep - x2_discrep)/2 / 6;
	double y_move = (y1_discrep - y2_discrep)/2 / 6;



	B.roi_xmove = x_move * 0.001;
	B.roi_ymove = y_move * 0.001;
	
	
	/*glob_m_pStage->MoveRelY(y_move);*/
	/**********************************************************************/

	/***********************************************
	* normally, show the plain picture             *
	***********************************************/

	if ( B.pictype == 0 )
	{
		for ( c=0, b=0; c < Length; c++ ) 
		{	
			rj = double( *( good_pic + c ) - B.g_min ) * g_mp;

			val = (uC)(rj);

			*(pPic_main + b++) = val;
			*(pPic_main + b++) = val;
			*(pPic_main + b++) = val;
			*(pPic_main + b++) = val;
		}



		BITMAP bm;
		Bitmap_main.GetBitmap( &bm );	
		Bitmap_main.SetBitmapBits( bm.bmWidthBytes * bm.bmHeight, pPic_main );
		Draw_Bitmap();
		UpdateTitle();


	}

	delete [] roi;
	roi = NULL;
	delete [] bead;
	bead = NULL;
	if( B.savetofile ) 
		WritePic();
}      



/**
* Calculates statistics of the general picture and calculates general focus score
* @param Entire picture
*/
void COctopusPictureDisplay::BasicAnalysis( u16 *pict )
{
	//Parameters
	u16 temp   = 0;
	double diff  = 0;
	double focus = 0;
	double totalint = 0;
	B.g_min = LONG_MAX;
	B.g_max = 0;
	

	//Mean, Max, Min value calculation
	for ( int c = 0; c < Length; c++ ) 
	{
		temp = *( pict + c );
		*( first_picture + c ) = temp;
		if ( temp < B.g_min ) 
			B.g_min = double(temp); 
		else if ( temp > B.g_max ) 
			B.g_max = double(temp);	
		totalint = totalint + double(temp);
	}
	B.g_mean = totalint / Length;

	//Basic Focus Score Calculation
	for ( int c = 0; c < Length; c++ ) 
	{
		temp = *( pict + c );
		diff   = double(temp) - B.g_mean;
		focus += (diff * diff);
	}

	//this is the normalized focus score
	B.CurrFocusScore = focus / totalint;
}

/**
* Calculates statistics of the roi and calculates bead position
* @param ROI picture
*/
void COctopusPictureDisplay::ROIAnalysis( u16 *roi )
{
	u32 i              = 0;
	u16 min            = 65535;
	u16 max            = 0;
	u32 totalint       = 0;
	double mean        = 0.0;
	u16 temp           = 0;
	u16 x              = 0;
	u16 y              = 0;
	int n			   = B.roi_width * B.roi_height;

	//****************************************************
	//get min, max, mean

	for ( i = 0; i < n; i++ ) 
	{	
		temp    = *( roi + i );

		if ( temp < min ) 
		{
			min = temp;
		} 
		else if ( temp > max ) 
		{
			max = temp;
		}
		totalint += temp;
	}


	B.roi_max      = max;
	B.roi_min      = min;
	B.roi_mean     = double(totalint) / double(n);

	//****************************************************
	//compute weighted average of x, y

	u16    cut_high    = u16(0.98 * double(max));

	double xweightr	   = 0.0;
	double yweightr	   = 0.0;
	double counts      = 0.0;

	CString str;
	//ofstream ROI2File("ROI2.txt");
	for ( y = 0; y < B.roi_height; y++ ) 
	{
		for ( x = 0; x < B.roi_width; x++ ) 
		{	
			temp = *( roi + ( (y) * B.roi_width ) + x );
			//bead position
			CString tempString;
			tempString.Format(_T("%u "),temp);
			//ROI2File << tempString;
			if( temp > cut_high )
			{
				CString str_cuthigh;
				str_cuthigh.Format(_T("x: %u, y: %u, val: %u\n"), x, y, temp);
				//TRACE(str_cuthigh);
				counts   += 1.0;
				xweightr += double( x );
				yweightr += double( y );
			}
		}
		//ROI2File << _T("\n");
	}
	//ROI2File.close();

	B.roi_beadX  = (u16)(xweightr / counts);
	B.roi_beadY  = (u16)(yweightr / counts);
	B.roi_cuthigh = cut_high;

}
/**
*
*/
void COctopusPictureDisplay::BeadAnalysis( u16 *bead )
{
	
	u32 i              = 0;
	u16 min            = 65535;
	u16 max            = 0;
	u32 totalint       = 0;
	double mean        = 0.0;
	double temp           = 0;
	u16 x              = 0;
	u16 y              = 0;
	u16 beadsize		= 30;

	double *data				= new double [beadsize * beadsize];


	double       *p = new double[7];   			//parameters

	int m              = 7;
	int n				= beadsize * beadsize;
	int itmax			= 1000;

	double       *opts = new double[LM_OPTS_SZ];
	opts[0] = LM_INIT_MU; 
	opts[1] = 1E-15; 
	opts[2] = 1E-15; 
	opts[3] = 1E-20;
	opts[4] = LM_DIFF_DELTA; 

	double       *info = new double[LM_INFO_SZ];

	double       *work;
	u32 la         = (4*beadsize * beadsize) + (4*m) + (beadsize * beadsize*m) + (m*m);
	work           = new double[ la + (m*m)];

	double       *covar;	
	covar          = work + la;
	//****************************************************
	//get min, max, mean
//TRACE(_T("-----------------------------In Bead Analysis--------------------------------\n"));
	CString tempstr;
	for ( i = 0; i < beadsize * beadsize; i++ ) 
	{	
		temp    = (double) *( bead + i );
		data[i] =  temp;
		
		tempstr.Format(_T("%f "),temp);
		//TRACE(tempstr);
		
		if ( temp < min ) 
		{
			min = temp;
		} 
		else if ( temp > max ) 
		{
			max = temp;
		}
		totalint += temp;
	}
//TRACE(_T("-------------------------------------------------------------\n"));

	B.bead_max      = max;
	B.bead_min      = min;
	B.bead_mean     = double(totalint) / double( beadsize * beadsize);
	//****************************************************
	//LM fitting

	double lhalf = double(beadsize) / 2.0;

	p[0] = 0.0;					//??
	p[1] = B.bead_mean;			//background
	p[2] = B.bead_min;			//amp
	p[3] = lhalf;				//mean x
	p[4] = 2.5;					//sd x
	p[5] = lhalf;				//mean y
	p[6] = 2.5;					//sd y

	/******************************Parameter Optimization ******************************/
	//TRACE(_T("-----------------------------Before--------------------------------\n"));
	//int xv = 0;
	//int yv = 0;
	//int boxlength = beadsize;
	//for ( int xi = 0; xi < boxlength; xi++ )
	//{
	//	xv = pow( ( xi - p[3] ) / p[4], 2.0);
	//
	//	for ( int yi = 0; yi < boxlength; yi++ )
	//	{
	//		yv  = pow( ( yi - p[5] ) / p[6] , 2.0);
	//		int ii = p[1] + (p[2] * exp(-0.5 * (xv+yv)));
	//		CString iii;
	//		iii.Format("%d ", ii);
	//		TRACE(iii);
	//	}
	//	TRACE("\n");
	//}
	//TRACE(_T("-------------------------------------------------------------\n"));
	/*********************************************************************************/

	CString final_result;
	final_result.Format("Before Score: (X: %f,%f Y: %f,%f): %f\n",p[3],p[4],p[5],p[6],B.LM_fwhm);
	//TRACE(final_result);
	/*int ret = dlevmar_dif(Gaussian_2D, p, data, m, n, itmax, opts, info, work, covar, NULL);*/
	int ret = dlevmar_dif(Gaussian_2D, p, data, m, n, itmax, NULL, NULL, NULL, NULL, NULL);
	//if (ret < 0)
	//{
	//	TRACE(_T("dlevmar_dif failure"));
	//}


	//p[1] = result.mean;					//background
	//p[2] = result.max;					//amp

	B.LM_X     = p[3];			//mean x
	B.LM_X_sd  = p[4];					//sd x
	B.LM_Y     = p[5];			//mean y
	B.LM_Y_sd  = p[6];					//sd y
	B.LM_fwhm  = 2.354 * ((p[4] + p[6]) / 2.0); //full width half maximum = 2.354*sd
	if (p[4] < 0 || p[6] < 0)
		B.LM_fwhm = 20;
	if (p[4] < 1 || p[6] < 1)
		B.LM_fwhm = 20;
	if (B.LM_fwhm > 20)
		B.LM_fwhm = 20;
	//	TRACE(_T("-----------------------------After--------------------------------\n"));

	//for ( int xi = 0; xi < boxlength; xi++ )
	//{
	//	xv = pow( ( xi - p[3] ) / p[4], 2.0);
	//
	//	for ( int yi = 0; yi < boxlength; yi++ )
	//	{
	//		yv  = pow( ( yi - p[5] ) / p[6] , 2.0);
	//		int ii = p[1] + (p[2] * exp(-0.5 * (xv+yv)));
	//		CString iii;
	//		iii.Format("%d ", ii);
	//		TRACE(iii);
	//	}
	//	TRACE("\n");
	//}
	//TRACE(_T("-------------------------------------------------------------\n"));

	/************************************Levmar function tester******************************************/
//  /* generate some measurement using the exponential model with
 //  * parameters (5.0, 0.1, 1.0), corrupted with zero-mean
 //  * Gaussian noise of s=0.1
 //  */
	INIT_RANDOM(0);
	const int nn=40, mm=3; // 40 measurements, 3 parameters
	double *data4				= new double [128];
  for(i=0; i<nn; ++i)
    data4[i]=(5.0*exp(-0.1*i) + 1.0) + gNoise(0.0, 0.1);

  /* initial parameters estimate: (1.0, 0.0, 0.0) */
  p[0]=1.0; p[1]=0.0; p[2]=0.0;

  /* optimization control parameters; passing to levmar NULL instead of opts reverts to defaults */
  opts[0]=LM_INIT_MU; opts[1]=1E-15; opts[2]=1E-15; opts[3]=1E-20;
  opts[4]=LM_DIFF_DELTA; // relevant only if the finite difference Jacobian version is used 
  dlevmar_dif(expfunc, p, data4, mm, nn, 1000, opts, info, NULL, NULL, (void *)&data); // without Jacobian
  final_result.Format("After Score4: %f, %f, %f\n",p[0],p[1],p[2]);
  	delete [] data4;
	data4 = NULL;
	/************************************Levmar function tester******************************************/


	delete [] work;
	work = NULL;
	delete [] data;
	data = NULL;

}

double COctopusPictureDisplay::gNoise(double m, double s)
{
double r1, r2, val;

  r1=((double)rand())/DBL_RAND_MAX;
  r2=((double)rand())/DBL_RAND_MAX;

  val=sqrt(-2.0*log(r1))*cos(2.0*3.14*r2);

  val=s*val+m;

  return val;
}

bool COctopusPictureDisplay::Open_A_File( void ) 
{
	CString temp;
	B.files_written++;
	CString ZBuffNum;
	if (B.files_written < 10)
	{
		 ZBuffNum = "_000";
	}
	else if (B.files_written < 100)
	{
		ZBuffNum = "_00";
	}
	else if (B.files_written < 1000)
	{
		ZBuffNum = "_0";
	}
	else 
	{
		ZBuffNum = "_";
	}


	temp.Format(_T("_%s%s%1d.dat"), B.tag, ZBuffNum, B.files_written );
	filename = B.pathname + temp;	
	pFileData = fopen( filename , _T("wb"));//_wfopen( filename , _T("wb"));


	temp.Format(_T("_%s%s%1d.dth"), B.tag,ZBuffNum, B.files_written );
	filename = B.pathname + temp;
	pFileHead = fopen( filename , _T("wt"));//_wfopen( filename , _T("wt"));

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
	//AndorCamera//FreeInternalMemory ();
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
		//B.time_now = (double)glob_m_pGoodClock->End();

		str.Format(_T("N:%ld H:%d W:%d T:%.3f Filter:%d ND:%d X:%.5f Y:%.5f Z:%.5f ExpTime:%d EMGain:%d M:%.3f Tag: %s\n"),
		        pictures_written, h, w, B.time_now, B.filter_wheel, B.nd_setting, \
				B.position_x, B.position_y, B.position_z, \
				B.CameraExpTime_ms, B.EM_GAIN, B.grat_position_z, B.tag);
			
		
		pictures_written++;

		fprintf( pFileHead, str );	//fwprintf( pFileHead, str );	
	}
	fflush( pFileHead );

	fwrite( frames_to_save, sizeof(u16), w * h * g_NumFT, pFileData );
	//fwrite( pPic_main, sizeof(u16), w * h * g_NumFT, pFileData );
	
	fflush( pFileData );

	B.expt_time = B.time_now - B.savetime;
	B.expt_frame++;
}

void COctopusPictureDisplay::UpdateTitle( void ) 
{
	CString temp;
	
	temp.Format(_T("Min:%.1f Max:%.1f Cutoff:%.1f Mean:%.1f Focus:%.2f Cam_Freq(Hz):%.1f PicsTransfer:%d\n"), \
		         B.g_min, B.g_max, g_mp, B.g_mean, B.focus_score, camera_freq, g_NumFT);
	temp.AppendFormat(_T("Frames in current file:%d  Total saved frames:%d Time(s):%.1f SaveDuration(s):%.1f \n"), \
		         pictures_written + 1, B.expt_frame + 1, B.time_now, B.expt_time ); 
	temp.AppendFormat(_T("X:%.1f Y:%.1f Z:%.1f Mask:%d"), \
		         B.position_x, B.position_y, B.position_z, B.mask_now);

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


	//Marker1.x = B.ROI_actual.x2 - Marker2.x;
	//Marker2.x = B.ROI_actual.x2 - temp;

	//Marker1.y += B.ROI_actual.y1;
	//Marker2.y += B.ROI_actual.y1;

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

	B.ROI_target  = ROI;
	B.ROI_changed = true;
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
	
	CPen pen(PS_SOLID,2,COL_BLUE);
	pDC->SelectObject(&pen);
	pDC->MoveTo( Marker1.x , Marker1.y); 
	pDC->LineTo( point.x   , Marker1.y); 
	pDC->LineTo( point.x   , point.y  );
	pDC->LineTo( Marker1.x , point.y  );
	pDC->LineTo( Marker1.x , Marker1.y);
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

void __cdecl expfunc(double *p, double *x, int m, int n, void *data)
{
register int i;
struct xtradata *dat;

  dat=(struct xtradata *)data;
  /* user-supplied data can now be accessed as dat->msg, etc */

  for(i=0; i<n; ++i){
    x[i]=p[0]*exp(-p[1]*i) + p[2];
  }
}
// gaussian fitting
void __cdecl Gaussian_2D(double *p, double *x, int m, int n, void *data)
{
//% p(1) background
//% p(2) amplitude
//% p(3) meanx
//% p(4) SDx
//% p(5) meany
//% p(6) SDy
//int m,             /* I: parameter vector dimension (i.e. #unknowns) */
//int n,             /* I: measurement vector dimension */

	unsigned long boxlength = (unsigned long)sqrt( (double)n );
	double xv  = 0.0;
	double yv  = 0.0;
	unsigned long xi = 0;
	unsigned long yi = 0;

	for ( xi = 0; xi < boxlength; xi++ )
	{
		xv = pow( ( xi - p[3] ) / p[4], 2.0);
	
		for ( yi = 0; yi < boxlength; yi++ )
		{
			yv  = pow( ( yi - p[5] ) / p[6] , 2.0);
			x[(xi*boxlength)+yi] = p[1] + (p[2] * exp(-0.5 * (xv+yv)));
		}
	}
}

