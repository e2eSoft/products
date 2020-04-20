#pragma once

#include <string>
#include <vector>
#include <memory>

#include <SetupAPI.h>

class VCamRename
{
private:
	VCamRename();
	virtual ~VCamRename();

public:
	struct Camera
	{
		std::wstring	m_description;
		std::wstring	m_friendlyname;
		std::wstring	m_driver;
		std::wstring	m_service;
		std::wstring	m_classid;
        std::wstring    m_intance_id;
		SP_DEVINFO_DATA m_info;
		DWORD			m_index;
	};

	static VCamRename* GetInstance() {
		static VCamRename _instance;
		return &_instance;
	}

	bool Create();
	bool SetFriendlyName(LONG i_index, LPCWSTR);
	
	const std::vector<std::unique_ptr<Camera>>& GetCameras() const {
		return m_cameras;
	}

private:
	std::vector<std::unique_ptr<Camera>> m_cameras;
};

