#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <cstdlib>
#include <algorithm>
#include <memory>

#define DELIMITOR ". "

//============================================================
struct Item {
  unsigned num;
  std::string str;
};

typedef std::vector<Item> ItemList;

//============================================================
std::ostream& operator<<(std::ostream& ostr, const Item& obj)
{
  ostr << obj.num << DELIMITOR << obj.str;
  return ostr;
}

//============================================================
Item parseItem(std::string line) {
  Item item;
  int pos = line.find(". ");
  std::string numStr = line.substr(0, pos);
  item.num = atoi(numStr.c_str());
  item.str = line.substr(pos+2, line.size());

  return item;
}

//============================================================
// int compareItems(const void * p1, const void * p2) {
//   Item& a = *(Item*)p1;
//   Item& b = *(Item*)p2;

//   if (a.str <  b.str) return -1;
//   if (a.str >  b.str) return 1;
//   if (a.str == b.str) {
//     if (a.num < b.num) return -1;
//     if (a.num > b.num) return 1;
//   }
//   return 0;
// }

//============================================================
bool compareItems(const Item a, const Item b) {
  // Item& a = *(Item*)p1;
  // Item& b = *(Item*)p2;

  if (a.str <  b.str) return true;
  if (a.str >  b.str) return false;
  if (a.str == b.str) {
    if (a.num < b.num) return true;
    if (a.num > b.num) return false;
  }
  return false;
}

//============================================================
void openFile(std::string name, std::ifstream& istr) {
  istr.open(name.c_str(), std::ifstream::in);
  if (!istr.is_open()) {
    throw std::runtime_error("Unable to open file: " + name);
  }
}

//============================================================
ItemList* readFile(std::istream& myfile, size_t maxCount = std::string::npos) {

  ItemList* lst = new ItemList();

  int count = 0;
  std::string line;

  while (getline(myfile, line) && count < maxCount) {
    Item item = parseItem(line);
    lst->push_back(item);
    ++count;
  }

  return lst;
}

//============================================================
void printList(ItemList* list, std::ostream& ostr) {
  for (ItemList::const_iterator it = list->begin(); it != list->end(); ++it) {
    ostr << *it << std::endl;
  }
}

//============================================================
int main(int argc, char* argv[]) {

  std::cout << "Start" << std::endl;

  try {
    std::ifstream istr;

//    openFile("gen_data.txt", istr);
    openFile("../generate/gen_data.txt", istr);
    std::auto_ptr<ItemList> items;
    try {
      // items.reset(readFile(istr, 10));
      items.reset(readFile(istr));

      istr.close();
    }
    catch (std::exception e) {
      istr.close();
      throw e;
    }

    // printList(items.get(), std::cout);
    std::cout << "Readed lines: " << items->size() << std::endl;
    std::cout << "=============" << std::endl;

//    qsort(&(*items)[0], items->size(), sizeof(Item), compareItems);
    sort(items->begin(), items->end(), compareItems);

    // printList(items.get(), std::cout);
    std::ofstream ostr("sorted_data.txt");
    printList(items.get(), ostr);

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
