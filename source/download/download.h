#pragma once

#include <vector>
#include <queue>
#include <string>
#include "parsing/torrent.h"
#include "download/worker.h"
#include "tracker/tracker.h"
#include "parsing/buffer.h"

class download {
public:
    static const int BLOCK_SIZE = 16384;

    struct job {
        int index;
        int begin;
        int length;
        int requested;

        job(int i, int b, int l): index(i), begin(b), length(l), requested(0) {}
    };

    download(const std::vector<peer>& peers, torrent& t);
    void start();
    void add_received(int piece, int block, buffer piece_data);
    bool is_done();
    double completed();
    void push_job(job j);
    job pop_job();
    void set_download_directory(const std::string& dir);
    
    // Методы для работы с активными сидами
    int get_active_peers() const { return active_peers; }
    void increment_active_peers() { active_peers++; }
    void decrement_active_peers() { active_peers--; }

private:
    torrent& t;
    std::vector<peer> peers;
    std::vector<std::vector<bool>> received;
    std::vector<std::vector<bool>> is_in_queue;
    std::priority_queue<job> jobs;
    int received_count;
    int total_blocks;
    writer w;
    speed s;
    std::string download_dir;
    int active_peers = 0;
}; 