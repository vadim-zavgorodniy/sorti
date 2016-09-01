#include <iostream>
#include <fstream>
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
class ChunkIterator {
private:
  std::string fName;
  std::ifstream istream;
  Item current;
  bool isHasNext;

public:
  ChunkIterator(const std::string fileName) : fName(fileName) {
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
    isHasNext = getline(istream, line);
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
void openFileIn(std::string name, std::ifstream& istr) {
  istr.open(name.c_str(), std::ifstream::in);
  if (!istr.is_open()) {
    throw std::runtime_error("Unable to open file: " + name);
  }
}

//============================================================
ItemList* readItems(std::istream& istr, size_t& readSize) {

  ItemList* lst = new ItemList();

  size_t size = 0;
  std::string line;

  while (getline(istr, line)) {
    Item item = parseItem(line);
    lst->push_back(item);

    size += line.size();

    if (size >= readSize) {
      break;
    }
  }

  // return actual readed size
  readSize = size;

  return lst;
}

//============================================================
ItemList* getChunk(std::ifstream& istr, size_t size) {

  // readout items
  size_t actualSize(size);
  ItemList* items(readItems(istr, actualSize));

  std::cout << "Readed lines: " << items->size() << std::endl;
  std::cout << "Readed size: " << actualSize / 1024 / 1024 << " Mb" << std::endl;

  // sort
  qsort(&(*items)[0], items->size(), sizeof(Item), compareItemsQuick);

  return items;
}

//============================================================
void storeChunk(ItemList* list, std::string filename) {
  std::ofstream ostr(filename.c_str());

  try {
    outToStream(list, ostr);
    ostr.close();
  }
  catch (std::exception e) {
    ostr.close();
    throw e;
  }
}

//============================================================
void mergeChunks(std::vector<std::string> chunkNames, std::string destName) {

  std::list<ChunkIterator*> chunkFiles;

  // result stream
  std::ofstream ostr(destName.c_str());
  std::cout << "Merging to: " << destName << std::endl;
  long count = 0;
  try {
    typedef std::vector<std::string>::const_iterator VCSIterT;
    for (VCSIterT iter = chunkNames.begin(); iter != chunkNames.end(); ++iter) {
      ChunkIterator* citer = new ChunkIterator(*iter);
      chunkFiles.push_back(citer);
    }

    // assume the first is min
    ChunkIterator* min = *(chunkFiles.begin());
    while (!chunkFiles.empty()) {

      typedef std::list<ChunkIterator*>::iterator CIterT;
      for (CIterT iter = chunkFiles.begin(); iter != chunkFiles.end(); ++iter) {
        // if next item less then min
        if (compareItems(min->getItem(), (*iter)->getItem()) == false) {
          min = *iter;
        }
      }

      // write to dest file
      count++;
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

    ostr.close();
  }
  catch (std::exception e) {
    ostr.close();
    throw e;
  }
}

//============================================================
void doSort(std::string sourceName, std::string destName, size_t maxSize) {
  std::ifstream istr;

  openFileIn(sourceName.c_str(), istr);
  try {

    size_t actualSize(maxSize);
    // read and sort
    std::auto_ptr<ItemList> items(getChunk(istr, actualSize));

    // if we read all the file just sort it and write to destName file
    if (istr.eof()) {
      // write
      storeChunk(items.get(), destName.c_str());
    }
    else {

      std::vector<std::string> chunkNames;

      // new temp file name
      std::string tmpFileName = tmpnam(NULL);
      chunkNames.push_back(tmpFileName);
      std::cout << "tmpFilename: " << tmpFileName << std::endl;
      // write to temp file
      storeChunk(items.get(), tmpFileName.c_str());

      // read file by chunks
      while (!istr.eof()) {
        size_t actualSize(maxSize);
        // read and sort
        items.reset(getChunk(istr, actualSize));

        // new temp file name
        std::string tmpFileName = tmpnam(NULL);
        chunkNames.push_back(tmpFileName);

        std::cout << "tmpFilename: " << tmpFileName << std::endl;

        // write to temp file
        storeChunk(items.get(), tmpFileName.c_str());
      }

      // and finally merge it
      mergeChunks(chunkNames, destName);
    }

    istr.close();
  }
  catch (std::exception e) {
    istr.close();
    throw e;
  }

}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start." << std::endl;

  try {

    // process up to 2 Gb
    size_t size = long(2 * 1024 * 1024) * 1024;

    doSort("/home/warmouse/data/sorti-test/gen_data_10g.txt",
           "/home/warmouse/data/sorti-test/sorted_data_10g.txt", size);

    std::cout << "Finish." << std::endl;
  }
  catch (std::exception e) {
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
