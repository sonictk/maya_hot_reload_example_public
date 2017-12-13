#include "plugin_main.h"
#include <maya/MFnPlugin.h>

const char *kAUTHOR = "Siew Yi Liang";
const char *kVERSION = "1.0.0";
const char *kREQUIRED_API_VERSION = "Any";


MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, kAUTHOR, kVERSION, kREQUIRED_API_VERSION);

	// NOTE: (yliangsiew) Get the OS-specific path to the plugin
	MString pluginPath = plugin.loadPath();
	const char *pluginPathC = pluginPath.asChar();
	const sizet lenPluginPath = strlen(pluginPathC);
	char OSPluginPath[kMaxPathLen];
	strncpy(OSPluginPath, pluginPathC, lenPluginPath + 1);
	int replaced = convertPathSeparatorsToOSNative(OSPluginPath);
	if (replaced < 0) {
		MGlobal::displayError("Failed to format path of plugin to OS native version!");
		return MStatus::kFailure;
	}
	if (strlen(OSPluginPath) <= 0) {
		MGlobal::displayError("Could not find a path to the plugin!");
		return MStatus::kFailure;
	}

	kPluginLogicLibraryPath = getDeformerLogicLibraryPath(OSPluginPath);

	status = plugin.registerNode(kHotReloadableDeformerName,
								 kHotReloadableDeformerID,
								 &HotReloadableDeformer::creator,
								 &HotReloadableDeformer::initialize,
								 MPxNode::kGeometryFilter);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	MStatus status;

	if (kLogicLibrary.isValid && kLogicLibrary.handle) {
		unloadDeformerLogicDLL(kLogicLibrary);
	}

	status =  plugin.deregisterNode(kHotReloadableDeformerID);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}
