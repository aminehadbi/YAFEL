#include "lin_alg/bicgstab.hpp"
#include <cmath>
#include <iostream>

YAFEL_NAMESPACE_OPEN

Vector bicgstab(const sparse_csr & A, const Vector & rhs) {
  
  Vector r_old(rhs), 
    rhat(rhs), 
    x(rhs.getLength(), 0.0),
    v(rhs.getLength(),0.0), 
    p(rhs.getLength(), 0.0);
  
  double rho_old, rho_new, alpha, w_old, w_new;
  
  int conv_check_freq = std::max(1, rhs.getLength()/100);
  
  double residual_0 = rhs.dot(rhs);
  
  rho_old = 1.0;
  alpha= 1.0;
  w_old = 1.0;
  
  int k=0;
  while(true) {
    rho_new = rhat.dot(r_old);
    
    double beta = (rho_new/rho_old)*(alpha/w_old);
    
    p = r_old + p*beta - v*(w_old*beta);
    
    v = A*p;
    
    alpha = rho_new/rhat.dot(v);
    
    Vector s = r_old - v*alpha;
    Vector t = A*s;
    
    w_new = t.dot(s) / t.dot(t);
    
    x += p*alpha + s*w_new;
    
    if(k % conv_check_freq == 0) {
      Vector r = A*x - rhs;
      double res = r.dot(r);
      //convergence reporting
      //std::cout << k << "," << res/residual_0 << std::endl;
      if(res/residual_0 < BICGSTAB_TOL) {
	std::cerr << "converged in " << k << "steps.\n";
	break;
      }
    }
    
    //reset old/new vars for next iteration
    r_old = s - t*w_new;
    rho_old = rho_new;
    w_old = w_new;
    
    ++k;
  }
  
  return x;
}

YAFEL_NAMESPACE_CLOSE
