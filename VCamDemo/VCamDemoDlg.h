
// VCamDemoDlg.h : header file
//

#pragma once
#include "DSRender.h"

// CVCamDemoDlg dialog
class CVCamDemoDlg : public CDialogEx
{
protected:
	CDeviceEnumerator::CDevice* m_current_vcam = nullptr;
	ULONG_PTR			m_gdiplusToken;
	CDeviceEnumerator	m_input_devices;
	IBaseFilter*		m_vcam_renderer = nullptr;
	IVCamRenderer*		m_vcam = nullptr;	
	CDSRender			m_player;
	CSliderCtrl			m_video_progress;
	UINT				m_timer = 0;
	LONG				m_notification_monitor = FALSE;
	CString				m_cs_duration;
	HANDLE				m_thread = nullptr;
	
// Construction
public:
	CVCamDemoDlg(CWnd* pParent = NULL);	// standard constructor
	
	HRESULT SetupCameras();
	void CleanCameras();
	
	void DetectVCamUsage();
	static DWORD WINAPI notification_usage__proc(__inout LPVOID pv);
	void usage_proc();
	void ShowUsingInfo();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VCAMDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeComboVcam();
	afx_msg void OnBnClickedButtonTakephoto();
	afx_msg void OnBnClickedButtonBrowseVideo();
	afx_msg void OnCbnSelchangeComboFillmodes();
	afx_msg void OnBnClickedButtonBrowseImage();
	afx_msg void OnBnClickedButtonLicense();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonBrowseIdle();
	afx_msg void OnBnClickedButtonOutput();
	afx_msg void OnBnClickedButtonPlayDevice();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStopDevice();
	afx_msg void OnBnClickedButtonStopVideo();
	afx_msg void OnBnClickedButtonScreen();
	afx_msg void OnBnClickedButtonFriendlyName();
	afx_msg void OnBnClickedCheckMirror();
	afx_msg void OnBnClickedCheckFlip();
	afx_msg void OnBnClickedCheckRotate();
};
