
#include "stdafx.h"
#include "Octopus.h"
#include "MainFrm.h"

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

CMainFrame::CMainFrame() {}
CMainFrame::~CMainFrame() {}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW;
    cs.cy = 725; 
	cs.y  = 2;
    cs.cx = 930; 
    cs.x  = 20;

	return CFrameWnd::PreCreateWindow(cs);
}