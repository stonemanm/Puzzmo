#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

int NumConverter(absl::string_view s) {
  int value = 0;
  for (int l = 1, a = 36; l < s.length(); value += a, ++l, a *= 36);
  for (int j = s.length() - 1, a = 1; j >= 0; --j, a *= 36) {
    int o = s[j] - '0';
    if (o > 10) o -= 7;
    value += o * a;
  }
  // LOG(INFO) << "Converted " << s << " to " << value;
  return value;
}

struct PathThroughNode {
  absl::string_view text;
  int next_index;
  bool completes_word = true;

  PathThroughNode(absl::string_view input, int current_index,
                  const absl::flat_hash_map<std::string, int>& syms) {
    auto it = std::find_if(input.begin(), input.end(),
                           [](char c) { return !std::islower(c); });
    text = input.substr(0, it - input.begin());
    if (it != input.end()) {
      completes_word = false;
      absl::string_view numstr =
          input.substr(it - input.begin(), input.end() - it);
      int num = NumConverter(numstr);
      if (num < syms.size()) {
        next_index = syms.at(numstr);
      } else {
        next_index = current_index + num + 1 - syms.size();
      }
    }
  }
};

void DFS(absl::string_view prefix,
         const std::vector<std::vector<PathThroughNode>>& nodes, int node,
         std::ofstream& file) {
  // LOG(ERROR) << node;
  for (const auto& path_through_node : nodes[node]) {
    if (path_through_node.completes_word) {
      std::string word = absl::StrCat(prefix, path_through_node.text);
      file << word << std::endl;
    } else {
      std::string new_prefix = absl::StrCat(prefix, path_through_node.text);
      DFS(new_prefix, nodes, path_through_node.next_index, file);
    }
  }
}

int main(int argc, const char* argv[]) {
  std::ifstream infile("data/in.txt");
  if (!infile.is_open()) {
    LOG(ERROR) << "infile isn't open";
    return 1;
  }
  std::ofstream outfile("data/out.txt", std::ios_base::app);
  if (!outfile.is_open()) {
    LOG(ERROR) << "outfile isn't open";
    return 1;
  }

  std::string input;
  while (std::getline(infile, input)) {
    for (int i = input.size() - 1; i > 0; --i) {
      if (std::islower(input[i]) && !std::islower(input[i - 1]) &&
          input[i - 1] != ',' && input[i - 1] != ';' && input[i - 1] != ':') {
        input.insert(input.begin() + i, ',');
      }
    }

    // Split the string on semicolons
    std::vector<absl::string_view> nodes_as_string_views =
        absl::StrSplit(input, ';');
    int size = nodes_as_string_views.size();

    absl::flat_hash_map<std::string, int> syms;
    while (std::find(nodes_as_string_views[0].begin(),
                     nodes_as_string_views[0].end(),
                     ':') != nodes_as_string_views[0].end()) {
      // LOG(ERROR) << nodes_as_string_views[0];
      std::pair<std::string, std::string> split =
          absl::StrSplit(nodes_as_string_views[0], ":");
      syms[split.first] = NumConverter(split.second);
      --size;
      nodes_as_string_views.erase(nodes_as_string_views.begin());
    }

    std::vector<std::vector<PathThroughNode>> nodes(size);
    for (int i = 0; i < size; ++i) {
      for (const auto& str : absl::StrSplit(nodes_as_string_views[i], ',')) {
        nodes[i].push_back(PathThroughNode(str, i, syms));
      }
    }

    DFS("", nodes, 0, outfile);
  }

  infile.close();
  outfile.close();
  return 0;
}