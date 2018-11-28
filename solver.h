#ifndef SOLVER
#define SOLVER

#include <iostream>
#include "mesh.h"
#include "arap.h"
#include "elastic.h"
#include "cppoptlib/meta.h"
#include "cppoptlib/boundedproblem.h"
#include "cppoptlib/solver/lbfgsbsolver.h"

using namespace cppoptlib;
using Eigen::VectorXd;
template<typename T>
class StaticSolve : public BoundedProblem<T> {
private:
    Mesh* mesh;
    Arap* arap;
    Elastic* elas;

public:
    using typename cppoptlib::BoundedProblem<T>::Scalar;
    using typename cppoptlib::BoundedProblem<T>::TVector;

    StaticSolve(int dim, Mesh* m, Arap* a, Elastic* e): BoundedProblem<T>(dim) {
        mesh = m;
        arap = a;
        elas = e;

    }
    
    double value(const TVector &x) {
        for(int i=0; i<x.size(); i++){
            mesh->s()[i] = x[i];
        }

        // arap->minimize(*mesh);
        double En = elas->Energy(*mesh);
        std::cout<<"Energy"<<std::endl;
        std::cout<<En<<std::endl;
        // std::cout<<x.transpose()<<std::endl;
        return En;
    }
    void gradient(const TVector &x, TVector &grad) {
        std::cout<<"Grad"<<std::endl;
        for(int i=0; i<x.size(); i++){
            mesh->s()[i] = x[i];
        }

        VectorXd pegrad = elas->PEGradient(*mesh);
        for(int i=0; i< pegrad.size(); i++){
            grad[i] = pegrad[i];
        }
        // std::cout<<pegrad.transpose()<<std::endl;
    }
};

#endif