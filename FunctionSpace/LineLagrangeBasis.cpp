#include "LineLagrangeBasis.h"
#include "LineReferenceSpace.h"
#include "pointsGenerators.h"
#include "ElementType.h"

LineLagrangeBasis::LineLagrangeBasis(size_t order){
  // If order 0 (Nedelec): use order 1
  if(order == 0)
    order = 1;

  // Set Basis Type //
  this->order = order;

  type = 0;
  dim  = 1;

  nVertex   = 2;
  nEdge     = (order - 1);
  nFace     = 0;
  nCell     = 0;
  nFunction = nVertex + nEdge + nFace + nCell;

  // Init polynomialBasis //
  lBasis = new polynomialBasis(ElementType::getTag(TYPE_LIN, order, false));

  // Init Lagrange Point //
  lPoint = new fullMatrix<double>(gmshGeneratePointsLine(order));

  // Reference Space //
  refSpace  = new LineReferenceSpace;
  nRefSpace = getReferenceSpace().getNReferenceSpace();
}

LineLagrangeBasis::~LineLagrangeBasis(void){
  delete lBasis;
  delete lPoint;
  delete refSpace;
}
