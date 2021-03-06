//
// Created by tyler on 4/16/17.
//

#ifndef YAFEL_FESYSTEM_HPP
#define YAFEL_FESYSTEM_HPP

#include "yafel_globals.hpp"
#include "utils/DoFManager.hpp"
#include <Eigen/Sparse>

YAFEL_NAMESPACE_OPEN

/**
 * \class FESystem
 * \brief Class to hold all of the data structures that comprise a FEM simulation
 */
class FESystem
{

public:
    inline FESystem(DoFManager &dofm, int dim = 0)
            : dofm(dofm), simulation_dimension(dim), time(0)
    {
        int ndofs = dofm.dof_nodes.size() * dofm.dof_per_node;
        global_residual = Eigen::VectorXd::Constant(ndofs, 0.0);
        solution_vector = Eigen::VectorXd::Constant(ndofs, 0.0);
        global_tangent.resize(ndofs, ndofs);
    }

    inline auto &getGlobalTangent() { return global_tangent; }

    inline auto &getGlobalResidual() { return global_residual; }

    inline auto &getSolution() { return solution_vector; }

    inline auto &getSolutionGradient() { return solution_gradient; }

    inline auto &getDoFManager() { return dofm; }

    inline auto &getDimension() { return simulation_dimension; }

    inline auto &currentTime() { return time; }

    inline auto const &currentTime() const noexcept { return time; }

protected:
    DoFManager &dofm;
    Eigen::SparseMatrix<double, Eigen::ColMajor> global_tangent;
    Eigen::VectorXd global_residual;
    Eigen::VectorXd solution_vector;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic, Eigen::RowMajor> solution_gradient;

    int simulation_dimension;
    double time;
};

YAFEL_NAMESPACE_CLOSE

#endif //YAFEL_FESYSTEM_HPP
