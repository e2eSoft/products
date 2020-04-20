
// VCamDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <memory>
#include "VCamDemo.h"
#include "VCamDemoDlg.h"
#include "afxdialogex.h"
#include "VCamRename.h"


// VCam renderer interface
#import "VCamRenderer.tlb" no_namespace, raw_interfaces_only exclude("UINT_PTR") 
#include "VCamRenderer_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL SaveImageAsBitmap24(const std::wstring& i_file_name, const char* i_pixels, int i_width, int i_height, int i_stride)
{
	int stride = ((i_width * 3) + 3) & ((LONG)~3);
	BITMAPINFOHEADER infoHeader = { 0 };
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = i_width;
	infoHeader.biHeight = i_height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biSizeImage = stride * i_height;

	int fileHeaderSize = sizeof(BITMAPFILEHEADER);
	int infoHeaderSize = infoHeader.biSize;
	int fileSize = fileHeaderSize + infoHeaderSize + infoHeader.biSizeImage;

	BITMAPFILEHEADER fileHeader = { 0 };
	fileHeader.bfType = 0x4d42;
	fileHeader.bfSize = fileSize;
	fileHeader.bfOffBits = fileHeaderSize + infoHeaderSize;

	HANDLE hFile = CreateFileW(i_file_name.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD num_of_bytes = 0;
	WriteFile(hFile, &fileHeader, sizeof(fileHeader), &num_of_bytes, nullptr);
	if (num_of_bytes < sizeof(fileHeader)) {
		CloseHandle(hFile);
		return FALSE;
	}

	WriteFile(hFile, &infoHeader, sizeof(infoHeader), &num_of_bytes, nullptr);
	if (num_of_bytes < sizeof(infoHeader)) {
		CloseHandle(hFile);
		return FALSE;
	}

	const char* p_line = i_pixels + (i_height - 1) * i_stride;
	for (int i = 0; i < i_height; i++) {
		WriteFile(hFile, p_line, i_stride, &num_of_bytes, nullptr);
		p_line -= i_stride;
	}
	
	CloseHandle(hFile);
	return TRUE;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVCamDemoDlg dialog



CVCamDemoDlg::CVCamDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VCAMDEMO_DIALOG, pParent)
	//, m_camera(nullptr)
	, m_gdiplusToken(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVCamDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_VIDEO, m_video_progress);
}

BEGIN_MESSAGE_MAP(CVCamDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO_VCAM,		&CVCamDemoDlg::OnCbnSelchangeComboVcam)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_VIDEO,	&CVCamDemoDlg::OnBnClickedButtonBrowseVideo)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_IMAGE,	&CVCamDemoDlg::OnBnClickedButtonBrowseImage)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR,			&CVCamDemoDlg::OnBnClickedButtonClear)
	ON_CBN_SELCHANGE(IDC_COMBO_FILLMODES,	&CVCamDemoDlg::OnCbnSelchangeComboFillmodes)
	ON_BN_CLICKED(IDC_BUTTON_LICENSE,		&CVCamDemoDlg::OnBnClickedButtonLicense)
	ON_WM_CLOSE()
	ON_WM_DESTROY()	
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_IDLE,	&CVCamDemoDlg::OnBnClickedButtonBrowseIdle)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT,		&CVCamDemoDlg::OnBnClickedButtonOutput)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_DEVICE,	&CVCamDemoDlg::OnBnClickedButtonPlayDevice)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_STOP_DEVICE,	&CVCamDemoDlg::OnBnClickedButtonStopDevice)
	ON_BN_CLICKED(IDC_BUTTON_STOP_VIDEO,	&CVCamDemoDlg::OnBnClickedButtonStopVideo)
	ON_BN_CLICKED(IDC_BUTTON_SCREEN,		&CVCamDemoDlg::OnBnClickedButtonScreen)
	ON_BN_CLICKED(IDC_BUTTON_TAKEPHOTO,		&CVCamDemoDlg::OnBnClickedButtonTakephoto)
	ON_BN_CLICKED(IDC_BUTTON_FRIENDLY_NAME, &CVCamDemoDlg::OnBnClickedButtonFriendlyName)
	ON_BN_CLICKED(IDC_CHECK_MIRROR,			&CVCamDemoDlg::OnBnClickedCheckMirror)
	ON_BN_CLICKED(IDC_CHECK_FLIP,			&CVCamDemoDlg::OnBnClickedCheckFlip)
	ON_BN_CLICKED(IDC_CHECK_ROTATE,			&CVCamDemoDlg::OnBnClickedCheckRotate)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CVCamDemoDlg message handlers

