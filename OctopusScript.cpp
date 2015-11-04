
#include "stdafx.h"
#include "mainfrm.h"
#include "Octopus.h"
#include "OctopusDoc.h"
#include "OctopusView.h"
#include "OctopusGlobals.h"
#include "OctopusShutter.h"
#include "OctopusCameraDlg.h"
#include "OctopusScript.h"
#include "OctopusObjectivePiezo.h"
#include "Octopus_LED.h"
//#include "OctopusStage686.h"
//#include "OctopusPolarizer.h"
#include "OctopusScope.h"
#include "OctopusFocus.h"
#include "OctopusStage545.h"


extern COctopusGlobals B;

extern COctopusCamera*          glob_m_pCamera;
extern COctopusShutterAndWheel* glob_m_pShutterAndWheel;
extern Octopus_LED*             glob_m_pLED;
extern COctopusObjectivePiezo*  glob_m_pObjPiezo;
//extern COctopusStage686*        glob_m_pStage686;
//extern COctopusPolarizer*       glob_m_pPolarizer;
extern COctopusScope*           glob_m_pScope;
extern COctopusFocus*           glob_m_pFocus;
extern COctopusStage545*	    glob_m_pStage545;


COctopusScript::COctopusScript(CWnd* pParent)
: CDialog(COctopusScript::IDD, pParent)
{    

	m_SeqListIndex          = -1;
	command_index           =  0;
	cycles_to_do            =  0;
	cycles_to_do_in_program =  0;
	B.program_running       = false;
	saved_path = B.pathname;

	VERIFY(m_bmp_running.LoadBitmap(IDB_RUNNING));
	VERIFY(m_bmp_stopped.LoadBitmap(IDB_STOPPED));

	if( Create(COctopusScript::IDD, pParent) ) 
		ShowWindow( SW_SHOW );
}

void COctopusScript::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,	IDC_LIST_SEQUENCE,	 m_SeqList);
	DDX_LBIndex(pDX,	IDC_LIST_SEQUENCE,	 m_SeqListIndex);
	DDX_Control(pDX,    IDC_SCR_RUNNING_BMP, m_status_scr);
	DDX_Control(pDX,    IDC_SCR_CYCLE,       m_cycle_count);
}  

BEGIN_MESSAGE_MAP(COctopusScript, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEQ_LOAD, OnButtonSeqLoad)
	ON_BN_CLICKED(IDC_BUTTON_SEQ_RUN,  OnButtonSeqRun)
	ON_BN_CLICKED(IDC_BUTTON_SEQ_STOP, OnButtonSeqStop)
	ON_WM_TIMER()
END_MESSAGE_MAP()

COctopusScript::~COctopusScript() {}

/***************************************************************************/
/***************************************************************************/

void COctopusScript::OnButtonSeqLoad() 
{

	CFile flConfigs;
	CString str, str0, str1;
	CString cycles;
	CString sub;

	m_SeqList.ResetContent();
	m_SeqListIndex = 0;

	UpdateData(FALSE);

	CFileDialog FileDlg(TRUE, NULL, NULL, 4|2, "seq|*.seq||", NULL, 0);

	if( FileDlg.DoModal() == IDOK )
	{
		CStdioFile f(FileDlg.GetFileName(),CFile::modeRead);

		if( m_SeqList.GetCount() > 0 ) // zero the current sequence list
		{
			m_SeqList.ResetContent();
			m_SeqListIndex = 0;
		}

		bool looking_for_the_end = true;

		while ( looking_for_the_end )
		{
			f.ReadString(str);

			if( str.Find("#") >=0 ) // comment
			{	
				//do nothing
				//do not add them to the list
			} 
			else if (str.Find("subroutine") >= 0) 
			{
				str.Trim(); // remove leading and trailing space
				int n = str.Find(_T(" "));
				if( n > 0 ) /* space is at ... */
				{
					str.GetLength();
					sub = str.Right(str.GetLength() - n);
					sub.Trim();
					int insertcount = 0;
					m_SeqListIndex = m_SeqList.GetCount();

					CStdioFile subf(sub,CFile::modeRead);

					while ( subf.ReadString(str) )
					{
						if( str.Find("#") >=0 ) // comment
						{	
							//do nothing
							//do not add them to the list
						} 
						else if ( str.Find("subroutine") >= 0 ) 
						{
							//do nothing
							//do not add them to the list
						}
						else if ( str.Find("end") >= 0 ) 
						{
							//do nothing
							//do not add them to the list
						}
						else if ( str.Find("cycles") >= 0 ) 
						{
							//do nothing
							//do not add them to the list
						}
						else
						{
							m_SeqList.InsertString( m_SeqListIndex + insertcount, str );
							insertcount++;
						}
					}
				}
			}
			else if (str.Find("cycles") >= 0) 
			{
				// last line has been found - good
				// swscanf(str, _T("%*s %d"), &cycles_to_do_in_program);
				sscanf(str, "%*s %d", &cycles_to_do_in_program);
				m_SeqList.AddString(str);
				looking_for_the_end = false;
			}
			else if (str.Compare("\n") != 0)
			{
				m_SeqList.AddString(str);
			}
		}
		m_SeqListIndex = m_SeqList.GetCount();
		f.Close();
	}
	else
		return;

	UpdateData(FALSE);
}

