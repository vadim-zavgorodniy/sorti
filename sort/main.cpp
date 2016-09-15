#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <limits.h>

#include "item.hpp"

//============================================================
// AppConfig
//============================================================
class AppConfig {
public:
  std::string sourceName;
  std::string destName;
  size_t size;

  void parseFromOptions(int argc, char* argv[]);
};


//============================================================
// ChunkIterator
//============================================================
class ChunkIterator {
private:
  std::string fName;
  std::ifstream istream;
  Item current;
  bool isHasNext;

public:
  ChunkIterator(const std::string& fileName) : fName(fileName) {
    istream.open(fName.c_str());
    next();
  }

  ~ChunkIterator() {
    istream.close();
    remove(fName.c_str());
  }

  bool hasNext() {
    return isHasNext;
  }

  Item next() {
    std::string line;
    isHasNext = (getline(istream, line) ? true : false);
    if (isHasNext) {
      current = parseItem(line);
    }
    return current;
  }

  Item getItem() {
    return current;
  }
};


//============================================================
void openFileIn(const std::string& name, std::ifstream& istr) {
  istr.open(name.c_str());
  if (!istr.is_open()) {
    throw std::runtime_error("Unable to open file: " + name);
  }
}

//============================================================
size_t readItems(std::istream& istr, size_t readSize, ItemList& items) {

  size_t size = 0;
  std::string line;

  while (getline(istr, line)) {
    Item item = parseItem(line);
    items.push_back(item);

    size += line.size();

    if (size >= readSize) {
      break;
    }
  }

  return size;
}

//============================================================
size_t getChunk(std::ifstream& istr, size_t size, ItemList& items) {

  // readout items
  size_t actualSize = readItems(istr, size, items);

  std::cout << "Readed lines: " << items.size() << std::endl;
  std::cout << "Readed size: " << actualSize / 1024 / 1024 << " Mb" << std::endl;

  // sort
//  std::sort(items.begin(), items.end(), compareItems);
  qsort(&(items)[0], items.size(), sizeof(Item), compareItemsQuick);

  return actualSize;
}

//============================================================
void storeChunk(const ItemList& list, const std::string& filename) {
  std::ofstream ostr(filename.c_str());

  outToStream(list, ostr);
  ostr.close();
}

//============================================================
void mergeChunks(const std::vector<std::string>& chunkNames, const std::string& destName) {

  std::list<ChunkIterator*> chunkFiles;

  // result stream
  std::ofstream ostr(destName.c_str());
  std::cout << "Merging to: " << destName << std::endl;

  typedef std::list<ChunkIterator*>::iterator CIterT;
  typedef std::vector<std::string>::const_iterator VCSIterT;

  for (VCSIterT iter = chunkNames.begin(); iter != chunkNames.end(); ++iter) {
    ChunkIterator* citer = new ChunkIterator(*iter);
    chunkFiles.push_back(citer);
  }

  try {
    // assume the first is min
    ChunkIterator* min = *(chunkFiles.begin());
    while (!chunkFiles.empty()) {

      for (CIterT iter = chunkFiles.begin(); iter != chunkFiles.end(); ++iter) {
        // if next item less then min
        if (compareItems(min->getItem(), (*iter)->getItem()) == false) {
          min = *iter;
        }
      }

      // write to dest file
      ostr << min->getItem() << std::endl;

      // try to take next item
      min->next();
      // if failed then remove file
      if (!min->hasNext()) {
        // stream is empty, so remove
        chunkFiles.remove(min);
        delete min;

        if (!chunkFiles.empty()) {
          // assume the first is min
          min = *(chunkFiles.begin());
        }
      }
    }
  } catch (const std::exception& e) {
    for (CIterT iter = chunkFiles.begin(); iter != chunkFiles.end(); ++iter) {
      delete *iter;
    }
    throw e;
  }

  ostr.close();
}

//============================================================
void doSort(const std::string& sourceName, const std::string& destName, size_t maxSize) {
  maxSize *= 1024 * 1024;

  std::ifstream istr;

  openFileIn(sourceName, istr);

  // read and sort
  ItemList items;
  // reserve some mem (assume the average str len is 1024 / 2)
  items.reserve(maxSize / 1024 / 2);

  std::vector<std::string> chunkNames;
  // new temp file name
  std::string tmpFileName;

  // read file by chunks
  while (!istr.eof()) {
    // read and sort
    items.clear();
    getChunk(istr, maxSize, items);

    if (chunkNames.empty() && istr.eof()) {
      // write
      storeChunk(items, destName);
      break;
    }

    // new temp file name
    std::string tmpFileName = tmpnam(NULL);
    chunkNames.push_back(tmpFileName);

    std::cout << "tmpFilename: " << tmpFileName << std::endl;

    // write to temp file
    storeChunk(items, tmpFileName);
  }

  // and finally merge it
  if (!chunkNames.empty()) {
    mergeChunks(chunkNames, destName);
  }

  istr.close();
}

//============================================================
void AppConfig::parseFromOptions(int argc, char* argv[]) {
  const char* usage = "Usage: sort source_file dest_file [ram_size]\n" \
    "  source_file - file name to sort\n"                   \
    "  dest_file   - result file name\n"                      \
    "  ram_size - desired RAM usage size in Mb (default 4096 Mb)\n";

  if (argc < 3) {
    std::cerr << usage << std::endl;
    throw std::runtime_error("Illegal arguments");
  }
  try {
    sourceName = argv[1];
    destName = argv[2];

    if (argc > 3) {
      std::istringstream buffer(argv[3]);
      buffer.exceptions(std::ifstream::failbit);
      buffer >> size;
    }
  }
  catch (const std::exception& e) {
    std::cerr << usage << std::endl;
    throw std::runtime_error("Filed to parse the parameters");
  }
}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start." << std::endl;

  try {
    AppConfig conf;
    conf.size = 4096;
    conf.parseFromOptions(argc, argv);

    doSort(conf.sourceName, conf.destName, conf.size);

    std::cout << "Finish." << std::endl;
  }
  catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Program crashed." << std::endl;
    return 2;
  }

  return 0;
}

//============================================================
