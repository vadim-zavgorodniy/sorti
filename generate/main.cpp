#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <limits.h>

#define DELIMITER ". "

std::vector<int> CHAR_TABLE;

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

std::string generateString(size_t maxLength) {
  // actual string size
  size_t len = rand() % maxLength;

  std::string str;

  for (int i = 0; i < len; ++i) {
    int sym = rand() % CHAR_TABLE.size();

    str += (char)CHAR_TABLE[sym];
  }

  return str;
}

//============================================================
void generate(std::ostream& ostr, size_t maxSize) {
  initCharTable();

  srand(time(NULL));
  size_t size = 0;

  // while result size less then maxSize in kB
  while (size / 1024 < maxSize) {
    int num = rand() % INT_MAX;

    // counting string size
    char numBuf[20];
    sprintf (numBuf, "%d", num);

    std::string res(numBuf);
    res += DELIMITER;
    res += generateString(1024);

    ostr << res << std::endl;

    size += res.size();
  }
}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start" << std::endl;
  try {
    // std::cout << "Generated lines: " << items->size() << std::endl;
    // std::cout << "=============" << std::endl;

    std::ofstream ostr("gen_data.txt");
    generate(ostr, 1024*1024);
    ostr.close();
//    generate(std::cout, 10);

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
