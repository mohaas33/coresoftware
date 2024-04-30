// $Id: $

/*!
 * \file PHG4MagneticField.cc
 * \brief
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#include "PHG4MagneticField.h"

#include <phfield/PHField.h>

#include <cassert>

PHG4MagneticField::PHG4MagneticField(const PHField* field)
  : field_(field)
{
  assert(field_);
}

void PHG4MagneticField::GetFieldValue(const double Point[4], double* Bfield) const
{
  assert(field_);

  field_->GetFieldValue(Point, Bfield);
}
