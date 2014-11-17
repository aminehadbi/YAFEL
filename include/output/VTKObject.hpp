#ifndef _YAFEL_VTKOBJECT_HPP
#define _YAFEL_VTKOBJECT_HPP

#include <cstdio>
#include <string>
#include "yafel_globals.hpp"

YAFEL_NAMESPACE_OPEN

class VTKObject {
public:
  enum VTKDataType {
    NONE,
    VTKSCALARDATA,
    VTKVECTORDATA,
    VTKTENSORDATA };

  enum VTKObjectType {
    VTKMESH,
    VTKPOINTDATA,
    VTKCELLDATA };

private:
  VTKDataType DType;
  VTKObjectType OType;
  std::string name;
  
public:
  VTKObject(VTKDataType dt, VTKObjectType ot, const std::string &s);
  virtual void write(FILE *fp)=0;
  inline VTKObjectType getObjectType() {return OType;}
  inline VTKDataType getDataType() {return DType;}
  inline std::string getName() {return name;}
};

YAFEL_NAMESPACE_CLOSE

#endif
