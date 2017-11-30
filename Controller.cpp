#include "Controller.h"


Controller::Controller(std::string root_node) :
	root_node_(root_node)
{
	serial_ = root_node;

	auto log_invalid_path = [=](osvr::clientkit::Interface& interface, std::string path_ext) {
		if (!interface.notEmpty()) {
			DriverLog(std::string("Path " + root_node_ + path_ext + " is invalid!\n").c_str());
		}
	};




	
	osvr_context_ = std::unique_ptr<osvr::clientkit::ClientContext>(new osvr::clientkit::ClientContext(osvrClientInit(std::string("net.inf.osb_" + root_node).c_str())));
	
	while (!osvr_context_->checkStatus()) {
		osvr_context_->update();
	}
	DriverLog(std::string("Controller " + root_node_ + " Connected to OSVR!\n").c_str());
	i_touch_x_ = osvr_context_->getInterface(std::string( "/" + root_node_ + "/touch_x").c_str());
	i_touch_x_.registerCallback(Controller::AnalogCallback, new CallbackData{ this, TOUCH_X });
	log_invalid_path(i_touch_x_, "/touch_x");
	
	i_touch_y_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/touch_y").c_str());
	i_touch_y_.registerCallback(Controller::AnalogCallback, new CallbackData{ this, TOUCH_Y });
	log_invalid_path(i_touch_y_, "/touch_y");

	i_trigger_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/trigger").c_str());
	i_trigger_.registerCallback(Controller::AnalogCallback, new CallbackData{ this, TRIGGER });
	log_invalid_path(i_trigger_, "/trigger");

	i_touch_touched_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/touch_touched").c_str());
	i_touch_touched_.registerCallback(Controller::ButtonCallback, new CallbackData{ this, TOUCH_TOUCHED });
	log_invalid_path(i_touch_touched_, "/touch_touched");

	i_touch_pressed_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/touch_pressed").c_str());
	i_touch_pressed_.registerCallback(Controller::ButtonCallback, new CallbackData{ this, TOUCH_PRESSED });
	log_invalid_path(i_touch_pressed_, "/touch_pressed");

	i_system_pressed_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/system_pressed").c_str());
	i_system_pressed_.registerCallback(Controller::ButtonCallback, new CallbackData{ this, SYSTEM_PRESSED });
	log_invalid_path(i_system_pressed_, "/system_pressed");

	i_menu_pressed_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/menu_pressed").c_str());
	i_menu_pressed_.registerCallback(Controller::ButtonCallback, new CallbackData{ this, MENU_PRESSED });
	log_invalid_path(i_menu_pressed_, "/menu_pressed");

	i_grip_pressed_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/grip_pressed").c_str());
	i_grip_pressed_.registerCallback(Controller::ButtonCallback, new CallbackData{ this, GRIP_PRESSED });
	log_invalid_path(i_grip_pressed_, "/grip_pressed");

	i_pose_ = osvr_context_->getInterface(std::string("/" + root_node_ + "/pose").c_str());
	i_pose_.registerCallback(Controller::PoseCallback, new CallbackData{ this, POSE });
	log_invalid_path(i_pose_, "/pose");
	

}

Controller::~Controller()
{
}


uint32_t Controller::getObjectID()
{
	return object_id_;
}

VRControllerState_t Controller::GetControllerState()
{
	return controller_state_;
}

DriverPose_t Controller::GetPose()
{
	return controller_pose_;
}

std::string Controller::getSerial() 
{
	return serial_;
}

void Controller::update()
{
	if (osvr_context_.get() != nullptr) {
		osvr_context_->update();
	}
}

