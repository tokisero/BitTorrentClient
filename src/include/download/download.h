#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <vector>
#include "tracker/tracker.h"
#include <queue>
#include "download/worker.h"
#include <string>

class download {

private:
	std::vector<std::vector<bool>> received;
	std::vector<std::vector<bool>> is_in_queue;
	const std::vector<peer>& peers;
	torrent& t;

	writer w;
	speed s;

	int received_count;
	int total_blocks;
	std::string download_dir;

	int active_peers = 0;

public:
	download(const std::vector<peer>& peers, torrent& t);
	void add_received(int piece, int block, buffer b);
	void set_download_directory(const std::string& dir);
	
	void start();
	double completed();
	bool is_done();
	bool is_cancelled() const { return s.is_cancelled(); }

	struct job {
		int index;
		int begin;
		int length;

		int requested;

		job(int i, int b, int l): index(i), begin(b), length(l), requested(0) {}
		job() {}
		bool operator<(const job& other) const {return this->requested > other.requested;}
	};

	void push_job(job j);
	job pop_job();

	static const int BLOCK_SIZE = (1<<14);

	int get_active_peers() const { return active_peers; }
	void increment_active_peers() { active_peers++; }
	void decrement_active_peers() { active_peers--; }

private:
	std::priority_queue<job> jobs;
};

#endif // DOWNLOAD_H