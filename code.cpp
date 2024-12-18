#include <cstring>
#include <iostream>
#include <memoryriver.hpp>

const int BLOCK_SIZE = 1000;
const std::string BLOCK_FILE_PREFIX = "block.txt";
const std::string INDEX_FILE = "index.txt";

struct Block {
  char index[64];
  int value;
};

struct Index {
  char name[64];
  int number;

  int nextplace;
  int link;
  int datanumber;
};

std::string instruction;
char readindex[64];
Index idx[BLOCK_SIZE + 1];
int head, len, startplace;
Block takeblock[BLOCK_SIZE + 1];
MemoryRiver<Index> lst(INDEX_FILE);
MemoryRiver<Block> block(BLOCK_FILE_PREFIX);

int findblock(char *index, int value) {
  int i = head;
  while (strcmp(idx[i].name, index) < 0 || strcmp(idx[i].name, index) == 0 && idx[i].number < value) {
    if (idx[i].link == 0) break;
    i = idx[i].link;
  }
  return i;
}
void getblock(int blockidx) {
  int ins = info_len * sizeof(int) + blockidx * sizeof(Block) * BLOCK_SIZE;
  for (int i = 0; i < idx[blockidx].datanumber; ++i) {
    block.read(takeblock[i], ins);
    ins += sizeof(Block);
  }
}

int main() {
  //lst.clear(); //return 0;
  block.open();
  lst.open();
  head = 0;
  lst.get_info(len, 1); //len 是 块数
  int ins = sizeof(int) * info_len, i = head;
  while (i != -1) {
    lst.read(idx[i], ins);
    ins = idx[i].nextplace;
    if (ins < sizeof(int) * info_len) break;
    i = idx[i].link;
  }

  int n;
  std::cin >> n;
  for (i = 0; i < n; ++i) {
    std::cin >> instruction >> readindex;
    if (instruction == "insert") {
      int value, blockidx;
      std::cin >> value;
      blockidx = findblock(readindex, value);
      getblock(blockidx);
      int j = idx[blockidx].datanumber - 1;
      while (!(strcmp(takeblock[j].index, readindex) < 0 || strcmp(takeblock[j].index, readindex) == 0 && takeblock[j].value < value) && j >= 0) {
        takeblock[j + 1] = takeblock[j];
        --j;
      }
      ++j;
      strcpy(takeblock[j].index, readindex);
      takeblock[j].value = value;
      ++idx[blockidx].datanumber;
      if (idx[blockidx].datanumber == BLOCK_SIZE) {  //裂块
        int num = idx[blockidx].datanumber / 2;
        Index newblock;
        newblock.datanumber = idx[blockidx].datanumber - num;
        newblock.link = idx[blockidx].link;
        newblock.nextplace = idx[blockidx].nextplace;
        strcpy(newblock.name, idx[blockidx].name);
        newblock.number = idx[blockidx].number;
        for (int p = 0; p < num; ++p) {
          block.write(takeblock[p], p + blockidx * BLOCK_SIZE);
        }
        for (int p = num; p < BLOCK_SIZE; ++p) {
          block.write(takeblock[p], p + len * BLOCK_SIZE);
        }

        idx[blockidx].datanumber = num;
        idx[blockidx].link = len;
        idx[blockidx].nextplace = sizeof(int) * info_len + blockidx * sizeof(Block);
        strcpy(idx[blockidx].name, takeblock[num - 1].index);
        idx[blockidx].number = takeblock[num - 1].value;
        ++len;
        lst.write_info(len, 1);

      } else {   //更新
        for (int p = 0; p < idx[blockidx].datanumber; ++p) {
          block.write(takeblock[p], p + blockidx * BLOCK_SIZE);
        }
      }
      //std::cout << instruction << readindex << value;
    } else if (instruction == "delete") {
      int value, blockidx;
      std::cin >> value;
      blockidx = findblock(readindex, value);
      getblock(blockidx);
      int l = 0, r = idx[blockidx].datanumber - 1, m;
      while (l < r) {
        m = (l + r) / 2;
        if (strcmp(takeblock[m].index, readindex) < 0 || strcmp(takeblock[m].index, readindex) == 0 && takeblock[m].value < value) {
          l = m + 1;
        } else {
          r = m;
        }
      }
      if (strcmp(takeblock[m].index, readindex) == 0 && takeblock[m].value == value) {
        --idx[blockidx].datanumber;
        for (int p = m; p < idx[blockidx].datanumber; ++p) {
          takeblock[p] = takeblock[p + 1];
          block.write(takeblock[p], p + blockidx * BLOCK_SIZE);
        }
        /*
        if (idx[blockidx].datanumber == 0) {

        } else if (m == idx[blockidx].datanumber) {
          idx[blockidx].name = takeblock[m - 1].index;
          idx[blockidx].number = takeblock[m - 1].value;
        }*/
      }
    } else if (instruction == "find") {
      int value = -1, blockidx;
      blockidx = findblock(readindex, value);
      getblock(blockidx);
      bool print = false;
      for (int p = 0; p < idx[blockidx].datanumber; ++p) {
        if (strcmp(takeblock[p].index,readindex) == 0) {
          std::cout << takeblock[p].value << ' ';
          print = true;
        }
      }
      if (idx[blockidx].nextplace > sizeof(int) * info_len) {
        blockidx = idx[blockidx].link;
        for (int p = 0; p < idx[blockidx].datanumber; ++p) {
          if (strcmp(takeblock[p].index,readindex) == 0) {
            std::cout << takeblock[p].value << ' ';
            print = true;
          }
        }
      }
      if (!print) {
        std::cout << "null";
      }
      std::cout << std::endl;
    }
  }

  lst.close();
  block.close();
  return 0;
}