void Controller::ButtonCallback(void * callback_data, const OSVR_TimeValue * time, const OSVR_ButtonReport * report)
{
	CallbackData data = *(CallbackData*)callback_data;
	Controller* self = data.context;
	InterfaceType type = (InterfaceType)data.type;

	self->controller_state_.unPacketNum++;
	EVRButtonId button;
	switch (type) {
	case TOUCH_PRESSED:
	case TOUCH_TOUCHED:
	{
		button = k_EButton_SteamVR_Touchpad;
	}break;
	case SYSTEM_PRESSED:
	{
		button = k_EButton_System;
	}break;
	case GRIP_PRESSED:
	{
		button = k_EButton_Grip;
	}break;
	case MENU_PRESSED:
	{
		button = k_EButton_ApplicationMenu;
	}break;
	default:
		// what?
		break;
	}
	if (type == TOUCH_TOUCHED) {
		if (report->state) {
			self->controller_state_.ulButtonTouched |= ButtonMaskFromId(button);
			VRServerDriverHost()->TrackedDeviceButtonTouched(self->object_id_, button, 0);
		}
		else {
			self->controller_state_.ulButtonTouched &= ~ButtonMaskFromId(button);
			VRServerDriverHost()->TrackedDeviceButtonUntouched(self->object_id_, button, 0);
		}
	}
	else {
		if (report->state) {
			self->controller_state_.ulButtonTouched |= ButtonMaskFromId(button);
			self->controller_state_.ulButtonPressed |= ButtonMaskFromId(button);
			VRServerDriverHost()->TrackedDeviceButtonTouched(self->object_id_, button, 0);
			VRServerDriverHost()->TrackedDeviceButtonPressed(self->object_id_, button, 0);
		}
		else {
			self->controller_state_.ulButtonTouched &= ~ButtonMaskFromId(button);
			self->controller_state_.ulButtonPressed &= ~ButtonMaskFromId(button);
			VRServerDriverHost()->TrackedDeviceButtonUnpressed(self->object_id_, button, 0);
			VRServerDriverHost()->TrackedDeviceButtonUntouched(self->object_id_, button, 0);
		}
	}
}

void Controller::AnalogCallback(void * callback_data, const OSVR_TimeValue * time, const OSVR_AnalogReport * report)
{
	CallbackData data = *(CallbackData*)callback_data;
	Controller* self = data.context;
	InterfaceType type = (InterfaceType)data.type;

	self->controller_state_.unPacketNum++;
	switch (type) {
	case TOUCH_X:
	{
		self->previous_touch_x_ = report->state;

		// convert from 0 - 255 to (-1,1)
		VRControllerAxis_t axis = { 2 * (self->previous_touch_x_ / 255.0) - 1, (2 * self->previous_touch_y_ / 255.0) - 1 };
		self->controller_state_.rAxis[0] = axis;
		VRServerDriverHost()->TrackedDeviceAxisUpdated(self->object_id_, 0, axis);
	}break;
	case TOUCH_Y:
	{
		self->previous_touch_y_ = report->state;

		// convert from 0 - 255 to (-1,1)
		VRControllerAxis_t axis = { 2 * (self->previous_touch_x_ / 255.0) - 1, (2 * self->previous_touch_y_ / 255.0) - 1 };
		self->controller_state_.rAxis[0] = axis;
		VRServerDriverHost()->TrackedDeviceAxisUpdated(self->object_id_, 0, axis);
	}break;
	case TRIGGER:
	{
		self->previous_trigger_ = report->state;
		VRControllerAxis_t trigger = { self->previous_trigger_ / 255, 0 };
		self->controller_state_.rAxis[1] = trigger;
		VRServerDriverHost()->TrackedDeviceAxisUpdated(self->object_id_, 1, trigger);
	}break;
	default:
		// ??
		break;
	}
}

