#include <maya/MSimple.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

DeclareSimpleCommand( helloWorld, "Pink Fluid", "0.1");

MStatus helloWorld::doIt( const MArgList& )
{
  MGlobal::displayInfo("hello!");

  return MS::kSuccess;
}
