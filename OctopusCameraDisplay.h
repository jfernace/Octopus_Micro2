///////////////////////////////////////////////////////////////////////////////
// CameraDisplay.h : header file
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_H_OctopusCameraDisplay)
#define AFX_H_OctopusCameraDisplay

#include "stdafx.h"
#include "OctopusGlobals.h"
#include <stdio.h>
#include "levmar.h"

using namespace std;

class COctopusPictureDisplay : public CDialog
{

public:
	
	COctopusPictureDisplay( CWnd* pParent = NULL, u16 x = 50, u16 y = 50 );
	~COctopusPictureDisplay();
	
	enum { IDD = IDC_CAS_DISP };

	void Update_Bitmap( u16 *pic, u16 FramesTransferred );	
	void Create_Bitmap( void );
	void Close_The_File(void);
	void BeadAnalysis( u16 *bead );
	double gNoise(double m, double s);

protected:

	void WritePic( void );
	bool Open_A_File( void );

	CBitmap Bitmap_main;

	uC*  pPic_main;

	u16*    first_picture;
	u16*	roi;
	u16*	bead;
	double* picture_SR;
		//double*	data;

	u16* picture_1;
	u16* picture_2;
	u16* picture_3;

	u16* frames_to_save;
	u16* first_picture_flip;

	u16 MemoryFramesOld;
	u16 g_NumFT;

	FILE * pFileData;
	FILE * pFileHead;

	CString filename; 
	COctopusROI ROI;

	void Draw_Bitmap( void );
	void UpdateTitle( void );
	
	bool	Marking;
	u16		Width; 
	u16		Height;

	u32 Length;
	u32 FullFrame;

	s16 mouse_dx;
	s16 mouse_dy;
	u16 mouse_x_old;
	u16 mouse_y_old;

	u16 pic_x1;
	u16 pic_y1;
	u16 pic_x2;
	u16 pic_y2;

	u16 save_halflength;
	u32 picture_size;
	
	double zoom;

	u16 control_height;
	u16 screen_wp;
	u16 screen_hp;

	double	g_mp;
	double	g_max;
	double	g_min;
	double  g_mean;
	double roi_X;
	double roi_Y;

	u32		pictures_written;

	double time_dx;
	double time_old;
	double frequency;

	double camera_freq;
	double display_freq;
	double       p[7];   //parameters

	CPoint Marker1;
	CPoint Marker2;
	CPoint Begin;
	CPoint End;
	
	void LeftButtonDown( CPoint point );
	void LeftButtonUp( CPoint point );
	void MouseMove( CPoint point );

	void ValidateMarkersAndSetROI( void );
	void BasicAnalysis(u16 *pict);
	void ROIAnalysis(u16 *roi);
	

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	CStatic m_info;

	double result_X;
	double result_Y;
	COctopusCentroid GetCenterGood( u16 *pic );
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnStnClickedTime();
//	afx_msg void OnStnDisableTime();
};

#endif
