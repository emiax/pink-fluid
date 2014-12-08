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
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>

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


    MObject particleObj = getParticleSystemByName(particleSystemName);
    MFnParticleSystem particleShape;
    particleShape.setObject(particleObj);
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

    MMatrix invFluidTransMat = getInverseFluidTransform();

    cout << "fluid inverse transform:" << endl;
    for (int j = 0; j < 4; ++j) {
      for (int i = 0; i < 4; ++i) {
        cout << invFluidTransMat(j, i) << "\t";
      }
      cout << endl;
    }

    return status;
  }

  MSelectionList select(MString name) {
    MSelectionList pList;
    pList.add(name);

    return pList;
  }

  MObject getParticleSystemByName(MString name) {
    MSelectionList pList = select(name);

    MObject particleObj;
    pList.getDependNode(0, particleObj);
    if (std::string(particleObj.apiTypeStr()) != std::string("kTransform")) {
      MGlobal::displayInfo(particleObj.apiTypeStr());
      MGlobal::displayInfo(MString(name + " is not the root node of a particle system."));
    }

    MFnTransform particleTransform(particleObj);
    MObject shape = particleTransform.child(0);

    return shape;
  }

  MMatrix getInverseFluidTransform() {
    const MString fluidObjName = "exported_OBJ_Seq_Mesh";
    MSelectionList fList = select(fluidObjName);

    MObject fluidObj;
    fList.getDependNode(0, fluidObj);
    MFnTransform fluidTransform(fluidObj);
    MTransformationMatrix fluidTransMat = fluidTransform.transformation();

    return fluidTransMat.asMatrixInverse();
  }

  static void* creator() {
    return new SetBubblesCmd;
  }
};