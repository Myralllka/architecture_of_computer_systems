//
// Created by solia on 4/5/20.
//

#include "../includes/parallel_program.h"
#include "../includes/linear_extractor.h"
#include "../includes/merge_maps.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <boost/locale.hpp>
#include <thread>
#include <cmath>
#include <set>
#include <algorithm>
#include <functional>

void count_words(std::vector<std::string> &data, const int start_position, const int end_position,
                 std::map<std::string, int> &map_of_words) {
    namespace bl =  boost::locale;
    std::string word;

    for (auto cur = data.begin() + start_position; cur != data.begin() + end_position; cur++) {
        std::string element = *cur;
        element = bl::to_lower(bl::fold_case(bl::normalize(element)));
        element.erase(std::remove_if(element.begin(), element.end(),
                                     [](const unsigned &c) { return !isspace(c) && !isalpha(c); }), element.end());
        for (auto &chr : element) {
            if (isalpha(chr))
                word += tolower(chr);
            else if (isspace(chr)) {
                auto itr = map_of_words.find(word);
                if (itr != map_of_words.end()) {
                    map_of_words[word] += 1;
                } else {
                    map_of_words[word] = 1;
                }
                word.clear();
            }
        }
    }

    for (auto &pair : map_of_words) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

void parallel_count(const std::string &input_filename, const std::string &output_filename_a,
                    const std::string &output_filename_n, const uint8_t num_threads) {
    // read entire binary archive into the buffer
    std::ifstream raw_file(input_filename, std::ios::binary);
    std::vector<std::string> data;

    // read buffer
    auto buffer = [&raw_file] {
        std::ostringstream ss{};
        ss << raw_file.rdbuf();
        return ss.str();
    }();
    extract_to_memory(buffer, &data);

    size_t data_portion_len = std::ceil(data.size() / num_threads);

    // parallel counting
    std::vector<std::map<std::string, int>> vector_of_maps;
    std::vector<std::thread> vector_of_threads(num_threads);
    for (uint8_t i = 0; i < num_threads; i++) {
        std::map<std::string, int> map;
        vector_of_maps.push_back(map);

        vector_of_threads.emplace_back(count_words, std::ref(data), i * data_portion_len,
                                                std::min((i + 1) * data_portion_len, data.size()),
                                                std::ref(vector_of_maps[i]));
    }

    for (auto &t: vector_of_threads) {
        t.join();
    }

    std::map<std::string, int> map_of_all_words;
    for (auto &map:vector_of_maps) {
        map_of_all_words = merge_maps(map_of_all_words, map);
    }
    vector_of_maps.clear();

    std::ofstream outfile_by_a(output_filename_a);
    std::ofstream outfile_by_n(output_filename_n);

    typedef std::function<bool(std::pair<std::string, int>, std::pair<std::string, int>)> Comparator;
    Comparator comparator_by_a =
            [](std::pair<std::string, int> el1 ,std::pair<std::string, int> el2)
            {
                return el1.first < el2.first;
            };
    Comparator comparator_by_n =
            [](std::pair<std::string, int> el1 ,std::pair<std::string, int> el2)
            {
                return el1.second < el2.second;
            };

    std::set<std::pair<std::string, int>, Comparator> set_of_words_by_a(
            map_of_all_words.begin(), map_of_all_words.end(), comparator_by_a);
    for (std::pair<std::string, int> element : set_of_words_by_a)
        outfile_by_a << element.first << ": " << element.second << std::endl;
    outfile_by_a.close();

    std::set<std::pair<std::string, int>, Comparator> set_of_words_by_n(
            map_of_all_words.begin(), map_of_all_words.end(), comparator_by_n);
    for (std::pair<std::string, int> element : set_of_words_by_n)
        outfile_by_n << element.first << ": " << element.second << std::endl;
    outfile_by_n.close();
}