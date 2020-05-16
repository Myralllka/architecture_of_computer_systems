//
// Created by solia on 4/5/20.
//

#include "../../includes/counting/parallel_program.h"
#include <vector>
#include <thread>
#include <boost/locale.hpp>
#include "../../includes/archivation/archive_t.h"
#include "../../includes/code_control.h"

#define MAP_PACKET_SIZE 10000
#define MAX_DATA_QUEUE_SIZE_PER_THREAD 10
#define MAX_MAP_QUEUE_SIZE 100
#define UNARCHIVE_THREADS 1

#include "tbb/concurrent_queue.h"

namespace ba = boost::locale::boundary;

void merge_maps(tbb::concurrent_bounded_queue<std::map<std::string, size_t>, tbb::cache_aligned_allocator<std::map<std::string, size_t>>> &queue, uint8_t num_of_threads) {
    for (; num_of_threads > 1; num_of_threads--) {
        std::map<std::string, size_t> map1, map2;
        queue.pop(map1);
        queue.pop(map2);
        for (auto &element:map1) {
            map2[element.first] += map1[element.first];
        }
        queue.push(std::move(map2));
    }
}

static void counting(tbb::concurrent_queue<file_packet, tbb::cache_aligned_allocator<file_packet>> *file_q,
                     tbb::concurrent_bounded_queue<std::map<std::string, size_t>, tbb::cache_aligned_allocator<std::map<std::string, size_t>>> *map_q) {
    file_packet packet;
//    tbb::concurrent_bounded_queue<std::string, tbb::cache_aligned_allocator<std::string>> data_q;
    std::map<std::string, size_t> map_of_words{};
    std::string content;
//    data_q.set_capacity(MAX_DATA_QUEUE_SIZE_PER_THREAD);
    while (file_q->try_pop(packet)) {
        if (packet.archived) {
            archive_t tmp_archive{std::move(packet.content)};
            tmp_archive.extract_all(&file_q);
        } else {
            content = boost::locale::to_lower(boost::locale::fold_case(boost::locale::normalize(packet.content)));
            ba::ssegment_index map(ba::word, content.begin(), content.end());
            map.rule(ba::word_any);
            for (auto it = map.begin(), e = map.end(); it != e; ++it)
                map_of_words[*it] += 1;
            content.clear();
        }
    }
    map_q->push(map_of_words);
}

void parallel_count(tbb::concurrent_queue<file_packet, tbb::cache_aligned_allocator<file_packet>> *loader_queue,
                    const std::string &output_filename_a, const std::string &output_filename_n,
                    const uint8_t number_of_threads) {
    std::vector<std::thread> vector_of_threads{};
    tbb::concurrent_bounded_queue<std::map<std::string, size_t>, tbb::cache_aligned_allocator<std::map<std::string, size_t>>> map_queue;
    std::map<std::string, size_t> result;
//    map_queue.set_capacity(MAX_MAP_QUEUE_SIZE);
    /////////////////////////// UNARCHIVE & COUNT ///////////////////
    for (uint8_t i = 0; i < UNARCHIVE_THREADS; i++)
        vector_of_threads.emplace_back(counting, loader_queue, &map_queue);
    /////////////////////////////////////////////////////////////////
    merge_maps(map_queue, number_of_threads);

    for (auto &t: vector_of_threads)
        t.join();
    /////////////////////////////////////////////////////////////////
    map_queue.pop(result);
    dump_map_to_files(result, output_filename_a, output_filename_n);
}

