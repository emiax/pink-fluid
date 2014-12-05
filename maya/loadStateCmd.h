#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MDGModifier.h>
#include <objExporter.h>
#include <fstream>
#include <state.h>
#include <stdio.h>
#include <unistd.h>
#include <face.h>
#include <sdfTessellation.h>

class LoadStateCmd : public MPxCommand {
public:
  void importBubbles(State *state) {
    /*    MFnParticleSystem ps;
    MPointArray points;
    std::vector<Bubble> bubbles = state->getBubbles();
    for (Bubble bubble : bubbles) {
      glm::vec3 pos = bubble.position;
      points.append(pos.x, pos.y, pos.z);
    }
    points.append(0, 0, 0);

    ps.create();
    ps.emit(points);
    ps.create();*/
  }

  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    MString statePath = args.asString(0, &status);
    if(state){
      delete state;
      state = nullptr;
    }
    state = new State(1,1,1);
    if (args.length() != 1 || status.error()) {
      return status;
    }

    std::ifstream inputFileStream(statePath.asChar(), std::ios::binary);

    if(inputFileStream.good()){
      state->read(inputFileStream);
      inputFileStream.close();
      MGlobal::displayInfo("State Loaded");


      MFnMesh fnMesh;
      
      SdfTessellation sdfTess(state->getSignedDistanceGrid());
      std::vector<glm::vec3> vertexList = sdfTess.getVertices();
      std::vector<Face> faceIndices = sdfTess.getFaces();
      
      for(int i = 0; i < faceIndices.size(); i++ ){
        Face &face = faceIndices[i];
        MPointArray polygon;

        polygon.append(MPoint(vertexList[face[0]][0], vertexList[face[0]][1], vertexList[face[0]][2]));
        polygon.append(MPoint(vertexList[face[1]][0], vertexList[face[1]][1], vertexList[face[1]][2]));
        polygon.append(MPoint(vertexList[face[2]][0], vertexList[face[2]][1], vertexList[face[2]][2]));
        

        fnMesh.addPolygon(polygon);
      }

      importBubbles(state);
      
      MObject smoothMesh = fnMesh.generateSmoothMesh();

      //Delete the original mesh
      MDGModifier modifier;
      modifier.deleteNode(fnMesh.object());
      modifier.doIt();
    }
    else{
      MGlobal::displayInfo(MString("Could not load state file: ") + statePath);
    }

    return status;
  }

  static void* creator() {
    return new LoadStateCmd;
  }

  State *state = nullptr;
};
