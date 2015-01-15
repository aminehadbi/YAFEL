#include "lin_alg/FullMatrix.hpp"
#include "lin_alg/LUDecomposition.hpp"
#include <stdio.h>
#include <stdlib.h>

YAFEL_NAMESPACE_OPEN

FullMatrix::FullMatrix(int m, int n) {
  rows = m;
  cols = n;
  transposed = false;
  data = new double[m*n];
  for(int i=0; i<m*n; ++i) {
    data[i] = 0.0;
  }
}

FullMatrix::FullMatrix(int m, int n, double val) {
  rows = m;
  cols = n;
  transposed = false;
  data = new double[m*n];
  for(int i=0; i<m*n; ++i) {
    data[i] = val;
  }
}

FullMatrix::FullMatrix(int n) {
  rows = n;
  cols = n;
  transposed = false;
  data = new double[n*n];
  for(int i=0; i<n*n; ++i) {
    data[i] = 0.0;
  }
}

FullMatrix::FullMatrix(int n, double val) {
  rows = n;
  cols = n;
  transposed = false;
  data = new double[n*n];
  for(int i=0; i<n*n; ++i) {
    data[i] = val;
  }
}

FullMatrix::FullMatrix(const FullMatrix & src) {
  rows = src.rows;
  cols = src.cols;
  data = new double[rows*cols];
  transposed = src.transposed;

  for(int i=0; i<rows*cols; ++i) {
    data[i] = src.data[i];
  }
}

FullMatrix::FullMatrix(const sparse_csr & csr) {
  this->rows = csr.getRows();
  this->cols = csr.getCols();
  this->transposed = false;
  data = new double[rows*cols];
  
  for(int i=0; i<getRows(); ++i) {
    for(int j=0; j<getCols(); ++j) {
      this->operator()(i,j) = csr(i,j);
    }
  }
}


FullMatrix::~FullMatrix() {
  delete[] data;
}

inline int FullMatrix::indexOf(int i, int j) const {
  if(!transposed) {
    return i*cols + j;
  }
  else {
    return j*cols+i;
  }
}

double & FullMatrix::operator()(int i, int j) const {
#ifndef _OPTIMIZED
  if ( i<0 || i>=rows) {
    printf("Attempted to index out of bounds: rows\n");
    exit(1);
  }
  if(j<0 || j>=cols) {    
    printf("Attempted to index out of bounds: cols\n");
    exit(1);
  }
#endif

  return data[indexOf(i,j)];
}

FullMatrix FullMatrix::operator*(const FullMatrix & rhs) const {

#ifndef _OPTIMIZED
  if(cols != rhs.rows) {
    printf("FullMatrix::operator* dimension mismatch\n");
    printf("\tlhs\trows = %d\n\t\tcols = %d\n", rows, cols);
    printf("\trhs\trows = %d\n\t\tcols = %d\n", rhs.rows, rhs.cols);
    exit(1);
  }
#endif

  FullMatrix ret_mat(rows, rhs.cols, 0.0);
  
  for(int i=0; i<getRows(); ++i) {
    for(int j=0; j<rhs.getCols(); ++j) {
      for(int k=0; k<cols; ++k) {
	ret_mat(i,j) += (*this)(i,k)*rhs(k,j); //data[indexOf(i,k)] * rhs(k,j);
      }
    }
  }
  
  return ret_mat;
}

FullMatrix &FullMatrix::operator+=(const FullMatrix &rhs)  {

#ifndef _OPTIMIZED  
  if(getRows() != rhs.rows || getCols() != rhs.cols) {
    printf("FullMatrix::operator+= dimension mismatch\n");
    printf("\tlhs\trows = %d\n\t\tcols = %d\n", rows, cols);
    printf("\trhs\trows = %d\n\t\tcols = %d\n", rhs.rows, rhs.cols);
    exit(1);
  }
#endif

  for(int i=0; i<getRows(); ++i) {
    for(int j=0; j<getCols(); ++j) {
      (*this)(i,j) += rhs(i,j);
    }
  }

  return *this;
}

FullMatrix FullMatrix::operator+(const FullMatrix &rhs) const {

#ifndef _OPTIMIZED  
  if(getRows() != rhs.rows || getCols() != rhs.cols) {
    printf("FullMatrix::operator+ dimension mismatch\n");
    printf("\tlhs\trows = %d\n\t\tcols = %d\n", rows, cols);
    printf("\trhs\trows = %d\n\t\tcols = %d\n", rhs.rows, rhs.cols);
    exit(1);
  }
#endif

  FullMatrix ret_mat(*this);
  ret_mat += rhs;

  return ret_mat;
}

FullMatrix &FullMatrix::operator-=(const FullMatrix &rhs) {

#ifndef _OPTIMIZED  
  if(rows != rhs.rows || cols != rhs.cols) {
    printf("FullMatrix::operator-= dimension mismatch\n");
    printf("\tlhs\trows = %d\n\t\tcols = %d\n", rows, cols);
    printf("\trhs\trows = %d\n\t\tcols = %d\n", rhs.rows, rhs.cols);
    exit(1);
  }
#endif
  
  for(int i=0; i<getRows(); ++i) {
    for(int j=0; j<getCols(); ++j) {
      (*this)(i,j) -= rhs(i,j);
    }
  }

  return *this;
}

FullMatrix FullMatrix::operator-(const FullMatrix &rhs) const {
#ifndef _OPTIMIZED  
  if(rows != rhs.rows || cols != rhs.cols) {
    printf("FullMatrix::operator- dimension mismatch\n");
    printf("\tlhs\trows = %d\n\t\tcols = %d\n", rows, cols);
    printf("\trhs\trows = %d\n\t\tcols = %d\n", rhs.rows, rhs.cols);
    exit(1);
  }
#endif


  FullMatrix ret_mat(*this);
  ret_mat -= rhs;

  return ret_mat;
}

