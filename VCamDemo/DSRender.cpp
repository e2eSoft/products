#include "stdafx.h"
#include <string>
#include "DSRender.h"
#include <Audioclient.h>

#pragma comment(lib, "Strmiids.lib")

// VCam renderer interface
#import "VCamRenderer.tlb" no_namespace, raw_interfaces_only exclude("UINT_PTR") 

static const GUID CLSID_LavVideoDecoder =
{ 0xEE30215D, 0x164F, 0x4A92,{ 0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F } };

static const GUID CLSID_LavSplitter =
{ 0x171252A0, 0x8820, 0x4AFE,{ 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x04 } };

// Retrieve available camera device
void CDeviceEnumerator::Enumerate()
{
	// Create the System Device Enumerator.
	CComPtr<ICreateDevEnum> create_dev_enum;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&create_dev_enum);
	if (create_dev_enum == nullptr)
		return;

	// Create an enumerator for the category.
	CComPtr<IEnumMoniker> enum_moniker;
	create_dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
	if (enum_moniker == nullptr)
		return;

	// Enumerate the monikers.
	CComPtr<IMoniker> moniker;
	ULONG fetched = 0;
	HRESULT hr = S_OK;

	while (enum_moniker->Next(1, &moniker, &fetched) == S_OK) {
		std::unique_ptr<CDevice> device(new CDevice());
		moniker->QueryInterface(&device->m_moniker);
		moniker = nullptr;

		CComPtr<IPropertyBag> prop_bag;
		if (SUCCEEDED(device->m_moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&prop_bag))) {
			VARIANT var_friendly_name; VariantInit(&var_friendly_name);
			if (SUCCEEDED(prop_bag->Read(L"FriendlyName", &var_friendly_name, 0))) {
				device->m_friendly_name = var_friendly_name.bstrVal;
				VariantClear(&var_friendly_name);
			}
		}

		if (device->m_moniker) {
			if (!IsVCamDevice(device.get()))
				m_devices.push_back(std::move(device));
			else
				m_vcam.push_back(std::move(device));
		}
	}
}

// Create camera device
void CDeviceEnumerator::CreateDevice(const CDeviceEnumerator::CDevice* i_device, IBaseFilter** o_filter)
{
	if (i_device->m_moniker == nullptr)
		return;

	IBindCtx* pbc = nullptr;
	CreateBindCtx(0, &pbc);
	HRESULT hr = i_device->m_moniker->BindToObject(pbc, 0, IID_IBaseFilter, (void**)o_filter);
	if (pbc) pbc->Release();
}

// Check if it's VCam
bool CDeviceEnumerator::IsVCamDevice(const CDeviceEnumerator::CDevice* i_device)
{
	// {BC919FA0-33B3-40FB-BDA3-621DECF9EE33}
	static const GUID VCAMSDK_CONTROL = { 0xbc919fa0, 0x33b3, 0x40fb,{ 0xbd, 0xa3, 0x62, 0x1d, 0xec, 0xf9, 0xee, 0x33 } };

	CComPtr<IBaseFilter> filter;
	CreateDevice(i_device, &filter);

	if (filter) {
		CComPtr<IKsPropertySet> property_set;
		filter->QueryInterface(IID_IKsPropertySet, (void **)&property_set);
		if (property_set) {
			DWORD type_supported = 0;
			HRESULT hr = property_set->QuerySupported(VCAMSDK_CONTROL, 0, &type_supported);
			if (NOERROR == hr) {
				if (type_supported & KSPROPERTY_SUPPORT_SET) {
					return true;
				}
			}
		}
	}

	return false;
}

