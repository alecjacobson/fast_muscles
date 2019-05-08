#ifndef ACAP_SOLVE_ENERGY_GRADIENT
#define ACAP_SOLVE_ENERGY_GRADIENT

#include "store.h"
#include <igl/writeDMAT.h>
#include <igl/readDMAT.h>

using namespace famu;
using namespace std;
using Store = famu::Store;

namespace famu
{
	namespace acap
	{
		std::vector<Eigen::Triplet<double>> to_triplets(Eigen::SparseMatrix<double> & M){
			std::vector<Eigen::Triplet<double>> v;
			for(int i = 0; i < M.outerSize(); i++){
				for(typename Eigen::SparseMatrix<double>::InnerIterator it(M,i); it; ++it){	
					v.emplace_back(it.row(),it.col(),it.value());
				}
			}
			return v;
		}

		double energy(Store& store){
			SparseMatrix<double> DS = store.D*store.S;
			double E1 =  0.5*(store.D*store.S*(store.x+store.x0) - store.dF*store.DSx0).squaredNorm();

			double E2 = 0.5*store.x.transpose()*store.StDtDS*store.x;
			double E3 = store.x0.transpose()*store.StDtDS*store.x;
			double E4 = 0.5*store.x0.transpose()*store.StDtDS*store.x0;
			double E5 = -store.x.transpose()*DS.transpose()*store.dF*DS*store.x0;
			double E6 = -store.x0.transpose()*DS.transpose()*store.dF*DS*store.x0;
			double E7 = 0.5*(store.dF*store.DSx0).transpose()*(store.dF*store.DSx0);
			double E8 = E2+E3+E4+E5+E6+E7;
			assert(fabs(E1 - E8)< 1e-6);
			return store.alpha_arap*E1;
		}

		double fastEnergy(Store& store, VectorXd& dFvec){
			double E1 = 0.5*store.x0tStDtDSx0;
			double E2 = store.x0tStDtDSY.dot(store.x);
			double E3 = 0.5*store.x.transpose()*store.YtStDtDSY*store.x;
			double E4 = -store.x0tStDt_dF_DSx0.dot(dFvec);
			double E5 = -store.x.transpose()*store.YtStDt_dF_DSx0*dFvec;
			double E6 = 0.5*dFvec.transpose()*store.x0tStDt_dF_dF_DSx0*dFvec;
		 
			double E9 = E1+E2+E3+E4+E5+E6;
			return store.alpha_arap*E9;
		}

		void fastGradient(Store& store, VectorXd& grad){
			grad = -store.alpha_arap*store.x0tStDt_dF_DSx0;
			grad += -store.alpha_arap*store.x.transpose()*store.YtStDt_dF_DSx0;
			grad += store.alpha_arap*store.dFvec.transpose()*store.x0tStDt_dF_dF_DSx0;
		}

		void fastHessian(Store& store, SparseMatrix<double>& hess){
			// std::cout<<"here1: "<<spJac.nonZeros()<<std::endl;
			// hess = store.x0tStDt_dF_dF_DSx0;
			// std::cout<<"here2"<<std::endl;
			// SparseMatrix<double> part = store.YtStDtDSY*spJac;
			// std::cout<<"here2.5"<<std::endl;
			// SparseMatrix<double> part2= part.transpose()*spJac;
			// hess += part2;
			// std::cout<<"here3"<<std::endl;
			// // SparseMatrix<double> part3 = -store.YtStDt_dF_DSx0.transpose()*spJac;
			// // hess += part3.transpose();
			// std::cout<<"here4"<<std::endl;
			// hess += -2*store.YtStDt_dF_DSx0.transpose()*spJac;
			// std::cout<<"here5"<<std::endl;

			hess -= store.YtStDt_dF_DSx0.transpose()*store.JacdxdF;
			// igl::writeDMAT("acap_hess.dmat",MatrixXd(hess));
			hess = store.x0tStDt_dF_dF_DSx0;
			hess *= store.alpha_arap;
		}

