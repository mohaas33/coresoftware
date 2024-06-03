#include "TpcGlobalPositionWrapper.h"
#include "TpcDistortionCorrection.h"
#include "TpcDistortionCorrectionContainer.h"

#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrCluster.h>

Acts::Vector3 TpcGlobalPositionWrapper::getGlobalPositionDistortionCorrected(const TrkrDefs::cluskey& key, TrkrCluster* cluster,
                                                                             ActsGeometry* tGeometry, const short int& crossing, const TpcDistortionCorrectionContainer* staticCorrection,
                                                                             const TpcDistortionCorrectionContainer* averageCorrection, const TpcDistortionCorrectionContainer* fluctuationCorrection)
{
  TpcClusterZCrossingCorrection m_crossingCorrection;
  TpcDistortionCorrection m_distortionCorrection;
  Acts::Vector3 global = tGeometry->getGlobalPosition(key, cluster);
  global[2] = m_crossingCorrection.correctZ(global.z(), TpcDefs::getSide(key), crossing);

  // apply distortion corrections
  if (staticCorrection)
  {
    global = m_distortionCorrection.get_corrected_position(global, staticCorrection);
  }
  if (averageCorrection)
  {
    global = m_distortionCorrection.get_corrected_position(global, averageCorrection);
  }
  if (fluctuationCorrection)
  {
    global = m_distortionCorrection.get_corrected_position(global, fluctuationCorrection);
  }

  return global;
}
