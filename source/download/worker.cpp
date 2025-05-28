#include "download/worker.h"
#include "download/download.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

void writer::start() {

	t = thread([this](){

		while(!finito) {

			unique_lock<mutex> lock(mx);
			cv.wait(lock, [this](){
				return !q.empty() || finito;
			});

			vector<job> vec;

			while(!q.empty()) {

				job j = q.front();
				q.pop();

				vec.push_back(move(j));
			}

			lock.unlock();

			for(job& j:vec) {

				out.seekp(j.offset, ios::beg);
				out.write(reinterpret_cast<char*>(j.b.data()), j.b.size());
			}
		}
	});
}

void writer::stop() {

	finito = true;
	cv.notify_one();
	t.join();
}

void writer::add(buffer& b, int offset) {

	unique_lock<mutex> lock(mx);
	q.push(job(move(b), offset));
	lock.unlock();
	cv.notify_one();
}

// Функция для настройки неблокирующего ввода
void set_nonblocking() {
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	ttystate.c_lflag &= ~(ICANON | ECHO);
	ttystate.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void speed::check_cancellation() {
	fd_set readfds;
	struct timeval tv;
	
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
		char c;
		if (read(STDIN_FILENO, &c, 1) == 1) {
			if (c == 'q' || c == 'Q') {
				cancelled = true;
			}
		}
	}
}

void speed::start() {
	set_nonblocking(); // Устанавливаем неблокирующий режим при старте
	
	cout << "Wait for the download to complete ..." << endl;
	draw(0.0, 0);

	t = thread([this]() {
		while(!finito) {
			if(mod % 5 == 0) {
				unsigned int v = bytes.exchange(0);
				unsigned int speed = 1000 * v / delay;
				prev = e_factor * speed + (1.0 - e_factor) * prev;
			}
			mod = (mod + 1) % 5;
			
			double progress = total > 0 ? (double)received / total : 0.0;
			draw(progress, prev);
			this_thread::sleep_for(chrono::milliseconds(delay / 5));
		}
		draw(total > 0 ? (double)received / total : 0.0, 0.0);
	});
}

void speed::stop() {
	finito = true;
	t.join();
	
	// Восстанавливаем нормальный режим терминала
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	ttystate.c_lflag |= ICANON | ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void speed::add(unsigned int b) {

	bytes += b;
	received += b;
}

void speed::draw(double progress, unsigned int speed) {
	static const int bar_width = 50;
	static const int line_width = 100; // Примерная ширина всей строки

	// Очищаем текущую строку
	cout << "\r" << string(line_width, ' ') << "\r";

	// Рисуем прогресс-бар
	cout << "[";
	int pos = bar_width * progress;
	for (int i = 0; i < bar_width; ++i) {
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(progress * 100.0) << " %   ";
	human_readable(speed);
	if (d) {
		cout << " | Активных сидов: " << d->get_active_peers();
	}
	cout << " | Для отмены и выхода нажмите q";
	cout.flush();
	
	check_cancellation();
}

void speed::set_total(long long t) {

	total = t;
}

void speed::human_readable(unsigned int b) {

	if (b < 1024) cout<<b<<" B/s";
	else if (b < 1024 * 1024) cout<<(double)b/1024<<" kB/s";
	else if (b < 1024 * 1024 * 1024) cout<<(double)b/(1024*1024)<<" Mb/s";
	else cout<<(double)b/(1024*1024*1024)<<" Gb/s";
}




