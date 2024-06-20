#include "TpcCombinedRawDataUnpacker.h"

#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrDefs.h>  // for hitkey, hitsetkey
#include <trackbase/TrkrHit.h>
#include <trackbase/TrkrHitSet.h>
#include <trackbase/TrkrHitSetContainer.h>
#include <trackbase/TrkrHitSetContainerv1.h>
#include <trackbase/TrkrHitSetv1.h>
#include <trackbase/TrkrHitv2.h>

#include <ffarawobjects/TpcRawHit.h>
#include <ffarawobjects/TpcRawHitContainer.h>
#include <ffarawobjects/TpcRawHitContainerv1.h>
#include <ffarawobjects/TpcRawHitv1.h>

#include <fun4all/Fun4AllServer.h>

#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

#include <g4detectors/PHG4TpcCylinderGeom.h>
#include <g4detectors/PHG4TpcCylinderGeomContainer.h>

#include <Acts/Definitions/Units.hpp>
#include <Acts/Surfaces/Surface.hpp>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>  // for PHIODataNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>  // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>  // for PHWHERE

#include <TSystem.h>

#include <TFile.h>
#include <TH1.h>
#include <TNtuple.h>

#include <cstdlib>   // for exit
#include <iostream>  // for operator<<, endl, bas...
#include <map>       // for _Rb_tree_iterator

TpcCombinedRawDataUnpacker::TpcCombinedRawDataUnpacker(std::string const& name, std::string const& outF)
  : SubsysReco(name)
  , outfile_name(outF)
{
  // Do nothing
}