// Release resource
void CDeviceEnumerator::Cleanup()
{
	for (auto& it : m_devices) {
		it.reset(nullptr);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDSRender
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDSRender::NukeDownstream(IGraphBuilder* i_graph_builder, IBaseFilter* i_filter)
{
	if (i_graph_builder == nullptr || i_filter == nullptr)
		return;

	CComPtr<IEnumPins> pins;
	HRESULT hr = i_filter->EnumPins(&pins);
	if (pins == nullptr)
		return;

	hr = pins->Reset();

	while (NOERROR == hr) {
		CComPtr<IPin> pin; ULONG fetched = 0;
		hr = pins->Next(1, &pin, &fetched);

		if (hr == S_OK && pin) {
			CComPtr<IPin> pin_to;
			pin->ConnectedTo(&pin_to);

			if (pin_to) {
				PIN_INFO pininfo;
				ZeroMemory(&pininfo, sizeof(pininfo));
				hr = pin_to->QueryPinInfo(&pininfo);

				if (hr == NOERROR) {
					if (pininfo.dir == PINDIR_INPUT) {
						if (pininfo.pFilter) {
							NukeDownstream(i_graph_builder, pininfo.pFilter);
						}

						i_graph_builder->Disconnect(pin_to);
						i_graph_builder->Disconnect(pin);

						i_graph_builder->RemoveFilter(pininfo.pFilter);
					}

					if (pininfo.pFilter) pininfo.pFilter->Release();
				}
			}

			// release the pin has been fetched
			pin = nullptr;
		}
	}
}

void CDSRender::GetFilterPin(IBaseFilter* i_filter, DWORD i_index, PIN_DIRECTION i_direction, IPin** o_pin)
{
	CComPtr<IEnumPins> pins;
	i_filter->EnumPins(&pins);

	pins->Reset();

	CComPtr<IPin> pin;
	ULONG fetched = 0;

	while (SUCCEEDED(pins->Next(1, &pin, &fetched))) {
		if (fetched == 0)
			break;

		PIN_DIRECTION dir = PINDIR_INPUT;
		pin->QueryDirection(&dir);
		if (dir == i_direction) {
			if (i_index == 0)
				break;

			--i_index;
		}

		pin = nullptr;
	}

	if (pin) pin->QueryInterface(o_pin);
}

LRESULT CDSRender::Run()
{
	LRESULT hr = E_FAIL;

	if (m_media_control) {
		hr = m_media_control->Run();
	}

	return hr;
}

LRESULT CDSRender::Stop()
{
	if (m_media_control) {
		return m_media_control->Stop();
	}

	return E_FAIL;
}

LRESULT CDSRender::Pause()
{
	if (m_media_control) {
		return m_media_control->Pause();
	}

	return E_FAIL;
}

LRESULT CDSRender::Cleanup()
{
	LRESULT hr = S_OK;

	bool is_stopping = false;

	if (m_media_control) {
		hr = m_media_control->Stop();
		is_stopping = true;
	}

	if (m_source_filter) {
		NukeDownstream(m_graph_builder, m_source_filter);
		m_graph_builder->RemoveFilter(m_source_filter);
	}

	if (m_source_filter) m_source_filter->Release(), m_source_filter = nullptr;
	if (m_render_video_filter) m_render_video_filter->Release(), m_render_video_filter = nullptr;
	
	if (m_media_seeking) m_media_seeking->Release(), m_media_seeking = nullptr;	
	if (m_media_position) m_media_position->Release(), m_media_position = nullptr;
	if (m_media_control) m_media_control->Release(), m_media_control = nullptr;

	if (m_graph_builder) m_graph_builder->Release(), m_graph_builder = nullptr;

	return hr;
}

LRESULT CDSRender::Seek(float i_milliseconds)
{
	LRESULT hr = E_FAIL;
	
	if (m_media_position) {
		REFTIME seconds = i_milliseconds;
		if (seconds >= m_duration) {
			seconds = 0;
		}

		// convert to seconds
		seconds /= 1000.0f;
		hr = m_media_position->put_CurrentPosition(seconds);
	}

	return hr;
}


bool CDSRender::OpenDevice(const CDeviceEnumerator::CDevice* i_device)
{
	HRESULT hr = S_OK;
	m_duration = -1;

	// open camera device
	CDeviceEnumerator::CreateDevice(i_device, &m_source_filter);
	if (m_source_filter == nullptr)
		return false;
	
	// create IGraphBuilder instance
	if (FAILED(hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_graph_builder))))
		return false;
	
	if (FAILED(hr = m_graph_builder->AddFilter(m_source_filter, L"Source Filter")))
		return false;
	
	// Get IBaseFilter from VCam Render
	if (FAILED(hr = m_render->QueryInterface(&m_render_video_filter))) 
		return false;

	// Add VCam Render Filterer to graph
	if (FAILED(hr = m_graph_builder->AddFilter(m_render_video_filter, L"VCam Renderer Filter"))) {
		if (m_render_video_filter) m_render_video_filter->Release(), m_render_video_filter = nullptr;
		return false;
	}
	
	// Connect source filter with VCam Render Filter, the Render Filter will push video frames to VCam
	if (!RenderFilter(m_source_filter))
		return false;

	if (!GetControls())
		return false;
	
	return true;
}

