#if !defined(AFX_H_OctopusCamera)
#define AFX_H_OctopusCamera

#include "stdafx.h"
#include "OctopusGlobals.h"

class COctopusCamera : public CDialog
{

public:
	
	COctopusCamera(CWnd* pParent = NULL);
	virtual ~COctopusCamera();

	enum { IDD = IDC_CAMERA };

	void SetROI_To_Default( void );

	void StartSaving( void );
	void StopSaving( void );

	void StartMovie( void );
	void StopCameraThread( void );

	void TakePicture( void );
	void TakeKineticPicture( void);
	void FileClose( void );

	u32 GetExposureTime_Single_ms( void ) { return u32_Exposure_Time_Single_ms; };
	void SetExposureTime_Single_ms( int u32_Exposure_Time_Single_ms );
	void SetExposureTime_Movie_ms( int u32_Exposure_Time_Movie_ms );
	void SetCameraGain( int u32_EMCCDGain );
	void SetMovieGain( int u32_EMCCDGain );
	void SetNKinetics( int u16_Kinetic_Series_Length_frames );

protected:

	void SetROI( void );
	void BinChange( void ); 
	void StartMovie(  float exptime_ms, u8 gain );

	//void StepGrating( void );

	CBitmap m_bmp_yes;
	CBitmap m_bmp_no;

	CStatic	m_Temperature_Now;
	CStatic m_status_save;

	CStatic m_g1_text1;
	CStatic m_g1_text2;

	CButton m_ctlFTCheckBox;
	CButton m_ctlTTLCheckBox;
	CButton m_ctlSRCheckBox;
	CButton m_ctlALCheckBox;
	CButton m_ctlRAWCheckBox;

	bool FrameTransfer;
	bool TTL;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	void OpenWindow( void );
	
	CString pathname_display;	
	CString debug;

	HANDLE memhandle;

	afx_msg void OnSetPath();

	int m_DISPLAY_GAIN_CHOICE;
	int m_DISPLAY_BIN_CHOICE;
	int m_DISPLAY_VSSPEED;
	int m_DISPLAY_HSSPEED;
	int m_DISPLAY_GL;
	int m_DISPLAY_PIC;
	int m_DISPLAY_LMM;
	
	double m_IDC_MANUAL_GAIN;
	
	u32 m_IDC_PICTURES_PER_FILE;
	u32 m_IDC_SKIP;

	u32 u32_Exposure_Time_Single_ms;
	u32 u32_Exposure_Time_Movie_ms;
	int u16_kinetic_series_length_frames;

	int  u16_Gain_Mult_Factor_Single;
	int  u16_Gain_Mult_Factor_Movie;

	int m_CCD_target_temp;
	int m_CCD_current_temp;

	u16 em_limit; 

    void Opencam( void );
	
	void EnableDlg( void );
	void DisableDlg( void );
	void DisableDlgMovie( void );
	void OnFileChange( void );
	void OnFocusChange( void );

	afx_msg void OnBinning1x1( void );
	afx_msg void OnBinning2x2( void );
	afx_msg void OnBinning4x4( void );
	afx_msg void OnBinning8x8( void );
		
	afx_msg void OnVSSPEED_0( void );
	afx_msg void OnVSSPEED_1( void );
	afx_msg void OnVSSPEED_2( void );
	afx_msg void OnVSSPEED_3( void );
	afx_msg void OnVSSPEED_4( void );

	void OnHSSPEED_0( void );
	void OnHSSPEED_1( void );
	void OnHSSPEED_2( void );
	void OnHSSPEED_3( void );
	void OnHSSPEED_4( void );
	void OnHSSPEED_5( void );

	afx_msg void OnGL_0( void );
	afx_msg void OnGL_1( void );
	afx_msg void OnGL_2( void );

	afx_msg void OnPIC_0( void );
	afx_msg void OnPIC_1( void );
	afx_msg void OnPIC_2( void );

	afx_msg void OnLMM_0( void );
	afx_msg void OnLMM_1( void );
	afx_msg void OnLMM_2( void );
	afx_msg void OnLMM_3( void );

	void OnVSSPEED_Report( void );
	void OnHSSPEED_Report( void );

	afx_msg void OnResizeWindow();
	afx_msg void OnKillfocusGeneral();
	afx_msg void OnKillfocusTempTarget();
	afx_msg void OnKillfocusManualGain();
	afx_msg void OnKillfocusPicturesPerFile();
	afx_msg void OnDisplayGainManual();
	afx_msg void OnDisplayGainAutomatic();

	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()

	void StartWaitThread( void );  //Starts the thread to wait for SDK events
	void KillWaitThread( void );   //Terminates the SDK event thread

	HANDLE WaitThread;
	
public:

	afx_msg void OnBnClickedCasFt();
	afx_msg void OnBnClickedCasTTL();
	afx_msg void OnBnClickedCasSR();
	afx_msg void OnBnClickedCasAL();
	afx_msg void OnBnClickedCasRAW();
	afx_msg void OnBnClickedCheck1();
	int roi_length;
	afx_msg void OnBnClickedSetroi();
private:
	
	bool focus;
public:
	afx_msg void OnBnClickedGeneralFocus();
	void StartPicture();
	double m_focusInterval;
	int totalSteps;
	int numStep;
	int bestFocusScoreStep;
	afx_msg void OnEnChangeFocusEdit();
	CString m_focusResult;
	int m_focusType;
	afx_msg void OnStnClickedStaticGain2();
	afx_msg void OnEnChangeCasPicturesPerFile();
	afx_msg void OnEnChangeCasTempTarget();
	afx_msg void OnStnClickedCasTempCurrent();
};

#endif
