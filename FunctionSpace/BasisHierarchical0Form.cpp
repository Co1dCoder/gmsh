#include <sstream>
#include "Exception.h"
#include "BasisHierarchical0Form.h"

using namespace std;

BasisHierarchical0Form::BasisHierarchical0Form(void){
  // Scalar Basis ? //
  scalar = true;

  // Grad Basis //
  hasGrad = false;
  grad    = NULL;

  // PreEvaluation //
  preEvaluated     = false;
  preEvaluatedGrad = false;

  preEvaluatedFunction     = NULL;
  preEvaluatedGradFunction = NULL;
}

BasisHierarchical0Form::~BasisHierarchical0Form(void){
  // Grad Basis //
  if(hasGrad){
    for(size_t i = 0; i < nRefSpace; i++){
      for(size_t j = 0; j < nFunction; j++)
        delete grad[i][j];

      delete[] grad[i];
    }

    delete[] grad;
  }

  // PreEvaluation //
  if(preEvaluated){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedFunction[i];

    delete[] preEvaluatedFunction;
  }

  if(preEvaluatedGrad){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedGradFunction[i];

    delete[] preEvaluatedGradFunction;
  }
}

void BasisHierarchical0Form::
getFunctions(fullMatrix<double>& retValues,
             const MElement& element,
             double u, double v, double w) const{

  // Define Orientation //
  const size_t orientation = refSpace->getReferenceSpace(element);

  // Fill Matrix //
  for(size_t i = 0; i < nFunction; i++)
    retValues(i, 0) = basis[orientation][i]->at(u, v, w);
}

void BasisHierarchical0Form::
getFunctions(fullMatrix<double>& retValues,
             size_t orientation,
             double u, double v, double w) const{

  // Fill Matrix //
  for(size_t i = 0; i < nFunction; i++)
    retValues(i, 0) = basis[orientation][i]->at(u, v, w);
}

void BasisHierarchical0Form::getDerivative(fullMatrix<double>& retValues,
                                           const MElement& element,
                                           double u, double v, double w) const{
  // Get Grad //
  if(!hasGrad)
    getGrad();

  // Define Orientation //
  const size_t orientation = refSpace->getReferenceSpace(element);

  // Fill Matrix //
  for(size_t i = 0; i < nFunction; i++){
    retValues(i, 0) = grad[orientation][i]->at(0).at(u, v, w);
    retValues(i, 1) = grad[orientation][i]->at(1).at(u, v, w);
    retValues(i, 2) = grad[orientation][i]->at(2).at(u, v, w);
  }
}

void BasisHierarchical0Form::
preEvaluateFunctions(const fullMatrix<double>& point) const{
  // Delete if older //
  if(preEvaluated){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedFunction[i];

    delete[] preEvaluatedFunction;
  }

  // Alloc //
  const size_t nPoint = point.size1();
  preEvaluatedFunction      = new fullMatrix<double>*[nRefSpace];

  for(size_t i = 0; i < nRefSpace; i++)
    preEvaluatedFunction[i] =
      new fullMatrix<double>(nFunction, nPoint);

  // Fill Matrix //
  for(size_t i = 0; i < nRefSpace; i++)
    for(size_t j = 0; j < nFunction; j++)
      for(size_t k = 0; k < nPoint; k++)
        (*preEvaluatedFunction[i])(j, k) =
          basis[i][j]->at(point(k, 0),
                          point(k, 1),
                          point(k, 2));

  // PreEvaluated //
  preEvaluated = true;
}

void BasisHierarchical0Form::
preEvaluateDerivatives(const fullMatrix<double>& point) const{
  // Build Grad //
  if(!hasGrad)
    getGrad();

  // Delete if older //
  if(preEvaluatedGrad){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedGradFunction[i];

    delete[] preEvaluatedGradFunction;
  }

  // Alloc //
  const size_t nPoint  = point.size1();
  const size_t nPoint3 = nPoint * 3;
  preEvaluatedGradFunction   = new fullMatrix<double>*[nRefSpace];

  for(size_t i = 0; i < nRefSpace; i++)
    preEvaluatedGradFunction[i] =
      new fullMatrix<double>(nFunction, nPoint3);

  // Fill Matrix //
  fullVector<double> tmp(3);

  for(size_t i = 0; i < nRefSpace; i++){
    for(size_t j = 0; j < nFunction; j++){
      for(size_t k = 0; k < nPoint; k++){
        tmp = Polynomial::at(*grad[i][j],
                             point(k, 0),
                             point(k, 1),
                             point(k, 2));

        (*preEvaluatedGradFunction[i])(j, 3 * k)     = tmp(0);
        (*preEvaluatedGradFunction[i])(j, 3 * k + 1) = tmp(1);
        (*preEvaluatedGradFunction[i])(j, 3 * k + 2) = tmp(2);
      }
    }
  }

  // PreEvaluated //
  preEvaluatedGrad = true;
}

const fullMatrix<double>& BasisHierarchical0Form::
getPreEvaluatedFunctions(const MElement& element) const{
  return getPreEvaluatedFunctions(refSpace->getReferenceSpace(element));
}

const fullMatrix<double>& BasisHierarchical0Form::
getPreEvaluatedDerivatives(const MElement& element) const{
  return getPreEvaluatedDerivatives(refSpace->getReferenceSpace(element));
}

const fullMatrix<double>& BasisHierarchical0Form::
getPreEvaluatedFunctions(size_t orientation) const{
  if(!preEvaluated)
    throw Exception("getPreEvaluatedFunction: function has not been preEvaluated");

  return *preEvaluatedFunction[orientation];
}

const fullMatrix<double>& BasisHierarchical0Form::
getPreEvaluatedDerivatives(size_t orientation) const{
  if(!preEvaluatedGrad)
    throw Exception("getPreEvaluatedDerivative: gradient has not been preEvaluated");

  return *preEvaluatedGradFunction[orientation];
}

void BasisHierarchical0Form::getGrad(void) const{
  // Alloc //
  grad = new vector<Polynomial>**[nRefSpace];

  for(size_t s = 0; s < nRefSpace; s++)
    grad[s] = new vector<Polynomial>*[nFunction];

  // Grad //
  for(size_t s = 0; s < nRefSpace; s++)
    for(size_t f = 0 ; f < nFunction; f++)
      grad[s][f] =
        new vector<Polynomial>(basis[s][f]->gradient());

  // Has Grad //
  hasGrad = true;
}

string BasisHierarchical0Form::toString(void) const{
  stringstream stream;
  size_t i = 0;
  const size_t refSpace = 0;

  stream << "Vertex Based:" << endl;
  for(; i < nVertex; i++)
    stream << "f("  << i + 1                 << ") = "
           << basis[refSpace][i]->toString() << endl;

  stream << "Edge Based:"   << endl;
  for(; i < nVertex + nEdge; i++)
    stream << "f(" << i + 1                  << ") = "
           << basis[refSpace][i]->toString() << endl;

  stream << "Face Based:"   << endl;
  for(; i < nVertex + nEdge + nFace; i++)
    stream << "f(" << i + 1                  << ") = "
           << basis[refSpace][i]->toString() << endl;

  stream << "Cell Based:"   << endl;
  for(; i < nVertex + nEdge + nFace + nCell; i++)
    stream << "f("  << i + 1                 << ") = "
           << basis[refSpace][i]->toString() << endl;

  return stream.str();
}
