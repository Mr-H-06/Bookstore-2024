#include <iostream>
#include <block.hpp>
#include <vector>
#include <cstring>
#include <iomanip>

char selectbook[10000][20];
char user[10000][30];
int privilege[10000];
int signedins = 0;
int financenumber = 0;


struct Finance {
  char idx[6];
  double in;
  double out;
};

//template class BlockManager<Book>;
std::vector<Data<Book>> find_books;
std::vector<Data<Account>> find_accounts;
std::vector<Data<Bookname>> find_booknames;
std::vector<Data<Author>> find_authors;
std::vector<Data<Keyword>> find_keywords;
char s[60];
BlockManager<Book> book("book_" + INDEX_FILE, "book_" + BLOCK_FILE_PREFIX);
BlockManager<Account> account("account_" + INDEX_FILE, "account_" + BLOCK_FILE_PREFIX);
BlockManager<Bookname> bookname("bookname_" + INDEX_FILE, "bookname_" + BLOCK_FILE_PREFIX);
BlockManager<Author> author("author_" + INDEX_FILE, "author_" + BLOCK_FILE_PREFIX);
BlockManager<Keyword> keyword("keyword_" + INDEX_FILE, "keyword_" + BLOCK_FILE_PREFIX);
BlockManager<Log> log("log_" +INDEX_FILE, "log_" + BLOCK_FILE_PREFIX); //日志
MemoryRiver<Finance> finance("finance.txt");

inline void error() {
  std::cout << "Invalid\n";
}

inline void coutnumber(double number) {
  std::cout << std::fixed << std::setprecision(2) << number;
}

//account
bool legal_basicdata(char *str) {  // userid password
  if (strlen(str) > 30) return false;
  while (*str) {
    if (!isalnum(*str) && *str != '_') {
      return false;
    }
    ++str;
  }
  return true;
}

bool legal_name(char *str) { //合法名字
  if (strlen(str) > 30) return false;
  while (*str) {
    if (*str < 32 || *str > 126) {
      return false;
    }
    ++str;
  }
  return true;
}

bool legal_number(char *str) {  //整数判断
  if (str == nullptr || *str == '0') {
    return false;
  }
  while (*str) {
    if (!isdigit(*str)) {
      return false;
    }
    ++str;
  }
  if (strlen(str) > 10 || strlen(str) == 10 && strcmp(str, "2147483647") > 0) {
    return false;
  }
  return true;
}

//book
bool legal_ISBN(char *str) {
  if (strlen(str) > 20) return false;
  while (*str) {
    if (*str < 32 || *str > 126) {
      return false;
    }
    ++str;
  }
  return true;
}

bool legal_bookdata(char *str) {   //bookname author keyword
  if (strlen(str) > 60) return false;
  while (*str) {
    if (*str < 32 || *str > 126 || *str == '"') {
      return false;
    }
    ++str;
  }
  return true;
}

bool legal_price(char *str) {
  char *endptr;
  strtod(str, &endptr);
  if (strlen(str) > 13 || endptr != str + strlen(str)) {
    error();
    return false;
  }
  return true;
}

void deletekey(char *k) {
  char *key;
  key = strtok(k, "|");
  while (key != nullptr) {
    keyword.deletevalue(key, selectbook[signedins]);
    key = strtok(nullptr, "|");
  }
}
void insertkey(char *k) {
  char *key;
  key = strtok(k, "|");
  Keyword a;
  while (key != nullptr) {
    keyword.insert(k, selectbook[signedins], a);
    key = strtok(nullptr, "|");
  }
}

char *numbertoid(int n) {
  std::string result;
  while (n > 0) {
    result = static_cast<char> ((n % 10) + '0') + result;
    n /= 10;
  }
  while (result.size() < 5) {
    result = "0" + result;
  }
  return const_cast<char *> (result.c_str());
}

void login(char *userid, char *password) {
  find_accounts = account.find(userid);
  if (find_accounts.empty()) {
    error();
    return;
  }
  if (privilege[signedins] > find_accounts[0].other.privilege && strcmp(password, "") == 0) {
    ++signedins;
    privilege[signedins] = find_accounts[0].other.privilege;
    strcpy(user[signedins], userid);
    strcpy(selectbook[signedins], "");
  } else if (strcmp(find_accounts[0].other.password, password) == 0) {
    ++signedins;
    privilege[signedins] = find_accounts[0].other.privilege;
    strcpy(user[signedins], userid);
    strcpy(selectbook[signedins], "");
  } else {
    error();
  }
}

