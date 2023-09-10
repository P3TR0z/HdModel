//
// File: Cluster.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "Sector.h"
#include "FileImage.h"

class ClusterT
  : public std::vector<SectorT>
{
public:
  using LengthT = unsigned char;
  ClusterT() = default;
  ClusterT(const SectorT& initValue, LengthT clusterLength)
    : vector(clusterLength, initValue)
  {
  }
  void Serialize(FileImageT& image) const
  {
    for (auto& sector : *this)
      for (auto& dataUnit : sector)
        image.Serialize(dataUnit);
  }
  void Deserialize(const FileImageT& image)
  {
    for (auto& sector : *this)
      for (auto& sectorDataUnit : sector)
        image.Deserialize(sectorDataUnit);
  }
  ClusterT& Set(long long numberOfBytes, SectorT::LengthT numberOfBytesPerSector)
  {
    LengthT numberOfSectors{ LengthT((numberOfBytes - 1) / numberOfBytesPerSector) };
    SectorT::LengthT lastSectorLength{ SectorT::LengthT(numberOfBytes - ((long long)(numberOfSectors)) * numberOfBytesPerSector) };
    resize(size_t(numberOfSectors), { DataUnitT(0), numberOfBytesPerSector });
    push_back({ DataUnitT(0), lastSectorLength });
    return *this;
  }
};
