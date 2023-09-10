//
// File: PatitionEntry.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include "GeneralTypes.h"
#include "ChsAddress.h"
#include "Sector.h"

struct PartitionEntryT
{
  DataUnitT status{ 0 };
  ChsAddressT firstAbsoluteSectorChs{ (unsigned short int)(0), (unsigned char)(0), (unsigned char)(0) };
  DataUnitT type{ 0 };
  ChsAddressT lastAbsoluteSectorChs{ (unsigned short int)(0), (unsigned char)(0), (unsigned char)(0) };
  unsigned int firstAsn{ 0 };
  unsigned int numberOfSectors{ 0 };
  PartitionEntryT(DataUnitT partitionStatus, DataUnitT partitionType, unsigned int partitionFirstAsn, unsigned int sectorsCount)
    : status{ partitionStatus }
    , type{ partitionType }
    , firstAsn{ partitionFirstAsn }
    , numberOfSectors{ sectorsCount }
  {
    firstAbsoluteSectorChs.Set(firstAsn);
    lastAbsoluteSectorChs.Set(firstAsn + numberOfSectors - 1);
  }
  void Serialize(SectorT& image) const
  {
    image.Serialize(status);
    firstAbsoluteSectorChs.Serialize(image);
    image.Serialize(type);
    lastAbsoluteSectorChs.Serialize(image);
    image.Serialize(reinterpret_cast<const DataUnitT&>(firstAsn), sizeof(firstAsn));
    image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfSectors), sizeof(numberOfSectors));
  }
  void Deserialize(const SectorT& image)
  {
    image.Deserialize(status);
    firstAbsoluteSectorChs.Deserialize(image);
    image.Deserialize(type);
    lastAbsoluteSectorChs.Deserialize(image);
    image.Deserialize(reinterpret_cast<DataUnitT&>(firstAsn), sizeof(firstAsn));
    image.Deserialize(reinterpret_cast<DataUnitT&>(numberOfSectors), sizeof(numberOfSectors));
  }
  PartitionEntryT& Set(DataUnitT partitionStatus, DataUnitT partitionType, unsigned int partitionFirstAsn, unsigned int sectorsCount)
  {
    status = partitionStatus;
    type = partitionType;
    firstAsn = partitionFirstAsn;
    numberOfSectors = sectorsCount;
    firstAbsoluteSectorChs.Set(firstAsn);
    lastAbsoluteSectorChs.Set(firstAsn + numberOfSectors - 1);
    return *this;
  }
};
