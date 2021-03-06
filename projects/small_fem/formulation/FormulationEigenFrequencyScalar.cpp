#include "BasisGenerator.h"
#include "GroupOfJacobian.h"
#include "Quadrature.h"

#include "Exception.h"
#include "FormulationEigenFrequencyScalar.h"

using namespace std;

const double FormulationEigenFrequencyScalar::cSquare = 1;

FormulationEigenFrequencyScalar::
FormulationEigenFrequencyScalar(GroupOfElement& goe,
                                size_t order){
  // Can't have 0th order //
  if(order == 0)
    throw
      Exception("Can't have a scalar Eigen Frequency formulation of order 0");

  // Function Space & Basis //
  basis  = BasisGenerator::generate(goe.get(0).getType(),
                                    0, order, "hierarchical");

  fspace = new FunctionSpaceScalar(goe, *basis);

  // Gaussian Quadrature //
  Quadrature gaussGradGrad(goe.get(0).getType(), order - 1, 2);
  Quadrature gaussFF(goe.get(0).getType(), order, 2);

  const fullMatrix<double>& gC1 = gaussGradGrad.getPoints();
  const fullVector<double>& gW1 = gaussGradGrad.getWeights();

  const fullMatrix<double>& gC2 = gaussFF.getPoints();
  const fullVector<double>& gW2 = gaussFF.getWeights();

  // Local Terms //
  basis->preEvaluateDerivatives(gC1);
  basis->preEvaluateFunctions(gC2);

  GroupOfJacobian jac1(goe, *basis, gC1, "invert");
  GroupOfJacobian jac2(goe, *basis, gC2, "jacobian");

  localTerms1 = new TermGradGrad(jac1, *basis, gW1);
  localTerms2 = new TermFieldField(jac2, *basis, gW2);
}

FormulationEigenFrequencyScalar::~FormulationEigenFrequencyScalar(void){
  delete basis;
  delete fspace;

  delete localTerms1;
  delete localTerms2;
}

std::complex<double>
FormulationEigenFrequencyScalar::weak(size_t dofI, size_t dofJ,
                                      size_t elementId) const{

  return std::complex<double>
    (localTerms1->getTerm(dofI, dofJ, elementId), 0);
}


std::complex<double>
FormulationEigenFrequencyScalar::weakB(size_t dofI, size_t dofJ,
                                       size_t elementId) const{

  return std::complex<double>
    (localTerms2->getTerm(dofI, dofJ, elementId) / cSquare, 0);
}

std::complex<double>
FormulationEigenFrequencyScalar::rhs(size_t equationI,
                                     size_t elementId) const{
  return std::complex<double>(0, 0);
}

bool FormulationEigenFrequencyScalar::isGeneral(void) const{
  return true;
}

const FunctionSpace& FormulationEigenFrequencyScalar::fs(void) const{
  return *fspace;
}
