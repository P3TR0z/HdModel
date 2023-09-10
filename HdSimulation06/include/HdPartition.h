//
// File: HdPartition.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include "Sector.h"
#include "HdModel.h"

struct HdPartitionT
{
  HdModelT& hdModel;
  long long lsnZero{ 0 };   // absolute sector number of the first lsn
  long long numberOfSectors{ 0 };
  HdPartitionT(HdModelT& hardDisk)
    :hdModel{hardDisk}
  {
  }
  HdPartitionT(HdModelT& hardDisk, long long lsnFirst, long long sectorsCount)
    : hdModel{ hardDisk }
    , lsnZero{ lsnFirst }
    , numberOfSectors{ sectorsCount }
  {
  }
  HdPartitionT& Set(long long asnFirst, long long sectorsCount)
  {
    lsnZero = asnFirst;
    numberOfSectors = sectorsCount;
    return *this;
  }
  bool ReadSector(SectorT& virtualSector, long long lsn) const
  {
    return hdModel.ReadSector(virtualSector, lsn + lsnZero);
  }
  bool WriteSector(long long lsn, const SectorT& virtualSector) const
  {
    return hdModel.WriteSector(lsn + lsnZero, virtualSector);
  }
};
