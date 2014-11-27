#include <maya/MGlobal.h>
#include <maya/MSimple.h>

class SetGridResolutionCmd : public MPxCommand {
public:
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    int w = args.asInt(0, &status);
    int h = args.asInt(1, &status);
    int d = args.asInt(2, &status);

    if (args.length() != 3 || status.error()) {
      MGlobal::displayInfo("usage: pfSetGridResolution(int, int, int)");
    } else {
      MGlobal::displayInfo(MString("res: (w = ") + w + ", h = " + h + ", d = " + d + ")");
    }

    return status;
  }

  static void* creator() {
    return new SetGridResolutionCmd;
  }
};