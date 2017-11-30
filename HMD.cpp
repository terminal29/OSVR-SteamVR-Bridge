#include "HMD.h"



HMD::HMD(std::string root_node):
	root_node_(root_node)
{
	osvr_context_ = std::unique_ptr<osvr::clientkit::ClientContext>(new osvr::clientkit::ClientContext(osvrClientInit(std::string("net.inf.osb_" + root_node).c_str())));

	while (!osvr_context_->checkStatus()) {
		osvr_context_->update();
	}

	DriverLog(std::string("HMD " + root_node_ + " Connected to OSVR!\n").c_str());
	i_pose_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/pose").c_str());
	i_pose_.registerCallback(HMD::PoseCallback, this);

}


HMD::~HMD()
{

}

void HMD::update() {
	if (osvr_context_.get() != nullptr) {
		osvr_context_->update();
	}
	
}

void HMD::PoseCallback(void * callback_data, const OSVR_TimeValue * time, const OSVR_PoseReport * report)
{
	HMD* self = (HMD*)callback_data;
	OSVR_PoseState new_pose = report->pose;

	// Update pose timestamp so we can work out velocity
	std::chrono::milliseconds time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	std::chrono::milliseconds time_delta = time_now - self->previous_pose_timestamp_;
	self->previous_pose_timestamp_ = time_now;

	// OSVR PoseState only contains position and rotations so we have to work out linear velocity ourselves
	self->hmd_pose_.vecVelocity[0] = (new_pose.translation.data[0] - self->previous_pose_.translation.data[0]) * (time_delta.count() / 1000.0);
	self->hmd_pose_.vecVelocity[1] = (new_pose.translation.data[1] - self->previous_pose_.translation.data[1]) * (time_delta.count() / 1000.0);
	self->hmd_pose_.vecVelocity[2] = (new_pose.translation.data[2] - self->previous_pose_.translation.data[2]) * (time_delta.count() / 1000.0);

	// Copy translation and rotation data
	std::copy(new_pose.translation.data, new_pose.translation.data + 3, self->hmd_pose_.vecPosition);
	self->hmd_pose_.vecPosition[1] += 1;
	self->hmd_pose_.qRotation.w = new_pose.rotation.data[0];
	self->hmd_pose_.qRotation.x = new_pose.rotation.data[1];
	self->hmd_pose_.qRotation.y = new_pose.rotation.data[2];
	self->hmd_pose_.qRotation.z = new_pose.rotation.data[3];

	// Update SteamVR
	VRServerDriverHost()->TrackedDevicePoseUpdated(self->object_id_, self->hmd_pose_, sizeof(DriverPose_t));

	// Save pose for next callback
	std::memcpy(&(self->previous_pose_), &new_pose, sizeof(OSVR_PoseState));
	
}

EVRInitError HMD::Activate(uint32_t unObjectId)
{
	object_id_ = unObjectId;
	PropertyContainerHandle_t prop_handle = VRProperties()->TrackedDeviceToPropertyContainer(object_id_);
	VRProperties()->SetFloatProperty(prop_handle, Prop_UserIpdMeters_Float, 0.065f);
	VRProperties()->SetFloatProperty(prop_handle, Prop_UserHeadToEyeDepthMeters_Float, 0.f);
	VRProperties()->SetFloatProperty(prop_handle, Prop_DisplayFrequency_Float, 90.f);
	VRProperties()->SetFloatProperty(prop_handle, Prop_SecondsFromVsyncToPhotons_Float, 0.1f);
	VRProperties()->SetUint64Property(prop_handle, Prop_CurrentUniverseId_Uint64, 2);
	VRProperties()->SetBoolProperty(prop_handle, Prop_IsOnDesktop_Bool, false);

	hmd_pose_.poseIsValid = true;
	hmd_pose_.result = TrackingResult_Running_OK;
	hmd_pose_.deviceIsConnected = true;

	hmd_pose_.qWorldFromDriverRotation = { 1, 0, 0, 0 };
	hmd_pose_.qDriverFromHeadRotation = { 1, 0, 0, 0 };

	return EVRInitError::VRInitError_None;
}

void HMD::Deactivate()
{
}

void HMD::EnterStandby()
{
}

void * HMD::GetComponent(const char * pchComponentNameAndVersion)
{
	if (0 == strcmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
	{
		return (vr::IVRDisplayComponent*)this;
	}

	return NULL;
}

void HMD::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

DriverPose_t HMD::GetPose()
{
	return hmd_pose_;
}

void HMD::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	*pnX = 1366;
	*pnY = 0;
	*pnWidth = window_width_;
	*pnHeight = window_height_;
}

bool HMD::IsDisplayOnDesktop()
{
	return true;
}

bool HMD::IsDisplayRealDisplay()
{
	return true;
}

void HMD::GetRecommendedRenderTargetSize(uint32_t * pnWidth, uint32_t * pnHeight)
{
	*pnWidth = window_width_;
	*pnHeight = window_height_;
}

void HMD::GetEyeOutputViewport(EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight)
{
	float offset = 75;

	*pnY = 0;
	*pnWidth = (window_width_ / 2) - offset;
	*pnHeight = window_height_;

	if (eEye == Eye_Left)
	{
		*pnX = offset;
	}
	else
	{
		*pnX = window_width_ / 2;
	}
}

void HMD::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom)
{
	*pfLeft = -1.0;
	*pfRight = 1.0;
	*pfTop = -1.0;
	*pfBottom = 1.0;
}

DistortionCoordinates_t HMD::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	vr::DistortionCoordinates_t oDistortion{};
	oDistortion.rfBlue[0] = fU;
	oDistortion.rfBlue[1] = fV;
	oDistortion.rfGreen[0] = fU;
	oDistortion.rfGreen[1] = fV;
	oDistortion.rfRed[0] = fU;
	oDistortion.rfRed[1] = fV;
	return oDistortion;

}
