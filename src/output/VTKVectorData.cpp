#include "output/VTKVectorData.hpp"


YAFEL_NAMESPACE_OPEN


VTKVectorData::VTKVectorData(const std::vector<Vector> &d, 
			     VTKObject::VTKObjectType ot, const std::string &s) :
  VTKObject(VTKObject::VTKVECTORDATA, ot, s), data(d) {}


void VTKVectorData::write(FILE *fp) {
  
  fprintf(fp, "<DataArray type=\"Float32\" format=\"ascii\" Name=\"%s\" NumberOfComponents=\"3\">\n",
	  getName().c_str());
  
  for(unsigned i=0; i<data.size(); ++i) {
    
    fprintf(fp, "%f %f %f\n", 
	    data[i](0),
	    (data[i].getLength()>=2)?data[i](1):0,
	    (data[i].getLength()>=3)?data[i](2):0);
  }

  fprintf(fp, "</DataArray>\n");
}


YAFEL_NAMESPACE_CLOSE
