#include "BasisGenerator.h"
#include "GaussIntegration.h"
#include "GroupOfJacobian.h"

#include "Exception.h"
#include "FormulationVibration.h"

using namespace std;

FormulationVibration::FormulationVibration(GroupOfElement& goe,
					   unsigned int order){
  // Can't have 0th order //
  if(order == 0)
    throw
      Exception("Can't have a Vibration formulation of order 0");

  // Function Space & Basis //
  basis  = BasisGenerator::generate(goe.get(0).getType(),
                                    0, order, "hierarchical");

  goe.orientAllElements(*basis);
  fspace = new FunctionSpaceScalar(goe, *basis);

  // Gaussian Quadrature Data //
  // NB: We need to integrad a grad * grad !
  //     and order(grad f) = order(f) - 1
  fullMatrix<double> gC;
  fullVector<double> gW;

  // Look for 1st element to get element type
  // (We suppose only one type of Mesh !!)
  gaussIntegration::get(goe.get(0).getType(), (order - 1) + (order - 1), gC, gW);

  // Local Terms //
  basis->preEvaluateDerivatives(gC);

  GroupOfJacobian jac(goe, gC, "invert");

  localTerms = new TermGradGrad(jac, *basis, gW);
}

FormulationVibration::~FormulationVibration(void){
  delete basis;
  delete fspace;

  delete localTerms;
}

double FormulationVibration::weak(unsigned int dofI, unsigned int dofJ,
                                  unsigned int elementId) const{

  return localTerms->getTerm(dofI, dofJ, elementId);
}

bool FormulationVibration::isGeneral(void) const{
  return false;
}

double FormulationVibration::weakB(unsigned int dofI, unsigned int dofJ,
                                   unsigned int elementId) const{
  return 0;
}

double FormulationVibration::rhs(unsigned int equationI,
                                 unsigned int elementId) const{
  return 0;
}

const FunctionSpace& FormulationVibration::fs(void) const{
  return *fspace;
}
