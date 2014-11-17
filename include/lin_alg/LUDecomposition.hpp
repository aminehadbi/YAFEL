#ifndef _YAFEL_LUDECOMPOSITION_HPP
#define _YAFEL_LUDECOMPOSITION_HPP

#include "yafel_globals.hpp"
#include "lin_alg/sparse_csr.hpp"
#include "lin_alg/FullMatrix.hpp"
#include "lin_alg/Vector.hpp"

YAFEL_NAMESPACE_OPEN

class LUDecomposition {
  
private:
  void computeLU(const FullMatrix &A);
  Vector f_subst(const Vector &rhs) const;
  Vector b_subst(const Vector &rhs) const;
  
public:
  FullMatrix L, U;
  LUDecomposition(const FullMatrix &A);
  
  Vector linsolve(const Vector & rhs) const;
};

YAFEL_NAMESPACE_CLOSE

#endif