void logout() {
  if (signedins > 0) {
    --signedins;
  } else {
    error();
  }
}

void registerAccount(char *userid, char *password, char *username) {
  find_accounts = account.find(userid);
  if (!find_accounts.empty()) {
    error();
    return;
  }
  Account newaccount;
  strcpy(newaccount.password, password);
  strcpy(newaccount.username, username);
  newaccount.privilege = 1;
  account.insert(userid, "", newaccount);
}

void changePassword(char *userid, char *currentpassword, char *newpassword) {
  find_accounts = account.find(userid);
  if (find_accounts.empty()
    || (privilege[signedins] != 7 && strcmp(find_accounts[0].other.password, currentpassword) != 0)
    || strcmp(currentpassword, newpassword) == 0
    || !legal_basicdata(newpassword)) {
    error();
    return;
  }
  account.deletevalue(userid, "");
  Account newaccount = find_accounts[0].other;
  strcpy(newaccount.password, newpassword);
  account.insert(userid, "", newaccount);
}

void addUser(char *userid, char *password, int newprivilege, char *username) {
  find_accounts = account.find(userid);
  if (!find_accounts.empty()
    || newprivilege >= privilege[signedins]) {
    error();
    return;
  }
  if (newprivilege != 7 && newprivilege != 3 && newprivilege != 1 && newprivilege != 1) {
    error();
    return;
  }
  Account newaccount;
  strcpy(newaccount.password, password);
  newaccount.privilege = newprivilege;
  strcpy(newaccount.username, username);
  account.insert(userid, "", newaccount);
}

void deleteUser(char *userid) {
  find_accounts = account.find(userid);
  if (strcmp(userid, user[signedins]) == 0 || find_accounts.empty()) {
    error();
    return;
  }
  account.deletevalue(userid, "");
}

void showBooks(char *type, char *message) {
  if (type == nullptr) {
    book.printall();
    return;
  }
  if (message == nullptr) {
    error();
    return;
  }
  if (strcmp(type, "-ISBN") == 0) {
    find_books = book.find(message);
    if (find_books.empty()) {
      std::cout << '\n';
    } else {
      for (Data<Book> p : find_books) {
        std::cout << p.index << '\t'
          << p.other.bookname << '\t'
          << p.other.author << '\t'
          << p.other.keyword << '\t'
          << std::fixed << std::setprecision(2) << p.other.price << '\t'
          << p.other.quantity << '\n';
      }
    }
  } else {
    if (message[0] != '"' || message[strlen(message) - 1] != '"') {
      error();
      return;
    }
    for (int i = 1; i < strlen(message) - 1; ++i) {
      message[i - 1] = message[i];
    }
    message[strlen(message) - 2] = '\0';
    if (strcmp(type, "-name") == 0) {
      find_booknames = bookname.find(message);
      if (find_booknames.empty()) {
        std::cout << '\n';
      } else {
        for (Data<Bookname> o : find_booknames) {
          find_books = book.find(o.value);
          for (Data<Book> p : find_books) {
            std::cout << p.index << '\t'
              << p.other.bookname << '\t'
              << p.other.author << '\t'
              << p.other.keyword << '\t'
              << std::fixed << std::setprecision(2) << p.other.price << '\t'
              << p.other.quantity << '\n';
          }
        }
      }
    } else if (strcmp(type, "-author") == 0) {
      find_authors = author.find(message);
      if (find_authors.empty()) {
        std::cout << '\n';
      } else {
        for (Data<Author> o : find_authors) {
          find_books = book.find(o.value);
          for (Data<Book> p : find_books) {
            std::cout << p.index << '\t'
              << p.other.bookname << '\t'
              << p.other.author << '\t'
              << p.other.keyword << '\t'
              << std::fixed << std::setprecision(2) << p.other.price << '\t'
              << p.other.quantity << '\n';
          }
        }
      }
    } else if (strcmp(type, "-keyword") == 0) {
      find_keywords = keyword.find(message);
      if (find_keywords.empty()) {
        std::cout << '\n';
      } else {
        for (Data<Keyword> o : find_keywords) {
          find_books = book.find(o.value);
          for (Data<Book> p : find_books) {
            std::cout << p.index << '\t'
              << p.other.bookname << '\t'
              << p.other.author << '\t'
              << p.other.keyword << '\t'
              << std::fixed << std::setprecision(2) << p.other.price << '\t'
              << p.other.quantity << '\n';
          }
        }
      }
    } else {
      error();
    }
  }
}

