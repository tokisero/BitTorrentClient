#include "parsing/torrent.h"
#include <iostream>
#include "download/peer_id.h"
#include "tracker/tracker.h"
#include "download/download.h"
#include <filesystem>
#include <string>

using namespace std;

void print_torrent_info(const torrent& t) {
	cout << "\nИнформация о торренте:" << endl;
	cout << "Название: " << t.name << endl;
	cout << "Размер: " << (t.length / (1024.0 * 1024.0)) << " MB" << endl;
	cout << "Размер части: " << (t.piece_length / 1024.0) << " KB" << endl;
	cout << "Количество частей: " << t.pieces << endl;
	cout << "URL трекера: " << t.url.host << ":" << t.url.port << endl;
}

bool confirm_download() {
	string response;
	while (true) {
		cout << "\nНачать загрузку? (y/n): ";
		cin >> response;
		if (response == "y" || response == "Y") return true;
		if (response == "n" || response == "N") return false;
		cout << "Пожалуйста, введите 'y' или 'n'" << endl;
	}
}

int main(int argc, const char** argv) {
	if(argc < 2) {
		cout << "Использование: BitTorrent <torrent_file> [директория_скачивания] [-info]" << endl;
		return 0;
	}

	string torrent_path = argv[1];
	string download_dir = ".";
	bool show_info = false;

	// Обработка аргументов
	for (int i = 2; i < argc; i++) {
		string arg = argv[i];
		if (arg == "-info") {
			show_info = true;
		} else {
			download_dir = arg;
		}
	}

	// Проверка существования директории
	if (!filesystem::exists(download_dir)) {
		cout << "Ошибка: директория " << download_dir << " не существует" << endl;
		return 1;
	}

	srand(time(NULL));
	peer_id::generate();

	torrent t(torrent_path);

	if (show_info) {
		print_torrent_info(t);
		if (!confirm_download()) {
			cout << "Загрузка отменена пользователем" << endl;
			return 0;
		}
	}

	cout << "Получение пиров от трекера... (Это может занять некоторое время)" << endl;
	vector<peer> v = tracker::get_peers(t);
	cout << "Получено " << v.size() << " пиров" << endl;

	download d(v, t);
	d.set_download_directory(download_dir);
	d.start();

	cout << endl << "Загрузка успешно завершена!" << endl;

	return 0;
}
