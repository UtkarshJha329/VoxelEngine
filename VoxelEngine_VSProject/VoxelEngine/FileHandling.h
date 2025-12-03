#pragma once

#include <fstream>
#include <string>

#include <iostream>

struct ContentsOfFile {
public:
    int parsedState = -1;
    std::string contents;
};

ContentsOfFile ReturnContentsOfFile(std::string filePath) {

    ContentsOfFile returnContents = { 0, "" };
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open " << filePath << std::endl;
        returnContents.parsedState = -1;
        return returnContents; // Indicate an error
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        //std::cout << "Read line: " << line << std::endl;
        returnContents.contents += line;
        returnContents.contents += "\n";
    }

    inputFile.close();

    returnContents.parsedState = 1;
    return returnContents;
}
