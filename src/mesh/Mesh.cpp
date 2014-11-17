#include "mesh/Mesh.hpp"

YAFEL_NAMESPACE_OPEN

Mesh::Mesh() {}

Mesh::Mesh(const std::vector<Vector> & nodes,
	   const std::vector< std::vector<int> > & elems,
	   const std::vector<int> & eltype) {
  this->nodal_coords = nodes;
  this->elements = elems;
  this->element_type = eltype;

  this->n_elems = elems.size();
  this->n_nodes = nodes.size();
}

Mesh::Mesh(const std::vector< Vector > & nodes,
	   const std::vector< std::vector<int> > & elems,
	   const std::vector<int> & eltype,
	   const std::vector< std::vector<int> > & _tags) {
  this->nodal_coords = nodes;
  this->elements = elems;
  this->element_type = eltype;
  this->el_tags = _tags;

  this->n_elems = elems.size();
  this->n_nodes = nodes.size();
}


YAFEL_NAMESPACE_CLOSE
