#pragma once

#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MIOstream.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>

class SetBubblesCmd : public MPxCommand {
public:
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;

    // try fetching args
    MString particleSystemName = args.asString(0, &status);
    int frameNumber = args.asInt(1, &status);
    if (status.error()) {
      MGlobal::displayInfo(MString("invalid arguments for SetBubbles: ") + status.errorString());
      return status;
    }

    MSelectionList sList;
    status = sList.add(particleSystemName);
    if (status.error()) {
      MGlobal::displayInfo(MString(particleSystemName + " is not in the scene: ") + status.errorString());
      return status;
    }

    MObject particleObj;
    sList.getDependNode(0, particleObj);
    if (std::string(particleObj.apiTypeStr()) != std::string("kTransform")) {
      MGlobal::displayInfo(particleObj.apiTypeStr());
      MGlobal::displayInfo(MString(particleSystemName + " is not the root node of a particle system."));
      return status;
    }

    MFnTransform particleTransform(particleObj);
    MObject shape = particleTransform.child(0);
    MFnParticleSystem particleShape;
    particleShape.setObject(shape);

    cout << "Particle count = " << particleShape.count() << endl;

    MVectorArray particlePositions;
    particleShape.position(particlePositions);
    MDoubleArray particleRadii;
    particleShape.radius(particleRadii);

    for (int i = 0; i < particleRadii.length(); ++i) {
      MVector pos = particlePositions[i];
      cout << "particle " << i
           << ": radius = " << particleRadii[i]
           << ", position = (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << endl;
    }

    return status;
  }

  static void* creator() {
    return new SetBubblesCmd;
  }
};