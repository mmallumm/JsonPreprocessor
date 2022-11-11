#include <fstream>
#include <iostream>
#include <map>

extern char** environ;

namespace {
typedef std::map<std::string, std::string> VarDictionary;

bool lineIsComment_(const std::string& line) {
  int counter = 0;
  for (auto& symbol : line) {
    if (symbol == ' ' || symbol == '\t') {
      continue;
    }
    if (symbol == '#' && counter == 0) {
      return true;
    }
    ++counter;
  }
  return false;
}

bool lineIsJsonHead_(const std::string& line) {
  int counter = 0;
  for (auto& symbol : line) {
    if (symbol == ' ' || symbol == '\t') {
      continue;
    }
    if (symbol == '{' && counter == 0) {
      return true;
    }
    ++counter;
  }
  return false;
}

bool lineIsVarDeclaration_(const std::string& line) {
  for (auto& symbol : line) {
    if (symbol == '=') {
      return true;
    }
  }
  return false;
}

std::string& parseComment_(std::string& line) {
  if (lineIsComment_(line)) {
    line.clear();
  }
  return line;
}

VarDictionary parseVars_(std::ifstream& jsonFile) {
  VarDictionary vars;
  std::string buffer;
  std::streampos oldPos = jsonFile.tellg();
  while (std::getline(jsonFile, buffer)) {
    if (lineIsJsonHead_(buffer)) {
      jsonFile.seekg(oldPos);
      break;
    }
    oldPos = jsonFile.tellg();
    if (!lineIsVarDeclaration_(buffer) || lineIsComment_(buffer)) {
      continue;
    }
    auto buffIter = buffer.begin();
    std::string varName;
    std::string varValue;
    while (*buffIter != '=') {
      if (*buffIter != ' ' || *buffIter != '\t' || *buffIter != '\n') {
        varName.push_back(*buffIter);
        buffIter++;
      }
    }
    buffIter++;
    while (buffIter != buffer.end()) {
      if (*buffIter != ' ' || *buffIter != '\t' || *buffIter != '\n') {
        varValue.push_back(*buffIter);
        buffIter++;
      }
    }

    vars.emplace(varName, varValue);
  }

  return vars;
}

void getGlobalVars(VarDictionary& dictionary) {}

}  // namespace

std::string readJson(std::string path) {
  std::string json;
  std::string buffer;
  std::ifstream stream(path);
  VarDictionary vars;
  if (stream.is_open()) {
    vars = parseVars_(stream);
    while (std::getline(stream, buffer)) {
      json += parseComment_(buffer) + '\n';
    }
  }
  return json;
}

int main() {
  // std::string filer = readJson("../conf.json");
  // std::cout << filer << std::endl;

  char** s = environ;

  for (; *s; s++) {
    printf("%s\n", *s);
  }
}