BOOL CVCamDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_video_progress.SetRangeMin(0);

	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_ADDSTRING, 0, (LPARAM)_T("Aspect Fit"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_ADDSTRING, 0, (LPARAM)_T("Aspect Fill"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_ADDSTRING, 0, (LPARAM)_T("Stretch"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_SETCURSEL, 0, (LPARAM)0);

	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_ADDSTRING, 0, (LPARAM)_T(" 320 x 240"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_ADDSTRING, 0, (LPARAM)_T(" 640 x 360"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_ADDSTRING, 0, (LPARAM)_T(" 640 x 480"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_ADDSTRING, 0, (LPARAM)_T("1280 x 720"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_ADDSTRING, 0, (LPARAM)_T("1920 x 1080"));
	::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_SETCURSEL, 2, (LPARAM)0);

	// set default FPS for UI
	SetDlgItemInt(IDC_EDIT_FPS, 25, TRUE);
	SetDlgItemInt(IDC_EDIT_SCREEN_X,   0, FALSE);
	SetDlgItemInt(IDC_EDIT_SCREEN_Y,   0, FALSE);
	SetDlgItemInt(IDC_EDIT_SCREEN_W, 640, FALSE);
	SetDlgItemInt(IDC_EDIT_SCREEN_H, 480, FALSE);

	// initialize Gdiplus
	Gdiplus::GdiplusStartupInput GdiPlusStartupInput;
	Gdiplus::Status Ret = GdiplusStartup(&m_gdiplusToken, &GdiPlusStartupInput, 0);
	
	// Test code
	//VCamRename::GetInstance()->Create();
	//VCamRename::GetInstance()->SetFriendlyName(0, L"mytestcamera");

	// initialize camera
	SetupCameras();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVCamDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVCamDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVCamDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
// When dialog quit, release camera resource
//
void CVCamDemoDlg::OnClose()
{
	CleanCameras();
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	CDialogEx::OnClose();
}

//
// When dialog quit, release camera resource
//
void CVCamDemoDlg::OnDestroy()
{
	CleanCameras();
	CDialogEx::OnDestroy();
}


//
// When dialog quit, release camera resource
//
void CVCamDemoDlg::CleanCameras()
{
	if (m_timer) {
		KillTimer(m_timer);
		m_timer = 0;
	}

	// set an Empty Event Handle for VCam usage 
	m_notification_monitor = FALSE;
	if (m_vcam)	m_vcam->SetConnectionNotificationEvent(reinterpret_cast<__int64>(nullptr));

	if (m_vcam_renderer) m_vcam_renderer->Release(), m_vcam_renderer = nullptr;
	if (m_vcam) m_vcam->Release(), m_vcam = nullptr;
	
	m_input_devices.Cleanup();
	m_player.Cleanup();
}

//
// Choose VCam instance
//
void CVCamDemoDlg::OnCbnSelchangeComboVcam()
{
	HWND hwnd_devices = ::GetDlgItem(m_hWnd, IDC_COMBO_VCAM);
	long sel = ::SendMessage(hwnd_devices, CB_GETCURSEL, 0, 0);
	if (sel != CB_ERR) {
		auto p_device = reinterpret_cast<CDeviceEnumerator::CDevice*>(::SendMessage(hwnd_devices, CB_GETITEMDATA, sel, 0));
		if (p_device == m_current_vcam || p_device == nullptr)
			return;

		// When set current VCam Device, make sure there is no video file or camera playing
		m_player.Cleanup();
		Sleep(1000);

		// quit previous vcam-using detect thread
		if (m_current_vcam) {
			// flag to quit detect thread
			m_notification_monitor = FALSE;

			// cancel previous event that has been set as detect VCam using
			if (m_vcam)	m_vcam->SetConnectionNotificationEvent(reinterpret_cast<__int64>(nullptr));
			if (m_thread) {
				WaitForSingleObject(m_thread, INFINITE);
				CloseHandle(m_thread);
				m_thread = nullptr;
			}
		}

		// set VCam instance we want to use
		if (m_current_vcam = p_device) {
			m_vcam->SetCurrentDevice((BSTR)m_current_vcam->m_friendly_name.c_str());
			SetDlgItemText(IDC_EDIT_FRIENDLY_NAME, m_current_vcam->m_friendly_name.c_str());
			DetectVCamUsage();
		}
	}
}

//
// Play a camera device
//
void CVCamDemoDlg::OnBnClickedButtonPlayDevice()
{
	if (m_timer) {
		KillTimer(m_timer);
		m_timer = 0;
	}

	m_player.Cleanup();

	HWND hwnd_devices = ::GetDlgItem(m_hWnd, IDC_COMBO_DEVICE);
	long sel = ::SendMessage(hwnd_devices, CB_GETCURSEL, 0, 0);
	if (sel != CB_ERR) {
		auto p_device = reinterpret_cast<CDeviceEnumerator::CDevice*>(::SendMessage(hwnd_devices, CB_GETITEMDATA, sel, 0));
		if (m_player.OpenDevice(p_device)) {
			m_player.Run();

			auto p_button = GetDlgItem(IDC_BUTTON_STOP_DEVICE);
			p_button->EnableWindow(TRUE);

			p_button = GetDlgItem(IDC_BUTTON_PLAY_DEVICE);
			p_button->EnableWindow(FALSE);

			p_button = GetDlgItem(IDC_BUTTON_TAKEPHOTO);
			p_button->EnableWindow(TRUE);
		}
	}
}

//
// Stop camera playing
//
void CVCamDemoDlg::OnBnClickedButtonStopDevice()
{
	if (m_vcam) {
		m_player.Stop();

		auto p_button = GetDlgItem(IDC_BUTTON_STOP_DEVICE);
		p_button->EnableWindow(FALSE);

		p_button = GetDlgItem(IDC_BUTTON_PLAY_DEVICE);
		p_button->EnableWindow(TRUE);

		p_button = GetDlgItem(IDC_BUTTON_TAKEPHOTO);
		p_button->EnableWindow(FALSE);
	}
}

//
// Play a video file
//
void CVCamDemoDlg::OnBnClickedButtonBrowseVideo()
{
	if (m_timer) {
		KillTimer(m_timer);
		m_timer = 0;
	}

	m_player.Cleanup();
	
	std::wstring filter = L"Video files|*.mp4;*.mkv;*.mov;*.rmvb;*.mpg;*.avi|All files|*.*||";
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, filter.c_str());
	if (IDOK == fileDialog.DoModal()) {
		std::wstring filename = fileDialog.GetPathName();
		SetDlgItemText(IDC_EDIT_VIDEO_FILENAME, fileDialog.GetFileName());
		if (m_vcam) {
			if (m_player.OpenFile(filename.c_str())) {
				int all_seconds = static_cast<int>(m_player.GetDuration() / 1000.0f);
				int hour = (int)(all_seconds / (3600));
				int minutes = (int)((all_seconds / 60) - (hour * 60));
				int seconds = (int)((all_seconds)- (minutes * 60) - (hour * 3600));
				
				m_cs_duration.Format(L" / %02d:%02d:%02d", hour, minutes, seconds);
				
				CString content; 
				content.Format(L"%02d:%02d:%02d%s", 0, 0, 0, m_cs_duration);

				SetDlgItemText(IDC_STATIC_DURATION, content);

				m_video_progress.SetRangeMax(all_seconds, TRUE);
				m_video_progress.EnableWindow(TRUE);
				m_player.Run();

				auto p_button = GetDlgItem(IDC_BUTTON_STOP_VIDEO);
				p_button->EnableWindow(TRUE);

				p_button = GetDlgItem(IDC_BUTTON_BROWSE_VIDEO);
				p_button->EnableWindow(FALSE);

				p_button = GetDlgItem(IDC_BUTTON_TAKEPHOTO);
				p_button->EnableWindow(TRUE);

				// create a timer to update current position
				m_timer = SetTimer(1, 1000, nullptr);
			}
		}
	}
}

//
// Stop playing Video
//
void CVCamDemoDlg::OnBnClickedButtonStopVideo()
{
	if (m_vcam) {
		m_player.Stop();

		m_video_progress.EnableWindow(FALSE);

		auto p_button = GetDlgItem(IDC_BUTTON_STOP_VIDEO);
		p_button->EnableWindow(FALSE);

		p_button = GetDlgItem(IDC_BUTTON_BROWSE_VIDEO);
		p_button->EnableWindow(TRUE);

		p_button = GetDlgItem(IDC_BUTTON_TAKEPHOTO);
		p_button->EnableWindow(FALSE);

		SetDlgItemText(IDC_STATIC_DURATION, L"");

		if (m_timer) {
			KillTimer(m_timer);
			m_timer = 0;
		}
	}
}

//
// Initialize virtual camera
//
HRESULT CVCamDemoDlg::SetupCameras()
{
	HRESULT hr = ::CoInitialize(nullptr);

	// Create VCam renderer filter
	if (FAILED(hr = CoCreateInstance(CLSID_VCamRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter,
		reinterpret_cast<void**>(&m_vcam_renderer)))) {
		MessageBox(_T("VCamRenderer filter not registered!"), _T("Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	
	// get [IVCamRender] interface from VCam Renderer filter
	if (FAILED(hr = m_vcam_renderer->QueryInterface(&m_vcam))) {
		MessageBox(_T("VCam Driver not installed!"), _T("Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	// pass it to player
	m_player.SetVCamRender(m_vcam);

	// list available video capture devices
	m_input_devices.Enumerate();
	HWND hwnd_devices = ::GetDlgItem(m_hWnd, IDC_COMBO_DEVICE);
	for (auto& it : m_input_devices.m_devices) {
		int idx = static_cast<int>(::SendMessage(hwnd_devices, CB_ADDSTRING, 0, (LPARAM)it->m_friendly_name.c_str()));
		::SendMessage(hwnd_devices, CB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(it.get()));
	}

	if (!m_input_devices.m_devices.empty())
		::SendMessage(hwnd_devices, CB_SETCURSEL, 0, (LPARAM)0);

	// list multiple VCam source Device
	hwnd_devices = ::GetDlgItem(m_hWnd, IDC_COMBO_VCAM);
	for (auto& it : m_input_devices.m_vcam) {
		int idx = static_cast<int>(::SendMessage(hwnd_devices, CB_ADDSTRING, 0, (LPARAM)it->m_friendly_name.c_str()));
		::SendMessage(hwnd_devices, CB_SETITEMDATA, idx, reinterpret_cast<LPARAM>(it.get()));
		if (m_current_vcam == nullptr) {
			m_current_vcam = it.get();
			SetDlgItemText(IDC_EDIT_FRIENDLY_NAME, m_current_vcam->m_friendly_name.c_str());
		}
	}

	if (!m_input_devices.m_vcam.empty())
		::SendMessage(hwnd_devices, CB_SETCURSEL, 0, (LPARAM)0);
	
	::EnableWindow(hwnd_devices, m_input_devices.m_vcam.size() <= 1 ? FALSE : TRUE);

	// create a thread to detect VCam usage.
	if (m_vcam) {
		long default_filling_mode = -1;
		m_vcam->GetFillMode(&default_filling_mode);
		if (default_filling_mode >= 0) 
			::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_SETCURSEL, default_filling_mode, (LPARAM)0);

		DetectVCamUsage();
	}


	return hr;
}

//
// Display an image file
//
void CVCamDemoDlg::OnBnClickedButtonBrowseImage()
{
	std::wstring filter = L"Image files|*.jpg; *.jpeg; *.png; *.gif; *.bmp||";
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, filter.c_str());
	if (IDOK == fileDialog.DoModal()) {
		std::wstring filename = fileDialog.GetPathName();
		SetDlgItemText(IDC_EDIT_IMAGE_FILENAME, fileDialog.GetFileName());
				
		Gdiplus::Bitmap bitmap((LPCWSTR)filename.c_str());
		Gdiplus::BitmapData bits;

		if (Gdiplus::Ok == bitmap.LockBits(NULL, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bits)) {
			if (m_vcam) {
				// height < 0: pixels from bottom to top
				// height > 0: picels from top to bottom
				int height = bitmap.GetHeight();
				m_vcam->SendFrame((LPBYTE)bits.Scan0, bitmap.GetWidth(), - height, bits.Stride);
			}

			bitmap.UnlockBits(&bits);
		}
	}	
}


//
// Display idle image
//
void CVCamDemoDlg::OnBnClickedButtonClear()
{
	if (m_vcam) {
		m_vcam->SendFrame(nullptr, 0, 0, 0);
	}
}

//
// Capture a ROI of screen
//
void CVCamDemoDlg::OnBnClickedButtonScreen()
{
	if (m_vcam) {
		int x = GetDlgItemInt(IDC_EDIT_SCREEN_X, nullptr, FALSE);
		int y = GetDlgItemInt(IDC_EDIT_SCREEN_Y, nullptr, FALSE);
		int w = GetDlgItemInt(IDC_EDIT_SCREEN_W, nullptr, FALSE);
		int h = GetDlgItemInt(IDC_EDIT_SCREEN_H, nullptr, FALSE);

		m_vcam->CaptureScreen(x, y, w, h);
	}
}

//
// Set an image as Idle image file
//
void CVCamDemoDlg::OnBnClickedButtonBrowseIdle()
{
	if (m_vcam == nullptr)
		return;

	// create a Folder to save idle file
	std::wstring idle_file_save_path;
	WCHAR app_data_folder[256];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data_folder))) {
		idle_file_save_path = app_data_folder;
		idle_file_save_path += L"\\e2eSoft\\VCamSDK";
		SHCreateDirectory(nullptr, idle_file_save_path.c_str());
		idle_file_save_path += L"\\vcam_idle.bmp";
	}

	std::wstring filter = L"Image files|*.jpg; *.jpeg; *.png; *.bmp";
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, filter.c_str());
	if (IDOK == fileDialog.DoModal()) {
		std::wstring filename = fileDialog.GetPathName();
		SetDlgItemText(IDC_EDIT_IDLE_FILENAME, fileDialog.GetFileName());

		Gdiplus::Bitmap bitmap((LPCWSTR)filename.c_str());
		Gdiplus::BitmapData bits;

		if (Gdiplus::Ok == bitmap.LockBits(NULL, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bits)) {
			// save idle file, idle file must be 24 bits RGB bitmap
			if (SaveImageAsBitmap24(idle_file_save_path, reinterpret_cast<char*>(bits.Scan0), 
				bitmap.GetWidth(), bitmap.GetHeight(), bits.Stride)) 
			{
				m_vcam->SetIdleFileName((BSTR)idle_file_save_path.c_str());
			}
			bitmap.UnlockBits(&bits);
		}
	}
	else {
		// restore default Idle image
		m_vcam->SetIdleFileName(_T(""));
	}
}


//
// Change filling mode
//
void CVCamDemoDlg::OnCbnSelchangeComboFillmodes()
{
	// Apply new fill mode
	long i_fill_mode = ::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_FILLMODES), CB_GETCURSEL, 0, 0);
	if (i_fill_mode != CB_ERR) {
		i_fill_mode = std::max<long>(0, std::min<long>(2, i_fill_mode));
		if (m_vcam) {
			m_vcam->SetFillMode(i_fill_mode);
		}
	}
}

//
// Set license code
//
void CVCamDemoDlg::OnBnClickedButtonLicense()
{
	if (m_vcam == nullptr)
		return;

	CString license_code; GetDlgItemText(IDC_EDIT_LICENSE, license_code);
	if (!license_code.IsEmpty()) {
		HRESULT hr = m_vcam->SetLicenseCode((BSTR)(LPCWSTR)license_code);
	}
}


void CVCamDemoDlg::OnBnClickedButtonOutput()
{
	if (m_vcam == nullptr)
		return;

	long sel = ::SendMessage(::GetDlgItem(m_hWnd, IDC_COMBO_OUTPUT), CB_GETCURSEL, 0, 0);
	long width, height;
	switch (sel)
	{
	case 0:
		width = 320; height = 240;
		break;
	case 1:
		width = 640; height = 360;
		break;
	case 3:
		width = 1280; height = 720;
		break;
	case 4:
		width = 1920; height = 1080;
		break;
	case 2:
	default:
		width = 640; height = 480;
		break;
	}
	int fps = GetDlgItemInt(IDC_EDIT_FPS, nullptr, TRUE);
	m_vcam->SetOutputFormat(width, height, fps);
}

void CVCamDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int pos = m_video_progress.GetPos();
	float position = pos * 1000.0f;
	m_player.Seek(position);

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVCamDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	double milliseconds = m_player.GetPositoin();
	int pos = static_cast<int>(milliseconds / 1000.0f);
	m_video_progress.SetPos(pos);
	
	int hour = (int)(pos / (3600));
	int minutes = (int)((pos / 60) - (hour * 60));
	int seconds = (int)((pos)-(minutes * 60) - (hour * 3600));
	
	static WCHAR content[128];
	swprintf_s(content, 128, L"%02d:%02d:%02d%s", hour, minutes, seconds, (LPCWSTR)m_cs_duration);
	SetDlgItemText(IDC_STATIC_DURATION, content);

	CDialogEx::OnTimer(nIDEvent);
}

