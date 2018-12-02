#include <igl/opengl/glfw/Viewer.h>
#include <igl/writeOBJ.h>
#include <igl/barycenter.h>
#include <igl/readOFF.h>
#include <igl/readMESH.h>
#include <igl/readOBJ.h>
#include <igl/slice.h>
#include <unsupported/Eigen/NumericalDiff>


#include <json.hpp>

#include "mesh.h"
#include "arap.h"
// #include "elastic.h"
// #include "solver.h"


using json = nlohmann::json;

using namespace Eigen;
using namespace std;
json j_input;

std::vector<int> getMaxVerts_Axis_Tolerance(MatrixXd& mV, int dim, double tolerance=1e-5){
    auto maxX = mV.col(dim).maxCoeff();
    std::vector<int> maxV;
    for(unsigned int ii=0; ii<mV.rows(); ++ii) {

        if(fabs(mV(ii,dim) - maxX) < tolerance) {
            maxV.push_back(ii);
        }
    }
    return maxV;
}

std::vector<int> getMinVerts_Axis_Tolerance(MatrixXd& mV, int dim, double tolerance=1e-5){
    auto maxX = mV.col(dim).minCoeff();
    std::vector<int> maxV;
    for(unsigned int ii=0; ii<mV.rows(); ++ii) {

        if(fabs(mV(ii,dim) - maxX) < tolerance) {
            maxV.push_back(ii);
        }
    }
    return maxV;
}

int checkARAP(Mesh& mesh, Arap& arap){
	double eps = j_input["fd_eps"];
	double E0 = arap.Energy(mesh);
	std::cout<<"Energy0: "<<E0<<std::endl;
	//CHECK E,x-------------
	auto Ex = [&mesh, &arap, E0, eps](){
		VectorXd real = arap.dEdx(mesh);
		VectorXd z = mesh.red_x();
		VectorXd fake = VectorXd::Zero(real.size());
		#pragma omp parallel for
		for(int i=0; i<fake.size(); i++){
			z[i] += 0.5*eps;
			double Eleft = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			z[i] -= 0.5*eps;
			z[i] -= 0.5*eps;
			double Eright = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			z[i] += 0.5*eps;
			fake[i] = (Eleft - Eright)/eps;
		}
		std::cout<<"Ex error:"<<(real-fake).norm()<<std::endl;	
	};
	//-----------------------

	//CHECK E,r-------------
	auto Er = [&mesh, &arap, E0, eps](){
		VectorXd real = arap.dEdr(mesh);
		VectorXd z = mesh.red_x();
		VectorXd fake = VectorXd::Zero(real.size());
		for(int i=0; i<fake.size(); i++){
			mesh.red_r()[i] += 0.5*eps;
			mesh.setGlobalF(true, false, false);
			double Eleft = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			mesh.red_r()[i] -= 0.5*eps;
			mesh.red_r()[i] -= 0.5*eps;
			mesh.setGlobalF(true, false, false);
			double Eright = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			mesh.red_r()[i] += 0.5*eps;
			fake[i] = (Eleft - Eright)/eps;
		}
		mesh.setGlobalF(true, false, false);
		std::cout<<"Er error:"<<(real-fake).norm()<<std::endl;	
		std::cout<<fake.transpose()<<std::endl;
		std::cout<<real.transpose()<<std::endl;
	};
	//-----------------------

	//CHECK E,s-------------
	auto Es = [&mesh, &arap, E0, eps](){
		VectorXd real = arap.dEds(mesh);
		VectorXd z = mesh.red_x();
		VectorXd fake = VectorXd::Zero(real.size());
		for(int i=0; i<fake.size(); i++){
			mesh.red_s()[i] += 0.5*eps;
			mesh.setGlobalF(false, true, false);
			double Eleft = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			mesh.red_s()[i] -= 0.5*eps;
			mesh.red_s()[i] -= 0.5*eps;
			mesh.setGlobalF(false, true, false);
			double Eright = arap.Energy(mesh, z, mesh.GR(), mesh.GS(), mesh.GU());
			mesh.red_s()[i] += 0.5*eps;
			fake[i] = (Eleft - Eright)/eps;
		}
		mesh.setGlobalF(false, true, false);
		std::cout<<"Es error:"<<(real-fake).norm()<<std::endl;	
		std::cout<<fake.transpose()<<std::endl;
		std::cout<<real.transpose()<<std::endl;
	};
	//-----------------------

	Ex();
	Er();
	Es();

}

int main(int argc, char *argv[])
{
    std::cout<<"-----Configs-------"<<std::endl;
    json j_config_parameters;
    std::ifstream i("../input/input.json");
    i >> j_input;

    
    MatrixXd V;
    MatrixXi T;
    MatrixXi F;
    igl::readMESH(j_input["mesh_file"], V, T, F);
    
    std::vector<int> fix = getMaxVerts_Axis_Tolerance(V, 1);
    std::sort (fix.begin(), fix.end());
    std::vector<int> mov = {};
    std::sort (mov.begin(), mov.end());

    std::cout<<"-----Mesh-------"<<std::endl;
    Mesh* mesh = new Mesh(T, V, fix, mov, j_input);
    mesh->red_s()[0] -=0.1;
    mesh->red_r()[0] += 0.1;
    mesh->setGlobalF(true, true, false);
    std::cout<<"-----ARAP-----"<<std::endl;
    Arap* arap = new Arap(*mesh);

    checkARAP(*mesh, *arap);

    // std::cout<<"-----Neo-------"<<std::endl;
    // Elastic* neo = new Elastic(*mesh);


}