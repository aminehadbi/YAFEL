//
// Created by tyler on 4/16/17.
//

#ifndef YAFEL_CGASSEMBLY_HPP
#define YAFEL_CGASSEMBLY_HPP

#include "yafel_globals.hpp"
#include "element/Element.hpp"
#include "element/ElementFactory.hpp"
#include "fe_system/FESystem.hpp"


#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <vector>

YAFEL_NAMESPACE_OPEN

enum class AssemblyRequirement : int
{
    Residual,
    Tangent,
    DtMass,
    DtDtMass
};

template<typename Physics>
constexpr bool hasLocalResidual()
{
    using namespace Eigen;
    return std::is_same<void,
            typename std::result_of<decltype(&Physics::LocalResidual)(Element const &, int, double,
            Map<Matrix<double,Dynamic,1>> &)>::type > ::value;

}

template<typename Physics>
constexpr bool hasLocalTangent()
{
    using namespace Eigen;
    return std::is_same<void, typename std::result_of<decltype(&Physics::LocalTangent)(Element const &, int, double,
            Map<Matrix<double,Dynamic,Dynamic,RowMajor>> &)>::type > ::value;
}


/**
 *
 * \brief General-purpose Continuous-Galerkin finite element assembly
 *
 * @tparam Physics Class that defines the local element matrix/vector construction in static void methods
 */
template<int NSD, typename Physics>
void CGAssembly(FESystem &feSystem,
                std::vector<AssemblyRequirement> requirements = {AssemblyRequirement::Residual,
                                                                        AssemblyRequirement::Tangent})
{
    static_assert(hasLocalResidual<Physics>(), "Must Implement static void method 'LocalResidual' in Physics class");
    static_assert(hasLocalTangent<Physics>(), "Must Implement static void method 'LocalTangent' in Physics class");


    // Unpack the FESystem
    auto &GlobalTangent = feSystem.getGlobalTangent();
    auto &GlobalResidual = feSystem.getGlobalResidual();
    auto &dofm = feSystem.getDoFManager();
    auto dof_per_node = dofm.dof_per_node;
    constexpr int simulation_dimension = NSD;
    auto time = feSystem.getTime();


    bool assemble_tangent{false};
    bool assemble_residual{false};
    bool assemble_dt_mass{false};
    bool assemble_dtdt_mass{false};
    for(auto req : requirements) {
        switch(req) {
            case AssemblyRequirement::Residual:
                assemble_residual = true; break;
            case AssemblyRequirement::Tangent:
                assemble_tangent = true; break;
            case AssemblyRequirement::DtMass:
                assemble_dt_mass = true; break;
            case AssemblyRequirement::DtDtMass:
                assemble_dtdt_mass = true; break;
        }
    }

    //storage buffers
    std::vector<double> local_tangent_buffer;
    std::vector<double> local_residual_buffer;
    std::vector<int> global_dof_buffer;
    std::vector<Eigen::Triplet<double>> tangent_triplets;


    //Create an ElementFactory
    ElementFactory EF;

    for (auto elnum : IRange(0, dofm.nCells())) {

        auto et = dofm.element_types[elnum];
        if (et.topoDim != simulation_dimension) {
            continue;
        }

        std::cout << elnum << std::endl;

        auto &E = EF.getElement(et);
        dofm.getGlobalDofs(elnum,global_dof_buffer);


        auto local_dofs = E.localMesh.nNodes() * dof_per_node;
        if(local_tangent_buffer.size() < local_dofs*local_dofs) {
            local_tangent_buffer.resize(local_dofs*local_dofs, 0.0);
        }
        if(local_residual_buffer.size() < local_dofs) {
            local_residual_buffer.resize(local_dofs, 0.0);
        }

        for(auto &x : local_tangent_buffer) {
            x = 0;
        }
        for(auto &x : local_residual_buffer) {
            x = 0;
        }

        Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>> local_tangent(local_tangent_buffer.data(), local_dofs, local_dofs);
        Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,1>> local_residual(local_residual_buffer.data(), local_dofs);

        auto nqp = E.nQP();
        for (auto qpi : IRange(0, nqp)) {
            E.update<NSD>(elnum, qpi, dofm);

            if(assemble_tangent) {
                Physics::LocalTangent(E, qpi, time, local_tangent);
            }
            if(assemble_residual) {
                Physics::LocalResidual(E, qpi, time, local_residual);
            }

        }//end quadrature point loop


        //Assemble into global
        //omp critical
        for(auto A : IRange(0,local_dofs)) {
            auto GA = global_dof_buffer[A];
            if (assemble_tangent || assemble_dt_mass || assemble_dtdt_mass) {
                for (auto B : IRange(0, local_dofs)) {
                    auto GB = global_dof_buffer[B];
                    tangent_triplets.emplace_back(GA, GB, local_tangent(A, B));
                }
            }
            if(assemble_residual) {
                GlobalResidual(GA) = local_residual(A);
            }
        }

    }// end element loop

    GlobalTangent.setFromTriplets(tangent_triplets.begin(), tangent_triplets.end());
}

YAFEL_NAMESPACE_CLOSE

#endif //YAFEL_CGASSEMBLY_HPP
