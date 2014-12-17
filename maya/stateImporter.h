#pragma once
#include <state.h>
#include <sstream>
#include <sdfTessellation.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMesh.h>

class StateImporter{
public:
  struct StateOutput;
  
  std::string importBubbles(State *state, int frame = -1) {
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

    std::string addSpriteScaleXPPAttrCmd = "addAttr -ln spriteScaleXPP -dt doubleArray " + std::string(pSystemName.asChar()) + ";";
    std::string addSpriteScaleXPP0AttrCmd = "addAttr -ln spriteScaleXPP0 -dt doubleArray " + std::string(pSystemName.asChar()) + ";";
    std::string addSpriteScaleYPPAttrCmd = "addAttr -ln spriteScaleYPP -dt doubleArray " + std::string(pSystemName.asChar()) + ";";
    std::string addSpriteScaleYPP0AttrCmd = "addAttr -ln spriteScaleYPP0 -dt doubleArray " + std::string(pSystemName.asChar()) + ";";

    std::string setRenderTypeCmd = "setAttr \"" + std::string(pSystemName.asChar()) + ".particleRenderType\" 5";

    MGlobal::executeCommand(addRadiusAttrCmd.c_str());
    MGlobal::executeCommand(addRadiusDefaultAttrCmd.c_str());
    MGlobal::executeCommand(addSpriteScaleXPPAttrCmd.c_str());
    MGlobal::executeCommand(addSpriteScaleXPP0AttrCmd.c_str());
    MGlobal::executeCommand(addSpriteScaleYPPAttrCmd.c_str());
    MGlobal::executeCommand(addSpriteScaleYPP0AttrCmd.c_str());
    MGlobal::executeCommand(setRenderTypeCmd.c_str());

    std::string setRadiusCmd = "";
    for (int i = 0; i < bubbles.size(); ++i) {
      Bubble &b = bubbles[i];
      setRadiusCmd += "particle -e -attribute radiusPP -order "
                     + std::to_string(i) + " -fv "
                     + std::to_string(b.radius) + " "
                     + pSystemName.asChar() + ";\n";
    }
    MGlobal::executeCommand(setRadiusCmd.c_str());

    std::string setSpriteScale = "dynExpression -s \"" +
        std::string(pSystemName.asChar()) + ".spriteScaleXPP = " + std::string(pSystemName.asChar()) + ".radiusPP" + ";\\n" +
        std::string(pSystemName.asChar()) + ".spriteScaleYPP = " + std::string(pSystemName.asChar()) + ".radiusPP" + ";\" " +
        "-rad " + std::string(pSystemName.asChar());

    MGlobal::executeCommand(setSpriteScale.c_str());

    return pSystemName.asChar();
  }

  std::string importMesh(State * state, int frame = -1){
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

  StateOutput importState(State *state, int frame = -1){
    return {importMesh(state, frame), importBubbles(state, frame)};
  }

  std::string connectStates(std::vector<std::string> meshNames, std::vector<int> frameNumbers){
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

  }
  
  struct StateOutput{
    std::string meshName;
    std::string bubbleName;
  };

private:
  MString createKeyframeCommand(int frame, std::string objectName, bool visible){
    std::string visibleString = std::to_string(visible? 1 : 0);
    std::string frameString = std::to_string(frame);
    std::string commandString = "setKeyframe -attribute \"visibility\" -t " + frameString + " -f " + frameString + " -v " + visibleString + " " + objectName + ";";
    return MString(commandString.c_str());
  }

};
