#include <velocityGrid.h>
#include <maya/MFnField.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>

class MayaVelocityGrid : public MFnField{
public:
  MayaVelocityGrid() : MFnField(){};
  MayaVelocityGrid(MObject &object, MStatus *ReturnStatus=NULL) : MFnField (object, ReturnStatus){};
  MayaVelocityGrid(const MDagPath &object, MStatus *ret=NULL) : MFnField (object, ret) {};
  void setVelocityGrid(VelocityGrid *vGrid){
    grid = vGrid;
  }
  MStatus getForceAtPoint(const MPointArray &point, const MVectorArray &velocity, const MDoubleArray &mass, MVectorArray &force, double deltaTime=1.0/24.0){
    MStatus status;
    for(int i = 0; i < point.length(); i++){
      glm::vec3 pPosition = glm::vec3(point[i][0], point[i][1], point[i][2]);
      glm::vec3 pVelocity = glm::vec3(velocity[i][0], velocity[i][1], velocity[i][2]);

      glm::vec3 gridVelocity = grid->getLerp(pPosition);

      glm::vec3 resultingVelocity = (gridVelocity - pVelocity);

      force[i] = MVector(resultingVelocity[0], resultingVelocity[1], resultingVelocity[2]);
    }
    return status;
  }
  
  MStatus getForceAtPoint(const MVectorArray &point, const MVectorArray &velocity, const MDoubleArray &mass, MVectorArray &force, double deltaTime=1.0/24.0){
    MStatus status;
    for(int i = 0; i < point.length(); i++){
      glm::vec3 pPosition = glm::vec3(point[i][0], point[i][1], point[i][2]);
      glm::vec3 pVelocity = glm::vec3(velocity[i][0], velocity[i][1], velocity[i][2]);

      glm::vec3 gridVelocity = grid->getLerp(pPosition);

      glm::vec3 resultingVelocity = (gridVelocity - pVelocity);

      force[i] = MVector(resultingVelocity[0], resultingVelocity[1], resultingVelocity[2]);
    }
    return status;
  }
private:
  VelocityGrid *grid;
};
