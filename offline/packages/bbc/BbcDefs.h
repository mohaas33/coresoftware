#ifndef BBC_BBCDEFS_H
#define BBC_BBCDEFS_H

#include <iostream>
#include <TMath.h>

namespace BbcDefs
{
  const Double_t index_refract = 1.4585;
  const Double_t v_ckov = 1.0 / index_refract;  // velocity threshold for CKOV
  const Double_t C = 29.9792458;                // cm/ns

  // kludge where we have the hardcoded positions of the tubes
  // These are the x,y for the south BBC (in cm).
  // The north inverts the x coordinate (x -> -x)

  /* unused causes compiler warning
  const float TubeLoc[64][2] = {
      { -12.2976,	4.26 },
      { -12.2976,	1.42 },
      { -9.83805,	8.52 },
      { -9.83805,	5.68 },
      { -9.83805,	2.84 },
      { -7.37854,	9.94 },
      { -7.37854,	7.1 },
      { -7.37854,	4.26 },
      { -7.37854,	1.42 },
      { -4.91902,	11.36 },
      { -4.91902,	8.52 },
      { -4.91902,	5.68 },
      { -2.45951,	12.78 },
      { -2.45951,	9.94 },
      { -2.45951,	7.1 },
      { 0,	11.36 },
      { 0,	8.52 },
      { 2.45951,	12.78 },
      { 2.45951,	9.94 },
      { 2.45951,	7.1 },
      { 4.91902,	11.36 },
      { 4.91902,	8.52 },
      { 4.91902,	5.68 },
      { 7.37854,	9.94 },
      { 7.37854,	7.1 },
      { 7.37854,	4.26 },
      { 7.37854,	1.42 },
      { 9.83805,	8.52 },
      { 9.83805,	5.68 },
      { 9.83805,	2.84 },
      { 12.2976,	4.26 },
      { 12.2976,	1.42 },
      { 12.2976,	-4.26 },
      { 12.2976,	-1.42 },
      { 9.83805,	-8.52 },
      { 9.83805,	-5.68 },
      { 9.83805,	-2.84 },
      { 7.37854,	-9.94 },
      { 7.37854,	-7.1 },
      { 7.37854,	-4.26 },
      { 7.37854,	-1.42 },
      { 4.91902,	-11.36 },
      { 4.91902,	-8.52 },
      { 4.91902,	-5.68 },
      { 2.45951,	-12.78 },
      { 2.45951,	-9.94 },
      { 2.45951,	-7.1 },
      { 0,	-11.36 },
      { 0,	-8.52 },
      { -2.45951,	-12.78 },
      { -2.45951,	-9.94 },
      { -2.45951,	-7.1 },
      { -4.91902,	-11.36 },
      { -4.91902,	-8.52 },
      { -4.91902,	-5.68 },
      { -7.37854,	-9.94 },
      { -7.37854,	-7.1 },
      { -7.37854,	-4.26 },
      { -7.37854,	-1.42 },
      { -9.83805,	-8.52 },
      { -9.83805,	-5.68 },
      { -9.83805,	-2.84 },
      { -12.2976,	-4.26 },
      { -12.2976,	-1.42 }
  };
  */


}





#endif
