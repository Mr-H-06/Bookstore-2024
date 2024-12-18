#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

const int info_len = 1;

template<class T, int info_len = info_len>
class MemoryRiver {
private:
  /* your code here */
  fstream file;
  string file_name;
  int sizeofT = sizeof(T);
public:
  MemoryRiver() = default;
  MemoryRiver(const string& file_name) : file_name(file_name) {}
  void open() {
    file.open(file_name, std::ios::in | std::ios::out);
    if(!file.is_open()){
      //std::cout << "open error\n";
      file.open(file_name, std::ios::out);
      file.close();
      file.open(file_name, std::ios::in | std::ios::out);
      int tmp = 0;
      for (int i = 0; i < info_len; ++i)
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }
  }
  void initialise(string FN = "") {
    if (FN != "") file_name = FN;
    file.open(file_name, std::ios::in | std::ios::out);
    int tmp = 0;
    for (int i = 0; i < info_len; ++i)
      file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
  }
  //读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len) return;
    /* your code here */
    //file.open(file_name, std::ios::in);
    file.seekg(sizeof(int) * (n - 1));
    file.read(reinterpret_cast<char *> (&tmp), sizeof(int));
    //if(!file) {std::cout << "err_get info";}
    //file.close();
  }
  //将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len) return;
    /* your code here */
    //file.open(file_name, std::ios::in | std::ios::out);
    file.seekp(sizeof(int) * (n - 1));
    file.write(reinterpret_cast<char *> (&tmp), sizeof(int));
    //if (!file) {std::cout << "   err write_info";}
    //file.close();
  }
  //在文件合适位置写入类对象t，并返回写入的位置索引index
  //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
  //位置索引index可以取为对象写入的起始位置
  void write(T &t, int ins) {
    /* your code here */
    //file.open(file_name, std::ios::out | std::ios::app);
    //if (!file) {std::cout << "   err write\n";}
    file.seekp(sizeof(int) * info_len + ins * sizeofT);
    //int index = file.tellp();
    //if (!file) {std::cout << "   err write\n";}
    file.write(reinterpret_cast<char *>(&t), sizeofT);
    //if (!file) {std::cout << "   err write\n";}
    //file.close();
    //return index;
  }
  //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(T &t, const int index) {
    /* your code here */
    //file.open(file_name, std::ios::in | std::ios::out);
    file.seekp(index);
    file.write(reinterpret_cast<char *>(&t), sizeofT);
    //file.close();
  }
  //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, const int index) {
    /* your code here */
    //file.open(file_name, std::ios::in);
    //if(!file) {std::cout << "   err read\n" ;}
    file.seekg(index);
    //if(!file) {std::cout << "   err read\n";}
    file.read(reinterpret_cast<char *>(&t), sizeofT);
    //if(!file) {std::cout << "   err read\n";}
    //file.close();
  }

  void close() {
    /* your code here */
    file.close();
  }
  void clear() {
    close();
    file.open(file_name,std::ios::out | std::ios::trunc);
    close();
    //file.open(file_name,std::ios::in | std::ios::out);
  }
};


#endif //BPT_MEMORYRIVER_HPP