Vector FullMatrix::operator*(const Vector & rhs) const {
#ifndef _OPTIMIZED
  if(cols != rhs.getLength()) {
    perror("FullMatrix::operator*(const Vector&) dimension mismatch");
    exit(1);
  }
#endif

  Vector ret_vec(rows, 0.0);
  
  for(int i=0; i<rows; ++i) {
    for(int j=0; j<cols; ++j) {
      ret_vec(i) += this->operator()(i,j) * rhs(j);
    }
  }
  
  return ret_vec;
}

void FullMatrix::transpose() {
  transposed = !transposed;
}

FullMatrix FullMatrix::getTransposed() const {
  
  FullMatrix ret_mat(cols, rows);

  for(int i=0; i<rows; ++i) {
    for(int j=0; j<cols; ++j) {
      ret_mat(j,i) = data[indexOf(i,j)];
    }
  }
  
  return ret_mat;
}

FullMatrix FullMatrix::getInverse() const {

#ifndef _OPTIMIZED  
  if(rows != cols) {
    perror("FullMatrix::getInverse() : must be square matrix");
    exit(1);
  }
#endif
  
  if(rows == 2) {
    return inverse2x2();
  }
  else if(rows == 3) {
    return inverse3x3();
  }
  else {   
    
    LUDecomposition LU(*this);
    
    FullMatrix Ainv(rows, cols, 0.0);
    Vector rhs(rows,0.0);

    for(int i=0; i<cols; ++i) {
      rhs(i) = 1;
      Vector Ainv_col = LU.linsolve(rhs);
      
      for(int j=0; j<rows; ++j) {
	Ainv(j,i) = Ainv_col(j);
      }
      
      rhs(i) = 0.0;
    }

    return Ainv;
  }
}

double FullMatrix::det() const {

#ifndef _OPTIMIZED
  if(getRows() != getCols()) {
    perror("FullMatrix::det() : must be square matrix");
    exit(1);
  }
#endif

  if(getRows() == 2) {
    return (*this)(0,0)*(*this)(1,1) - (*this)(1,0)*(*this)(0,1);
  }
  else if(getRows() == 3) {
    return (*this)(0,0)*( (*this)(1,1)*(*this)(2,2) - (*this)(1,2)*(*this)(2,1) )
      - (*this)(0,1)*( (*this)(1,0)*(*this)(2,2) - (*this)(1,2)*(*this)(2,0) )
      + (*this)(0,2)*( (*this)(1,0)*(*this)(2,1) - (*this)(1,1)*(*this)(2,0) );
  }
  else {
    LUDecomposition LU(*this);

    double prod = 1.0;
    for(int i=0; i<getRows(); ++i) {
      prod *= LU.U(i,i); //ignore L since LUDecomposition makes UNIT lower triangular
    }

    return prod;
  }
}


FullMatrix FullMatrix::inverse2x2() const {
  
  double a,b,c,d;
  a = (*this)(0,0);
  b = (*this)(0,1);
  c = (*this)(1,0);
  d = (*this)(1,1);

  double detA = a*d - b*c;

  FullMatrix Ainv(2,2);

  Ainv(0,0) = d/detA;
  Ainv(0,1) = -b/detA;
  Ainv(1,0) = -c/detA;
  Ainv(1,1) = a/detA;
  
  return Ainv;
}

FullMatrix FullMatrix::inverse3x3() const {
  
  double a,b,c,d,e,f,g,h,i;
  a = (*this)(0,0);
  b = (*this)(0,1);
  c = (*this)(0,2);
  d = (*this)(1,0);
  e = (*this)(1,1);
  f = (*this)(1,2);
  g = (*this)(2,0);
  h = (*this)(2,1);
  i = (*this)(2,2);
  
  double detA = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);

  FullMatrix Ainv(3,3);

  Ainv(0,0) = (e*i - f*h)/detA;
  Ainv(0,1) = -(b*i - c*h)/detA;
  Ainv(0,2) = (b*f - c*e)/detA;
  Ainv(1,0) = -(d*i - f*g)/detA;
  Ainv(1,1) = (a*i - c*g)/detA;
  Ainv(1,2) = -(a*f - c*d)/detA;
  Ainv(2,0) = (d*h - e*g)/detA;
  Ainv(2,1) = -(a*h - b*g)/detA;
  Ainv(2,2) = (a*e - b*d)/detA;

  return Ainv;
}


Vector FullMatrix::slice_col(int col) const {
#ifndef _OPTIMIZED
  if( (col<0) || (col>=cols)) {
    perror("FullMatrix::slice_col() index out of bounds");
    exit(1);
  }
#endif

  Vector ret_vec(rows, 0.0);
  for(int i=0; i<rows; ++i) {
    ret_vec(i) = (*this)(i, col);
  }
  return ret_vec;
}

Vector FullMatrix::slice_row(int row) const {
#ifndef _OPTIMIZED
  if( (row<0) || (row>=rows)) {
    perror("FullMatrix::slice_row() index out of bounds");
    exit(1);
  }
#endif

  Vector ret_vec(cols, 0.0);
  for(int i=0; i<cols; ++i) {
    ret_vec(i) = (*this)(row, i);
  }
  return ret_vec;
}


void FullMatrix::print() {
  for(int i=0; i<rows; ++i) {
    for(int j=0; j<cols; ++j) {
      printf("%f ", data[indexOf(i,j)]);
    }
    printf("\n");
  }
  printf("\n");
}

YAFEL_NAMESPACE_CLOSE
