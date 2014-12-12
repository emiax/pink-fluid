#pragma once

// Maya
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

// Pink-Fluid + deps
#include <stdio.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <bubbleConfig.h>

#include "pluginState.h"

class SetBubblesCmd : public MPxCommand {
public:
  static void* creator() {
    return new SetBubblesCmd;
  }

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
    MMatrix invFluidTransMat = getInverseFluidTransform();

    cout << "Current Fluid Translation Matrix: " << endl;
    float transMatData[16];
    for (int j = 0; j < 4; ++j) {
      for (int i = 0; i < 4; ++i) {
        transMatData[j*4 + i] = invFluidTransMat(j, i);
        cout << invFluidTransMat(j, i) << "\t";
      }
      cout << endl;
    }
    glm::mat4 glmInvTransMat = glm::make_mat4(transMatData);

    for (int j = 0; j < 4; ++j){
      for (int i = 0; i < 4; ++i) printf("%f ", glmInvTransMat[i][j]);
      printf("\n");
    }

    // Create bubbles
    std::vector<Bubble> bubbles;
    bubbles.reserve(particleRadii.length());
    for (int i = 0; i < particleRadii.length(); ++i) {
      MVector pos = particlePositions[i];
      double r = particleRadii[i];
      glm::vec4 p(pos.x, pos.y, pos.z, 1.0);
      p = glmInvTransMat*p;
      Bubble b(glm::vec3(p), r, glm::vec3(0.0), -1);
      bubbles.push_back(b);
    }

    // Update conf file
    const std::string BUBBLE_CONF_FILE_NAME = PluginStateManager::instance()->getExecutionPath() + "/bubbleConf.pf";
    cout << "file path = " << BUBBLE_CONF_FILE_NAME << endl;
    updateBubbleFile(bubbles, frameNumber, BUBBLE_CONF_FILE_NAME);

    return status;
  }

private:
  void updateBubbleFile(std::vector<Bubble> bubbles, int frameNumber, std::string filename) {
    std::ifstream bubbleConfigInFile(filename);
    BubbleConfig bubbleConfig;
    if (!bubbleConfigInFile.fail()) {
      cout << "Reading bubble config from existing file" << endl;
      bubbleConfig.read(bubbleConfigInFile);
    }

    bubbleConfig.addBubbles(frameNumber, bubbles);
    std::ofstream bubbleConfigOutFile(filename);
    bubbleConfig.write(bubbleConfigOutFile);
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
};