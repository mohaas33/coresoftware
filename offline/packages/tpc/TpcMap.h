#ifndef __TpcMap_H__
#define __TpcMap_H__

#include <map>
#include <string>

class TpcMap
{
 public:
  // constructors and destructors
  TpcMap() = default;
  virtual ~TpcMap() = default;

  virtual int getLayer(const unsigned int FEE, const unsigned int FEEChannel, const unsigned int packetid = 0) const;
  virtual unsigned int getPad(const unsigned int FEE, const unsigned int FEEChannel, const unsigned int packetid = 0) const;
  virtual double getR(const unsigned int FEE, const unsigned int FEEChannel, const unsigned int packetid = 0) const;
  virtual double getPhi(const unsigned int FEE, const unsigned int FEEChannel, const unsigned int packetid = 0) const;
  virtual void setMapNames(const std::string &r1, const std::string &r2, const std::string &r3);

 private:
  int digest_map(const std::string &fileName, const unsigned int section_offset);

  int _broken = 0;
  struct tpc_map
  {
    unsigned int padnr;
    int layer;
    unsigned int FEE;
    unsigned int FEEChannel;
    double PadR;
    double PadPhi;
  };

  std::map<unsigned int, struct tpc_map> tmap;
};

#endif
