//
// Created by myralllka on 3/25/20.
//

#include <map>
#include <string>
#include <algorithm>
#include <boost/locale.hpp>
#include <deque>
#include "../../includes/files/file_interface.h"
#include "../../includes/archivation/archive_t.h"
#include "../../includes/counting/linear_program.h"

namespace ba = boost::locale::boundary;

//void linear_count(const std::vector<std::string> &file_names,
//                  tbb::concurrent_bounded_queue<std::map<std::string, size_t>> &map_q) {
void linear_count(const std::vector<std::string> &file_names, std::map<std::string, size_t> &map_q) {
    std::map<std::string, size_t> map_of_words;
    struct {
        file_packet data{};

        void push(file_packet &&source) {
            data = std::move(source);
            is_empty = false;
        }

        [[nodiscard]] bool empty() const { return is_empty; }

        void clear() {
            data.content.clear();
            is_empty = true;
        }

    private:
        bool is_empty = true;
    } archive_buf{};

    std::deque<std::string> file_buf{};

    for (const std::string &file_n : file_names) {
        read_input_file_gen(file_n, archive_buf); // generic method to load all files
        if (!archive_buf.empty()) {
            if (archive_buf.data.archived) {
                archive_t::extract_to(std::move(archive_buf.data.content), file_buf);
            } else {
                file_buf.emplace_back(std::move(archive_buf.data.content));
            }
            archive_buf.clear();
        }

        while (!file_buf.empty()) {
            std::string content{std::move(file_buf.front())};
            file_buf.pop_front();
            content = boost::locale::to_lower(boost::locale::fold_case(boost::locale::normalize(content)));
            ba::ssegment_index map(ba::word, content.begin(), content.end());
            map.rule(ba::word_letters);
            for (auto it = map.begin(), e = map.end(); it != e; ++it)
                ++map_of_words[*it];
            content.clear();
        }
    }
//    map_q.push(std::move(map_of_words));
    map_q = std::move(map_of_words);
}
