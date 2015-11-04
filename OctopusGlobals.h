/*
Copyright (c) 2003 Jan T. Liphardt (JTLiphardt@lbl.gov), 
Lawrence Berkeley National Laboratory
*/

#if !defined(AFX_H_OctopusCascadeGlobals)
#define AFX_H_OctopusCascadeGlobals

#include "stdafx.h"

struct COctopusROI 
{
	int x1;
	int x2;
	int y1;
	int y2;
	int bin;
};

struct COctopusCentroid
{
	double X;
	double Y;
	double Intensity;
	double max;
	double min;
	double mean;
	double totalint;
	double cut_high;

	double LM_X;
	double LM_Y;
	double LM_X_sd;
	double LM_Y_sd;
	double LM_fwhm;

};

struct COctopusGlobals 
{
	CString tag;
	u32		files_written;
	bool	savetofile;
	u16		W;
	u16		H;
	bool	automatic_gain;
	double	manual_gain;
	u32		pics_per_file;
	u16		CCD_x_phys_e;
	u16		CCD_y_phys_e;
	bool	Camera_Thread_running;
	CString pathname;
	u16     bin;
	u16*    memory;
	double  savetime;
	u32     expt_frame;
	double  expt_time;
	double  time_now;
	bool    Ampl_Conv;
	//u32     Ampl_Setting;
	u32     Ampl_Setting_old;
	u32     CameraExpTime_ms;
	int		EM_GAIN;
	bool    Andor_new;
	int		roi_length;
	int focusType;
	int KSeriesLength_frames;
	int Acq_Timings;

	bool        ROI_changed;
	COctopusROI ROI_actual;
	COctopusROI ROI_target;
	COctopusROI Focus_ROI;
	bool        Focus_ROI_Set;

	//let's try this again
	bool SetFocus;
	bool moveG;
	u16  pictype;
	u16  mask_now;
	u16  mask_next;
	u16  lines_mm;

	//Shutter and filter
	u8      nd_setting;
	bool    load_wheel_failed;
	u8      filter_wheel;
	bool    program_running;
	
	//autofocus
	bool    focus_in_progress;
	double  focus_score;
	double	focus_beadX;
	double	focus_beadY;
	double	focus_beadX_LM;
	double	focus_beadY_LM;
	double  focus_beadX_sd;
	double  focus_beadY_sd; 
	double  focus_bead_fwhm; 
	double	focus_min;
	double	focus_max;
	double  CurrFocusScore;
	//Camera Autofocus Globals
	double g_mean;
	double g_max;
	double g_min;
	double g_focus_score;


	//Canera ROI Globals
	int roi_width;
	int roi_height;
	double roi_max;
	double roi_min;
	double roi_mean;
	double roi_focus_score;
	u16 roi_beadX;
	u16 roi_beadY;
	double roi_cuthigh;
	double roi_xmove;
	double roi_ymove;

	//Camera Bead Globals
	u16 bead_x1;
	u16 bead_x2;
	u16 bead_y1;
	u16 bead_y2;
	u16 bead_max;
	u16 bead_min;
	double bead_mean;

	//Camera LM Globals
	double LM_X;
	double LM_Y;
	double LM_X_sd;
	double LM_Y_sd;
	double LM_fwhm;

	//piezo and scope
	double  position_x;
	double  position_y;
	double  position_z;

	double  grat_position_z;

	double  position_x_volt;
	double  position_y_volt;
	double  position_z_volt;

	////polarizer
	//float  position_degrees_B;
	//float  position_degrees_T;
	//bool   POL_loaded;
	//bool   POL_scan_in_progress;

	double ADC_1;

	bool    LED_loaded;

	bool Laser_561_is_On;
	bool Laser_488_is_On;
	bool Laser_405_is_On;
	bool Laser_639_is_On;
	bool Lasers_loaded;

	bool XCite_loaded;
	bool Cleaning;

	//AOTF
	bool AOTF_loaded;
	bool AOTF_running;

};



#endif