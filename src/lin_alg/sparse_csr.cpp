#include "yafel_globals.hpp"
#include "lin_alg/sparse_csr.hpp"
#include "lin_alg/sparse_coo.hpp"
#include <stdio.h>
#include <stdlib.h>

YAFEL_NAMESPACE_OPEN
  
sparse_csr::sparse_csr(sparse_coo & coo) {
  init_from_coo(coo); //separate function since this routine is needed elsewhere
}

sparse_csr::sparse_csr(const sparse_csr & src) {
  
  rows = src.rows;
  cols = src.cols;
  nnz = src.nnz;
  
  row_ptr = new int[rows+1];
  col_index = new int[nnz];
  data = new double[nnz];

  for(int i=0; i<rows+1; ++i) {
    row_ptr[i] = src.row_ptr[i];
  }
  for(int i=0; i<nnz; ++i) {
    col_index[i] = src.col_index[i];
    data[i] = src.data[i];
  }

}

void sparse_csr::init_from_coo(sparse_coo & coo) {
  coo.sort();
  this->nnz = coo.nnz();
  coo.compress(); //lots of redundant work avoided by coo.consistentNNZ==true
  
  this->rows = coo.getRows();
  this->cols = coo.getCols();

  row_ptr = new int[rows+1];
  col_index = new int[nnz];
  data = new double[nnz];
  
  for(int i=0; i<nnz; ++i) {
    col_index[i] = coo.col_index[i];
    data[i] = coo.data[i];
  }

  row_ptr[0] = 0;
  row_ptr[rows] = nnz;
  int prev_row = 0;
  int row_i = 0;
  for(int i=0; i<nnz; ++i) {
    if(coo.row_index[i] != prev_row) {
      for(int j=row_i+1; j<=coo.row_index[i]; ++j) {
	row_ptr[j] = i;
      }
      row_i = coo.row_index[i];
    }
  }
  
  if(row_i < rows-1) {
    for(int i = row_i+1; i<rows+1; ++i) {
      row_ptr[i] = nnz;
    }
  }
  
}


sparse_csr::~sparse_csr() {
  delete[] row_ptr;
  delete[] col_index;
  delete[] data;
}

void sparse_csr::print_sparse() {

  int ckpt = (rows+1 < nnz) ? rows+1 : nnz;
  int end = (rows+1 < nnz) ? nnz : rows+1;

  printf("row_ptr\tcol_idx\tdata\n");

  for(int i=0; i<ckpt; ++i) {
    printf("%d\t%d\t%f\n", row_ptr[i], col_index[i], data[i]);
  }

  if(rows+1 > nnz) {
    for(int i=ckpt; i<end; ++i) {
      printf("%d\n",row_ptr[i]);
    }
  }
  else {
    for(int i=ckpt; i<end; ++i) {
      printf("\t%d\t%f\n", col_index[i], data[i]);
    }
  }
}

double sparse_csr::operator()(int row, int col, bool & flag) const{
  // bool & flag returns whether the requested (row,col) entry exists
  // in the sparse matrix since the function can return 0.0 in either case.
  
  int i_start = row_ptr[row];
  int i_end = row_ptr[row+1];

  double ret_val = 0;
  flag = false;
  for(int i=i_start; i<i_end; ++i) {
    if(col_index[i]==col) {
      ret_val = data[i];
      flag = true;
      break;
    }
  }
  
  return ret_val;
}

double sparse_csr::operator()(int row, int col) const {
  //version of operator() where caller doesn't care about value of bool &flag.
  bool trash;
  return operator()(row, col, trash);
}

sparse_csr sparse_csr::operator*(const sparse_csr & rhs) const {
  /*
    Create by first constructing sparse_coo, then convert to sparse_csr
  */

#ifndef _OPTIMIZED  
  if(cols != rhs.getRows()) {
    fprintf(stderr, "sparse_csr operator* matrix dimension error\n");
    exit(1);
  }
#endif  

  sparse_coo coo;

  for(int row=0; row<rows; ++row) {
    for(int col=0; col<rhs.getCols(); ++col){
      for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
	int k = col_index[i];
	bool nonzero_flag;
	double rhsval = rhs(k,col,nonzero_flag);
	if(nonzero_flag) {
	  double val = data[i] * rhsval; // val = lhs(row,k)*rhs(k,col)
	  coo.add(row, col, val);
	}
      }
    }
  }
  sparse_csr ret_mat(coo);
  return ret_mat;
}

