#include "ExactSolution.h"
#include "Exception.h"

ExactSolution::ExactSolution(void){
  nodalScalarValue = NULL;
  nodalVectorValue = NULL;
}

ExactSolution::~ExactSolution(void){
  if(scalar)
    delete nodalScalarValue;

  else
    delete nodalVectorValue;
}

void ExactSolution::write(const std::string name, Writer& writer) const{
  // Set Writer
  writer.setDomain(domain->getAll());

  if(scalar)
    writer.setValues(*nodalScalarValue);
  
  else
    writer.setValues(*nodalVectorValue);
  
  // Write
  writer.write(name);
}

void ExactSolution::compute(void){
  // Init
  const Mesh& mesh                       = domain->getMesh();
  const unsigned int nTotVertex          = mesh.getVertexNumber();
  const std::vector<const MVertex*> node = mesh.getAllVertex(); 

  // Scalar or Vector ?
  if(scalar)
    nodalScalarValue = new std::vector<double>(nTotVertex);

  else
    nodalVectorValue = new std::vector<fullVector<double> >(nTotVertex);

  // Iterate on Nodes
  for(unsigned int i = 0; i < nTotVertex; i++){	
    // Get Node coordinate
    double x = node[i]->x();
    double y = node[i]->y();
    double z = node[i]->z();

    // Compute f (Scalar Or Vector)
    if(scalar)
      (*nodalScalarValue)[node[i]->getNum() - 1] = 
	fScalar(x, y, z);
    
    else
      (*nodalVectorValue)[node[i]->getNum() - 1] = 
	fVector(x, y, z);
  }
}

double ExactSolution::fScalar(double x, double y, double z){
  return 42;
}

fullVector<double> ExactSolution::fVector(double x, double y, double z){
  fullVector<double> v(3);

  v(0) = 42;
  v(1) = 42;
  v(2) = 42;

  return v;
}

std::vector<double>& ExactSolution::getNodalScalarValue(void) const{
  if(!scalar)
    throw Exception("Solution: try to get Scalar value in a Vectorial Solution");

  else
    return *nodalScalarValue;
}

std::vector<fullVector<double> >& ExactSolution::getNodalVectorValue(void) const{
  if(scalar)
    throw Exception("Solution: try to get Vectorial value in a Scalar Solution");

  else
    return *nodalVectorValue;  
}