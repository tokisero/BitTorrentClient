#ifndef WORKER_H
#define WORKER_H

#include <string>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "parsing/buffer.h"
#include <string>
#include <filesystem>

class download; // forward declaration

class speed {

private:
	std::atomic<unsigned int> bytes, received;
	std::atomic<bool> finito;
	std::thread t;
	
	unsigned int prev;
	long long total;
	const double e_factor = 0.6321205588; // 1 - 1/e
	const int delay = 500;
	int mod;

	void human_readable(unsigned int b); 

    download* d = nullptr; // ссылка на download
    std::atomic<bool> cancelled{false}; // флаг отмены

public:
	speed(): total(0), bytes(0), finito(false), prev(0), mod(0), d(nullptr) {}
	void set_total(long long t);
	void start();
	void stop();
	void add(unsigned int b);
	void draw(double progress, unsigned int speed);
    void set_download(download& d) { this->d = &d; }
    bool is_cancelled() const { return cancelled; }
    void check_cancellation(); // новый метод для проверки отмены
};

class writer {

private:
	std::ofstream out;
	std::atomic<bool> finito;
	std::mutex mx;
	std::condition_variable cv;
	std::thread t;
	std::string download_dir;
	std::string filename;

	struct job {
		buffer b;
		int offset;
		job(buffer b, int offset): b(b), offset(offset) {};
	};

	std::queue<job> q;

public:
	writer(const std::string& filename, const std::string& dir = "."): 
		download_dir(dir), filename(filename), finito(false) {
		open_file();
	}

	void set_directory(const std::string& dir) {
		std::lock_guard<std::mutex> lock(mx);
		download_dir = dir;
		open_file();
	}

private:
	void open_file() {
		if (out.is_open()) {
			out.close();
		}
		std::filesystem::path filepath = std::filesystem::path(download_dir) / filename;
		out.open(filepath, std::ios::binary);
	}

public:
	void start();
	void stop();
	void add(buffer& b, int offset);
};

#endif // WORKER_H
