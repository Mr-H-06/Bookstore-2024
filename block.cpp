#include <block.hpp>
#include <cstring>
#include <iostream>
#include <vector>
#include <iomanip>

template<typename T>
BlockManager<T>::BlockManager(const std::string &namelst, const std::string &nameblock)
  : lst(namelst), block(nameblock) {
  open();
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
      strcpy(takeblock.data[i].value, "");
    }
    block.write(takeblock, 0);
  }
}

template<typename T>
BlockManager<T>::~BlockManager() {
  lst.write_info(len, 1);
  for (int i = 0; i < len; ++i) {
    lst.write(idx[i], i);
  }
  close();
}

template<typename T>
void BlockManager<T>::open() {
  lst.open();
  block.open();
}

template<typename T>
void BlockManager<T>::close() {
  lst.close();
  block.close();
}

template<typename T>
void BlockManager<T>::insert(char *index, char *value, T message) {
  int blockidx = findblock(index, value);
  getblock(blockidx);
  int j = idx[blockidx].datanumber - 1;
  while (j >= 0 && !(strcmp(takeblock.data[j].index, index) < 0 || strcmp(takeblock.data[j].index, index) == 0 && strcmp(takeblock.data[j].value, value) < 0)) {
    takeblock.data[j + 1] = takeblock.data[j];
    --j;
  }
  ++j;
  strcpy(takeblock.data[j].index, index);
  strcpy(takeblock.data[j].value, value);
  takeblock.data[j].other = message;
  ++idx[blockidx].datanumber;
  if (idx[blockidx].datanumber == BLOCK_SIZE) {
    //裂块
    int num = idx[blockidx].datanumber / 2;
    idx[len].datanumber = idx[blockidx].datanumber - num;
    idx[len].link = idx[blockidx].link;
    strcpy(idx[len].key, idx[blockidx].key);
    strcpy(idx[len].value, idx[blockidx].value);
    Block<T> otherblock;
    for (int p = num; p < BLOCK_SIZE; ++p) {
      otherblock.data[p - num] = takeblock.data[p];
    }
    block.write(otherblock, len);

    idx[blockidx].datanumber = num;
    idx[blockidx].link = len;
    strcpy(idx[blockidx].key, takeblock.data[num - 1].index);
    strcpy(idx[blockidx].value, takeblock.data[num - 1].value);
    ++len;
  }
  block.write(takeblock, blockidx);
}

template<typename T>
void BlockManager<T>::deletevalue(char *index, char *value) {
  int blockidx = findblock(index, value);
  if (idx[blockidx].datanumber == 0) return;;
  getblock(blockidx);
  int l = 0, r = idx[blockidx].datanumber - 1, m;
  while (l < r) {
    m = (l + r) / 2;
    if (strcmp(takeblock.data[m].index, index) < 0 ||
      strcmp(takeblock.data[m].index, index) == 0 && strcmp(takeblock.data[m].value, value) < 0) {
      l = m + 1;
      } else {
        r = m;
      }
  }
  if (strcmp(takeblock.data[l].index, index) ==0 && strcmp(takeblock.data[l].value, value) == 0) {
    --idx[blockidx].datanumber;
    for (int p = l; p < idx[blockidx].datanumber; ++p) {
      takeblock.data[p] = takeblock.data[p + 1];
    }
    block.write(takeblock, blockidx);
  }
}

template<typename T>
std::vector<Data<T>> BlockManager<T>::find(char *index) {
  std::vector<Data<T>> ans;
  int blockidx = findblock(index, "");
  getblock(blockidx);
  //bool print = false;
  for (int p = 0; p < idx[blockidx].datanumber; ++p) {
    if (strcmp(takeblock.data[p].index,index) == 0) {
      //std::cout << takeblock.data[p].value << ' ';
      ans.push_back(takeblock.data[p]);
      //print = true;
    }
  }
  blockidx = idx[blockidx].link;
  getblock(blockidx);
  while (blockidx != 0 && strcmp(takeblock.data[0].index, index) == 0) {
    for (int p = 0; p < idx[blockidx].datanumber; ++p) {
      if(strcmp(takeblock.data[p].index, index) == 0) {
        //std::cout << takeblock.data[p].value << ' ';
        ans.push_back(takeblock.data[p]);
        //print = true;
      }
    }
    blockidx = idx[blockidx].link;
    if (blockidx == 0) {
      break;
    } else {
      getblock(blockidx);
    }
  }
  //if (!print) {
  //  std::cout << "null";
  //}
  //std::cout << std::endl;
  return ans;
}

template<>
void BlockManager<Book>::printall() {
  int blockidx = 0;
  getblock(blockidx);
  bool print = false;
  for (int p = 0; p < idx[blockidx].datanumber; ++p) {
    print = true;
    std::cout << takeblock.data[p].index << '\t'
      << takeblock.data[p].other.bookname << '\t'
      << takeblock.data[p].other.author << '\t'
      << takeblock.data[p].other.keyword << '\t'
      << std::fixed << std::setprecision(2) <<takeblock.data[p].other.price << '\t'
      << takeblock.data[p].other.quantity << '\n';
  }
  blockidx = idx[blockidx].link;
  getblock(blockidx);
  while (blockidx != 0) {
    for (int p = 0; p < idx[blockidx].datanumber; ++p) {
      print = true;
      std::cout << takeblock.data[p].index << '\t'
        << takeblock.data[p].other.bookname << '\t'
        << takeblock.data[p].other.author << '\t'
        << takeblock.data[p].other.keyword << '\t'
        << std::fixed << std::setprecision(2) << takeblock.data[p].other.price << '\t'
        << takeblock.data[p].other.quantity << '\n';
    }
    blockidx = idx[blockidx].link;
    if (blockidx == 0) {
      break;
    } else {
      getblock(blockidx);
    }
  }
  if (!print) {
    std::cout << '\n';
  }
}

template<>
void BlockManager<Account>::report(bool type) { // true-employee false-all
  int k;
  if (type == true) {
    k = 2;
  } else {
    k = 0;
  }
  std::cout << "UserID\tUserName\tprivilege\tpay\tspand\n";
  int blockidx = 0;
  getblock(blockidx);
  for (int p = 0; p < idx[blockidx].datanumber; ++p) {
    if (takeblock.data[p].other.privilege > k) {
      std::cout << takeblock.data[p].index << '\t'
        << takeblock.data[p].other.username << '\t'
        << takeblock.data[p].other.privilege << '\t'
        << std::fixed << std::setprecision(2) << takeblock.data[p].other.in << '\t'
        << std::fixed << std::setprecision(2) << takeblock.data[p].other.out << '\n';
    }
  }
  blockidx = idx[blockidx].link;
  getblock(blockidx);
  while (blockidx != 0) {
    for (int p = 0; p < idx[blockidx].datanumber; ++p) {
      if (takeblock.data[p].other.privilege > k) {
        std::cout << takeblock.data[p].index << '\t'
          << takeblock.data[p].other.username << '\t'
          << takeblock.data[p].other.privilege << '\t'
          << std::fixed << std::setprecision(2) << takeblock.data[p].other.in << '\t'
          << std::fixed << std::setprecision(2) << takeblock.data[p].other.out << '\n';
      }
    }
    blockidx = idx[blockidx].link;
    if (blockidx == 0) {
      break;
    } else {
      getblock(blockidx);
    }
  }
}


template class BlockManager<Book>;
template class BlockManager<Account>;
template class BlockManager<Bookname>;
template class BlockManager<Author>;
template class BlockManager<Keyword>;