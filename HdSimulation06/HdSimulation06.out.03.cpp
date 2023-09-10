//
// File: HdSimulation06.cpp
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description: Enuntul problemei
//
//    Simularea unui hard disk la nivel fizic si la nivel logic
//
//    - un hard disk este organizat fizic in sectoare si logic in partitii
//    - un sector este un grup cu lungime fixa de octeti
//    - o partitie este o portiune contigua dintr-un hard disk
//    - o partitie este organizata logic in clustere
//    - un cluster este un grup cu lungime fixa de sectoare
//    - un fisier este un grup contiguu cu lungime variabila de date
//      [] in RAM: este organizat ca un grup contiguu cu lungime variabila de octeti
//      [] pe partitie (respectiv pe hard disk): este organizat in clustere (respectiv sectoare)
//      [] abstract: este considerat ca un grup contiguu de clustere, suficiente pentru a contine toate datele
//    - terminologie:
//      [] sectoarele hard disk-ului se numesc ABSOLUTE
//      [] sectoarele si clusterele partitiei se numesc LOGICE
//      [] clusterele si sectoarele din organizarea abstracta a fisierului se numesc VIRTUALE
//    - fisierele pot fi stocate (scrise) pe partitie, incarcate (citite) de pe partitie sau eliminate (sterse) de pe partitie
//    - pentru realizarea actiunilor mentionate, se vor defini functiile WriteFile, ReadFile si DeleteFile
//      [] aceste functii vor fi legate de structura HdImageT, care va gestiona o partitie
//    - reguli simultane de respectat la scrierea pe partitie:
//      [] se scrie sector cu sector in ordinea numerelor lor
//      [] clusterele logice se aleg in ce secventa se doreste, inclusiv aleator
//      [] odata ales un cluster logic, fie se completeaza, fie se termina sectoarele
//      [] un sector virtual se scrie pe un sector logic
//      [] sectoarele logice consecutive stocheaza sectoare virtuale consecutive
//      [] (redundant) un cluster logic se completeaza incepind cu primul sau sector
//    - un grup de clustere, consecutive ca numere si simultan consecutive ca alegere, formeaza un fragment
//
//    - catalogarea (cartografierea) fragmentelor  ->  mapping pairs
//      [] fragmentul:
//        () este determinat de o pozitie(numarul primului sau cluster) si o lungime(numarul sau de clustere)
//        () <pozitia fragmentului> = <deplasamentul pozitiei fragmentului> + <pozitia fragmentului anterior>
//        () <pozitia fragmentului anterior> pentru primul fragment este 0
//        () lungimea fragmentului si <deplasamentul pozitiei fragmentului> sint valori aritmetice cu semn
//      [] mapping pairs:
//        () reprezinta un sir de inregistrari cu lungime variabila
//        () inregistrarea este formata din:
//          o. primul octet:
//            - 4 LSb = numarul de octeti pentru valoarea lungimii fragmentului
//              (daca  0 == 4 LSb  atunci valoarea lungimii fragmentului este 0)
//            - 4 MSb = numarul de octeti pentru valoarea deplasamentului pozitiei fragmentului
//              (daca  0 == 4 MSb  atunci valoarea deplasamentului pozitiei fragmentului este 0)
//          o. octetii pentru valoarea lungimii fragmentului in ordinea  LE
//          o. octetii pentru valoarea deplasamentului pozitiei fragmentului in ordinea  LE
//        () ultima inregistrare are primul octet cu valoarea 0
//      [] OBS.:
//        () un octet pentru una din valori reprezinta o cifra a acelei valori in baza de numeratie 256
//        () penultima inregistrare nu poate reprezenta un fragment cu lungime zero
//        () desi se permite altfel, primul octet al unei inregistrari corect formatate indeplineste conditia <( 0 != 4 LSb ) si ( 0 != 4 MSb )>

