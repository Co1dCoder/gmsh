#include "Exception.h"
#include "TermGradGrad.h"

using namespace std;

TermGradGrad::TermGradGrad(const GroupOfJacobian& goj,
                           const Basis& basis,
                           const fullVector<double>& integrationWeights){
  // Basis Check //
  bFunction getFunction;

  switch(basis.getType()){
  case 0:
    getFunction = &Basis::getPreEvaluatedDerivatives;
    break;

  case 1:
    getFunction = &Basis::getPreEvaluatedFunctions;
    break;

  default:
    throw
      Exception
      ("A Grad Grad Term must use a 1form basis, or a (gradient of) 0form basis");
  }

  // Orientations & Functions //
  orientationStat = &goj.getAllElements().getOrientationStats();
  nOrientation    = basis.getReferenceSpace().getNReferenceSpace();
  nFunction       = basis.getNFunction();

  // Compute //
  fullMatrix<double>** cM;
  fullMatrix<double>** bM;

  computeC(basis, getFunction, integrationWeights, cM);
  computeB(goj, integrationWeights.size(), bM);

  allocA(nFunction * nFunction);
  computeA(bM, cM);

  // Clean up //
  clean(bM, cM);
}

TermGradGrad::~TermGradGrad(void){
}

void TermGradGrad::computeC(const Basis& basis,
                            const bFunction& getFunction,
                            const fullVector<double>& gW,
                            fullMatrix<double>**& cM){

  const size_t nG = gW.size();

  // Alloc //
  cM = new fullMatrix<double>*[nOrientation];

  for(size_t s = 0; s < nOrientation; s++)
    cM[s] = new fullMatrix<double>(9 * nG, nFunction * nFunction);

  // Fill //
  //#pragma omp parallel
  for(size_t s = 0; s < nOrientation; s++){

    // Get functions for this Orientation
    const fullMatrix<double>& phi =
      (basis.*getFunction)(s);

    // fullMatrix is in *Column-major* //
    //  --> iterate on column first    //
    //       --> iterate on functions  //

    // Loop on Functions
    //#pragma omp for
    for(size_t i = 0; i < nFunction; i++){
      for(size_t j = 0; j < nFunction; j++){

        // Loop on Gauss Points
        for(size_t g = 0; g < nG; g++){
          for(size_t a = 0; a < 3; a++){
            for(size_t b = 0; b < 3; b++){
              (*cM[s])(g * 9 + a * 3 + b, i * nFunction + j) =
                gW(g) * phi(i, g * 3 + a) * phi(j, g * 3 + b);
            }
          }
        }
      }
    }
  }
}

void TermGradGrad::computeB(const GroupOfJacobian& goj,
                            size_t nG,
                            fullMatrix<double>**& bM){
  size_t offset = 0;
  size_t j;
  size_t k;

  // Alloc //
  bM = new fullMatrix<double>*[nOrientation];

  for(size_t s = 0; s < nOrientation; s++)
    bM[s] = new fullMatrix<double>((*orientationStat)[s], 9 * nG);

  // Fill //

  // Despite that fullMatrix is Column-major  //
  // Row-major fill seems faster for matrix B //

  for(size_t s = 0; s < nOrientation; s++){
    // Loop On Element
    j = 0;

    for(size_t e = offset; e < offset + (*orientationStat)[s]; e++){
      // Get Jacobians
      const vector<const pair<const fullMatrix<double>*, double>*>& invJac =
        goj.getJacobian(e).getInvertJacobianMatrix();

      // Loop on Gauss Points
      k = 0;

      for(size_t g = 0; g < nG; g++){
        for(size_t a = 0; a < 3; a++){
          for(size_t b = 0; b < 3; b++){
            (*bM[s])(j, k) = 0;

            for(size_t i = 0; i < 3; i++)
              (*bM[s])(j, k) +=
                (*invJac[g]->first)(i, a) * (*invJac[g]->first)(i, b);

            (*bM[s])(j, k) *= fabs(invJac[g]->second);

            k++;
          }
        }
      }

      // Next Element in Orientation[s]
      j++;
    }

    // New Offset
    offset += (*orientationStat)[s];
  }
}
