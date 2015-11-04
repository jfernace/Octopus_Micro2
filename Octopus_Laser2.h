#pragma once


// Octopus_Laser2 dialog

class Octopus_Laser2 : public CDialog
{
	DECLARE_DYNAMIC(Octopus_Laser2)

public:
	Octopus_Laser2(CWnd* pParent = NULL);   // standard constructor
	virtual ~Octopus_Laser2();

// Dialog Data
	enum { IDD = IDD_LASER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
