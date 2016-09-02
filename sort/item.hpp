#pragma once
// #ifndef __ITEM_HPP__
// #define __ITEM_HPP__

#include <string>
#include <vector>

struct Item {
  unsigned num;
  std::string str;
};

typedef std::vector<Item> ItemList;

Item parseItem(std::string line);

std::ostream& operator<<(std::ostream& ostr, const Item& obj);

int compareItemsQuick(const void * p1, const void * p2);

bool compareItems(const Item& a, const Item& b);

void outToStream(const ItemList& list, std::ostream& ostr);

// #endif