void buyBook(char *isbn, int quantity) {
  find_books = book.find(isbn);
  if (find_books.empty()) {
    error();
    return;
  }
  if (find_books[0].other.quantity < quantity) {
    error();
    return;
  }
  coutnumber(quantity * find_books[0].other.price);
  std::cout << '\n';
  //修改log
}

void select(char *isbn) {
  find_books = book.find(isbn);
  strcpy(selectbook[signedins], isbn);
  if (find_books.empty()) {
    Book newbook;
    strcpy(newbook.author, "");
    strcpy(newbook.bookname, "");
    strcpy(newbook.keyword, "");
    newbook.price = 0;
    newbook.quantity = 0;
    book.insert(isbn, "", newbook);
  }
}

void modifyBookInfo(char *type, char *message) {
  find_books = book.find(selectbook[signedins]);
  book.deletevalue(selectbook[signedins], "");
  if (strcmp(type, "-ISBN") == 0) {
    if (strcmp(find_books[0].other.bookname, "") != 0) {
      bookname.deletevalue(find_books[0].other.bookname, selectbook[signedins]);
    }
    if (strcmp(find_books[0].other.author, "") != 0) {
      author.deletevalue(find_books[0].other.author, selectbook[signedins]);
    }
    deletekey(find_books[0].other.keyword);/*
    char *k;
    strcpy(k, find_books[0].other.keyword);
    char *key;
    key = strtok(k, "|");
    while (key != nullptr) {
      keyword.deletevalue(key, selectbook[signedins]);
      key = strtok(nullptr, "|");
    }*/
    strcpy(selectbook[signedins], message);
    insertkey(find_books[0].other.keyword);/*
    char *k, *key;
    strcpy(k, find_books[0].other.keyword);
    key = strtok(k, "|");
    Keyword a;
    while (key != nullptr) {
      keyword.insert(key, selectbook[signedins], a);
      key = strtok(nullptr, "|");
    }*/
    if (strcmp(find_books[0].other.bookname, "") != 0) {
      Bookname b;
      bookname.insert(find_books[0].other.bookname, selectbook[signedins], b);
    }
    if (strcmp(find_books[0].other.author, "") != 0){
      Author c;
      author.insert(find_books[0].other.author, selectbook[signedins], c);
    }
  }
  if (strcmp(type, "-name") == 0) {
    bookname.deletevalue(find_books[0].other.bookname, selectbook[signedins]);
    strcpy(find_books[0].other.bookname, message);
    Bookname a;
    bookname.insert(message, selectbook[signedins], a);
  }
  if (strcmp(type, "-author") == 0) {
    author.deletevalue(find_books[0].other.author, selectbook[signedins]);
    strcpy(find_books[0].other.author, message);
    Author a;
    author.insert(message, selectbook[signedins], a);
  }
  if (strcmp(type, "-keyword") == 0) {
    char *k;
    strcpy(k, find_books[0].other.keyword);
    char *key = strtok(k, "|");
    while (key != nullptr) {
      keyword.deletevalue(key, selectbook[signedins]);
      key = strtok(nullptr, "|");
    }
    strcpy(find_books[0].other.keyword, message);
    strcpy(k, message);
    key = strtok(k, "|");
    Keyword a;
    while (key != nullptr) {
      keyword.insert(key, selectbook[signedins], a);
      key = strtok(nullptr, "|");
    }
  }
  if (strcmp(type, "-price") == 0) {
    find_books[0].other.price = strtod(message, nullptr);
  }
  book.insert(selectbook[signedins], "", find_books[0].other);
  //修改log
}

void importBooks(char *quantity, char *totalcost) {
  find_books = book.find(selectbook[signedins]);
  find_books[0].other.quantity += std::atoi(quantity);
  book.deletevalue(selectbook[signedins], "");
  book.insert(selectbook[signedins], "", find_books[0].other);
  //修改log
  char *id = numbertoid(financenumber + 1);
  Finance read;
  if (financenumber > 0) {
    finance.read(read, info_len * sizeof(int) + (financenumber - 1) * sizeof(Finance));
  } else {
    read.in = 0;
    read.out = 0;
  }
  strcpy(read.idx, id);
  read.out += strtod(totalcost, nullptr);
  finance.write(read, financenumber);
  ++financenumber;
}