		VectorXd fd_gradient(Store& store){
			VectorXd fake = VectorXd::Zero(store.dFvec.size());
			VectorXd dFvec = store.dFvec;
			double eps = 0.00001;
			for(int i=0; i<dFvec.size(); i++){
				dFvec[i] += 0.5*eps;
				double Eleft = fastEnergy(store, dFvec);
				dFvec[i] -= 0.5*eps;

				dFvec[i] -= 0.5*eps;
				double Eright = fastEnergy(store, dFvec);
				dFvec[i] += 0.5*eps;
				fake[i] = (Eleft - Eright)/eps;
			}
			return fake;
		}

		MatrixXd fd_hessian(Store& store){
			MatrixXd fake = MatrixXd::Zero(store.dFvec.size(), store.dFvec.size());
			VectorXd dFvec = store.dFvec;
			double eps = 1e-3;
			double E0 = fastEnergy(store, dFvec);
			for(int i=0; i<11; i++){
				for(int j=0; j<11; j++){
					dFvec[i] += eps;
					dFvec[j] += eps;
					double Eij = fastEnergy(store, dFvec);
					dFvec[i] -= eps;
					dFvec[j] -= eps;

					dFvec[i] += eps;
					double Ei = fastEnergy(store, dFvec);
					dFvec[i] -=eps;

					dFvec[j] += eps;
					double Ej = fastEnergy(store, dFvec);
					dFvec[j] -=eps;

					fake(i,j) = ((Eij - Ei - Ej + E0)/(eps*eps));
				}
			}
			return fake;
		}

		void solve(Store& store, VectorXd& dFvec){

			VectorXd zer = VectorXd::Zero(store.JointConstraints.rows());
			// VectorXd constrains = store.ConstrainProjection.transpose()*store.dx;
			VectorXd top = store.YtStDt_dF_DSx0*dFvec - store.x0tStDtDSY;
			VectorXd KKT_right(top.size() + zer.size());
			KKT_right<<top, zer;
			// igl::writeDMAT("rhs.dmat", KKT_right);

			VectorXd result = store.SPLU.solve(KKT_right);
			store.x = result.head(top.size());
		
		}

		void setJacobian(Store& store){
			// std::string outputfile = store.jinput["output"];
			// igl::readDMAT(outputfile+"/dxdF.dmat", store.JacdxdF);
			// igl::readDMAT(outputfile+"/dlambdadF.dmat", store.JacdlambdadF);
			// if(store.JacdxdF.rows()!=0){
			// 	return;
			// }
			SparseMatrix<double> rhs = store.YtStDt_dF_DSx0.leftCols(9*store.bone_tets.size());
			MatrixXd top = MatrixXd(rhs);
			// MatrixXd zer = MatrixXd::Zero(store.JointConstraints.rows(), top.cols());

			// MatrixXd KKT_right(top.rows() + zer.rows(), top.cols());
			// KKT_right<<top, zer;

			UmfPackLU<SparseMatrix<double>> SPLU;
			SPLU.compute(store.YtStDtDSY);
			MatrixXd result = SPLU.solve(top);

			SparseMatrix<double> spRes = result.sparseView();

			std::vector<Trip> res_trips = to_triplets(spRes);
			SparseMatrix<double> spJac(spRes.rows(), store.dFvec.size());
			spJac.setFromTriplets(res_trips.begin(), res_trips.end());


			store.JacdxdF = spJac;
			cout<<"jac dims: "<<store.JacdxdF.rows()<<", "<<store.JacdxdF.cols()<<", "<<store.JacdxdF.nonZeros()<<endl;

			// igl::writeDMAT(outputfile+"/dxdF.dmat", store.JacdxdF);
			// igl::writeDMAT(outputfile+"/dlambdadF.dmat", store.JacdlambdadF);
		
		}

		
	}

}

#endif