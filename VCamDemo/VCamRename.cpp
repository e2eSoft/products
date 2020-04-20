#include "stdafx.h"
#include "VCamRename.h"

#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

struct __declspec(uuid("6BDD1FC6-810F-11D0-BEC7-08002BE2092F")) GUID_DEVINTERFACE_IMAGE
{
};

GUID MY_GUID_DEV_IMAGE = __uuidof(GUID_DEVINTERFACE_IMAGE);

VCamRename::VCamRename()
{
}

VCamRename::~VCamRename()
{
}

bool VCamRename::Create()
{
	HDEVINFO hdev = SetupDiGetClassDevs(&MY_GUID_DEV_IMAGE, NULL, 0, DIGCF_PRESENT);
	if (hdev == INVALID_HANDLE_VALUE)
		return false;

	SP_DEVINFO_DATA device_info_data = {};
	device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

	const std::wstring e2esoft = L"e2eSoft";
	const LONG capacity = 2048;
	WCHAR content[capacity];	
	
	for (DWORD i = 0; SetupDiEnumDeviceInfo(hdev, i, &device_info_data); i++)
	{
		DWORD data_type = 0, data_size = 0;
		
		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (!SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_MFG, &data_type, (PBYTE)content, capacity, &data_size))
			continue;

		if (_wcsicmp(content, e2esoft.c_str()) != 0)
			continue;

		// Allocate a camera instance
		std::unique_ptr<Camera> camera(new Camera());
		camera->m_index = i;
        
        ZeroMemory(content, sizeof(WCHAR) * capacity);
        if (SetupDiGetDeviceInstanceId(hdev, &device_info_data, content, MAX_PATH, NULL))
            camera->m_intance_id = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_CLASSGUID, &data_type, (PBYTE)content, capacity, &data_size))
			camera->m_classid = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_DEVICEDESC, &data_type, (PBYTE)content, capacity, &data_size))
			camera->m_description = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_FRIENDLYNAME, &data_type, (PBYTE)content, capacity, &data_size))
			camera->m_friendlyname = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_SERVICE, &data_type, (PBYTE)content, capacity, &data_size))
			camera->m_service = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_DRIVER, &data_type, (PBYTE)content, capacity, &data_size))
			camera->m_driver = content;

		camera->m_info = device_info_data;

		m_cameras.push_back(std::move(camera));
	}

	SetupDiDestroyDeviceInfoList(hdev);

	bool ret = !m_cameras.empty();
	return ret;
}

//
// 此函数更改【设备管理器】中的设备描述和显示名称，需管理员权限。函数并不更改DirectShow显示名。函数支持 Windows 2000及以上版本。
//
bool VCamRename::SetFriendlyName(LONG i_index, LPCWSTR i_new_name)
{
	bool ret_description = false, ret_friendly_name = false;

	if (i_index < 0 || i_index >= static_cast<LONG>(m_cameras.size()) || i_new_name == nullptr)
		return false;

	HDEVINFO hdev = SetupDiGetClassDevs(&MY_GUID_DEV_IMAGE, NULL, 0, DIGCF_PRESENT);
	if (hdev == INVALID_HANDLE_VALUE)
		return false;
	
	auto p_camera = m_cameras[i_index].get();
	
	SP_DEVINFO_DATA device_info_data = {};
	device_info_data.cbSize   = sizeof(SP_DEVINFO_DATA);
	device_info_data          = p_camera->m_info;
	
	if (SetupDiEnumDeviceInfo(hdev, p_camera->m_index, &device_info_data)) {
		// The caller of this function must be a member of the Administrators group.
		DWORD      new_name_data_len = static_cast<DWORD>(wcslen(i_new_name) + 1) * sizeof(WCHAR);
		DWORD      old_name_data_len = 0;
		
		const LONG capacity = 2048;
		WCHAR content[capacity] = {0};

        std::wstring instance_id;
        ZeroMemory(content, sizeof(WCHAR) * capacity);
        if (SetupDiGetDeviceInstanceId(hdev, &device_info_data, content, MAX_PATH, NULL))
            instance_id = content;

		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_DEVICEDESC, NULL, (PBYTE)content, capacity, &old_name_data_len)) {
			if (SetupDiSetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_DEVICEDESC, (const BYTE*)i_new_name, new_name_data_len)) {
				ret_description = true;
			}
		}
		
		ZeroMemory(content, sizeof(WCHAR) * capacity);
		if (SetupDiGetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_FRIENDLYNAME, NULL, (PBYTE)content, capacity, &old_name_data_len)) {
			if (SetupDiSetDeviceRegistryProperty(hdev, &device_info_data, SPDRP_FRIENDLYNAME, (const BYTE*)i_new_name, new_name_data_len)) {
				ret_friendly_name = true;
			}
		}
	}

	SetupDiDestroyDeviceInfoList(hdev);

	return ret_description || ret_friendly_name;
}