void showFinance(char *count) {
  if (count == nullptr) {
    Finance read;
    finance.read(read, info_len * sizeof(int) + (financenumber - 1) * sizeof(Finance));
    std::cout << "+ ";
    coutnumber(read.in);
    std::cout << " - ";
    coutnumber(read.out);
    std::cout << '\n';
  } else {
    if (!legal_number(count)) {
      error();
      return;
    }
    int k = financenumber - std::atoi(count);
    if (k < 0) {
      error();
      return;
    } else {
      if (k == 0) {
        Finance read;
        finance.read(read, info_len * sizeof(int) + (financenumber - 1) * sizeof(Finance));
        std::cout << "+ ";
        coutnumber(read.in);
        std::cout << " - ";
        coutnumber(read.out);
        std::cout << '\n';
      } else {
        Finance read1, read2;
        finance.read(read1, info_len * sizeof(int) + (financenumber - 1) * sizeof(Finance));
        finance.read(read2, info_len * sizeof(int) + (k - 1) * sizeof(Finance));
        std::cout << "+ ";
        coutnumber(read1.in - read2.in);
        std::cout << " - ";
        coutnumber(read1.out - read2.out);
        std::cout << '\n';
      }
    }
  }
}

void reportFinance() {
  //no testcase
  Finance read;
  if (financenumber > 0) {
    finance.read(read, info_len * sizeof(int) + (financenumber - 1) * sizeof(Finance));
  } else {
    read.in = 0;
    read.out = 0;
  }
  std::cout << "+ ";
  coutnumber(read.in);
  std::cout << " - ";
  coutnumber(read.out);
  std::cout << '\n';
}

void reportEmployee() {  //no testcase
  std::cout << "Not yet complete\n";
}

void logAction() {
  std::cout << "Not yet complete\n";
}