void Controller::PoseCallback(void * callback_data, const OSVR_TimeValue * time, const OSVR_PoseReport * report)
{
	CallbackData data = *(CallbackData*)callback_data;
	Controller* self = data.context;
	InterfaceType type = (InterfaceType)data.type;
	if (type == POSE) {
		OSVR_PoseState new_pose = report->pose;

		// Update pose timestamp so we can work out velocity
		std::chrono::milliseconds time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds time_delta = time_now - self->previous_pose_timestamp_;
		self->previous_pose_timestamp_ = time_now;

		// OSVR PoseState only contains position and rotations so we have to work out linear velocity ourselves
		self->controller_pose_.vecVelocity[0] = (new_pose.translation.data[0] - self->previous_pose_.translation.data[0]) * (time_delta.count() / 1000.0);
		self->controller_pose_.vecVelocity[1] = (new_pose.translation.data[1] - self->previous_pose_.translation.data[1]) * (time_delta.count() / 1000.0);
		self->controller_pose_.vecVelocity[2] = (new_pose.translation.data[2] - self->previous_pose_.translation.data[2]) * (time_delta.count() / 1000.0);

		// Copy translation and rotation data
		std::copy(new_pose.translation.data, new_pose.translation.data + 3, self->controller_pose_.vecPosition);
		self->controller_pose_.vecPosition[1] += 1;
		self->controller_pose_.qRotation.w = new_pose.rotation.data[0];
		self->controller_pose_.qRotation.x = new_pose.rotation.data[1];
		self->controller_pose_.qRotation.y = new_pose.rotation.data[2];
		self->controller_pose_.qRotation.z = new_pose.rotation.data[3];

		// Update SteamVR
		VRServerDriverHost()->TrackedDevicePoseUpdated(self->object_id_, self->controller_pose_, sizeof(DriverPose_t));

		// Save pose for next callback
		std::memcpy(&(self->previous_pose_), &new_pose, sizeof(OSVR_PoseState));
	}
	else {
		// what?
	}
}


bool Controller::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
	return true;
}

EVRInitError Controller::Activate(uint32_t unObjectId)
{
	object_id_ = unObjectId;

	PropertyContainerHandle_t prop_handle = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

	VRProperties()->SetBoolProperty(prop_handle, vr::Prop_WillDriftInYaw_Bool, false);
	VRProperties()->SetBoolProperty(prop_handle, vr::Prop_DeviceIsWireless_Bool, true);
	VRProperties()->SetBoolProperty(prop_handle, vr::Prop_HasControllerComponent_Bool, true);

	VRProperties()->SetInt32Property(prop_handle, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
	VRProperties()->SetInt32Property(prop_handle, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);

	VRProperties()->SetStringProperty(prop_handle, Prop_SerialNumber_String, serial_.c_str());
	VRProperties()->SetStringProperty(prop_handle, Prop_ModelNumber_String, "Vive Controller MV");
	VRProperties()->SetStringProperty(prop_handle, Prop_RenderModelName_String, "vr_controller_vive_1_5");
	VRProperties()->SetStringProperty(prop_handle, Prop_ManufacturerName_String, "HTC");
	
	uint64_t available_buttons =	vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
									vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) |
									vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) |
									vr::ButtonMaskFromId(vr::k_EButton_System) |
									vr::ButtonMaskFromId(vr::k_EButton_Grip);

	VRProperties()->SetUint64Property(prop_handle, Prop_SupportedButtons_Uint64, available_buttons);

	controller_pose_.deviceIsConnected = true;
	controller_pose_.poseIsValid = true;
	controller_pose_.qDriverFromHeadRotation = { 1,0,0,0 };
	controller_pose_.qWorldFromDriverRotation = { 1,0,0,0 };
	controller_pose_.result = ETrackingResult::TrackingResult_Running_OK;
	
	return EVRInitError::VRInitError_None;
}

void Controller::Deactivate()
{
}

void Controller::EnterStandby()
{
}

void * Controller::GetComponent(const char * pchComponentNameAndVersion)
{
	if (0 == strcmp(IVRControllerComponent_Version, pchComponentNameAndVersion))
	{
		return (vr::IVRControllerComponent*)this;
	}

	return NULL;
}

void Controller::DebugRequest(const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}
