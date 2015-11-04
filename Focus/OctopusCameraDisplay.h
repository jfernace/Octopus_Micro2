///////////////////////////////////////////////////////////////////////////////
// CameraDisplay.h : header file
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_H_OctopusCameraDisplay)
#define AFX_H_OctopusCameraDisplay

#include "stdafx.h"
#include "OctopusGlobals.h"
#include "OctopusLM.h"
#include <stdio.h>
#include "CSLScope.h"

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

protected:

	void WritePic( void );
	bool Open_A_File( void );

	CBitmap Bitmap_main;
	CBitmap Bitmap_zoom;
	CBitmap Bitmap_bead;

	uC*  pPic_main;
	uC*  pPic_zoom;
	uC*  pPic_bead;

	u16* first_picture;
	u16* frames_to_save;
	u16* first_picture_flip;
	u16* tp;
	u16* tb;

	u16 MemoryFramesOld;
	u16 g_NumFT;

	FILE * pFileData;
	FILE * pFileHead;

	double pos_x_old;
	double pos_y_old;

	CString filename; 
	COctopusROI ROI;

	COctopusLM* pLM;

	void Draw_Bitmap( void );
	void UpdateTitle( void );
	
	void PlotGraph( void );

	bool	Marking;
	u16		Width; 
	u16		Height;

	u16 roi_x_begin;
	u16 roi_x_end;
	u16 roi_y_begin;
	u16 roi_y_end;
	u16 roi_Width;
	u16 roi_Height;
	u16 roi_bead;

	u32 plottime;
	u16 first;
	u16 last;

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
	
	double plotx0;
	double ploty0;
	double plotf0;
	double plotw0;

	double	g_mp;
	u16	    g_max;
	u16	    g_min;
	double  g_mean;
	u32		pictures_written;

	double time_dx;
	double time_old;
	double frequency;

	double camera_freq;
	double display_freq;

	CPoint Marker1;
	CPoint Marker2;
	CPoint Begin;
	CPoint End;
	
	void LeftButtonDown( CPoint point );
	void LeftButtonUp( CPoint point );
	void MouseMove( CPoint point );

	void ValidateMarkersAndSetROI( void );

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();


	CTSLScope Scope;
	//int Counter;

	CStatic m_info;
	CStatic m_Scope;

	float DataArrayX[ 1000 ];
	float DataArrayY[ 1000 ];
	float DataArrayF[ 1000 ];
	float DataArrayW[ 1000 ];

	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};

#endif
