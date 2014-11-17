#include "output/VTKOutput.hpp"
#include <cstdlib>
#include <cstdio>

YAFEL_NAMESPACE_OPEN

VTKOutput::VTKOutput() {
  vtkmesh = NULL;
  pointData.clear();
  cellData.clear();
}

void VTKOutput::addVTKObject(VTKObject *VO) {
  switch(VO->getObjectType()) {
  case VTKObject::VTKMESH:
    vtkmesh = VO;
    break;
  case VTKObject::VTKPOINTDATA:
    pointData.push_back(VO);
    break;
  case VTKObject::VTKCELLDATA:
    cellData.push_back(VO);
    break;
  }
}

void VTKOutput::write(const char *fname) {
  
  FILE *fp = fopen(fname, "w");
  if(fp == NULL) {
    perror("VTKOutput::write() : Could not open file.");
    exit(1);
  }

  if(vtkmesh == NULL) {
    perror("VTKOutput::write() : no mesh assigned.");
    exit(1);
  }
  
  fprintf(fp, "<?xml version=\"1.0\"?>\n");
  fprintf(fp, "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\">\n");
  vtkmesh->write(fp);
  
  //write point data
  if(pointData.size() > 0) {
    fprintf(fp, "<PointData>\n");
    for(unsigned i=0; i<pointData.size(); ++i) {
      pointData[i]->write(fp);
    }
    fprintf(fp, "</PointData>\n");
  }

  //write cell data
  if(cellData.size() > 0) {
    fprintf(fp, "<CellData>\n");
    for(unsigned i=0; i<cellData.size(); ++i) {
      cellData[i]->write(fp);
    }
    fprintf(fp, "</CellData>\n");
  }

  //conclude file
  fprintf(fp, "</Piece>\n");
  fprintf(fp, "</UnstructuredGrid>\n");
  fprintf(fp, "</VTKFile>");

  fclose(fp);
}

YAFEL_NAMESPACE_CLOSE
