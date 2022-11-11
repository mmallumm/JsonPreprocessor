#include <fstream>
#include <iostream>
#include <map>
#include <vector>

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
  for (iter; iter != line.end(); iter++) {
    if (*iter == '$') {
      step[0] = true;
      break;
    }
  }
  iter++;
  if (iter != line.end() && *iter == '{') {
    step[1] = true;
  }
  for (iter; iter != line.end(); iter++) {
    if (*iter == '}') {
      step[2] = true;
      break;
    }
  }
  return (step[0] && step[1] && step[2]);
}

bool iterIsVarUsageBegin_(const std::string& line,
                          std::string::const_iterator iter) {
  if (iter != line.end() && iter + 1 != line.end()) {
    return (*iter == '$' && *(iter + 1) == '{') ? true : false;
  }
  return false;
}

bool iterIsVarUsageEnd_(const std::string& line,
                        std::string::const_iterator iter) {
  if (iter != line.end()) {
    return (*iter == '}') ? true : false;
  }
  return true;
}

std::string parseVarUsage_(const std::string& line, VarDictionary& dictionary) {
  std::string parsedLine;
  for (auto currentIter = line.begin(); currentIter != line.end();
       ++currentIter) {
    if (iterIsVarUsageBegin_(line, currentIter)) {
      auto endVarIter = currentIter;
      while (!iterIsVarUsageEnd_(line, endVarIter)) {
        endVarIter++;
      }
      if (endVarIter != line.end()) {
        std::string varName((currentIter + 2), (endVarIter));
        std::string varValue = dictionary[varName];
        parsedLine += varValue;
        currentIter = endVarIter;
      } else {
        parsedLine.push_back(*currentIter);  
      }
    } else {
      parsedLine.push_back(*currentIter);
    }
  }
  return parsedLine;
}

std::string parseComment_(const std::string& line) {
  std::string parsedLine;
  if (!lineIsComment_(line)) {
    parsedLine = line;
  }
  return parsedLine;
}

std::string parseJson_(std::ifstream& jsonFile, VarDictionary& dictionary) {
  std::string json;
  std::string buffer;
  while (std::getline(jsonFile, buffer)) {
    std::string localBuffer = parseComment_(buffer);
    localBuffer = parseVarUsage_(localBuffer, dictionary);
    json += !localBuffer.empty() ? (localBuffer + '\n') : localBuffer;
  }
  return json;
}

void parseVarDeclarationFromLine_(const std::string& buffer,
                                  VarDictionary& dictionary) {
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
    parseVarDeclarationFromLine_(buffer, dictionary);
  }

  return dictionary;
}

void parseGlobalVars_(VarDictionary& dictionary) {
  for (char** vars = environ; *vars != 0; vars++) {
    char* thisVar = *vars;
    std::string buffer(thisVar);
    parseVarDeclarationFromLine_(buffer, dictionary);
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
    json = parseJson_(jsonFile, vars);
  }
  return json;
}

int main() {
  std::string filer = readJson("../conf.json");
  std::cout << filer << std::endl;

  // char** s = environ;

  // for (; *s; s++) {
  //   printf("%s\n", *s);
  // }
}