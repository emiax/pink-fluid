#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <objExporter.h>
#include <fstream>
#include <state.h>
#include <stdio.h>
#include <unistd.h>

class LoadStateCmd : public MPxCommand {
public:
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
      ObjExporter exporter;

      std::vector<glm::vec3> vertexList;
      std::vector<std::vector<int>> faceIndices;
      
      exporter.triangulateLevelSet(state, vertexList, faceIndices);
      for(int i = 0; i < faceIndices.size(); i++ ){
        std::vector<int> &face = faceIndices[i];
        MPointArray polygon;
        std::cout << "Creating face" << std::endl;
        for(auto index : face){
          std::cout << index << " ";
          std::cout << vertexList[index][0]<< " " << vertexList[index][1] << " " << vertexList[index][2] << " - ";
          polygon.append(MPoint(vertexList[index][0], vertexList[index][1], vertexList[index][2]));
        }
        std::cout << std::endl << "Created face" << std::endl;
        fnMesh.addPolygon(polygon);
      }
      fnMesh.generateSmoothMesh();
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
