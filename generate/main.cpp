#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <limits.h>

#define DELIMITER ". "

//============================================================
// DataGeneragor
//============================================================
class DataGeneragor {
private:
  std::vector<char> charTable;

public:
  struct Statistics {
    // row count
    int count;
    // string repeats count
    int repeats;
  };

protected:
  void initCharTable();
  std::string generateString(size_t maxLength);
public:
  Statistics generate(std::ostream& ostr, size_t maxSize);
};


//============================================================
// AppConfig
//============================================================
class AppConfig {
public:
  size_t size;
  std::string filename;

  void parseFromOptions(int argc, char* argv[]);
};


//============================================================
void DataGeneragor::initCharTable() {
  charTable.clear();
  charTable.push_back(32); // space
  charTable.push_back(44); // ,
  charTable.push_back(46); // .
  // digits and : ;
  for (int i = 48; i < 60; ++i) {
    charTable.push_back(i);
  }

  // digits and : ;
  for (int i = 48; i <= 59; ++i) {
    charTable.push_back(i);
  }

  // capital letters
  for (int i = 65; i <= 90; ++i) {
    charTable.push_back(i);
  }

  // small letters
  for (int i = 97; i <= 122; ++i) {
    charTable.push_back(i);
  }
}

//============================================================
std::string DataGeneragor::generateString(size_t maxLength) {
  // assume that min maxLenth is 10
  const int MIN_BLOCK = 10;
  if (maxLength < MIN_BLOCK) {
    maxLength = MIN_BLOCK;
  }

  // actual string size
  size_t len = rand() % (maxLength-MIN_BLOCK) + MIN_BLOCK;

  std::string str;

  // let's gen it
  for (int i = 0; i < len; i += MIN_BLOCK) {
    char buf[MIN_BLOCK + 1];
    memset(buf, '\0', MIN_BLOCK+1);
    // table offset
    int offset = rand() % (charTable.size() - MIN_BLOCK);

    // copy blocks to result string
    strncpy(buf, &charTable[offset], MIN_BLOCK);
    str += buf;
  }

  return str.substr(0, maxLength);
}

//============================================================
DataGeneragor::Statistics DataGeneragor::generate(std::ostream& ostr, size_t maxSize) {
  initCharTable();

  Statistics stat;
  stat.count = 0;
  stat.repeats = 0;

  srand(time(NULL));
  // file size
  size_t size = 0;
  // skip amount before repeat string update
  int repeatUpdate = 50;
  std::string repeatStr = "";

  // while result size less then maxSize in Mb
  while (size / (1024*1024) < maxSize) {
    int num = rand() % INT_MAX;

    // counting string size
    char numBuf[20];
    sprintf (numBuf, "%d", num);
    std::string res(numBuf);
    std::string str = "";

    // 2 times often we try to repeat string
    if (stat.count % (repeatUpdate / 2) == 0) {
      if (rand() % 2 == 1) {
        if (!repeatStr.empty()) {
          str = repeatStr;
          stat.repeats++;
        }
      }
    }

    // if it is not a repeat gen new str
    if (str.empty()) {
      str = generateString(1024);
    }

    res += DELIMITER + str;

    ostr << res << std::endl;

    // file size
    size += res.size();
    // record count
    stat.count++;

    if (stat.count % repeatUpdate == 0) {
      // shell we remember it?
      if (rand() % 2 == 1) {
        repeatStr = str;
      }
    }
  }

  return stat;
}

//============================================================
void AppConfig::parseFromOptions(int argc, char* argv[]) {
  const char* usage = "Usage: generate file_size file_name\n" \
    "  file_size - desired data file size in Mb\n"            \
    "  file_name - file name to generate";

  if (argc < 3) {
    std::cerr << usage << std::endl;
    throw std::runtime_error("Illegal arguments");
  }

  try {
    std::istringstream buffer(argv[1]);
    buffer.exceptions(std::ifstream::failbit);
    buffer >> size;

    filename = argv[2];
  }
  catch (std::exception e) {
    std::cerr << usage << std::endl;
    throw std::runtime_error("Filed to parse the parameters");
  }
}


//============================================================
// main
//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start." << std::endl;

  try {
    AppConfig conf;
    // conf.filename = "/home/warmouse/data/sorti-test/gen_data.txt";
    // conf.size = 1024;
    conf.parseFromOptions(argc, argv);

    std::ofstream ostr(conf.filename.c_str());

    DataGeneragor dgen;
    DataGeneragor::Statistics stat = dgen.generate(ostr, conf.size);
    ostr.close();

    std::cout << "Rows count: " << stat.count << std::endl;
    std::cout << "Repeats count: " << stat.repeats << std::endl;

    std::cout << "Finish." << std::endl;
  }
  catch (std::runtime_error e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (std::exception e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 2;
  }
  catch (...) {
    std::cerr << "Program crashed." << std::endl;
    return 3;
  }

  return 0;
}

//============================================================