#include <iostream>
#include <fstream>
#include "include\Sector.h"
#include "include\GeneralTypes.h"
#include "include\Cluster.h"
#include "include\HdModel.h"
#include "include\FileImage.h"
#include "include\MappingPairs.h"
#include "include\HdImage.h"
#include "include\Mbr.h"
#include "include\FileRecord.h"

using namespace std;

// Globals   .........................
//  potential input/output files
const char fHdSimulation06Cpp[]{ "HdSimulation06.cpp" };
const char fHdImageCpp[]{ "HdImage.cpp" };
const char fHdImageH[]{ ".\\include\\HdImage.h" };
const char f1HdSimulation06outCpp[]{ "HdSimulation06.out.01.cpp" };
const char f2HdSimulation06outCpp[]{ "HdSimulation06.out.02.cpp" };
const char f3HdSimulation06outCpp[]{ "HdSimulation06.out.03.cpp" };
const char f4HdSimulation06outCpp[]{ "HdSimulation06.out.04.cpp" };
//  physical hdModel disk structure
const SectorT::LengthT numberOfBytesPerSector{ 512 };
const long long numberOfSectors{ 1033 };
//  logical hdModel disk structure
//    hdPartition disk structure
const long long mbrTrack{ 63 };
const DataUnitT partitionStatusActive{ 1 };
const DataUnitT partitionStatusInactive{ 0 };
const DataUnitT partitionType{ 27 };
//    hdImage disk structure
const ClusterT::LengthT numberOfSectorsPerCluster{ 8 };

//  set working conditions
//    mains (selected)
//      select input file(s) (or none)
const char* f1Name{ fHdSimulation06Cpp };
const char* f2Name{ fHdImageH };
const char* f3Name{ fHdImageCpp };
//      select output file(s) (or none)
const char* f1OutName{ f1HdSimulation06outCpp };
const char* f2OutName{ f2HdSimulation06outCpp };
const char* f3OutName{ f3HdSimulation06outCpp };
const char* f4OutName{ f4HdSimulation06outCpp };
//      select appropriate value
//    additionals (computed)
const DataUnitT firstPartitionNumber{ 0 };
const DataUnitT firstPartitionStatus{ partitionStatusActive };
const unsigned int firstPartitionFirstAsn{ (unsigned int)(mbrTrack) };
const unsigned int firstPartitionNumberOfSectors{ 480 };
const DataUnitT secondPartitionNumber{ 1 };
const DataUnitT secondPartitionStatus{ partitionStatusInactive };
const unsigned int secondPartitionFirstAsn{ (unsigned int)(mbrTrack + firstPartitionNumberOfSectors) };
const unsigned int secondPartitionNumberOfSectors{ (unsigned int)(numberOfSectors - mbrTrack - firstPartitionNumberOfSectors) };
//  end <set working conditions>

bool SetWorkingConditions(HdModelT& hdModel);
void ReadFileBinaryToImage(const char* fName, FileImageT& fileImage, ClusterT::LengthT numberOfSectorsPerCluster, SectorT::LengthT numberOfBytesPerSector);
void ShowMappingPairs(const MappingPairsT& mappingPairs);
void ShowMappingPairs(const char* fName, HdImageT& hdImage);
void ShowFile(const char* fName, const FileImageT& fileImage);
void ErrReadFile(const char* fName, HdImageT& hdImage);
void ErrWriteFile(const char* fName);

bool SetWorkingConditions(HdModelT& hdModel)
{
  MbrT mbr;
  mbr.partitions[firstPartitionNumber].Set(firstPartitionStatus, partitionType, firstPartitionFirstAsn, firstPartitionNumberOfSectors);
  mbr.partitions[secondPartitionNumber].Set(secondPartitionStatus, partitionType, secondPartitionFirstAsn, secondPartitionNumberOfSectors);
  return mbr.Write(hdModel);
}

