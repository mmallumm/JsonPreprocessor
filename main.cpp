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

bool lineIsVarDeclaration_(const std::string& line) {
  for (auto& symbol : line) {
    if (symbol == '=') {
      return true;
    }
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

bool lineWithVarUsage_(const std::string& line) {
  bool step[3] = {false};
  auto iter = line.begin();
  for(iter; iter != line.end(); iter++) {
    if(*iter == '$') {
      step[0] = true;
      break;
    }
  }
  if(*(++iter) == '{') {
    step[1] = true;
  }
  for(iter; iter != line.end(); iter++) {
    if(*iter == '}') {
      step[2] = true;
      break;
    }
  }
  return (step[0] && step[1] && step[2]);
}

std::string parseVarUsage_(std::string& line) {
  //auto dollarIter
}

std::string& parseComment_(std::string& line) {
  if (lineIsComment_(line)) {
    line.clear();
  }
  return line;
}

std::string parseJson_(std::ifstream& jsonFile) {
  std::string json;
  std::string buffer;
  while (std::getline(jsonFile, buffer)) {
    json += parseComment_(buffer) + '\n';
  }
  return json;
}

void parseVarFromLine_(const std::string& buffer, VarDictionary& dictionary) {
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

  auto varLocation = dictionary.find(varName);
  if (varLocation == dictionary.end()) {
    dictionary.emplace(varName, varValue);
  } else {
    dictionary[varName] = varValue;
  }
}

VarDictionary& parseJsonVars_(std::ifstream& jsonFile,
                              VarDictionary& dictionary) {
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
    parseVarFromLine_(buffer, dictionary);
  }

  return dictionary;
}

void parseGlobalVars_(VarDictionary& dictionary) {
  for (char** vars = environ; *vars != 0; vars++) {
    char* thisVar = *vars;
    std::string buffer(thisVar);
    parseVarFromLine_(buffer, dictionary);
  }
}

}  // namespace

std::string readJson(std::string path) {
  std::ifstream jsonFile(path);
  std::string json;
  std::string buffer;
  VarDictionary vars;
  if (jsonFile.is_open()) {
    parseGlobalVars_(vars);
    parseJsonVars_(jsonFile, vars);
    json = parseJson_(jsonFile);
  }
  return json;
}

int main() {
  std::string filer = readJson("../conf.json");
  std::cout << filer << std::endl;

  std::string check = "${asdasasasd}";
  bool test = lineWithVarUsage_(check);

  // char** s = environ;

  // for (; *s; s++) {
  //   printf("%s\n", *s);
  // }
}