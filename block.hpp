#ifndef BLOCK_H
#define BLOCK_H

#include <memoryriver.hpp>
#include <vector>
#include <cstring>

constexpr int BLOCK_SIZE = 20;
const std::string BLOCK_FILE_PREFIX = "block.txt";
const std::string INDEX_FILE = "index.txt";

struct Account {
  // UserID作为index内容，内存开在块链的库里  30可用
  char password[30];
  int privilege;
  char username[30];
};

struct Book {
  // ISBN作为index内容，内存开在块链的库里
  char bookname[60];
  char author[60];
  char keyword[60];
  double price;   //单价
  int quantity;   //库存量
};

struct Bookname {};

struct Author{};

struct Keyword {}; // keyword to ISBN

struct Log {};

struct Employee{};

template<typename T>
struct Data {
  char index[64];
  char value[20];

  T other;
};

template<typename T>
struct Block {
  Data<T> data[BLOCK_SIZE];
};

struct Index {
  char key[64];
  char value[20]; //仅关键词为索引时启用

  int link;
  int datanumber;
};

template<typename T>
class BlockManager {
private:
  //std::string instruction;
  Index idx[BLOCK_SIZE];
  int head, len, startplace;
  Block<T> takeblock;
  MemoryRiver<Index> lst;
  MemoryRiver<Block<T>> block;

  inline int findblock(char *index, char *value) {
    int i = 0;
    while (strcmp(idx[i].key, index) < 0 || strcmp(idx[i].key, index) == 0 && strcmp(idx[i].value, value) < 0) {
      if (idx[i].link == 0) break;
      i = idx[i].link;
    }
    return i;
  }
  inline void getblock(int blockidx) {
    block.read(takeblock, info_len * sizeof(int) + blockidx * sizeof(Block<T>));
  }
public:
  BlockManager(const std::string &namelst, const std::string &nameblock);
  ~BlockManager();
  void insert(char *index, char *value, T message);
  void deletevalue(char *index, char *value);
  std::vector<Data<T>> find(char *index);
  void printall();
  void open();
  void close();
};


/*
std::string instruction;
char readindex[64];
Index idx[BLOCK_SIZE + 1];
int head, len, startplace;
Block takeblock;
MemoryRiver<Index> lst(INDEX_FILE);
MemoryRiver<Block> block(BLOCK_FILE_PREFIX);

int findblock(char *index, int value) {
  int i = 0;
  while (strcmp(idx[i].name, index) < 0 || strcmp(idx[i].name, index) == 0 && idx[i].number < value) {
    if (idx[i].link == 0) break;
    i = idx[i].link;
  }
  return i;
}
void getblock(int blockidx) {
  block.read(takeblock, info_len * sizeof(int) + blockidx * sizeof(Block));
}

int main() {
  block.open();
  lst.open();
  lst.get_info(len, 1); //len 是 块数
  if (len != 0) {
    for (int i = 0; i < len; ++i) {
      lst.read(idx[i], sizeof(int) * info_len + i * sizeof(Index));
    }
  } else {
    len = 1;
    lst.write_info(1, 1);
    for (int i = 0; i < BLOCK_SIZE; ++i) {
      strcpy(takeblock.data[i].index, "");
      takeblock.data[i].value = 0;
    }
    block.write(takeblock, 0);
  }

  int n;
  std::cin >> n;
  for (int i = 0; i < n; ++i) {
    std::cin >> instruction >> readindex;
    if (instruction == "insert") {
      int value, blockidx;
      std::cin >> value;
      blockidx = findblock(readindex, value);
      getblock(blockidx);
      int j = idx[blockidx].datanumber - 1;
      while (j >= 0 && !(strcmp(takeblock.data[j].index, readindex) < 0 || strcmp(takeblock.data[j].index, readindex) == 0 && takeblock.data[j].value < value)) {
        takeblock.data[j + 1] = takeblock.data[j];
        --j;
      }
      ++j;
      strcpy(takeblock.data[j].index, readindex);
      takeblock.data[j].value = value;
      ++idx[blockidx].datanumber;
      if (idx[blockidx].datanumber == BLOCK_SIZE) {  //裂块
        int num = idx[blockidx].datanumber / 2;
        idx[len].datanumber = idx[blockidx].datanumber - num;
        idx[len].link = idx[blockidx].link;
        strcpy(idx[len].name, idx[blockidx].name);
        idx[len].number = idx[blockidx].number;
        Block otherblock;
        for (int p = num; p < BLOCK_SIZE; ++p) {
          otherblock.data[p - num] = takeblock.data[p];
        }
        block.write(otherblock, len);

        idx[blockidx].datanumber = num;
        idx[blockidx].link = len;
        strcpy(idx[blockidx].name, takeblock.data[num - 1].index);
        idx[blockidx].number = takeblock.data[num - 1].value;
        ++len;
      }  //更新
      block.write(takeblock, blockidx);
      //std::cout << instruction << readindex << value;
    } else if (instruction == "delete") {
      int value, blockidx;
      std::cin >> value;
      blockidx = findblock(readindex, value);
      if (idx[blockidx].datanumber == 0) continue;
      getblock(blockidx);
      int l = 0, r = idx[blockidx].datanumber - 1, m;
      while (l < r) {
        m = (l + r) / 2;
        if (strcmp(takeblock.data[m].index, readindex) < 0 || strcmp(takeblock.data[m].index, readindex) == 0 && takeblock.data[m].value < value) {
          l = m + 1;
        } else {
          r = m;
        }
      }
      if (strcmp(takeblock.data[l].index, readindex) == 0 && takeblock.data[l].value == value) {
        --idx[blockidx].datanumber;
        for (int p = l; p < idx[blockidx].datanumber; ++p) {
          takeblock.data[p] = takeblock.data[p + 1];
        }
        block.write(takeblock, blockidx);
      }
    } else if (instruction == "find") {
      int value = -1, blockidx;
      blockidx = findblock(readindex, value);
      getblock(blockidx);
      bool print = false;
      for (int p = 0; p < idx[blockidx].datanumber; ++p) {
        if (strcmp(takeblock.data[p].index,readindex) == 0) {
          std::cout << takeblock.data[p].value << ' ';
          print = true;
        }
      }
      blockidx = idx[blockidx].link;
      getblock(blockidx);
      while (blockidx != 0 && strcmp(takeblock.data[0].index, readindex) == 0) {
        for (int p = 0; p < idx[blockidx].datanumber; ++p) {
          if(strcmp(takeblock.data[p].index,readindex) == 0) {
            std::cout << takeblock.data[p].value << ' ';
            print = true;
          }
        }
        blockidx = idx[blockidx].link;
        if (blockidx == 0) {
          break;
        } else {
          getblock(blockidx);
        }
      }
      if (!print) {
        std::cout << "null";
      }
      std::cout << std::endl;
    }
  }
  lst.write_info(len, 1);
  for (int blockidx = 0; blockidx < len; ++blockidx) {
    lst.write(idx[blockidx], blockidx);
  }

  lst.close();
  block.close();
  return 0;
}*/

#endif //BLOCK_H
