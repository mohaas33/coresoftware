#include "Gl1Packetv2.h"

#include <phool/phool.h>

#include <iomanip>

void Gl1Packetv2::Reset()
{
  OfflinePacketv1::Reset();
  packet_nr = 0;
  BunchNumber = std::numeric_limits<uint64_t>::max();
  TriggerInput = 0;
  TriggerVector = 0;
  GTMBusyVector = 0;
  for (auto &row : scaler)
  {
    row.fill(0);
  }
  return;
}

void Gl1Packetv2::identify(std::ostream &os) const
{
  os << "Gl1Packetv2: " << std::endl;
  OfflinePacketv1::identify(os);
  os << "bunch number: " << BunchNumber << std::endl;
  return;
}

void Gl1Packetv2::FillFrom(const Gl1Packet *pkt)
{
  setBunchNumber(pkt->getBunchNumber());
  setPacketNumber(pkt->getPacketNumber());
  setTriggerInput(pkt->getTriggerInput());
  setTriggerVector(pkt->getTriggerVector());
  setGTMBusyVector(pkt->getGTMBusyVector());
  for (int i = 0; i < 64; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      setScaler(i, j, pkt->lValue(i, j));
    }
  }
  std::string gl1p_names[3] {"GL1PRAW","GL1PLIVE","GL1PSCALED"};
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      setGl1pScaler(i, j, pkt->lValue(i, gl1p_names[j]));
    }
  }
  OfflinePacketv1::FillFrom(pkt);
}

int Gl1Packetv2::iValue(const int i) const
{
  if (i == 0)
  {
    return packet_nr;
  }
  std::cout << PHWHERE << " Bad argument for iValue: " << i << std::endl;
  return std::numeric_limits<int>::min();
}

long long Gl1Packetv2::lValue(const int i, const int j) const
{
  return scaler.at(i).at(j);
}

long long Gl1Packetv2::lValue(const int i, const std::string &what) const
{
  if (what == "BCO")
  {
    return getBCO();
  }
  if (what == "TriggerInput")
  {
    return getTriggerInput();
  }
  if (what == "TriggerVector")
  {
    return getTriggerVector();
  }
  if (what == "GTMBusyVector")
  {
    return getGTMBusyVector();
  }
  if (what == "BunchNumber")
  {
    return getBunchNumber();
  }
  if (what == "GL1PRAW")
  {
    return gl1pscaler.at(i).at(0);
  }
  if (what == "GL1PLIVE")
  {
    return gl1pscaler.at(i).at(1);
  }
  if (what == "GL1PSCALED")
  {
    return gl1pscaler.at(i).at(2);
  }
  std::cout << "option " << what << " not implemented" << std::endl;
  return std::numeric_limits<uint64_t>::max();
}

void Gl1Packetv2::dump(std::ostream &os) const
{
  os << "packet nr:       " << iValue(0) << std::endl;
  os << "Beam Clock:      "
     << "0x" << std::hex << lValue(0, "BCO") << std::dec << "   " << lValue(0, "BCO") << std::endl;
  os << "Trigger Input:   "
     << "0x" << std::hex << lValue(0, "TriggerInput") << std::dec << "   " << lValue(0, "TriggerInput") << std::endl;
  os << "Trigger Vector:  "
     << "0x" << std::hex << lValue(0, "TriggerVector") << std::dec << "   " << lValue(0, "TriggerVector") << std::endl;
  os << "GTM Busy Vector: " << "0x" << std::hex <<  lValue(0, "GTMBusyVector") << std::dec << "   " << lValue(0, "GTMBusyVector") << std::endl;
  os << "Bunch Number:    " << lValue(0, "BunchNumber") << std::endl
     << std::endl;
  os << "Trg #                  raw              live              scaled" << std::endl;
  os << "----------------------------------------------------------------" << std::endl;

  int i;

  for (i = 0; i < 64; i++)
  {
    if (lValue(i, 0) || lValue(i, 1) || lValue(i, 2))
    {
      os << std::setw(3) << i << "    ";
      os << " " << std::setw(18) << lValue(i, 0)
         << " " << std::setw(18) << lValue(i, 1)
         << " " << std::setw(18) << lValue(i, 2)
         << std::endl;
    }
  }
  os << std::endl;
  os << "Gl1P #                raw              live              scaled" << std::endl;
  os << "----------------------------------------------------------------" << std::endl;
  
  for (i = 0; i< 16; i++)
    {
      if ( lValue(i, "GL1PRAW") ||  lValue(i, "GL1PLIVE") ||  lValue(i, "GL1PSCALED") )
	{
	  os << std::setw(3) << i << "    ";
	  os << " " << std::setw(18) << lValue(i, "GL1PRAW")
	     << " " << std::setw(18) << lValue(i, "GL1PLIVE")
	     << " " << std::setw(18) << lValue(i, "GL1PSCALED")
	     << std::endl;
	}
    }
  os << std::endl;
}
