#pragma once

#include <strmif.h>
#include <dshow.h>
#include <string>
#include <vector>
#include <memory>

class CDeviceEnumerator
{
public:
	class CDevice
	{
	public:
		CDevice() {}
		CDevice(const CDevice&) = delete;
		CDevice& operator = (const CDevice&) = delete;

		virtual ~CDevice() {
			if (m_moniker) m_moniker->Release(), m_moniker = nullptr;
		}

	public:
		std::wstring	m_friendly_name;
		IMoniker*		m_moniker = nullptr;
	};

	CDeviceEnumerator() {}	
	virtual ~CDeviceEnumerator() {}	
	CDeviceEnumerator(const CDeviceEnumerator&) = delete; // disabled
	CDeviceEnumerator& operator = (const CDeviceEnumerator&) = delete; // disabled

	void Enumerate();
	
	static void CreateDevice(const CDevice*, IBaseFilter**);

	void Cleanup();

private:
	bool IsVCamDevice(const CDeviceEnumerator::CDevice* i_device);

public:
	std::vector<std::unique_ptr<CDevice>> m_devices;
	std::vector<std::unique_ptr<CDevice>> m_vcam;
};

struct IVCamRenderer;
class CDSRender
{
public:
	CDSRender() {}
	virtual ~CDSRender() {}

	void SetVCamRender(IVCamRenderer* i_render) { m_render = i_render; }
	double GetDuration() const { return m_duration; }
	double GetPositoin();

	bool OpenDevice(const CDeviceEnumerator::CDevice* i_video_device);
	bool OpenFile(const wchar_t * i_file_name);
	
	LRESULT Run();
	LRESULT Stop();
	LRESULT Pause();
	LRESULT Cleanup();
	LRESULT Seek(float i_milliseconds);
	
private:
	bool RenderFilter(IBaseFilter* i_video_device);
	bool GetControls();
	void NukeDownstream(IGraphBuilder* i_graph_builder, IBaseFilter* i_filter);
	void GetFilterPin(IBaseFilter* i_filter, DWORD i_index, PIN_DIRECTION i_direction, IPin**);
	bool IsFilterConnected(IBaseFilter* i_filter);

private:
	IVCamRenderer*						m_render = nullptr;
	IGraphBuilder*						m_graph_builder = nullptr;
	IMediaControl*						m_media_control = nullptr;
	IMediaPosition*						m_media_position = nullptr;
	IMediaSeeking*						m_media_seeking = nullptr;
	IBaseFilter*						m_render_video_filter = nullptr;
	IBaseFilter*						m_source_filter = nullptr;		
	double								m_duration = 0;
};

