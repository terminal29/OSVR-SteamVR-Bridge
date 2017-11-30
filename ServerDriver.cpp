#include "ServerDriver.h"

ServerDriver::ServerDriver()
{
	
	
	
}

ServerDriver::~ServerDriver()
{
}

EVRInitError ServerDriver::Init(IVRDriverContext * pDriverContext)
{
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

		running_ = true;
		
		InitDriverLog(vr::VRDriverLog());
		DriverLog("===============================================================================\n");
		DriverLog("============================= OSVR STEAMVR BRIDGE =============================\n");
		DriverLog("===============================================================================\n");
		
		DriverLog("Starting updater thread...!\n");

		updater_thread_ = std::thread([&]() {
		
			Controller* con1 = new Controller("controller1");
			Controller* con2 = new Controller("controller2");

			HMD* hmd1 = new HMD("hmd");

			VRServerDriverHost()->TrackedDeviceAdded("controller1", vr::TrackedDeviceClass_Controller, con1);
			VRServerDriverHost()->TrackedDeviceAdded("controller2", vr::TrackedDeviceClass_Controller, con2);

			VRServerDriverHost()->TrackedDeviceAdded("hmd", vr::TrackedDeviceClass_HMD, hmd1);

			while (running_) {
				con1->update();
				con2->update();
				hmd1->update();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		});
		


		DriverLog("Done!\n");
		DriverLog("===============================================================================\n");
	return EVRInitError::VRInitError_None;
}

void ServerDriver::Cleanup()
{
	// TODO
	running_ = false;
	updater_thread_.join();
}

const char * const * ServerDriver::GetInterfaceVersions()
{
	return k_InterfaceVersions;
}

void ServerDriver::RunFrame()
{
	
	
}

bool ServerDriver::ShouldBlockStandbyMode()
{
	return false;
}

void ServerDriver::EnterStandby()
{
}

void ServerDriver::LeaveStandby()
{
}
