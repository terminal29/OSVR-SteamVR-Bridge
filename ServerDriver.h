#pragma once

#include "driverlog.h"

#include <memory>
#include <algorithm>
#include <chrono>
using namespace std::chrono;

#include <osvr/ClientKit/Context.h>
#include <openvr_driver.h>
using namespace vr;

#include "Controller.h"
#include "HMD.h"

class ServerDriver : public IServerTrackedDeviceProvider
{
public:
	ServerDriver();

	~ServerDriver();

	EVRInitError Init(IVRDriverContext *pDriverContext) override;

	void Cleanup() override;

	const char * const *GetInterfaceVersions() override;

	void RunFrame() override;

	bool ShouldBlockStandbyMode() override;
    
	void EnterStandby() override;

	void LeaveStandby() override;

private:
	osvr::clientkit::ClientContext* osvr_client_context_{ nullptr };

	Controller* con1{ nullptr };
	Controller* con2{ nullptr };

	std::thread updater_thread_;

	bool running_;
};

