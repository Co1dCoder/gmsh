#include <sstream>
#include "Exception.h"
#include "BasisHierarchical1Form.h"

using namespace std;

BasisHierarchical1Form::BasisHierarchical1Form(void){
  // Scalar Basis ?//
  scalar = false;

  // Curl Basis //
  hasCurl = false;
  curl    = NULL;

  // PreEvaluation //
  preEvaluated     = false;
  preEvaluatedCurl = false;

  preEvaluatedFunction     = NULL;
  preEvaluatedCurlFunction = NULL;
}

BasisHierarchical1Form::~BasisHierarchical1Form(void){
  // Curl Basis //
  if(hasCurl){
    for(size_t i = 0; i < nRefSpace; i++){
      for(size_t j = 0; j < nFunction; j++)
        delete curl[i][j];

      delete[] curl[i];
    }

    delete[] curl;
  }

  // PreEvaluation //
  if(preEvaluated){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedFunction[i];

    delete[] preEvaluatedFunction;
  }

  if(preEvaluatedCurl){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedCurlFunction[i];

    delete[] preEvaluatedCurlFunction;
  }
}

void BasisHierarchical1Form::
getFunctions(fullMatrix<double>& retValues,
             const MElement& element,
             double u, double v, double w) const{

  // Define Orientation //
  size_t orientation = refSpace->getReferenceSpace(element);

  // Fill Vector //
  for(size_t i = 0; i < nFunction; i++){
    fullVector<double> eval =
      Polynomial::at(*basis[orientation][i], u, v, w);

    retValues(i, 0) = eval(0);
    retValues(i, 1) = eval(1);
    retValues(i, 2) = eval(2);
  }
}

void BasisHierarchical1Form::
getFunctions(fullMatrix<double>& retValues,
             size_t orientation,
             double u, double v, double w) const{

  // Fill Vector //
  for(size_t i = 0; i < nFunction; i++){
    fullVector<double> eval =
      Polynomial::at(*basis[orientation][i], u, v, w);

    retValues(i, 0) = eval(0);
    retValues(i, 1) = eval(1);
    retValues(i, 2) = eval(2);
  }
}

void BasisHierarchical1Form::getDerivative(fullMatrix<double>& retValues,
                                           const MElement& element,
                                           double u, double v, double w) const{
  throw Exception("Not Implemented");
}

void BasisHierarchical1Form::
preEvaluateFunctions(const fullMatrix<double>& point) const{
  // Delete if older //
  if(preEvaluated){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedFunction[i];

    delete[] preEvaluatedFunction;
  }

  // Alloc //
  const size_t nPoint  = point.size1();
  const size_t nPoint3 = nPoint * 3;
  preEvaluatedFunction       = new fullMatrix<double>*[nRefSpace];

  for(size_t i = 0; i < nRefSpace; i++)
    preEvaluatedFunction[i] =
      new fullMatrix<double>(nFunction, nPoint3);

  // Fill Matrix //
  fullVector<double> tmp(3);

  for(size_t i = 0; i < nRefSpace; i++){
    for(size_t j = 0; j < nFunction; j++){
      for(size_t k = 0; k < nPoint; k++){
        tmp = Polynomial::at(*basis[i][j],
                             point(k, 0),
                             point(k, 1),
                             point(k, 2));

        (*preEvaluatedFunction[i])(j, 3 * k)     = tmp(0);
        (*preEvaluatedFunction[i])(j, 3 * k + 1) = tmp(1);
        (*preEvaluatedFunction[i])(j, 3 * k + 2) = tmp(2);
      }
    }
  }

  // PreEvaluated //
  preEvaluated = true;
}

