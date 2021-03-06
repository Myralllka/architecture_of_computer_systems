//
// Created by fenix on 4/6/20.
//
#include <boost/filesystem.hpp>
#include <filesystem>
#include <iostream>

#include "../../includes/files/file_interface.h"
#include "../../includes/counting/map_helpers.h"

void dump_map_to_files(const std::map<std::string, size_t> &map_of_words, const std::string &output_filename_a,
                       const std::string &output_filename_n) {
    std::ofstream outfile_alpha;
    std::ofstream outfile_number;
    std::multimap<size_t, std::string> multimap_of_words;
    outfile_alpha.open(output_filename_a);
    outfile_number.open(output_filename_n);

    for (auto &pair:map_of_words) {
        outfile_alpha << pair.first << "\t" << pair.second << std::endl;
    }

    flip_map(multimap_of_words, map_of_words);

    auto it = multimap_of_words.rbegin();
    while (it != multimap_of_words.rend()) {
        outfile_number << (*it).second << "\t" << (*it).first << std::endl;
        ++it;
    }

    outfile_alpha.close();
    outfile_number.close();
}


std::string read_binary_file(const std::string &filename) {
    if (!boost::filesystem::exists(filename)) {
        std::cerr << "Error: File '" << filename << "' do not exist!" << std::endl;
        return std::string{};
    }
    std::ifstream raw_file(filename, std::ios::binary);
    std::ostringstream ss{};
    ss << raw_file.rdbuf();
    return ss.str();
}

// WARNING: do not list empty files!!!
void list_all_files_from(const std::string &dir_path, std::vector<std::string> &res) {
    for (const auto &file : std::filesystem::recursive_directory_iterator(dir_path)) {
        if (std::filesystem::is_regular_file(file) && !std::filesystem::is_empty(file)) {
            res.emplace_back(file.path().string());
        } else {
            std::cerr << "Warning directory contain not valid or empty files! They are not counted!" << std::endl;
        }
    }
}
