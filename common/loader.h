#ifndef LOADER_H
#define LOADER_H

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

static std::vector<std::string> load_lexicon(const char *path) {
  std::vector<std::string> res;

  std::ifstream fs(path);
  std::string line;
  while (std::getline(fs, line)) {
    auto tab_i = line.find('\t');
    if (tab_i != std::string::npos) {
      res.push_back(line.substr(0, tab_i));
    } else {
      res.push_back(line);
    }
  }

  return res;
}

#endif // LOADER_H