void BasisHierarchical1Form::
preEvaluateDerivatives(const fullMatrix<double>& point) const{
  // Build Curl //
  if(!hasCurl)
    getCurl();

  // Delete if older //
  if(preEvaluatedCurl){
    for(size_t i = 0; i < nRefSpace; i++)
      delete preEvaluatedCurlFunction[i];

    delete[] preEvaluatedCurlFunction;
  }

  // Alloc //
  const size_t nPoint  = point.size1();
  const size_t nPoint3 = nPoint * 3;
  preEvaluatedCurlFunction   = new fullMatrix<double>*[nRefSpace];

  for(size_t i = 0; i < nRefSpace; i++)
    preEvaluatedCurlFunction[i] =
      new fullMatrix<double>(nFunction, nPoint3);

  // Fill Matrix //
  fullVector<double> tmp(3);

  for(size_t i = 0; i < nRefSpace; i++){
    for(size_t j = 0; j < nFunction; j++){
      for(size_t k = 0; k < nPoint; k++){
        tmp = Polynomial::at(*curl[i][j],
                             point(k, 0),
                             point(k, 1),
                             point(k, 2));

        (*preEvaluatedCurlFunction[i])(j, 3 * k)     = tmp(0);
        (*preEvaluatedCurlFunction[i])(j, 3 * k + 1) = tmp(1);
        (*preEvaluatedCurlFunction[i])(j, 3 * k + 2) = tmp(2);
      }
    }
  }

  // PreEvaluated //
  preEvaluatedCurl = true;
}

const fullMatrix<double>& BasisHierarchical1Form::
getPreEvaluatedFunctions(const MElement& element) const{
  return getPreEvaluatedFunctions(refSpace->getReferenceSpace(element));
}

const fullMatrix<double>& BasisHierarchical1Form::
getPreEvaluatedDerivatives(const MElement& element) const{
  return getPreEvaluatedDerivatives(refSpace->getReferenceSpace(element));
}

const fullMatrix<double>& BasisHierarchical1Form::
getPreEvaluatedFunctions(size_t orientation) const{
  if(!preEvaluated)
    throw Exception("getPreEvaluatedFunction: function has not been preEvaluated");

  return *preEvaluatedFunction[orientation];
}

const fullMatrix<double>& BasisHierarchical1Form::
getPreEvaluatedDerivatives(size_t orientation) const{
  if(!preEvaluatedCurl)
    throw Exception("getPreEvaluatedDerivative: curl has not been preEvaluated");

  return *preEvaluatedCurlFunction[orientation];
}

void BasisHierarchical1Form::getCurl(void) const{
  // Alloc //
  curl = new vector<Polynomial>**[nRefSpace];

  for(size_t s = 0; s < nRefSpace; s++)
    curl[s] = new vector<Polynomial>*[nFunction];

  // Curl //
  for(size_t s = 0; s < nRefSpace; s++)
    for(size_t f = 0 ; f < nFunction; f++)
      curl[s][f] =
        new vector<Polynomial>(Polynomial::curl(*basis[s][f]));

  // Has Curl //
  hasCurl = true;
}

string BasisHierarchical1Form::toString(void) const{
  stringstream stream;
  size_t i = 0;
  const size_t refSpace = 0;

  stream << "Vertex Based:" << endl;
  for(; i < nVertex; i++)
    stream << "f("   << i + 1                               << ") = " << endl
           << "\t[ " << (*basis[refSpace][i])[0].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[1].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[2].toString() << " ]"   << endl;

  stream << "Edge Based:"   << endl;
  for(; i < nVertex + nEdge; i++)
    stream << " f("  << i + 1                               << ") = " << endl
           << "\t[ " << (*basis[refSpace][i])[0].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[1].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[2].toString() << " ]"   << endl;

  stream << "Face Based:"   << endl;
  for(; i < nVertex + nEdge + nFace; i++)
    stream << " f("  << i + 1                               << ") = " << endl
           << "\t[ " << (*basis[refSpace][i])[0].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[1].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[2].toString() << " ]"   << endl;

  stream << "Cell Based:"   << endl;
  for(; i < nVertex + nEdge + nFace + nCell; i++)
    stream << " f("  << i + 1                               << ") = " << endl
           << "\t[ " << (*basis[refSpace][i])[0].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[1].toString() << " ]"   << endl
           << "\t[ " << (*basis[refSpace][i])[2].toString() << " ]"   << endl;

  return stream.str();
}
