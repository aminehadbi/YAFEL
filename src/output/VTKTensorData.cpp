#include "output/VTKTensorData.hpp"


YAFEL_NAMESPACE_OPEN


VTKTensorData::VTKTensorData(const std::vector<FullMatrix> &d, 
			     VTKObject::VTKObjectType ot, const std::string &s) :
  VTKObject(VTKObject::VTKVECTORDATA, ot, s), data(d) {}


void VTKTensorData::write(FILE *fp) {

  fprintf(fp, "<DataArray type=\"Float32\" format=\"ascii\" Name=\"%s\" NumberOfComponents=\"9\">\n",
	  getName().c_str());
  
  for(unsigned i=0; i<data.size(); ++i) {
    
    fprintf(fp, "%f %f %f\n%f %f %f\n%f %f %f\n\n", 
	    data[i](0,0), data[i](0,1), data[i](0,2),
	    data[i](1,0), data[i](1,1), data[i](1,2),
	    data[i](2,0), data[i](2,1), data[i](2,2));
  }

  fprintf(fp, "</DataArray>\n");
}


YAFEL_NAMESPACE_CLOSE
