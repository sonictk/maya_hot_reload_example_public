#ifndef DEFORMER_H
#define DEFORMER_H

#include <maya/MPxGeometryFilter.h>
#include <maya/MItGeometry.h>
#include <maya/MGlobal.h>


static const MTypeId kHotReloadableDeformerID = 0x0008002E;
static const char *kHotReloadableDeformerName = "hotReloadableDeformer";



struct HotReloadableDeformer : MPxGeometryFilter
{
	static void *creator();

	void postConstructor();

	static MStatus initialize();

	MStatus deform(MDataBlock &block,
				   MItGeometry &iterator,
				   const MMatrix &matrix,
				   unsigned int multiIndex);
};

#endif /* DEFORMER_H */
