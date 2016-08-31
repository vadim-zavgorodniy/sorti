#include <iostream>
#include <fstream>
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

std::vector<char> CHAR_TABLE;

//============================================================
void initCharTable() {
  CHAR_TABLE.clear();
  CHAR_TABLE.push_back(32); // space
  CHAR_TABLE.push_back(44); // ,
  CHAR_TABLE.push_back(46); // .
  // digits and : ;
  for (int i = 48; i < 60; ++i) {
    CHAR_TABLE.push_back(i);
  }

  // digits and : ;
  for (int i = 48; i <= 59; ++i) {
    CHAR_TABLE.push_back(i);
  }

  // capital letters
  for (int i = 65; i <= 90; ++i) {
    CHAR_TABLE.push_back(i);
  }

  // small letters
  for (int i = 97; i <= 122; ++i) {
    CHAR_TABLE.push_back(i);
  }
}

//============================================================
std::string generateString(size_t maxLength) {
  // assume that min maxLenth is 10
  const int MIN_BLOCK = 10;
  if (maxLength < MIN_BLOCK) {
    maxLength = MIN_BLOCK;
  }

  // actual string size
  size_t len = rand() % maxLength;

  std::string str;

  // let's gen it
  for (int i = 0; i < len; i += MIN_BLOCK) {
    char buf[MIN_BLOCK + 1];
    memset(buf, '\0', MIN_BLOCK+1);
    // table offset
    int offset = rand() % (CHAR_TABLE.size() - MIN_BLOCK);

    // copy blocks to result string
    strncpy(buf, &CHAR_TABLE[offset], MIN_BLOCK);
    str += buf;
  }

  return str;
}

//============================================================
void generate(std::ostream& ostr, size_t maxSize) {
  initCharTable();

  srand(time(NULL));
  // file size
  size_t size = 0;
  // row count
  long count = 0;
  // skip amount before repeat string update
  int repeatUpdate = 50;
  std::string repeatStr = "";
  int repeats = 0;

  // while result size less then maxSize in kB
  while (size / 1024 < maxSize) {
    int num = rand() % INT_MAX;

    // counting string size
    char numBuf[20];
    sprintf (numBuf, "%d", num);
    std::string res(numBuf);
    std::string str = "";

    // 2 times often we try to repeat string
    if (count % (repeatUpdate / 2) == 0) {
      if (rand() % 2 == 1) {
        if (!repeatStr.empty()) {
          str = repeatStr;
          repeats++;
        }
      }
    }

    // if it is not a repeat gen new str
    if (str.empty()) {
      str = generateString(1024);
    }

    res += DELIMITER;
    res += str;

    ostr << res << std::endl;

    // file size
    size += res.size();
    // record count
    ++count;

    if (count % repeatUpdate == 0) {
      // shell we remember it?
      if (rand() % 2 == 1) {
        repeatStr = str;
      }
    }
  }

  std::cout << "Rows count: " << count << std::endl;
  std::cout << "Repeats count: " << repeats << std::endl;
}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start." << std::endl;
  try {
    std::ofstream ostr("gen_data.txt");
    generate(ostr, 1024*1024);
    ostr.close();
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
