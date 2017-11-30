#pragma once

#include "driverlog.h"

#include <osvr/ClientKit/ClientKit.h>

#include <chrono>
#include <windows.h>

#include <openvr_driver.h>
using namespace vr;

class HMD: public ITrackedDeviceServerDriver, public IVRDisplayComponent
{
public:
	HMD(std::string root_node);
	~HMD();
	virtual void update();
	static void PoseCallback(void* callback_data /* is a pointer to the HMD the callback is for */, const OSVR_TimeValue* time, const OSVR_PoseReport* report);

	// Inherited via ITrackedDeviceServerDriver
	virtual EVRInitError Activate(uint32_t unObjectId) override;
	virtual void Deactivate() override;
	virtual void EnterStandby() override;
	virtual void * GetComponent(const char * pchComponentNameAndVersion) override;
	virtual void DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) override;
	virtual DriverPose_t GetPose() override;

	// Inherited via IVRDisplayComponent
	virtual void GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual bool IsDisplayOnDesktop() override;
	virtual bool IsDisplayRealDisplay() override;
	virtual void GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) override;
	virtual void GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) override;
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) override;

private:
	uint32_t object_id_;
	int window_width_ = 1280;
	int window_height_ = 720;
	std::string root_node_;
	DriverPose_t hmd_pose_;

	std::unique_ptr<osvr::clientkit::ClientContext> osvr_context_;
	osvr::clientkit::Interface i_pose_;
	std::chrono::milliseconds previous_pose_timestamp_;
	OSVR_PoseState previous_pose_;

};