void CVCamDemoDlg::DetectVCamUsage()
{
	ShowUsingInfo();

	// create a thread to detect vcam usage
	m_notification_monitor = TRUE;
	m_thread = CreateThread(nullptr, 0, notification_usage__proc, this, 0, nullptr);
}

DWORD WINAPI CVCamDemoDlg::notification_usage__proc(LPVOID data)
{
	CVCamDemoDlg* impl = reinterpret_cast<CVCamDemoDlg*>(data);
	impl->usage_proc();
	return 0;
}

void CVCamDemoDlg::usage_proc()
{
	// create a event and set it to VCam, when VCam usage number is changed, this event will be notified
	HANDLE h_notification = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (FAILED(m_vcam->SetConnectionNotificationEvent(reinterpret_cast<__int64>(h_notification)))) {
		CloseHandle(h_notification);
		h_notification = nullptr;
		return;
	}

	while (m_notification_monitor) {
		// wait for the notification event
		WaitForSingleObject(h_notification, INFINITE);

		// update using information
		if (m_notification_monitor)
			ShowUsingInfo();
	}
}

void CVCamDemoDlg::ShowUsingInfo()
{
	if (m_vcam == nullptr)
		return;

	long connected_num = 0;
	if (m_vcam) {
		m_vcam->GetConnectedCount(&connected_num);
	}

	CString tt = _T("No application is ");
	if (connected_num == 1) tt = _T("1 application is ");
	else if (connected_num > 1) tt.Format(_T("%d applications are "), connected_num);

	CString message = tt + _T("using VCam.");
	SetDlgItemText(IDC_STATIC_VCAM_USAGE, message);
}

