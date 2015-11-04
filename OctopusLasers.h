
#if !defined(AFX_H_OctopusLasers)
#define AFX_H_OctopusLasers

#include "stdafx.h"
#include "OctopusGlobals.h"
#include "APTMotor.h"
#include "APTSystem.h"

class COctopusLasers : public CDialog
{

public:
	
	COctopusLasers(CWnd* pParent = NULL);
	virtual ~COctopusLasers();
	enum { IDD = IDC_LASERS };

	void Laser_405_On( void );
	void Laser_405_Off( void );
	void Laser_488_On( void );
	void Laser_488_Off( void );
	void Laser_561_On( void );
	void Laser_561_Off( void );
	void Laser_639_On( void );
	void Laser_639_Off( void );

protected:

	CBitmap m_bmp_yes;
	CBitmap m_bmp_no;
	
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	CStatic m_status_405;
	CStatic m_status_488;
	CStatic m_status_561;
	CStatic m_status_639;

	bool Laser_405_good;
    bool Laser_488_good;
    bool Laser_561_good;
	bool Laser_639_good;

public:

	afx_msg void OnClicked405();
	afx_msg void OnClicked488();
	afx_msg void OnClicked561();
    afx_msg void OnClicked639();

	void Initialize( void );

	CMgmotorctrl1 Laser405Shutter;
	CMgmotorctrl1 Laser488Shutter;
	CMgmotorctrl1 Laser561Shutter;
	CMgmotorctrl1 Laser639Shutter;

};

#endif