void COctopusScript::OnButtonSeqRun() 
{
	command_index  = 0;
	cycles_to_do   = cycles_to_do_in_program;
	m_nTimer       = SetTimer( TIMER_SCRIPT, 200, NULL );

	m_status_scr.SetBitmap( m_bmp_running );

	B.program_running = true;

	CString temp;

	temp.Format("Cycles to do:%d\nCycles done:%d", \
		cycles_to_do, cycles_to_do_in_program - cycles_to_do);

	m_cycle_count.SetWindowText( temp );
}

void COctopusScript::OnButtonSeqStop() 
{ 
	cycles_to_do = 0;
	B.program_running = false;
	m_status_scr.SetBitmap( m_bmp_stopped );
	KillTimer( m_nTimer );
}

void COctopusScript::OnTimer( UINT nIDEvent ) 
{
	char command[256]; 
	if( nIDEvent == TIMER_SCRIPT ) 
	{
		CString str;

		if( B.focus_in_progress    ) return;

//		if( B.POL_scan_in_progress ) return;

		if ( cycles_to_do > 0 )
		{	
			m_SeqList.GetText(command_index, str);

			str.MakeLower();

			if(str.Find(_T("pause")) >= 0)
			{				
				command_index++;	
				float etime = 4.0;
				//swscanf(str, _T("%*s %e %*s"), &etime);
				sscanf(str, "%*s %f %*s", &etime);
				m_nTimer = SetTimer( TIMER_SCRIPT, (u32)etime * 1000, NULL );				
			}
			else if( str.Find(_T("filter:"))>=0 ) // we have a filter command
			{
				command_index++;
				u8 filter = 1;
				sscanf(str, "%*s %d", &filter);
				// this will 'fill' the string command with the right stuff,
				// assuming myFile and convertedFile are strings themselves
				//sprintf (command, "cd C:\\Documents and Settings\\Owner\\Desktop\\MTTTYJOHNNY\\Release & MITTY.exe %d", filter); 
				sprintf (command, "cd C:\\Program Files\\Octopus\\Josh's Octopus Files\\MTTTY\\Release & MITTY.exe %d", filter); 

				// system call
				system(command);
				B.filter_wheel = filter;
				m_nTimer = SetTimer( TIMER_SCRIPT, 200, NULL );
			}
			else if( str.Find(_T("mirrorunit:"))>=0 ) // we have a mirrorunit command
			{
				command_index++;
				if ( glob_m_pScope == NULL ) return;
				u8 mu = 1;
				sscanf(str,"%*s %d", &mu);
				glob_m_pScope->EpiFilterWheel( mu );
				m_nTimer = SetTimer( TIMER_SCRIPT, 200, NULL );
			}
			else if( str.Find(_T("brightnd:"))>=0 ) // we have a bright field filter command
			{
				command_index++;
				if ( glob_m_pScope == NULL ) return;
				u8 bf = 1;
				sscanf(str,"%*s %d", &bf);
				glob_m_pScope->BrightFieldFilterWheel( bf );
				m_nTimer = SetTimer( TIMER_SCRIPT, 200, NULL );
			}
			else if( str.Find(_T("moveobjrel")) >= 0 )
			{
				command_index++;

				float move_microns = 0.0;

				sscanf(str, "%*s %e %*s", &move_microns);

				if( move_microns < -20.0 ) move_microns = -20.0;
				if( move_microns > +20.0 ) move_microns = +20.0;

				if ( glob_m_pObjPiezo != NULL )
					glob_m_pObjPiezo->MoveRelZ( move_microns );

				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find(_T("save on")) >= 0 )
			{
				command_index++;
				//CString path = "";
				//char path [100];
				//sscanf(str, _T("%*s %*s %[^\n]"), &path);
				//TRACE(path);
				//TRACE(_T("IN SAVE ON\n"));

				if (glob_m_pCamera == NULL) return;
				//TRACE(_T("Cammera not null\n"));

				//TRACE(_T("Global variable accessed\n"));

				glob_m_pCamera->StartSaving();
				//TRACE(_T("Starting to save\n"));
				//B.pathname = path;
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find(_T("save off")) >=0 )
			{
				command_index++;
				if (glob_m_pCamera == NULL) return;
				//TRACE(_T("IN SAVE OFF\n"));
				glob_m_pCamera->StopSaving();
				//B.pathname = saved_path;
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find(_T("shutter on")) >=0 ) // we have a shutter command
			{	
				command_index++;

				if (glob_m_pShutterAndWheel == NULL) return;

				u8 intensity = 1;

				sscanf(str, _T("%*s %*s %d"), &intensity);

				if( intensity < 0 ) 
					glob_m_pShutterAndWheel->ShutterClose();
				else if ( intensity > 100 )
					glob_m_pShutterAndWheel->ShutterOpen();
				else
					glob_m_pShutterAndWheel->ShutterPartial( intensity ); 

				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find(_T("shutter off")) >=0 ) // we have a shutter command
			{	
				command_index++;

				if (glob_m_pShutterAndWheel == NULL) return;

				glob_m_pShutterAndWheel->ShutterClose();

				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find(_T("movie on")) >= 0 ) // we have a camera command
			{	
				command_index++;

				if (glob_m_pCamera == NULL) return;
				CString tag;
				int gain;
				int exptime;
				sscanf(str, _T("%*s %*s %d %d %s"), &gain, &exptime, tag);
				B.tag = tag;
				glob_m_pCamera->SetMovieGain(gain);
				glob_m_pCamera->SetExposureTime_Movie_ms(exptime);
				//Sleep( 1000 );
				glob_m_pCamera->StartMovie();
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find("movie off") >= 0 ) // we have a camera command
			{	
				command_index++;
				if (glob_m_pCamera == NULL) return;
				glob_m_pCamera->StopCameraThread();
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			//else if( str.Find("pol scan") >= 0 ) // we have a POL command
			//{	
			//	command_index++;
			//	if (glob_m_pPolarizer == NULL) return;
			//	glob_m_pPolarizer->ScanStartStop(); 
			//	m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			//}
			else if( str.Find("take picture") >= 0 ) // we have a camera command
			{	
				command_index++;
				int gain;
				int exptime;
				CString tag;
				sscanf(str, _T("%*s %*s %d %d %s"), &gain, &exptime,tag);
				if (glob_m_pCamera == NULL) return;
				B.tag = tag;
				glob_m_pCamera->SetExposureTime_Single_ms(exptime);
				glob_m_pCamera->SetCameraGain(gain);
				//Sleep( 1000 );
				u32 etime = glob_m_pCamera->GetExposureTime_Single_ms() + 100;
				glob_m_pCamera->m_focusType = 2;
				glob_m_pCamera->StartPicture();

				m_nTimer = SetTimer(TIMER_SCRIPT, etime + 100, NULL);
			}
			else if(str.Find(_T("snap")) >= 0){
				command_index++;
				int gain;
				int exptime, intensity;
				CString tag;
				sscanf(str, _T("%*s %d %d %d %s"), &gain, &exptime,&intensity,tag); 
				if (glob_m_pCamera == NULL) return;
				B.tag = tag;
				glob_m_pCamera->SetExposureTime_Single_ms(exptime);
				glob_m_pCamera->SetCameraGain(gain);
				glob_m_pShutterAndWheel->m_snap_power_percent = intensity;
				u32 etime = glob_m_pCamera->GetExposureTime_Single_ms() + 100;
				glob_m_pShutterAndWheel->OnSnap();

				m_nTimer = SetTimer(TIMER_SCRIPT, etime + 100, NULL);
				/*

				CString tag;
				sscanf(str, _T("%*s %d %d %d %s"), &gain, &exptime,&intensity,tag); 

				if( intensity < 0 ) 
				glob_m_pShutterAndWheel->ShutterClose();
				else if ( intensity >= 100 )
				glob_m_pShutterAndWheel->ShutterOpen();
				else
				glob_m_pShutterAndWheel->ShutterPartial( intensity );

				if (glob_m_pCamera == NULL) return;
				glob_m_pCamera->SetExposureTime_Single_ms(exptime);
				glob_m_pCamera->SetCameraGain(gain);
				//glob_m_pCamera->m_focusType = 2;
				//u32 etime = glob_m_pCamera->GetExposureTime_Single_ms() + 100;
				glob_m_pCamera->TakePicture();
				//glob_m_pCamera->StartPicture();
				//Sleep(exptime+100);
				//m_nTimer = SetTimer( TIMER_SCRIPT, (u32)10 * 1000, NULL );
				m_nTimer = SetTimer(TIMER_SCRIPT, exptime + 100, NULL);
				glob_m_pShutterAndWheel->ShutterClose();

				*/
			}

			else if( str.Find(_T("kinetic on")) >= 0 ) // we have a kinetic imaging command
			{
				command_index++;

				if (glob_m_pCamera == NULL) return;
				CString tag;
				int gain;
				int exptime;
				int kslength;
				sscanf(str, _T("%*s %*s %d %d %d %s"), &gain, &exptime, &kslength, tag);
				B.tag = tag;
				glob_m_pCamera->SetExposureTime_Single_ms(exptime); //Should be Done
				glob_m_pCamera->SetCameraGain(gain); //should be ok
				glob_m_pCamera->SetNKinetics(kslength); //Should be done
				//Sleep( 1000 );
				u32 etime = glob_m_pCamera->GetExposureTime_Single_ms()*kslength + 100;
				//glob_m_pCamera->m_focusType = 2; //No Need for this
				glob_m_pCamera->TakeKineticPicture(); //Done

				m_nTimer = SetTimer(TIMER_SCRIPT, etime + 100, NULL);
			}
			else if( str.Find(_T("autofocus")) >=0 ) // we have an autofocus command
			{
				command_index++;
				int focustype = 0;
				float stepsize = 1;
				sscanf(str, _T("%*s %d %f"), &focustype, &stepsize);
				if (glob_m_pCamera == NULL) return;
				u32 etime = glob_m_pCamera->GetExposureTime_Single_ms() + 100;
				glob_m_pCamera->m_focusType = focustype;
				glob_m_pCamera->m_focusInterval = stepsize;
				//B.focus_in_progress = true;
				glob_m_pCamera->StartPicture();
				etime = etime * 80;
				CString str;
				str.Format(_T("Time: %d\n"), etime);
				TRACE(str);
				m_nTimer = SetTimer(TIMER_SCRIPT, etime + 100, NULL); 
			}
			else if( str.Find("led off") >= 0 )
			{	
				command_index++;
				if (glob_m_pLED == NULL) return;
				glob_m_pLED->LED_Off();
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find("led on") >= 0 )
			{	
				command_index++;
				int intensity = 0;
				sscanf (str,"led on %d",&intensity);
				TRACE("Intensity from Script: %d\n", intensity);
				if (glob_m_pLED == NULL) return;
				glob_m_pLED->LED_On(intensity);
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find("laser off") >= 0 )
			{	
				command_index++;
				if (glob_m_pLED == NULL) return;
				glob_m_pLED->Laser_Off();
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find("laser on") >= 0)
			{
				command_index++;
				int intensity = 0;
				sscanf (str,"laser on %d",&intensity);
				TRACE("Intensity from Script: %d\n", intensity);
				if (glob_m_pLED == NULL) return;
				glob_m_pLED->Laser_On(intensity);
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL);
			}
			else if( str.Find("flow") >= 0 )
			{	
				command_index++;
				int dout1 = 0;
				int dout2 = 0;
				int dout3 = 0;
				int dout4 = 0;
				int dout5 = 0;
				int dout6 = 0;
				int dout7 = 0;
				int dout8 = 0;
				sscanf (str,"flow %d %d %d %d %d %d %d %d",&dout1, &dout2, &dout3, &dout4, &dout5, &dout6, &dout7, &dout8);
				int val = 1 * dout1 + 2 * dout2 + 4 * dout3 + 8 * dout4 + 16 * dout5 + 32 * dout6 + 64 * dout7 + 128 * dout8;
				if (glob_m_pLED == NULL) return;
				glob_m_pLED->SetFlow(val);
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			/*
			else if( str.Find(_T("led:")) >=0 ) // we have a shutter command
			{	
			command_index++;
			if (glob_m_pLED == NULL) return;
			u8 intensity = 1;
			sscanf(str, _T("%*s %d"), &intensity);

			if( intensity == 0 ) 
			glob_m_pLED->LED_Off();
			else
			glob_m_pLED->LED_On( intensity);

			m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			*/

			else if( str.Find(_T("movestage")) >= 0 )
			{
				command_index++;

				float move_x = 0.0;
				float move_y = 0.0;

				sscanf(str, _T("%*s %e %e"), &move_x, &move_y);

				move_x = move_x / 1000;
				move_y = move_y / 1000;

				if ( glob_m_pStage545 == NULL ) 
					return;

				if( move_x < -1 ) move_x = -1;
				if( move_x > 1 ) move_x = 1;
				if( move_y < -1 ) move_y = -1;
				if( move_y > 1 ) move_y = 1;


				glob_m_pStage545->MoveRelX( move_y );
				Sleep( 1000 ); //decreased these because we are forcing move's to be a mm or less...(so fast)
				glob_m_pStage545->MoveRelY( -move_x );
				Sleep( 1000 );
				m_nTimer = SetTimer(TIMER_SCRIPT, 100, NULL); //Decreased this because we're sleeping anyways...
			}
			else if( str.Find(_T("gotoxy")) >= 0 )
			{
				command_index++;

				int pos_x = 0;
				int pos_y = 0;

				sscanf(str, _T("%*s %d %d"), &pos_x, &pos_y);


				if ( glob_m_pStage545 == NULL ) 
					return;

				if( pos_x < -25000 ) pos_x = -25000;
				if( pos_x > 25000 ) pos_x = 25000;
				if( pos_y < -25000 ) pos_y = -25000;
				if( pos_y > 25000 ) pos_y = 25000;

				glob_m_pStage545->GetPosition();

				float change_x = ((float)pos_x)/1000 - (25 - B.position_y) ;


				float change_y = ((float)pos_y)/1000 - (B.position_x) ;


				CString str;
				str.Format("Movetoxy: %d %d Position: %f %f Change: %f %f\n", pos_x, pos_y, 25 - B.position_y, B.position_x, change_x, change_y);
				TRACE(str);
				TRACE("MOVING X1\n");
				glob_m_pStage545->MoveRelY( -change_x ); 
				TRACE("MOVING X2\n");
				Sleep( 3000 );

				TRACE("MOVING Y1\n");
				glob_m_pStage545->MoveRelX( change_y );
				Sleep( 3000 );

				TRACE("MOVING Y2\n");


				m_nTimer = SetTimer(TIMER_SCRIPT, 2000, NULL); 
			}
			/*
			else if( str.Find("laser561 off")>=0 )
			{	
			command_index++;
			if (glob_m_pLasers == NULL) return;
			glob_m_pLasers->Laser_561_Off();
			m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			else if( str.Find("laser561 on")>=0 )
			{	
			command_index++;
			if (glob_m_pLasers == NULL) return;
			glob_m_pLasers->Laser_561_On();
			m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL); 
			}
			*/
			else if(str.Find("cycles")>=0) // we've hit the last line in the list of commands - repeat
			{	
				command_index = 0;
				cycles_to_do--;
				CString temp;
				temp.Format("Cycles to do:%d\nCycles done:%d", cycles_to_do, cycles_to_do_in_program - cycles_to_do);
				m_cycle_count.SetWindowText( temp );
				m_nTimer = SetTimer(TIMER_SCRIPT, 200, NULL);
			}
			else 
			{
				command_index++;
				CString err;
				err.Format("Invalid command, please check the script for errors!\n");
				err.Append( str );
				AfxMessageBox(err);
			}

		}
		else 
		{
			//we are done
			B.program_running = false;
			m_status_scr.SetBitmap( m_bmp_stopped );
			KillTimer(m_nTimer);
		}
	}
	CDialog::OnTimer(nIDEvent);
}