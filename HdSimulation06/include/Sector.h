//
// File: Sector.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "GeneralTypes.h"

class SectorT
	: public std::vector<DataUnitT>
{
public:
	using LengthT = unsigned short int;
	SectorT(DataUnitT initValue, LengthT sectorLength)
		: vector(sectorLength, initValue)
		, dataUnit{ new const_iterator {begin()} }
	{
	}
	SectorT(const SectorT& rhs)
		: vector(rhs)
		, dataUnit{ new const_iterator {begin()} }
	{
	}
	~SectorT()
	{
		delete (dataUnit);
	}
	// generic <serialization to>/<deserialization from> sector
	void Serialize(const DataUnitT& multiDataUnitType, size_t numberOfDataUnitT)
	{
		insert(end(), numberOfDataUnitT, DataUnitT(0));
		iterator dataUnit{ end() - numberOfDataUnitT };
		for (size_t i{ 0 }; numberOfDataUnitT > i; ++i)
			dataUnit[i] = (&multiDataUnitType)[i];
	}
	void Serialize(size_t numberOfDataUnitT)
	{
		insert(end(), numberOfDataUnitT, DataUnitT(0));
	}
	void Serialize(const DataUnitT& singleByte)
	{
		push_back(singleByte);
	}
	void DeserializeSet() const
	{
		*dataUnit = begin();
	}
	void Deserialize(DataUnitT& multiDataUnitType, size_t numberOfDataUnitT) const
	{
		for (size_t i{ 0 }; numberOfDataUnitT > i; ++i)
			(&multiDataUnitType)[i] = (*dataUnit)[i];
		*dataUnit += numberOfDataUnitT;
	}
	void Deserialize(size_t numberOfDataUnitT) const
	{
		*dataUnit += numberOfDataUnitT;
	}
	void Deserialize(DataUnitT& singleByte) const
	{
		singleByte = **dataUnit;
		++(*dataUnit);
	}
private:
	const_iterator* dataUnit{ nullptr };
};
