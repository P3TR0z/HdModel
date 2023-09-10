//
// File: HdModel.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "Sector.h"

class HdModelT
{
public:
  std::vector<SectorT> sectors;
  long long numberOfSectors{ 0 };
  SectorT::LengthT numberOfBytesPerSector{ SectorT::LengthT(0) };
  HdModelT(long long sectorsCount, SectorT::LengthT bytesPerSector)
    : sectors{ size_t(sectorsCount), { DataUnitT(0), bytesPerSector } }
    , numberOfSectors{ sectorsCount }
    , numberOfBytesPerSector{ bytesPerSector }
  {
  }
  bool ReadSector(SectorT& virtualSector, long long asn) const
  {
    const SectorT& src{ sectors[reinterpret_cast<size_t&>(asn)] };
    for (size_t byteIndex = 0; virtualSector.size() > byteIndex; ++byteIndex)
      virtualSector[byteIndex] = src[byteIndex];
    return true;
  }
  bool WriteSector(long long asn, const SectorT& virtualSector)
  {
    SectorT& dst{ sectors[reinterpret_cast<size_t&>(asn)] };
    for (size_t byteIndex = 0; virtualSector.size() > byteIndex; ++byteIndex)
      dst[byteIndex] = virtualSector[byteIndex];
    return true;
  }
};
