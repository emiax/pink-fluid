#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MDGModifier.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MIOstream.h>
#include <objExporter.h>
#include <fstream>
#include <state.h>
#include <stdio.h>
#include <unistd.h>
#include <face.h>
#include <sdfTessellation.h>
#include <sstream>

class LoadStateCmd : public MPxCommand {
public:
  void importBubbles(State *state, int frame = -1) {
    MPointArray points;
    std::vector<Bubble> bubbles = state->getBubbles();
    for (Bubble bubble : bubbles) {
      glm::vec3 pos = bubble.position;
      points.append(pos.x, pos.y, pos.z);
    }
    MStatus status;

    int nBubbles = bubbles.size(); 
    std::cout << "gonna emit " << nBubbles << " particles." << std::endl;

    std::stringstream ss;
    ss << "particle";
    for (auto &bubble : bubbles) {
      glm::vec3 pos = bubble.position;
      ss << " -p " << pos.x << " " << pos.y << " " << pos.z;
    }
    ss << ";";

    MStringArray returnStrings;
    MGlobal::executeCommand(MString(ss.str().c_str()), returnStrings);
    MString pSystemName = returnStrings[1];

    if(frame != -1){
      MGlobal::executeCommand(createKeyframeCommand(frame+0, pSystemName.asChar(), false));
      MGlobal::executeCommand(createKeyframeCommand(frame+1, pSystemName.asChar(), true));
      MGlobal::executeCommand(createKeyframeCommand(frame+2, pSystemName.asChar(), false));
    }
    
    std::string addRadiusAttrCmd = "addAttr -ln radiusPP -dt doubleArray " + std::string(pSystemName.asChar()) + ";";
    std::string addRadiusDefaultAttrCmd = "addAttr -ln radiusPP0 -dt doubleArray " + std::string(pSystemName.asChar()) + ";";
    MGlobal::executeCommand(addRadiusAttrCmd.c_str());
    MGlobal::executeCommand(addRadiusDefaultAttrCmd.c_str());

    std::string setRadiusCmd = "";
    for (int i = 0; i < bubbles.size(); ++i) {
      Bubble &b = bubbles[i];
      setRadiusCmd += "particle -e -attribute radiusPP -order "
                     + std::to_string(i) + " -fv "
                     + std::to_string(b.radius) + " "
                     + pSystemName.asChar() + ";\n";
    }
    MGlobal::executeCommand(setRadiusCmd.c_str());
    cout << setRadiusCmd << endl;

    // todo:
    // radius.
    // keyframing.
  }

  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    unsigned int index = 0;
    const MStringArray pathArray = args.asStringArray(index, &status);

    State *state = new State(1,1,1);
    if (args.length() != 1 || status.error()) {
      return status;
    }

    std::vector<std::string> meshNames;
    std::vector<int> frameNumbers;
    MString numMeshesString = std::to_string(pathArray.length()).c_str();
    std::string createProgressCommand = "progressWindow  -status \"Importing OBJ Sequence...\" -maxValue " + std::to_string(pathArray.length()) + " -title \"Importing\" -isInterruptable true;";
    MGlobal::executeCommand(createProgressCommand.c_str());
    for(int i = 0; i < pathArray.length(); i++){
      std::ifstream inputFileStream(pathArray[i].asChar(), std::ios::binary);
      int result;
      MGlobal::executeCommand("progressWindow -query -isCancelled", result);
      if(result){
        break;
      }
      if(inputFileStream.good()){
        state->read(inputFileStream);
        inputFileStream.close();
        std::string singleMeshName = loadSingleState(state, i);
        MGlobal::displayInfo("State Loaded");
        meshNames.push_back(singleMeshName);
        frameNumbers.push_back(state->getFrameNumber());
      }
      else{
        MGlobal::displayInfo(MString("Could not load state file: ") + pathArray[i]);
      }
      MGlobal::executeCommand("progressWindow -e -step 1;");
    }
    MGlobal::executeCommand("progressWindow -endProgress;");
    MGlobal::clearSelectionList();
    std::string groupMeshCommand = "group -em -name pfFluidGroup";
    std::string selectMeshCommand = "select -r";

    std::string meshNamesString = "";
    std::string frameNumberString = "";
    
    for(auto name: meshNames){
      meshNamesString += " " + name;
    }

    for(auto frameNumber: frameNumbers){
      frameNumberString += " " + std::to_string(frameNumber);
    }

    groupMeshCommand += meshNamesString + ";";
    selectMeshCommand += meshNamesString + ";";
    
    MGlobal::executeCommand(groupMeshCommand.c_str());
    MGlobal::executeCommand(selectMeshCommand.c_str());
    
    MString resultString;
    MGlobal::executeCommand("connectStates pfFluidGroup", resultString);

    MString createFrameNumberAttr = "addAttr -dt Int32Array -ln \"frameNumbers\" -k 1 " + resultString + ";";
    MString attributeName = resultString + ".frameNumbers";
    MString setFrameNumberAttr = "setAttr " + attributeName + " -type Int32Array " + std::to_string(frameNumbers.size()).c_str() + frameNumberString.c_str() + ";";

    MGlobal::executeCommand(createFrameNumberAttr);
    MGlobal::executeCommand(setFrameNumberAttr);

    MGlobal::executeCommand("setAttr -lock on " + attributeName);
    return status;
  }

  std::string loadSingleState(State * state, int frame = -1){
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

    importBubbles(state, frame);
      
    MObject smoothMesh = fnMesh.generateSmoothMesh();

    //Delete the original mesh
    MObject fnObject = fnMesh.object();
    MGlobal::deleteNode(fnObject);

    MFnTransform fn(smoothMesh);
    MObject child = fn.child(0);
    fnMesh.setObject(child);
      
    MGlobal::select(child);

    MGlobal::executeCommand("addAttr -at long -ln \"frameNumber\" -k 0 " + fnMesh.name());
    MString attrName = fnMesh.name() + ".frameNumber";
    MString frameNumber = std::to_string(state->getFrameNumber()).c_str();
    MGlobal::executeCommand("setAttr " + attrName + " " + frameNumber);
    MGlobal::executeCommand("setAttr -lock on " + attrName);
    if(frame != -1){
      MGlobal::executeCommand(createKeyframeCommand(frame+0, fnMesh.name().asChar(), false));
      MGlobal::executeCommand(createKeyframeCommand(frame+1, fnMesh.name().asChar(), true));
      MGlobal::executeCommand(createKeyframeCommand(frame+2, fnMesh.name().asChar(), false));
    }
    return fnMesh.name().asChar();
  }

  MString createKeyframeCommand(int frame, std::string objectName, bool visible){
    std::string visibleString = std::to_string(visible? 1 : 0);
    std::string frameString = std::to_string(frame);
    std::string commandString = "setKeyframe -attribute \"visibility\" -t " + frameString + " -f " + frameString + " -v " + visibleString + " " + objectName + ";";
    return MString(commandString.c_str());
  }

  static void* creator() {
    return new LoadStateCmd;
  }
};
