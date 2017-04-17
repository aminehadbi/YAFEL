//
// Created by tyler on 3/14/17.
//

#include "yafel_globals.hpp"
#include "assembly/CGAssembly.hpp"
#include "boundary_conditions/DirichletBC.hpp"

#include "output/OutputData.hpp"
#include "output/OutputMesh.hpp"
#include "output/OutputFrame.hpp"
#include "output/VTUBackend.hpp"

#include <eigen3/Eigen/IterativeLinearSolvers>
#include <iostream>


using namespace yafel;

template<int NSD>
struct PoissonEquation
{
    static void LocalResidual(const Element &E, int qpi, double, Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 1>> &R_el)
    {
        /*
        for(auto A : IRange(0, static_cast<int>(R_el.rows()))) {
            R_el(A) += E.shapeValues[qpi](A)*E.jxw;
        }
         */
    }

    static void LocalTangent(const Element &E, int qpi, double,
                             Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> & K_el)
    {
        for(auto A: IRange(0,static_cast<int>(K_el.rows()))) {
            for(auto B: IRange(A,static_cast<int>(K_el.rows()))) {
                for(auto i : IRange(0,NSD)) {
                    K_el(A,B) -= E.shapeGrad(A,i)*E.shapeGrad(B,i)*E.jxw;
                }

                K_el(B,A) = K_el(A,B);
            }

        }
    }
};

Eigen::VectorXd solveSystem(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &rhs) {
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;
    solver.compute(A);
    //solver.factorize(A);
    return solver.solve(rhs);
}

int main()
{
    Mesh M("minsquare.msh");
    int p = 3;
    int dofpn = 1;
    DoFManager dofm(M, DoFManager::ManagerType::CG, p, dofpn);
    FESystem feSystem(dofm);
    CGAssembly<3, PoissonEquation<3>>(feSystem);

    DirichletBC bc0(dofm,0.0);
    bc0.selectByRegionID(1);
    DirichletBC bc1(dofm,1.0);
    bc1.selectByRegionID(2);

    bc0.apply(feSystem.getGlobalTangent(), feSystem.getGlobalResidual());
    bc1.apply(feSystem.getGlobalTangent(), feSystem.getGlobalResidual());

    Eigen::VectorXd U(feSystem.getGlobalResidual().rows());
    U = solveSystem(feSystem.getGlobalTangent(), feSystem.getGlobalResidual());

    OutputMesh outputMesh(dofm);
    OutputData data(U, "Solution");
    OutputFrame frame(outputMesh);
    frame.point_data.push_back(&data);

    VTUBackend vtuBackend;
    vtuBackend.initialize("first_solve");
    vtuBackend.write_frame(frame);
    vtuBackend.finalize();

    return 0;
}


