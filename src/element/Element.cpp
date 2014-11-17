#include "element/Element.hpp"
#include <stdio.h>

YAFEL_NAMESPACE_OPEN

Element::Element(int nsd, int nqp, int dofpn, int dofpe, int vtktype, int nodespe) :
  n_spaceDim(nsd), n_quadPoints(nqp), dof_per_node(dofpn), 
  dof_per_el(dofpe), vtk_type(vtktype), nodes_per_el(nodespe)
{
  
}

FullMatrix Element::calcJ_xi(Vector xi) {
  
  FullMatrix retMat(n_spaceDim, n_spaceDim, 0.0);

  for(int i=0; i<n_spaceDim; ++i) {
    for(int j=0; j<n_spaceDim; ++j) {
      for(int A=0; A<nodes_per_el; ++A) {
	retMat(i,j) += shape_grad_xi(A, j, xi) * nodal_coords[A](i);
      }
    }
  }
  
  return retMat;
}

void Element::calcJacobians() {
  jacobians.clear();
  detJ.clear();

  for(int i=0; i<n_quadPoints; ++i) {
    jacobians.push_back( calcJ_xi(quad_points[i]) );
    
    detJ.push_back(jacobians[i].det());
  }

}

void Element::calcGrads() {
  
  grads.clear();
  
  for(int qpi=0; qpi<n_quadPoints; ++qpi) {
    Vector qp = quad_points[qpi];
    
    FullMatrix Jinv = jacobians[qpi].getInverse();
    FullMatrix NG(nodes_per_el, n_spaceDim, 0.0);

    for(int A=0; A<nodes_per_el; ++A) {
      for(int j=0; j<n_spaceDim; ++j) {
	for(int k=0; k<n_spaceDim; ++k) {
	  NG(A,j) += shape_grad_xi(A,k,qp)*Jinv(k,j);
	}
      }
    }
    
    grads.push_back(NG);
  }
  
}

void Element::update_element(const Mesh &M, int elnum) {

  int Nnodes = M.elements[elnum].size();

  global_dofs.clear();
  nodal_coords.clear();
  element = M.elements[elnum];
  for(int i=0; i<Nnodes; ++i) {
    int nodeNum = M.elements[elnum][i];
    nodal_coords.push_back(M.nodal_coords[nodeNum]);
    for(int j=0; j<dof_per_node; ++j) {
      int dofNum = nodeNum*dof_per_node + j;
      global_dofs.push_back(dofNum);
    }
  }

  calcJacobians();
  calcGrads();
}



YAFEL_NAMESPACE_CLOSE