int TpcCombinedRawDataUnpacker::Init(PHCompositeNode* /*topNode*/)
{
  std::cout << "TpcCombinedRawDataUnpacker::Init(PHCompositeNode *topNode) Initializing" << std::endl;

  m_cdb = CDBInterface::instance();
  std::string calibdir = m_cdb->getUrl("TPC_FEE_CHANNEL_MAP");

  if (calibdir[0] == '/')
  {
    // use generic CDBTree to load
    m_cdbttree = new CDBTTree(calibdir);
    m_cdbttree->LoadCalibrations();
  }
  else
  {
    std::cout << "TpcRawDataDecoder::::InitRun No calibration file found" << std::endl;
    exit(1);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int TpcCombinedRawDataUnpacker::InitRun(PHCompositeNode* topNode)
{
  if (!topNode)
  {
    std::cout << "TpcCombinedRawDataUnpacker::InitRun(PHCompositeNode* topNode)" << std::endl;
    std::cout << "\tCould not retrieve topNode; doing nothing" << std::endl;
    exit(1);
    gSystem->Exit(1);

    return 1;
  }

  PHNodeIterator dst_itr(topNode);
  PHCompositeNode* dst_node = dynamic_cast<PHCompositeNode*>(dst_itr.findFirst("PHCompositeNode", "DST"));
  if (!dst_node)
  {
    if (Verbosity())
    {
      std::cout << "TpcCombinedRawDataUnpacker::InitRun(PHCompositeNode* topNode)" << std::endl;
    }
    if (Verbosity())
    {
      std::cout << "\tCould not retrieve dst_node; doing nothing" << std::endl;
    }
    exit(1);
    gSystem->Exit(1);

    return 1;
  }

  PHNodeIterator trkr_itr(dst_node);
  PHCompositeNode* trkr_node = dynamic_cast<PHCompositeNode*>(trkr_itr.findFirst("PHCompositeNode", "TRKR"));
  if (!trkr_node)
  {
    trkr_node = new PHCompositeNode("TRKR");
    dst_node->addNode(trkr_node);
  }

  TrkrHitSetContainer* trkr_hit_set_container = findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
  if (!trkr_hit_set_container)
  {
    if (Verbosity())
    {
      std::cout << "TpcCombinedRawDataUnpacker::InitRun(PHCompositeNode* topNode)" << std::endl;
    }
    if (Verbosity())
    {
      std::cout << "\tMaking TrkrHitSetContainer" << std::endl;
    }

    trkr_hit_set_container = new TrkrHitSetContainerv1;
    PHIODataNode<PHObject>* new_node = new PHIODataNode<PHObject>(trkr_hit_set_container, "TRKR_HITSET", "PHObject");
    trkr_node->addNode(new_node);
  }

  TpcRawHitContainerv1* tpccont = findNode::getClass<TpcRawHitContainerv1>(topNode, m_TpcRawNodeName);
  if (!tpccont)
  {
    std::cout << PHWHERE << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker::process_event(PHCompositeNode* topNode)" << std::endl;
    std::cout << "Could not get \"" << m_TpcRawNodeName << "\" from Node Tree" << std::endl;
    std::cout << "Removing module" << std::endl;

    Fun4AllServer *se = Fun4AllServer::instance();
    se->unregisterSubsystem(this);
    return Fun4AllReturnCodes::EVENT_OK;
  }

  if (m_writeTree)
  {
    m_file = new TFile(outfile_name.c_str(), "RECREATE");
    m_ntup = new TNtuple("NT", "NT", "event:gtmbco:packid:ep:sector:side:fee:chan:sampadd:sampch:nsamples");
  }

  if (Verbosity() >= 1)
  {
    std::cout << "TpcCombinedRawDataUnpacker:: _do_zerosup = " << m_do_zerosup << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker:: _do_noise_rejection = " << m_do_noise_rejection << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker:: _ped_sig_cut = " << m_ped_sig_cut << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker:: startevt = " << startevt << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker:: endevt = " << endevt << std::endl;
  }

  // check run number if presamples need to be shifted, which went from 80 -> 120
  // at 41624
  Fun4AllServer* se = Fun4AllServer::instance();
  if (se->RunNumber() < 41624)
  {
    m_presampleShift = 0;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int TpcCombinedRawDataUnpacker::process_event(PHCompositeNode* topNode)
{
  if (_ievent < startevt || _ievent > endevt)
  {
    if (Verbosity() > 1)
    {
      std::cout << " Skip event " << _ievent << std::endl;
    }
    _ievent++;
    return Fun4AllReturnCodes::DISCARDEVENT;
  }
  _ievent++;
  TH1F pedhist("pedhist", "pedhist", 251, -0.5, 1000.5);

  TrkrHitSetContainer* trkr_hit_set_container = findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
  if (!trkr_hit_set_container)
  {
    std::cout << PHWHERE << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker::process_event(PHCompositeNode* topNode)" << std::endl;
    std::cout << "Could not get \"TRKR_HITSET\" from Node Tree" << std::endl;
    std::cout << "Exiting" << std::endl;
    gSystem->Exit(1);
    exit(1);

    return Fun4AllReturnCodes::DISCARDEVENT;
  }

  TpcRawHitContainerv1* tpccont = findNode::getClass<TpcRawHitContainerv1>(topNode, m_TpcRawNodeName);
  if (!tpccont)
  {
    std::cout << PHWHERE << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker::process_event(PHCompositeNode* topNode)" << std::endl;
    std::cout << "Could not get \"" << m_TpcRawNodeName << "\" from Node Tree" << std::endl;
    std::cout << "Exiting" << std::endl;

    gSystem->Exit(1);
    exit(1);
  }

  PHG4TpcCylinderGeomContainer* geom_container =
      findNode::getClass<PHG4TpcCylinderGeomContainer>(topNode, "CYLINDERCELLGEOM_SVTX");
  if (!geom_container)
  {
    std::cout << PHWHERE << "ERROR: Can't find node CYLINDERCELLGEOM_SVTX" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  TrkrDefs::hitsetkey hit_set_key = 0;
  TrkrDefs::hitkey hit_key = 0;
  TrkrHitSetContainer::Iterator hit_set_container_itr;
  TrkrHit* hit = nullptr;

  uint64_t bco_min = UINT64_MAX;
  uint64_t bco_max = 0;

  const auto nhits = tpccont->get_nhits();

  int ntotalchannels = 0;
  int n_noisychannels = 0;
  for (unsigned int i = 0; i < nhits; i++)
  {
    TpcRawHit* tpchit = tpccont->get_hit(i);
    uint64_t gtm_bco = tpchit->get_gtm_bco();

    if (gtm_bco < bco_min)
    {
      bco_min = gtm_bco;
    }
    if (gtm_bco > bco_max)
    {
      bco_max = gtm_bco;
    }

    int fee = tpchit->get_fee();
    int channel = tpchit->get_channel();
    int feeM = FEE_map[fee];
    if (FEE_R[fee] == 2)
    {
      feeM += 6;
    }
    if (FEE_R[fee] == 3)
    {
      feeM += 14;
    }

    int side = 1;
    int32_t packet_id = tpchit->get_packetid();
    int ep = (packet_id - 4000) % 10;
    int sector = (packet_id - 4000 - ep) / 10;
    if (sector > 11)
    {
      side = 0;
    }

    unsigned int key = 256 * (feeM) + channel;
    std::string varname = "layer";
    int layer = m_cdbttree->GetIntValue(key, varname);
    // antenna pads will be in 0 layer
    if (layer <= 0)
    {
      continue;
    }

    uint16_t sampadd = tpchit->get_sampaaddress();
    uint16_t sampch = tpchit->get_sampachannel();
    uint16_t sam = tpchit->get_samples();

    //varname = "phi";  // + std::to_string(key);
    //double phi = (sector % 12) * M_PI / 6 - pow(-1,side)*m_cdbttree->GetDoubleValue(key, varname);
    //double phi_map = m_cdbttree->GetDoubleValue(key, varname);
    //    -1 * pow(-1, side) * m_cdbttree->GetDoubleValue(key, varname) + (sector % 12) * M_PI / 6;
    varname = "pad";
    int pad_n = m_cdbttree->GetIntValue(key, varname);

    PHG4TpcCylinderGeom* layergeom = geom_container->GetLayerCellGeom(layer);

    //unsigned int phibin = layergeom->get_phibin(phi,side);//layergeom->find_phibin(phi);
    //unsigned int phii = layergeom->find_vecbin(phi,side);
    int phibin = layergeom->get_phibins()/12*mc_sectors[sector % 12] + abs(pad_n - side*layergeom->get_phibins()/12);
    //double phi_new = layergeom->get_phi(phibin, side);
    //if(calc_pad_n-int(phibin)!=0){
    //if(abs(phi-phi_new)>1e-10){
    //  std::cout<< " phii = " << phii << std::endl;
    //  std::cout<< " phi_map = " << phi_map << std::endl;
    //  std::cout<< "TpcCombinedRawDataUnpacker::process_event: side = "<< side << 
    //                                                      " sector = "<< sector % 12<< 
    //                                                      " layer = " << layer << 
    //                                                      " phi_new = " << phi_new <<
    //                                                      " pad_n = " << pad_n << 
    //                                                      " phibin = " << phibin << 
    //                                                      " calc pad_n = " << calc_pad_n << 
    //                                                      " layergeom->get_phibins()/12 = " << layergeom->get_phibins()/12 << 
    //                                                      " diff pad = " << calc_pad_n-int(phibin) <<
    //                                                      " diff phi = " << phi-phi_new << std::endl;
    //}

    if (m_writeTree)
    {
      float fX[12];
      int n = 0;

      fX[n++] = _ievent - 1;
      fX[n++] = gtm_bco;
      fX[n++] = packet_id;
      fX[n++] = ep;
      fX[n++] = sector;
      fX[n++] = side;
      fX[n++] = fee;
      fX[n++] = channel;
      fX[n++] = sampadd;
      fX[n++] = sampch;
      fX[n++] = sam;
      m_ntup->Fill(fX);
    }
    //if(layer>54){
    //  std::cout<< "TpcCombinedRawDataUnpacker::process_event: layer = "<< layer << std::endl;
    //}
    hit_set_key = TpcDefs::genHitSetKey(layer, (mc_sectors[sector % 12]), side);
    hit_set_container_itr = trkr_hit_set_container->findOrAddHitSet(hit_set_key);


    if (!m_do_zerosup)
    {
      if (Verbosity() > 2)
      {
        std::cout << "TpcCombinedRawDataUnpacker:: no zero suppression" << std::endl;
      }
      for (uint16_t s = 0; s < sam; s++)
      {
        uint16_t adc = tpchit->get_adc(s);
        int t = s - m_presampleShift;

        hit_key = TpcDefs::genHitKey(phibin, (unsigned int) t);
        // find existing hit, or create new one
        hit = hit_set_container_itr->second->getHit(hit_key);
        if (!hit)
        {
          hit = new TrkrHitv2();
          hit->setAdc(float(adc));

          hit_set_container_itr->second->addHitSpecificKey(hit_key, hit);
        }
      }
    }
    else
    {
      if (Verbosity() > 2)
      {
        std::cout << "TpcCombinedRawDataUnpacker:: do zero suppression" << std::endl;
      }
      float hpedestal = 0;
      float hpedwidth = 0;
      pedhist.Reset();

      for (uint16_t sampleNum = 0; sampleNum < sam; sampleNum++)
      {
        uint16_t adc = tpchit->get_adc(sampleNum);
        pedhist.Fill(adc);
      }
      int hmax = 0;
      int hmaxbin = 0;
      for (int nbin = 1; nbin <= pedhist.GetNbinsX(); nbin++)
      {
        float val = pedhist.GetBinContent(nbin);
        if (val > hmax)
        {
          hmaxbin = nbin;
          hmax = val;
        }
      }

      // calculate pedestal mean and sigma

      if (pedhist.GetStdDev() == 0 || pedhist.GetEntries() == 0)
      {
        hpedestal = pedhist.GetBinCenter(pedhist.GetMaximumBin());
        hpedwidth = 999;
      }
      else
      {
        // calc peak position
        double adc_sum = 0.0;
        double ibin_sum = 0.0;
        double ibin2_sum = 0.0;

        for (int isum = -3; isum <= 3; isum++)
        {
          float val = pedhist.GetBinContent(hmaxbin + isum);
          float center = pedhist.GetBinCenter(hmaxbin + isum);
          ibin_sum += center * val;
          ibin2_sum += center * center * val;
          adc_sum += val;
        }

        hpedestal = ibin_sum / adc_sum;
        hpedwidth = sqrt(ibin2_sum / adc_sum - (hpedestal * hpedestal));
      }

      ntotalchannels++;
      if (m_do_noise_rejection)
      {

        if (hpedwidth < 0.5 || hpedestal < 10 || hpedwidth == 999)
        {
          n_noisychannels++;
          continue;
        }
      }

      for (uint16_t s = 0; s < sam; s++)
      {
        uint16_t adc = tpchit->get_adc(s);
        int t = s-m_presampleShift;
        if(t<0)
        {
          continue;
        }
        if ((float(adc) - hpedestal) > (hpedwidth * m_ped_sig_cut))
        {
          hit_key = TpcDefs::genHitKey(phibin, (unsigned int) t);
          // find existing hit, or create new one
          hit = hit_set_container_itr->second->getHit(hit_key);
          if (!hit)
          {
            hit = new TrkrHitv2();
            hit->setAdc(float(adc) - hpedestal);

            hit_set_container_itr->second->addHitSpecificKey(hit_key, hit);
          }
        }
      }
    }
  }

  if (m_do_noise_rejection && Verbosity() >= 2)
  {
    std::cout << " noisy / total channels = " << n_noisychannels << "/" << ntotalchannels << " = " << n_noisychannels / (double) ntotalchannels << std::endl;
  }

  if (Verbosity())
  {
    std::cout << " event BCO: " << bco_min << " - " << bco_max << std::endl;
    std::cout << "TpcCombinedRawDataUnpacker:: done" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int TpcCombinedRawDataUnpacker::End(PHCompositeNode* /*topNode*/)
{
  if (m_writeTree)
  {
    m_file->cd();
    m_ntup->Write();
    m_file->Close();
  }
  if (Verbosity())
  {
    std::cout << "TpcCombinedRawDataUnpacker::End(PHCompositeNode *topNode) This is the End..." << std::endl;
  }
  // if(m_Debug==1) hm->dumpHistos(m_filename, "RECREATE");

  return Fun4AllReturnCodes::EVENT_OK;
}