void CVCamDemoDlg::OnBnClickedButtonTakephoto()
{
	// set snapshot file name
	std::wstring snapshot_file_name;
	WCHAR picture_folder[256] = { 0 };
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, picture_folder))) {
		snapshot_file_name = picture_folder;
		snapshot_file_name += L"\\snapshot.bmp";
	}

	if (m_vcam) {
		// take snapshot on next frame
		m_vcam->Snapshot((BSTR)snapshot_file_name.c_str());

		// open folder and locate saved file
		StringCchPrintf(picture_folder, 256, _T("/e,/select,%s"), snapshot_file_name.c_str());
		SHELLEXECUTEINFO shex = { 0 };
		shex.cbSize = sizeof(SHELLEXECUTEINFO);
		shex.lpFile = _T("explorer.exe");
		shex.lpParameters = picture_folder;
		shex.lpVerb = _T("open");
		shex.nShow = SW_SHOWDEFAULT;
		shex.lpDirectory = NULL;
		ShellExecuteEx(&shex);
	}
}


void CVCamDemoDlg::OnBnClickedButtonFriendlyName()
{
	CString friendly_name; GetDlgItemText(IDC_EDIT_FRIENDLY_NAME, friendly_name);
	if (!friendly_name.IsEmpty()) {
		HRESULT hr = m_vcam->SetFriendlyName((BSTR)(LPCWSTR)friendly_name);
	}
}

void CVCamDemoDlg::OnBnClickedCheckMirror()
{
	LONG mirror = (IsDlgButtonChecked(IDC_CHECK_MIRROR) & BST_CHECKED) ? 1 : 0;
	if (m_vcam) 
		m_vcam->SetMirror(mirror);
}


void CVCamDemoDlg::OnBnClickedCheckFlip()
{
	LONG flip = (IsDlgButtonChecked(IDC_CHECK_FLIP) & BST_CHECKED) ? 1 : 0;
	if (m_vcam)
		m_vcam->SetFlip(flip);
}


void CVCamDemoDlg::OnBnClickedCheckRotate()
{
	LONG rotate = (IsDlgButtonChecked(IDC_CHECK_ROTATE) & BST_CHECKED) ? 1 : 0;
	if (m_vcam)
		m_vcam->SetRotateRight(rotate);
}
