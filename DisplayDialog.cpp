// DisplayDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Octopus.h"
#include "DisplayDialog.h"


// DisplayDialog dialog

IMPLEMENT_DYNAMIC(DisplayDialog, CDialog)

DisplayDialog::DisplayDialog(CWnd* pParent /*=NULL*/)
	: CDialog(DisplayDialog::IDD, pParent)
{

}

DisplayDialog::~DisplayDialog()
{
}

void DisplayDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DisplayDialog, CDialog)
END_MESSAGE_MAP()


// DisplayDialog message handlers
