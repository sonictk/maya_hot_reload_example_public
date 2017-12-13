#include "deformer.h"
#include "logic.h"
#include "deformer_platform.h"
#include <ssmath/common_math.h>
#include <maya/MPoint.h>
#include <maya/MGlobal.h>


void *HotReloadableDeformer::creator()
{
	return new HotReloadableDeformer;
}


void HotReloadableDeformer::postConstructor()
{
	LibraryStatus result = loadDeformerLogicDLL(kLogicLibrary);
	if (result != LibraryStatus_Success) {
		MGlobal::displayError("Failed to load shared library!");
		return;
	}

	return;
}


MStatus HotReloadableDeformer::initialize()
{
	MStatus result;

	attributeAffects(envelope, outputGeom);

	return result;
}


MStatus HotReloadableDeformer::deform(MDataBlock &block,
									  MItGeometry &iter,
									  const MMatrix &matrix,
									  unsigned int multiIndex)
{
	LibraryStatus status;

	if (!kLogicLibrary.isValid) {
#ifdef _DEBUG_MODE
		MGlobal::displayError("The logic DLL is not valid, attempting reload!");
#endif
		// NOTE: (sonictk) Just in case, we make sure the library is unloaded
		unloadDeformerLogicDLL(kLogicLibrary);
		status = loadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
			return MStatus::kFailure;
		}
	}

	// NOTE: (yliangsiew) Find the last modified time of the DLL and check if
	// there is a newer version; if so, unload the existing DLL and load the new
	// one, then fix up the function pointers again
	if (kPluginLogicLibraryPath.numChars() == 0) {
		return MStatus::kFailure;
	}

	// NOTE: (sonictk) We only reload the DLL *if* the DLL actually exists; this
	// is so we can rename the DLL on Windows to avoid having the DLL handle be locked.
	FileTime lastModified = getLastWriteTime(kPluginLogicLibraryPath.asChar());
	if (lastModified >= 0 && lastModified != kLogicLibrary.lastModified) {
#ifdef _DEBUG_MODE
		MGlobal::displayInfo("DEBUG: Reloading logic DLL...");
#endif // _DEBUG_MODE
		status = unloadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
#ifdef _DEBUG_MODE
			MGlobal::displayError("Unable to unload logic library!");
#endif
			return MStatus::kFailure;
		}
		status = loadDeformerLogicDLL(kLogicLibrary);
		if (status != LibraryStatus_Success) {
#ifdef _DEBUG_MODE
			MGlobal::displayError("Unable to load logic library!");
#endif
			return MStatus::kFailure;
		}
	}

	MStatus result;

	// NOTE: (yliangsiew) Simple example function code here
	MDataHandle envelopeHandle = block.inputValue(envelope, &result);
	CHECK_MSTATUS_AND_RETURN_IT(result);

	float envelope = envelopeHandle.asFloat();

	for (; !iter.isDone(); iter.next())
	{

		MPoint curPtPosPt = iter.position();
		Vec3 curPtPos = vec3((float)curPtPosPt.x, (float)curPtPosPt.y, (float)curPtPosPt.z);
		Vec3 finalPos = kLogicLibrary.deformCB(curPtPos, envelope);

		MPoint finalPosPt = MPoint(finalPos.x, finalPos.y, finalPos.z, 1);

		iter.setPosition(finalPosPt);
	}

	return result;
}