// Main   ............................
int main()
{
  cout << endl;
  // set working conditions
  HdModelT hdModel{ numberOfSectors, numberOfBytesPerSector };
  if (!SetWorkingConditions(hdModel))
  {
    cout << endl;
    return 0;
  }
  FileImageT f1Image;
  ReadFileBinaryToImage(f1Name, f1Image, numberOfSectorsPerCluster, numberOfBytesPerSector);
  FileImageT f2Image;
  ReadFileBinaryToImage(f2Name, f2Image, numberOfSectorsPerCluster, numberOfBytesPerSector);
  FileImageT f3Image;
  ReadFileBinaryToImage(f3Name, f3Image, numberOfSectorsPerCluster, numberOfBytesPerSector);
  // end <set working conditions>
  // Hard disk model - Exercise 1
  {
    const char _01_of_f1[]{ "Copy 01 of f1Name.cpp" };
    const char _01_of_f2[]{ "Copy 01 of f2Name.cpp" };
    const char _01_of_f3[]{ "Copy 01 of f3Name.cpp" };
    const char _02_of_f1[]{ "Copy 02 of f1Name.cpp" };
    // ...............................
    cout << endl;
    HdImageT hdImage01{ hdModel, firstPartitionNumber, numberOfSectorsPerCluster}; 
    if (!hdImage01.WriteFile(f3Name, f3Image))
      ErrWriteFile(f3Name);
    else
      ShowMappingPairs(f3Name, hdImage01);

    if (!hdImage01.WriteFile(f1Name, f1Image))
      ErrWriteFile(f1Name);
    else
      ShowMappingPairs(f1Name, hdImage01);

    if (!hdImage01.WriteFile(f2Name, f2Image))
      ErrWriteFile(f2Name);
    else
      ShowMappingPairs(f2Name, hdImage01);

    hdImage01.DeleteFile(f3Name);

    if (!hdImage01.WriteFile(_01_of_f1, f1Image))
      ErrWriteFile(_01_of_f1);
    else
      ShowMappingPairs(_01_of_f1, hdImage01);
    if (!hdImage01.WriteFile(_01_of_f2, f2Image))
      ErrWriteFile(_01_of_f2);
    else
      ShowMappingPairs(_01_of_f2, hdImage01);
    if (!hdImage01.WriteFile(_01_of_f3, f3Image))
      ErrWriteFile(_01_of_f3);
    else
      ShowMappingPairs(_01_of_f3, hdImage01);

    hdImage01.DeleteFile(f2Name);
    hdImage01.DeleteFile(_01_of_f2);

    if (!hdImage01.WriteFile(_02_of_f1, f1Image))
      ErrWriteFile(_02_of_f1);
    else
      ShowMappingPairs(_02_of_f1, hdImage01);

    FileImageT image;
    if (!hdImage01.ReadFile(_02_of_f1, image))
      ErrReadFile(_02_of_f1, hdImage01);
    else
      ShowFile(f1OutName, image);
    // ...............................
    cout << endl;
    HdImageT hdImage02{ hdModel, secondPartitionNumber, numberOfSectorsPerCluster };
    if (!hdImage02.WriteFile(f2Name, f2Image))
      ErrWriteFile(f2Name);
    else
      ShowMappingPairs(f2Name, hdImage02);
    if (!hdImage02.WriteFile(f1Name, f1Image))
      ErrWriteFile(f1Name);
    else
      ShowMappingPairs(f1Name, hdImage02);
    if (!hdImage02.WriteFile(f3Name, f3Image))
      ErrWriteFile(f3Name);
    else
      ShowMappingPairs(f3Name, hdImage02);
    hdImage02.DeleteFile(f2Name);
    if (!hdImage02.WriteFile(_01_of_f1, f1Image))
      ErrWriteFile(_01_of_f1);
    else
      ShowMappingPairs(_01_of_f1, hdImage02);
    if (!hdImage02.WriteFile(_01_of_f3, f3Image))
      ErrWriteFile(_01_of_f3);
    else
      ShowMappingPairs(_01_of_f3, hdImage02);
    if (!hdImage02.WriteFile(_01_of_f2, f2Image))
      ErrWriteFile(_01_of_f2);
    else
      ShowMappingPairs(_01_of_f2, hdImage02);
    hdImage02.DeleteFile(f3Name);
    hdImage02.DeleteFile(_01_of_f3);
    if (!hdImage02.WriteFile(_02_of_f1, f1Image))
      ErrWriteFile(_02_of_f1);
    else
      ShowMappingPairs(_02_of_f1, hdImage02);
    FileImageT fImage;
    if (!hdImage02.ReadFile(_02_of_f1, fImage))
      ErrReadFile(_02_of_f1, hdImage02);
    else
      ShowFile(f2OutName, fImage);
  }
  // end <Hard disk model - Exercise 1>

  // Hard disk model - Exercise 2
  {
    const char _02_of_f1[]{ "Copy 02 of f1Name.cpp" };
    // ...............................
    cout << endl;
    HdImageT hdImage01{ hdModel, firstPartitionNumber};
    HdImageT hdImage02{ hdModel, secondPartitionNumber};
    FileImageT fImage;
    if (!hdImage01.ReadFile(_02_of_f1, fImage))
      ErrReadFile(_02_of_f1, hdImage01);
    else
      ShowFile(f3OutName, fImage);
    if (!hdImage02.ReadFile(_02_of_f1, fImage))
      ErrReadFile(_02_of_f1, hdImage02);
    else
      ShowFile(f4OutName, fImage);
  }
  // end <Hard disk model - Exercise 2>

  cout << endl;
  return 0;
}
void ReadFileBinaryToImage(const char* fName, FileImageT& fileImage, ClusterT::LengthT numberOfSectorsPerCluster, SectorT::LengthT numberOfBytesPerSector)
{
  fstream fIn(fName, fstream::in | fstream::binary);
  DataUnitT temp;
  while (true)
  {
    fIn.get(reinterpret_cast<char&>(temp));
    if (fIn.eof())
      break;
    fileImage.push_back(temp);
  }
  cout << " " << fName << ": ";
  size_t fileImageSize{ fileImage.size() };
  if (0 == fileImageSize)
    cout << "empty\n";
  else
  {
    cout << fileImageSize << " bytes; ";
    cout << "for hdModel: ";
    size_t fileSectorsCnt{ 1 + ((fileImageSize - 1) / numberOfBytesPerSector) };
    cout << fileSectorsCnt << " sectors, ";
    cout << 1 + ((fileSectorsCnt - 1) / numberOfSectorsPerCluster) << " clusters\n";
  }
}
void ShowMappingPairs(const MappingPairsT& mappingPairs)
{
  for (const DataUnitT& dataUnit : mappingPairs)
    cout << " " << int(dataUnit);
}
void ShowMappingPairs(const char* fName, HdImageT& hdImage)
{
  FileRecordT fileRecord;
  hdImage.Mft().FindFile(fName, fileRecord);
  cout << " Mappingpairs of " << fName << ":";
  ShowMappingPairs(fileRecord.mappingPairs);
  cout << endl;
}
void ShowFile(const char* fName, const FileImageT& fileImage)
{
  fstream fOut(fName, fstream::out | fstream::binary);
  for (const DataUnitT& dataUnit : fileImage)
    fOut.put(reinterpret_cast<const char&>(dataUnit));
  cout << " " << fName << " created.\n";
}
void ErrReadFile(const char* fName, HdImageT& hdImage)
{
  FileRecordT fileRecord;
  hdImage.Mft().FindFile(fName, fileRecord);
  cout << " Error: Reading file with mapping pairs: [";
  ShowMappingPairs(fileRecord.mappingPairs);
  cout << "] failed\n";
}
void ErrWriteFile(const char* fName)
{
  cout << " Error: Writing file " << fName << " failed\n";
}
