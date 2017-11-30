#pragma once

#include "driverlog.h"

#include <functional>
#include <thread>
#include <memory>
#include <chrono>

#include <osvr/ClientKit/ClientKit.h>

#include <openvr_driver.h>
using namespace vr;

class Controller : public ITrackedDeviceServerDriver, public IVRControllerComponent
{
public:

	Controller(std::string root_node);
	virtual ~Controller();
	virtual uint32_t getObjectID();
	virtual std::string getSerial();
	virtual void update();

	/*
		Inherited from ITrackedDeviceServerDriver:
	*/

	EVRInitError Activate(uint32_t unObjectId) override;
	void Deactivate() override;
	void EnterStandby() override;
	void *GetComponent(const char* pchComponentNameAndVersion) override;
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
	DriverPose_t GetPose() override;

	/* 
		Inherited from IVRControllerComponent:
	*/

	VRControllerState_t GetControllerState() override;
	bool TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) override;

private:

	enum InterfaceType {
		TOUCH_X,
		TOUCH_Y,
		TRIGGER,
		POSE,
		TOUCH_TOUCHED,
		TOUCH_PRESSED,
		SYSTEM_PRESSED,
		GRIP_PRESSED,
		MENU_PRESSED,
	};
	struct CallbackData {
		Controller* context;
		InterfaceType type;
	};
	static void ButtonCallback(void* callback_data /* is of type CallbackData */, const OSVR_TimeValue* time, const OSVR_ButtonReport* report);
	static void AnalogCallback(void* callback_data /* is of type CallbackData */, const OSVR_TimeValue* time, const OSVR_AnalogReport* report);
	static void PoseCallback(void* callback_data /* is of type CallbackData */, const OSVR_TimeValue* time, const OSVR_PoseReport* report);
	
	VRControllerState_t controller_state_;
	DriverPose_t controller_pose_;
	uint32_t object_id_;
	std::string serial_;
	std::string root_node_;
	std::unique_ptr<osvr::clientkit::ClientContext> osvr_context_;

	// OSVR Controller Interfaces

	osvr::clientkit::Interface i_touch_x_;
	OSVR_AnalogState previous_touch_x_;
	osvr::clientkit::Interface i_touch_y_;
	OSVR_AnalogState previous_touch_y_;
	osvr::clientkit::Interface i_trigger_;
	OSVR_AnalogState previous_trigger_;

	osvr::clientkit::Interface i_touch_touched_;
	osvr::clientkit::Interface i_touch_pressed_;
	osvr::clientkit::Interface i_system_pressed_;
	osvr::clientkit::Interface i_grip_pressed_;
	osvr::clientkit::Interface i_menu_pressed_;

	osvr::clientkit::Interface i_pose_;
	std::chrono::milliseconds previous_pose_timestamp_;
	OSVR_PoseState previous_pose_;

};


