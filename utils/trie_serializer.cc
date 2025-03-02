#include <fstream>
#include <iostream>
#include <string>

#include "absl/log/log.h"
#include "src/spelltower/trie.h"

using namespace puzzmo;
using ::puzzmo::spelltower::Trie;

int main(int argc, const char *argv[]) {
  std::ifstream infile("utils/in.txt");
  if (!infile.is_open()) {
    LOG(ERROR) << "infile isn't open";
    return 1;
  }
  std::ofstream outfile("utils/out.txt", std::ios_base::app);
  if (!outfile.is_open()) {
    LOG(ERROR) << "outfile isn't open";
    return 1;
  }

  Trie trie;
  std::string word;
  while (std::getline(infile, word)) trie.insert(word);
  infile.close();

  outfile << trie;
  outfile.close();

  return 0;
}