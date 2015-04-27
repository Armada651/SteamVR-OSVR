/** @file
    @brief OSVR driver provider

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2015 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_driver_osvr_cpp_GUID_ECDDF736_3E03_4CB2_8D91_5E89646BD6A3
#define INCLUDED_driver_osvr_cpp_GUID_ECDDF736_3E03_4CB2_8D91_5E89646BD6A3

// Internal Includes
#include "ihmddriverprovider.h"
#include "ihmddriver.h"
#include "steamvr.h"

#include "osvr_compiler_detection.h"
#include "osvr_hmd.h"

#include "stringhasprefix.h"
#include "osvr_dll_export.h"

// Library/third-party includes
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>

// Standard includes
#include <vector>
#include <cstring>

class CDriver_OSVR : public vr::IHmdDriverProvider {
public:
	/**
	 * Initializes the driver.
	 *
	 * This is called when the driver is first loaded.
	 *
	 * @param user_config_dir the absoluate path of the directory where the
	 *     driver should store any user configuration files.
	 * @param driver_install_dir the absolute path of the driver's root
	 *     directory.
	 *
	 * If Init() returns anything other than \c HmdError_None the driver will be
	 * unloaded.
	 *
	 * @returns HmdError_None on success.
	 */
	virtual vr::HmdError Init(const char* pchUserConfigDir, const char* pchDriverInstallDir) OSVR_OVERRIDE;

	/**
	 * Performs any cleanup prior to the driver being unloaded.
	 */
	virtual void Cleanup() OSVR_OVERRIDE;

	/**
	 * Returns the number of detect HMDs.
	 */
	virtual uint32_t GetHmdCount() OSVR_OVERRIDE;

	/**
	 * Returns a single HMD by its index.
	 *
	 * @param index the index of the HMD to return.
	 */
	virtual vr::IHmdDriver* GetHmd(uint32_t index) OSVR_OVERRIDE;

	/**
	 * Returns a single HMD by its name.
	 *
	 * @param hmd_id the C string name of the HMD.
	 */
	virtual vr::IHmdDriver* FindHmd(const char* hmd_id) OSVR_OVERRIDE;

private:
	std::vector<std::unique_ptr<OSVRHmd>> hmds_;
	std::unique_ptr<osvr::clientkit::ClientContext> context_;
};

CDriver_OSVR g_driverOSVR;

void hmdTrackerCallback(void* /*userdata*/, const OSVR_TimeValue* /*timestamp*/, const OSVR_PoseReport* report) {
	std::cout << "Got POSE report: Position = ("
		<< report->pose.translation.data[0] << ", "
		<< report->pose.translation.data[1] << ", "
		<< report->pose.translation.data[2] << "), orientation = ("
		<< osvrQuatGetW(&(report->pose.rotation)) << ", ("
		<< osvrQuatGetX(&(report->pose.rotation)) << ", "
		<< osvrQuatGetY(&(report->pose.rotation)) << ", "
		<< osvrQuatGetZ(&(report->pose.rotation)) << "))" << std::endl;
	// FIXME TODO do something useful with this data!
}


vr::HmdError CDriver_OSVR::Init(const char* pchUserConfigDir, const char* pchDriverInstallDir)
{
	context_ = new osvr::clientkit::Context("com.osvr.SteamVR");
	osvr::clientkit::Interface display = context->getInterface("/display");
	display.registerCallback(&hmdTrackerCallback, NULL);
	context->update(); // FIXME move elsewhere and loop

	return vr::HmdError_None;
}

void CDriver_OSVR::Cleanup()
{
	// do nothing
}

uint32_t CDriver_OSVR::GetHmdCount()
{
	return hmds_.size();
}

vr::IHmdDriver* CDriver_OSVR::GetHmd(uint32_t index)
{
	if (index >= hmds_.size())
		return NULL;

	return hmds_[index].get();
}

vr::IHmdDriver* CDriver_OSVR::FindHmd(const char* hmd_id)
{
	for (auto& hmd : hmds_) {
		if (!std::strcmp(hmd_id, hmd->GetId()))
			return hmd.get();
	}

	return NULL;
}


static const char* IHmdDriverProvider_Prefix = "IHmdDriverProvider_";

OSVR_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	if (!StringHasPrefix(pInterfaceName, IHmdDriverProvider_Prefix)) {
		*pReturnCode = vr::HmdError_Init_InvalidInterface;
		return NULL;
	}

	if (0 != strcmp(vr::IHmdDriverProvider_Version, pInterfaceName)) {
		if (pReturnCode)
			*pReturnCode = vr::HmdError_Init_InterfaceNotFound;
		return NULL;
	}

	return &g_driverOSVR;
}

#endif // INCLUDED_driver_osvr_cpp_GUID_ECDDF736_3E03_4CB2_8D91_5E89646BD6A3
