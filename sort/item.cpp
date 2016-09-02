#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>

#include "item.hpp"

#define DELIMITER ". "

//============================================================
std::ostream& operator<<(std::ostream& ostr, const Item& obj)
{
  ostr << obj.num << DELIMITER << obj.str;
  return ostr;
}

//============================================================
Item parseItem(const std::string& line) {
  Item item;
  int pos = line.find(". ");
  std::string numStr = line.substr(0, pos);
  item.num = atoi(numStr.c_str());
  item.str = line.substr(pos+2, line.size());

  return item;
}

//============================================================
int compareItemsQuick(const void * p1, const void * p2) {
  Item& a = *(Item*)p1;
  Item& b = *(Item*)p2;

  if (a.str <  b.str) return -1;
  if (a.str >  b.str) return 1;
  if (a.str == b.str) {
    if (a.num < b.num) return -1;
    if (a.num > b.num) return 1;
  }
  return 0;
}

//============================================================
bool compareItems(const Item& a, const Item& b) {
  if (a.str <  b.str) return true;
  if (a.str >  b.str) return false;
  if (a.str == b.str) {
    if (a.num < b.num) return true;
    if (a.num > b.num) return false;
  }
  return false;
}

//============================================================
void outToStream(const ItemList& list, std::ostream& ostr) {
  for (ItemList::const_iterator it = list.begin(); it != list.end(); ++it) {
    ostr << *it << std::endl;
  }
}

//============================================================
