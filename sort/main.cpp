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
  maxSize *= 1024 * 1024;

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
        items.reset(NULL);
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
void AppConfig::parseFromOptions(int argc, char* argv[]) {
  const char* usage = "Usage: sort source_file dest_file [ram_size]\n" \
    "  source_file - file name to sort\n"                   \
    "  dest_file   - result file name\n"                      \
    "  ram_size - desired RAM usage size in Mb (default 4 Gb)\n";

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
  catch (std::exception e) {
    std::cerr << usage << std::endl;
    throw std::runtime_error("Filed to parse the parameters");
  }
}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start." << std::endl;

  try {
    AppConfig conf;
    // conf.sourceName = "/home/warmouse/data/sorti-test/gen_data_10g.txt";
    // conf.destName = "/home/warmouse/data/sorti-test/sorted_data_10g.txt";
    // process up to 4 Gb
    // conf.size = long(4 * 1024 * 1024) * 1024;
    conf.size = 4096;
    conf.parseFromOptions(argc, argv);

    doSort(conf.sourceName, conf.destName, conf.size);

    std::cout << "Finish." << std::endl;
  }
  catch (std::runtime_error e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
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
