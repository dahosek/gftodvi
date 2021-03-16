#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <numeric>
#include "GFReader.h"

int main() {
    std:: cout << "Opening input stream" << std::endl;
//    std::ifstream input("/Users/d.a.hosek/CLionProjects/gftodvi/data/cmr10.2602gf");

//    std::cout << std::to_string(input.good()) << std::endl;

    std::cout << "Creating GFReader" << std::endl;
//    GFReader reader {std::shared_ptr<std::ifstream>(&input)};
    auto stream = std::make_shared<std::ifstream>("/Users/d.a.hosek/CLionProjects/gftodvi/data/fgeit10.2602gf");
    GFReader reader {stream};

    std::cout << "reading file" << std::endl;
    FontContext fontContext = reader.read_file();


    for (auto special: fontContext.specialContext.getSpecialList()) {
        std::cout << special.first
                  << "("
                  << std::accumulate(special.second.begin(), special.second.end(), std::string{},
                                     [](std::string r, int p){return std::move(r) + std::to_string(p) + ", ";})
                  << ")" << std::endl;
    }

    // Sort the characters into numerical order
    std::sort(fontContext.characters.begin(), fontContext.characters.end(),
              [](CharacterContext a, CharacterContext b){ return a.code > b.code;}
              );


    for (auto character: fontContext.characters) {
        std::cout << "[" << character.code << "]" << std::endl;
        auto specials = character.specialContext.getSpecialList();
        for (auto special: specials) {
            std::cout << special.first
                      << "("
                      << std::accumulate(special.second.begin(), special.second.end(), std::string{},
                                         [](std::string r, int p){return std::move(r) + std::to_string(p) + ", ";})
                      << ")" << std::endl;
        }
    }



    std::cout << "Finished reading file" << std::endl;

    return 0;
}