void processCommand(char *line) {   //判断指令合法性
  char *command = strtok(line, " ");
  if (command == nullptr) {
    return;
  }
  if (strcmp(command,"su") == 0) {  //account
    char *userid, *password;
    userid = strtok(nullptr, " ");
    password = strtok(nullptr, " ");
    if (userid == nullptr || password == nullptr) {
      error();
      return;
    }
    if (strtok(nullptr, " ") != nullptr) {
      error();
      return;
    }
    if (!legal_basicdata(userid) || !legal_basicdata(password)) {
      error();
      return;
    }
    if (strcmp(password, "") == 0) {
      error();
      return;
    }
    login(userid, password);


  }
  else if (strcmp(command, "logout") == 0) {
    if (strtok(nullptr, " ") != nullptr) {
      error();
      return;
    }
    logout();


  }
  else if (strcmp(command, "register") == 0) {
    char *userid, *password, *username;
    userid = strtok(nullptr, " ");
    password = strtok(nullptr, " ");
    username = strtok(nullptr, " ");
    if (username == nullptr || !legal_basicdata(userid)
      || !legal_basicdata(password) || !legal_name(username)) {
      error();
      return;
    }
    if (strtok(nullptr, " ") != nullptr) {
      error();
      return;
    }
    registerAccount(userid, password, username);


  }
  else if (strcmp(command, "passwd") == 0) {
    char *userid, *currentpassword, *newpassword;
    userid = strtok(nullptr, " ");
    currentpassword = strtok(nullptr, " ");
    newpassword = strtok(nullptr, " ");
    if (newpassword == nullptr) {
      if (currentpassword == nullptr) {
        error();
        return;
      }
      if (privilege[signedins] == 7) {
        if (!legal_basicdata(currentpassword)) {
          error();
          return;
        }
        changePassword(userid, "", currentpassword);
        return;
      } else {
        error();
        return;
      }
    }
    if (!legal_basicdata(newpassword)) {
      error();
      return;
    }
    changePassword(userid, currentpassword, newpassword);


  }
  else if (strcmp(command, "useradd") == 0) {
    //addUser();
    char *userid, *password, *newprivilege, *username;
    int privilegenumber;
    userid = strtok(nullptr, " ");
    password = strtok(nullptr, " ");
    newprivilege = strtok(nullptr, " ");
    username = strtok(nullptr, " ");
    if (username == nullptr || !legal_basicdata(userid) || !legal_basicdata(password)
      || !legal_name(username) || strlen(newprivilege) > 1 || !isdigit(newprivilege[0])) {
      error();
      return;
    }
    privilegenumber = std::atoi(newprivilege);
    addUser(userid, password, privilegenumber, username);
  }


  else if (strcmp(command, "delete") == 0) {
    char *userid = strtok(nullptr, " ");
    if (userid == nullptr) {
      error();
      return;
    }
    deleteUser(userid);


  }
  else if (strcmp(command, "show") == 0) {
    char *type = strtok(nullptr, " ="), *message = strtok(nullptr, " ");
    if (type != nullptr && strcmp(type, "finance") == 0) {
      if (privilege[signedins] < 7) {
        error();
        return;
      }
      char *count = strtok(nullptr, " ");
      if (strcmp(count, "0") == 0) {
        std::cout << '\n';
      } else {
        showFinance(count);
      }
    } else {
      if (privilege[signedins] < 1) {
        error();
        return;
      }
      showBooks(type, message);
    }
  }


  else if (strcmp(command, "buy") == 0) {
    char *isbn = strtok(nullptr, " "), *quantity = strtok(nullptr, " ");
    if (quantity == nullptr || strtok(nullptr, " ") != nullptr
      || !legal_number(quantity) || privilege[signedins] < 1) {
      error();
      return;
    }
    int quantitynumber = std::atoi(quantity);
    buyBook(isbn, quantitynumber);
  }


  else if (strcmp(command, "select") == 0) {
    char *isbn = strtok(nullptr, " ");
    if (strtok(nullptr, " ") != nullptr || privilege[signedins] < 3) {
      error();
      return;
    }
    select(isbn);
  }


  else if (strcmp(command, "modify") == 0) {
    if (strcmp(selectbook[signedins], "") == 0 || privilege[signedins] < 3) {
      error();
      return;
    }
    char *type[6], *message[6];
    int i = 0;
    type[0] = strtok(nullptr, "=");
    message[0] = strtok(nullptr, " ");
    while (type[i] != nullptr) {
      if (i == 5) {
        error();
        return;
      }
      if(message[i] == nullptr) {
        error();
        return;
      }
      if (strcmp(type[i], "-ISBN") == 0) {
        if (!legal_ISBN(message[i]) || !book.find(message[i]).empty()) {
          error();
          return;
        }
      } else if (strcmp(type[i], "-name") == 0 || strcmp(type[i], "-author") == 0 || strcmp(type[i], "-keyword") == 0) {
        if (strlen(message[i]) < 3 || message[i][0] != '"' || message[i][strlen(message[i]) - 1] != '"') {
          error();
          return;
        } else {
          for (int k = 1; k < strlen(message[i]) - 1; ++k) {
            message[i][k - 1] =  message[i][k];
          }
          message[i][strlen(message[i]) - 2] = '\0';
        }
        if (!legal_bookdata(message[i])) {
          error();
          return;
        }
      } else if (strcmp(type[i], "-price") == 0) {
        if (!legal_price(message[i])) {
          error();
          return;
        }
      } else {
        error();
        return;
      }
      ++i;
      type[i] = strtok(nullptr, "=");
      message[i] = strtok(nullptr, " ");
    }
    while (i > 0) {
      --i;
      modifyBookInfo(type[i], message[i]);
    }
  }


  else if (strcmp(command, "import") == 0) {
    if (strcmp(selectbook[signedins], "") == 0 || privilege[signedins] < 3) {
      error();
      return;
    }
    char *quantity = strtok(nullptr, " "), *totalcost = strtok(nullptr, " ");
    if (strtok(nullptr, " ") != nullptr || !legal_number(quantity)) {
      error();
      return;
    }
    importBooks(quantity, totalcost);
  }


  else if (strcmp(command, "show") == 0) {
    //log
    char *count = strtok(nullptr, " ");
    if (strcmp(count, "finance") != 0) {
      error();
      return;
    }
    count = strtok(nullptr, "");
    if (strcmp(count, "0")) {
      std::cout << '\n';
    } else {
      showFinance(count);
    }
  }


  else if (strcmp(command, "log") == 0) {
    if (strtok(nullptr, " ") != nullptr) {
      error();
      return;
    }
    logAction();
  }


  else if (strcmp(command, "report") == 0) {
    if (command == "finance") {
      if (strtok(nullptr, " ") != nullptr) {
        error();
        return;
      }
      reportFinance();
    } else if (command == "employee") {
      if (strtok(nullptr, " ") != nullptr) {
        error();
        return;
      }
      reportEmployee();
    } else {
      error();
    }
  } else {
    error();
  }
}

int main() {
  finance.open();
  privilege[0] = 0;
  strcpy(s, "root");
  find_accounts = account.find(s);
  if (find_accounts.empty()) {
    Account root;
    strcpy(root.password,"sjtu");
    root.privilege = 7;
    strcpy(root.username, "Super Admin");
    account.insert(s, "", root);
  }
  std::string command;
  char *line;
  while (true) {
    std::getline(std::cin, command);
    if (command == "quit" || command == "exit") {
      finance.write_info(financenumber,1);
      finance.close();
      return 0;
    }
    line = const_cast<char *> (command.c_str());
    processCommand(line);
  }
}