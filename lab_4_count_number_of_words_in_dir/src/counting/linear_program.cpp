//
// Created by myralllka on 3/25/20.
//
#include "../../includes/files/file_interface.h"
#include "../../includes/counting/word_count.h"
#include "../../includes/archivation/archive_t.h"
#include <map>
#include <string>
#include <algorithm>
#include <boost/locale.hpp>
#include <deque>

void linear_count(const std::vector<std::string> &file_names, const std::string &output_filename_a,
                  const std::string &output_filename_n) {
    std::map<std::string, size_t> map_of_words;
    class : public std::deque<file_packet> {
    public:
        void emplace_front_force(file_packet&& data) { // Name do not match with real (does 'emplace_back')
            std::deque<file_packet>::emplace_back(data);
        }
    } archive_buf{};
    std::deque<std::string> file_buf{};

    for (const std::string &file_n : file_names) {
        read_input_file_gen(file_n, &archive_buf); // generic method to load all files
        while (!archive_buf.empty()) {
            if (archive_buf.front().archived) {
                archive_t::extract_to(std::move(archive_buf.front().content), &file_buf, &archive_buf);
            } else {
                file_buf.emplace_back(std::move(archive_buf.front().content));
            }
            archive_buf.pop_front();
        }

        while (!file_buf.empty()){
            std::string content{std::move(file_buf.front())};
            file_buf.pop_front();

            /////////////////////// NORMALIZE CONTENT /////////////////////////
            content = boost::locale::to_lower(boost::locale::fold_case(boost::locale::normalize(content)));
            content.erase(std::remove_if(content.begin(), content.end(),
                                         [](const unsigned &c) { return !isspace(c) && !isalpha(c); }), content.end());
            ///////////////////////////////////////////////////////////////////
            fast_count_words(content, &map_of_words);
        }
    }
    dump_map_to_files(map_of_words, output_filename_a, output_filename_n);
}