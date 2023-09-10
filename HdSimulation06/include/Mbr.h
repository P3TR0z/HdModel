//
// File: Mbr.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "PartitionEntry.h"
#include "Sector.h"
#include "HdModel.h"

struct MbrT
{
  const unsigned short int codeLength{ 446 };
  const unsigned char numberOfPartitions{ 4 };
  std::vector<PartitionEntryT> partitions{ numberOfPartitions, { DataUnitT(0), DataUnitT(0), (unsigned int)(0), (unsigned int)(0) } };
  const unsigned short int signature{ 0xAA55 };
  MbrT() = default;
  void Serialize(SectorT& image) const
  {
    image.Serialize((size_t)(codeLength));
    for (auto& partition : partitions)
      partition.Serialize(image);
    image.Serialize(reinterpret_cast<const DataUnitT&>(signature), sizeof(signature));
  }
  void Deserialize(const SectorT& image)
  {
    image.Deserialize((size_t)(codeLength));
    for (auto& partition : partitions)
      partition.Deserialize(image);
    image.Deserialize(sizeof(signature));
  }
  bool Read(HdModelT& hdModel)
  {
    SectorT sector{ DataUnitT(0), hdModel.numberOfBytesPerSector };
    hdModel.ReadSector(sector, 0);
    sector.DeserializeSet();
    Deserialize(sector);
    return true;
  }
  bool Write(HdModelT& hdModel) const
  {
    SectorT sector{ DataUnitT(0), SectorT::LengthT(0) };
    Serialize(sector);
    return hdModel.WriteSector(0, sector);
  }
};
