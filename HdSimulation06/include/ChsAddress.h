//
// File: ChsAddress.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include "GeneralTypes.h"
#include "Sector.h"

struct ChsAddressT
{
  DataUnitT head{ 0 };
  DataUnitT sectorAndHighCylinder{ 0 };
  DataUnitT cylinder{ 0 };
  ChsAddressT(unsigned short int track, unsigned char trackSide, unsigned char sector)
    : head{ DataUnitT(trackSide) }
    , sectorAndHighCylinder{ DataUnitT((0x3f & sector) | (0xC0 & (track >> 2))) }
    , cylinder{ DataUnitT(0xff & track) }
  {
  }
  void Serialize(SectorT& image) const
  {
    image.Serialize(head);
    image.Serialize(sectorAndHighCylinder);
    image.Serialize(cylinder);
  }
  void Deserialize(const SectorT& image)
  {
    image.Deserialize(head);
    image.Deserialize(sectorAndHighCylinder);
    image.Deserialize(cylinder);
  }
  ChsAddressT& Set(unsigned int logicalBlockAddress)
  {
    unsigned int trackSides{ logicalBlockAddress / 63 };
    unsigned short int track{ (unsigned short int)(trackSides / 255) };
    head = DataUnitT(trackSides - (((unsigned int)(track)) * 255));
    sectorAndHighCylinder = (0x3f & (DataUnitT(1 + logicalBlockAddress - (trackSides * 63)))) | (DataUnitT(0xC0 & (track >> 2)));
    cylinder = DataUnitT(0xff & track);
    return *this;
  }
  unsigned int ToLba() const
  {
    return (((0x0300 & (((unsigned int)(sectorAndHighCylinder)) << 2)) | cylinder) * 255 + head) * 63 + (0x3f & sectorAndHighCylinder) - 1;
  }
};
