#pragma once
#include"solver.hpp"
#include<fstream>
#include<filesystem>
struct Benchmark {
	vector<array<uint8_t, 81>> puzzles;
	Benchmark(const vector<string>& paths){
		const string dir{ (std::filesystem::current_path()/"").string()};
		for (const string& path : paths) {
			std::ifstream f{ dir + path };
			if (f.is_open()) {
				string line{};
				size_t count{ 0 };
				while (std::getline(f, line)) {
					array<uint8_t, 81> arr{};
					for (int i = 0; i < line.length(); ++i) {
						switch (line[i]) {
						case '.':
							arr[i] = 0;
							break;
						default:
							arr[i] = line[i] - '0';
							break;
						}
					}
					puzzles.push_back(arr);
				}
			}
			else {
				cout << "unable to open file " << path << "\n";
			}
		}
	}
	void run() const {
		std::chrono::steady_clock::time_point begin{ std::chrono::steady_clock::now() };
		for (int i = 0; i < puzzles.size();++i) {
			/*
			if (i && i % (puzzles.size() / 1000) == 0) {
				std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };
				cout << "finished " << i << " puzzles\n";
				cout << i / double(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * 1000000.0 << " pps\n";
			}
			*/
			//cout << "starting #" << i << " of "<<puzzles.size() << "\n";
			Solver s{ puzzles[i]};
			s.root_solve();

			//string str{};
			//cout << "press enter to continue\n";
			//std::getline(std::cin, str);
		}
		std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };

		cout << puzzles.size() / double(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * 1000000.0 << " pps\n";
		cout<<"solved all puzzles\n";
	}
};