Vector sparse_csr::operator*(const Vector & rhs) const {

#ifndef _OPTIMIZED  
  if(cols != rhs.getLength()) {
    printf("Incompatible sparse_csr*Vector dimensions\n");
    exit(1);
  }
#endif
  
  Vector ret_vec(rows, 0.0);

#pragma omp parallel for schedule(static, 32)
  for(int row=0; row<rows; ++row) {
    double tmp = 0.0;
    for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
      tmp += data[i] * rhs(col_index[i]);
    }
    
    ret_vec(row) = tmp;
  }
  
  return ret_vec;
}

Vector sparse_csr::slice_col(int col) const {

#ifndef _OPTIMIZED  
  if( (col>=cols) || (col<0) ) {
    perror("sparse_csr::slice_col() index out of bounds");
    exit(1);
  }
#endif

  Vector ret_vec(rows, 0.0);
  for(int i=0; i<rows; ++i) {
    ret_vec(i) = this->operator()(i, col);
  }
  return ret_vec;
}

Vector sparse_csr::slice_row(int row) const {

#ifndef _OPTIMIZED    
  if( (row>=rows) || (row<0) ) {
    perror("sparse_csr::slice_row() index out of bounds");
    exit(1);
  }
#endif  

  //Not using naive this->operator() method like slice col due to sparse_csr data layout
  Vector ret_vec(cols, 0.0);
  for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
    int col = col_index[i];
    ret_vec(col) = this->data[col];
  }
  return ret_vec;
}

void sparse_csr::zero_col(int col_to_del) {

  //changing implementation so that it does not modify
  // sparsity structure of sparse matrix.
  //Don't want this behavior when applying BC's in FEM.

  for(int i=0; i<nnz; ++i) {
    if(col_index[i] == col_to_del)
      data[i] = 0.0;
  }

  /*  
  sparse_coo coo(this->getRows(), this->getCols());
  
  for(int row=0; row<getRows(); ++row) {

    for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
      int col = col_index[i];
      if(col == col_to_del) {
	continue;
      }
      coo.add(row, col, data[i]);
    }
  }
  
  delete[] row_ptr;
  delete[] col_index;
  delete[] data;
  
  init_from_coo(coo);
  */
}

void sparse_csr::zero_row(int row_to_del) {

  //changing implementation so that it does not modify
  // sparsity structure of sparse matrix.
  //Don't want this behavior when applying BC's in FEM.
  for(int i=row_ptr[row_to_del]; i<row_ptr[row_to_del+1]; ++i) {
    data[i] = 0.0;
  }
  
  /*  
  sparse_coo coo(this->getRows(), this->getCols());
  
  for(int row=0; row<getRows(); ++row) {
    if(row == row_to_del) {
      continue;
    }

    for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
      int col = col_index[i];

      coo.add(row, col, data[i]);
    }
  }
  
  delete[] row_ptr;
  delete[] col_index;
  delete[] data;
  
  init_from_coo(coo);
  */
}

void sparse_csr::assign(int row, int col, double val) {
  //super-expensive (potentially)! Directly sets value in data vec
  //if already a non-zero. Otherwise, converts csr -> coo, adds val,
  //and converts back.
  bool in_sparsity = false;
  this->operator()(row, col, in_sparsity);
  if(in_sparsity) {
    for(int i=row_ptr[row]; i<row_ptr[row+1]; ++i) {
      if(col == col_index[i]) {
	data[i] = val;
	return;
      }
    }
  }
  else {
    sparse_coo coo(*this);
    coo.add(row,col,val);
    delete[] row_ptr;
    delete[] col_index;
    delete[] data;

    init_from_coo(coo);
  }
  
}


YAFEL_NAMESPACE_CLOSE