bool CDSRender::OpenFile(const wchar_t * i_file_name)
{
	HRESULT hr = S_OK;
	m_duration = -1;

	// create IGraphBuilder instance
	if (FAILED(hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_graph_builder))))
		return false;

	// Get IBaseFilter from VCam Render
	if (FAILED(hr = m_render->QueryInterface(&m_render_video_filter)))
		return false;

	// Add VCam Renderer Filter to graph, and it will push video frames to VCam
	// Source Filter's Video Output Pin will connect to this filter
	if (FAILED(hr = m_graph_builder->AddFilter(m_render_video_filter, L"VCam Renderer Filter"))) {
		if (m_render_video_filter) m_render_video_filter->Release(), m_render_video_filter = nullptr;
		return false;
	}

	// get IFilterGraph2 interface
	CComPtr<IFilterGraph2> filter_graph;
	m_graph_builder->QueryInterface(IID_PPV_ARGS(&filter_graph));
	if (nullptr == filter_graph)
		return false;

	// add source filter for file
	m_graph_builder->AddSourceFilter(i_file_name, nullptr, &m_source_filter);
	if (m_source_filter == nullptr)
		return false;

	// LAV video decoder is preferred
	CComPtr<IBaseFilter> lav_video_decoder_filter;
	hr = CoCreateInstance(CLSID_LavVideoDecoder, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&lav_video_decoder_filter);	
	if (lav_video_decoder_filter) 
		m_graph_builder->AddFilter(lav_video_decoder_filter, L"LAV Video Decoder");

	// LAV splitter is preferred
	CComPtr<IBaseFilter> lav_splitter = nullptr;
	hr = CoCreateInstance(CLSID_LavSplitter, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&lav_splitter);
	if (lav_splitter)
		m_graph_builder->AddFilter(lav_splitter, L"LAV Splitter");

	// get all output pins of source filter
	CComPtr<IEnumPins> pins;
	if (FAILED(m_source_filter->EnumPins(&pins)))
		return false;

	// reset enumerator
	if (FAILED(pins->Reset()))
		return false;

	// go through all output pins and render them
	long succeeded_pin_num = 0;
	CComPtr<IPin> pin;
	ULONG fetched = 0;

	while (SUCCEEDED(pins->Next(1, &pin, &fetched))) {
		if (fetched == 0)
			break;

		PIN_DIRECTION dir = PINDIR_INPUT;
		pin->QueryDirection(&dir);
		if (dir == PINDIR_OUTPUT) {
			if (SUCCEEDED(filter_graph->RenderEx(pin, 0, nullptr))) {
				succeeded_pin_num++;
			}
		}

		pin = nullptr;
	}

	// check if there is any pin connected
	if (succeeded_pin_num == 0)
		return false;

	// if LAVFilters are not used, remove them
	if (lav_splitter) {
		if (!IsFilterConnected(lav_splitter))
			m_graph_builder->RemoveFilter(lav_splitter);
	}

	if (lav_video_decoder_filter) {
		if (!IsFilterConnected(lav_video_decoder_filter))
			m_graph_builder->RemoveFilter(lav_video_decoder_filter);
	}

	lav_video_decoder_filter = nullptr;
	lav_splitter = nullptr;

	hr = m_graph_builder->QueryInterface(IID_IMediaSeeking, (void**)&m_media_seeking);

	if (!GetControls())
		return false;

	return true;
}

bool CDSRender::RenderFilter(IBaseFilter* i_source_filter)
{
	HRESULT hr = S_OK;

	// get first output pin of device filter
	CComPtr<IPin> pin_output_device;
	GetFilterPin(i_source_filter, 0, PIN_DIRECTION::PINDIR_OUTPUT, &pin_output_device);
	if (pin_output_device == nullptr)
		return false;

	// get first input pin of renderer filter
	CComPtr<IPin> pin_input_render;
	GetFilterPin(m_render_video_filter, 0, PIN_DIRECTION::PINDIR_INPUT, &pin_input_render);
	if (pin_input_render == nullptr)
		return false;

	// connect source filter with color space filter directly
	if (FAILED(hr = m_graph_builder->Render(pin_output_device)))
		return false;

	return true;
}

bool CDSRender::IsFilterConnected(IBaseFilter* i_filter)
{
	// get all output pins of source filter
	CComPtr<IEnumPins> pins;
	if (FAILED(i_filter->EnumPins(&pins)))
		return false;

	// reset enumerator
	if (FAILED(pins->Reset()))
		return false;

	// go through all output pins and render them
	long succeeded_pin_num = 0;
	CComPtr<IPin> pin = nullptr;
	ULONG fetched = 0;

	while (SUCCEEDED(pins->Next(1, &pin, &fetched))) {
		if (fetched == 0)
			break;

		CComPtr<IPin> pin_connect_to;
		pin->ConnectedTo(&pin_connect_to);
		if (pin_connect_to) {
			return true;
		}

		pin = nullptr;
	}

	return false;
}

bool CDSRender::GetControls()
{
	HRESULT hr = S_OK;

	hr = m_graph_builder->QueryInterface(IID_IMediaPosition, (void **)&m_media_position);
	if (FAILED(hr) || m_media_position == nullptr)
		return false;

	hr = m_graph_builder->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&m_media_control));
	if (FAILED(hr) || m_media_control == nullptr)
		return false;
	
	if (m_media_position) {
		// receives the total stream length, in seconds
		REFTIME tm_duration = 0; m_media_position->get_Duration(&tm_duration);
		m_duration = tm_duration * 1000.0f;
	}

	return true;
}

double CDSRender::GetPositoin()
{
	double position = 0;
	if (m_media_seeking) {
		REFERENCE_TIME time_now = 0;
		if (SUCCEEDED(m_media_seeking->GetCurrentPosition(&time_now))) {
			position = time_now / 10000.0f;
		}
	}
	return position;
}