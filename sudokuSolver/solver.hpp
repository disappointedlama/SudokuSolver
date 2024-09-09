#pragma once
#include<iostream>
#include<array>
#include<vector>
#include<unordered_map>
#include<string>
#include<algorithm>
#include<chrono>
#include<thread>
#include<memory>
using std::array, std::vector, std::cout, std::endl, std::unordered_map, std::string, std::pair;

#define StepByStep false
#define PrintInfoOnSolution false

inline void print_bits(const uint32_t n) {
	for (int i = 0; i < 32; ++i) {
		if (n & (uint32_t(1) << (31 - i))) {
			cout << '1';
		}
		else {
			cout << '0';
		}
		if ((31 - i) % 3 == 0) cout << ".";
	}
	cout << "\n";
}


/*
 0  1  2    3  4  5    6  7  8
 9 10 11   12 13 14   15 16 17
18 19 20   21 22 23   24 25 26

27 28 29   30 31 32   33 34 35
36 37 38   39 40 41   42 43 44
45 46 47   48 49 50   51 52 53

54 55 56   57 58 59   60 61 62
63 64 65   66 67 68   69 70 71
72 73 74   75 76 77   78 79 80
*/

static constexpr array<size_t, 9> centers{ 10,13,16,37,40,43,64,67,70 };
static constexpr array<array<size_t, 9>, 9> boxes{ {
	{0,1,2,9,10,11,18,19,20},
	{3,4,5,12,13,14,21,22,23},
	{6,7,8,15,16,17,24,25,26},
	{27,28,29,36,37,38,45,46,47},
	{30,31,32,39,40,41,48,49,50},
	{33,34,35,42,43,44,51,52,53},
	{54,55,56,63,64,65,72,73,74},
	{57,58,59,66,67,68,75,76,77},
	{60,61,62,69,70,71,78,79,80}
}};
struct Solver {
	array<uint8_t, 81> board;
	array<uint32_t, 81> meta;
	uint64_t nodes;
	uint64_t contradictions;
	uint8_t total_placed;
	array<uint8_t, 10> placed_digits;
	array<array<uint16_t, 81>, 82> ruled_out_by_force;
	Solver(const array<uint8_t, 81>& board) :board{ board }, meta{}, nodes{ 0 }, contradictions{ 0 }, total_placed{ 0 }, placed_digits{}, ruled_out_by_force{} {
		for (int i = 0; i < 81; ++i) {
			meta[i] = 0;
		}
		for (int i = 0; i < 81;++i) {
			if (board[i]) {
				place_digit(i, board[i]);
			}
		}
#if StepByStep
		cout << *this;
#endif
	}
	constexpr bool forbidden_in_box(const int index, const short digit) const {
		return (1 << (3 * digit)) & meta[index];
	}
	constexpr bool forbidden_in_row(const int index, const short digit) const {
		return (1 << (3 * digit + 1)) & meta[index];
	}
	constexpr bool forbidden_in_col(const int index, const short digit) const {
		return (1 << (3 * digit + 2)) & meta[index];
	}
	inline void place_digit(const size_t square, const short digit) {
		++total_placed;
		++placed_digits[digit];
		ruled_out_by_force[total_placed] = array<uint16_t, 81>{};

		const uint32_t forbid_in_box_mask{ uint32_t(1) << (3 * digit) };
		const uint32_t forbid_in_row_mask{ uint32_t(1) << (3 * digit + 1) };
		const uint32_t forbid_in_col_mask{ uint32_t(1) << (3 * digit + 2) };

		const size_t x{ (square % 9) / 3 };
		const size_t y{ square / 27 };
		const size_t center{ 1 + 3 * x + 9 * (1 + 3 * y) };
		meta[center] |= forbid_in_box_mask;
		meta[center + 1] |= forbid_in_box_mask;
		meta[center - 1] |= forbid_in_box_mask;
		meta[center + 8] |= forbid_in_box_mask;
		meta[center - 8] |= forbid_in_box_mask;
		meta[center + 9] |= forbid_in_box_mask;
		meta[center - 9] |= forbid_in_box_mask;
		meta[center + 10] |= forbid_in_box_mask;
		meta[center - 10] |= forbid_in_box_mask;

		const size_t col_start{ square % 9 };
		meta[col_start] |= forbid_in_col_mask;
		meta[col_start + 9] |= forbid_in_col_mask;
		meta[col_start + 9 * 2] |= forbid_in_col_mask;
		meta[col_start + 9 * 3] |= forbid_in_col_mask;
		meta[col_start + 9 * 4] |= forbid_in_col_mask;
		meta[col_start + 9 * 5] |= forbid_in_col_mask;
		meta[col_start + 9 * 6] |= forbid_in_col_mask;
		meta[col_start + 9 * 7] |= forbid_in_col_mask;
		meta[col_start + 9 * 8] |= forbid_in_col_mask;

		const size_t row_start{ size_t(9) * (square / 9) };
		meta[row_start] |= forbid_in_row_mask;
		meta[row_start + 1] |= forbid_in_row_mask;
		meta[row_start + 2] |= forbid_in_row_mask;
		meta[row_start + 3] |= forbid_in_row_mask;
		meta[row_start + 4] |= forbid_in_row_mask;
		meta[row_start + 5] |= forbid_in_row_mask;
		meta[row_start + 6] |= forbid_in_row_mask;
		meta[row_start + 7] |= forbid_in_row_mask;
		meta[row_start + 8] |= forbid_in_row_mask;

		board[square] = digit;
	}
	inline void remove_digit(const size_t square, const short digit) {
		--total_placed;
		--placed_digits[digit];

		const uint32_t allow_in_box_mask{ ~(uint32_t(1) << (3 * digit)) };
		const uint32_t allow_in_row_mask{ ~(uint32_t(1) << (3 * digit + 1)) };
		const uint32_t allow_in_col_mask{ ~(uint32_t(1) << (3 * digit + 2)) };

		const size_t x{ (square % 9) / 3 };
		const size_t y{ square / 27 };
		const size_t center{ 1 + 3 * x + 9 * (1 + 3 * y) };
		meta[center] &= allow_in_box_mask;
		meta[center + 1] &= allow_in_box_mask;
		meta[center - 1] &= allow_in_box_mask;
		meta[center + 8] &= allow_in_box_mask;
		meta[center - 8] &= allow_in_box_mask;
		meta[center + 9] &= allow_in_box_mask;
		meta[center - 9] &= allow_in_box_mask;
		meta[center + 10] &= allow_in_box_mask;
		meta[center - 10] &= allow_in_box_mask;

		const size_t col_start{ square % 9 };
		meta[col_start] &= allow_in_col_mask;
		meta[col_start + 9] &= allow_in_col_mask;
		meta[col_start + 9 * 2] &= allow_in_col_mask;
		meta[col_start + 9 * 3] &= allow_in_col_mask;
		meta[col_start + 9 * 4] &= allow_in_col_mask;
		meta[col_start + 9 * 5] &= allow_in_col_mask;
		meta[col_start + 9 * 6] &= allow_in_col_mask;
		meta[col_start + 9 * 7] &= allow_in_col_mask;
		meta[col_start + 9 * 8] &= allow_in_col_mask;

		const size_t row_start{ size_t(9) * (square / 9) };
		meta[row_start] &= allow_in_row_mask;
		meta[row_start + 1] &= allow_in_row_mask;
		meta[row_start + 2] &= allow_in_row_mask;
		meta[row_start + 3] &= allow_in_row_mask;
		meta[row_start + 4] &= allow_in_row_mask;
		meta[row_start + 5] &= allow_in_row_mask;
		meta[row_start + 6] &= allow_in_row_mask;
		meta[row_start + 7] &= allow_in_row_mask;
		meta[row_start + 8] &= allow_in_row_mask;

		board[square] = 0;
	}
	bool inRow(const int index, const short value) const {
		const int start{ 9 * (index / 9) };
		for (int i = start; i < start + 9; ++i) {
			if (board[i] == value && i != index) {
				return true;
			}
		}
		return false;
	}
	bool inColumn(const size_t index, const short value) const {
		for (int i = index % 9; i < board.size(); i+=9) {
			if (board[i] == value && i != index) {
				return true;
			}
		}
		return false;
	}
	bool inBox(const int index, const short value) const {
		int x{ (index % 9) / 3 };
		int y{ index / 27 };
		const int center{ 1 + 3 * x + 9 * (1 + 3 * y) };
		bool ret{ false };
		ret |= board[center] == value && center != index;
		ret |= board[center+1] == value && center+1 != index;
		ret |= board[center-1] == value && center-1 != index;
		ret |= board[center+9] == value && center+9 != index;
		ret |= board[center-9] == value && center-9 != index;
		ret |= board[center+8] == value && center+8 != index;
		ret |= board[center-8] == value && center-8 != index;
		ret |= board[center+10] == value && center+10 != index;
		ret |= board[center-10] == value && center-10 != index;
		return ret;
	}
	bool is_valid_place(const size_t index, const short value) const {
		return board[index] == 0 && !bool(meta[index] & (7 << 3 * value)) && !(ruled_out_by_force[total_placed][index] & uint16_t(1) << value);
	}
	bool solved() const {
		int empty_squares{ 0 };
		for (int i = 0; i < board.size(); ++i) {
			if (inRow(i, board[i])) return false;
			if (inColumn(i, board[i])) return false;
			if (inBox(i, board[i])) return false;
			empty_squares += !board[i];
		}
		return empty_squares == 0;
	}
	vector<short> emptySquares() const {
		vector<short> ret{};
		for (int i = 0; i < board.size(); ++i) {
			if (!board[i]) ret.push_back(i);
		}
		return ret;
	}
	bool has_contradiction() const {
		for (int i = 0; i < 9; ++i) {
			const size_t row_start{ 9ull * i };
			const size_t col_start{ size_t(i) };
			for (int digit = 1; digit < 10; ++digit) {
				bool found{false};
				int possible_spaces{ 0 };
				for (int j = 0; j < 9; ++j) {
					const size_t square{ row_start + j };
					possible_spaces += is_valid_place(square, digit);
					found |= board[square] == digit;
				}
				if (!found && possible_spaces == 0) {

#if StepByStep
					cout << "contradiction in row " << i << " on digit " << digit << " (no possible place)\n";
#endif
					return true;
				}
				found = false;
				possible_spaces = 0;
				for (int j = 0; j < 9; ++j) {
					const size_t square{ col_start + j * 9ull };
					possible_spaces += is_valid_place(square, digit);
					found |= board[square] == digit;
				}
				if (!found && possible_spaces == 0) {
#if StepByStep
					cout << "contradiction in column " << i << " on digit " << digit << " (no possible place)\n";
#endif
					return true;
				}
				found = false;
				possible_spaces = 0;
				found |= board[centers[i]] == digit;
				possible_spaces += is_valid_place(centers[i], digit);
				possible_spaces += is_valid_place(centers[i] + 1, digit);
				possible_spaces += is_valid_place(centers[i] - 1, digit);
				possible_spaces += is_valid_place(centers[i] + 9, digit);
				possible_spaces += is_valid_place(centers[i] - 9, digit);
				possible_spaces += is_valid_place(centers[i] + 8, digit);
				possible_spaces += is_valid_place(centers[i] - 8, digit);
				possible_spaces += is_valid_place(centers[i] + 10, digit);
				possible_spaces += is_valid_place(centers[i] - 10, digit);

				found |= board[centers[i] + 1] == digit;
				found |= board[centers[i] - 1] == digit;
				found |= board[centers[i] + 9] == digit;
				found |= board[centers[i] - 9] == digit;
				found |= board[centers[i] + 8] == digit;
				found |= board[centers[i] - 8] == digit;
				found |= board[centers[i] + 10] == digit;
				found |= board[centers[i] - 10] == digit;
				if (!found && possible_spaces == 0) {
#if StepByStep
					cout << "contradiction in box " << i << " on digit " << digit << " (no possible place)\n";
#endif
					return true;
				}
			}
		}
		for (int i = 0; i < board.size(); ++i) {
			if (board[i] != 0) continue;
			int candidates{ 0 };
			for (int j = 1; j < 10; ++j) {
				candidates += is_valid_place(i, j);
			}
			if (candidates == 0) {
#if StepByStep
				cout << "contradiction on square " << i << " no possible digits\n";
#endif
				return true;
			}
		}
		return false;
	}
	bool root_solve() {
#if StepByStep
		cout << *this;
#endif
		std::chrono::steady_clock::time_point begin{ std::chrono::steady_clock::now() };
		solve_unique_patterns();
#if StepByStep
		cout << *this;
#endif
		bool ret{ solve() };
		if (ret) {
#if PrintInfoOnSolution
			cout << *this;
			const double contradiction_perc{ (nodes != 0) ? (100 * double(contradictions) / nodes) : 0 };
			cout << contradiction_perc << "% pruned by contradiction\n";
			std::chrono::steady_clock::time_point end{ std::chrono::steady_clock::now() };
			cout << nodes / double(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * 1000000 << " nps\n";
			cout << nodes << " total nodes\n";
			begin = end;
			nodes = 0;
			contradictions = 0;
#endif
		}
		else {
			cout << "failed to solve\n";
		}
		return ret;
	}
	vector<pair<short,short>> sorted_moves() const {
		vector<pair<short, short>> ret{};
		const vector<short> empty{ emptySquares() };
		unordered_map<uint32_t, uint8_t> scores{};
		for (const uint8_t square : empty) {
			const int x{ (square % 9) / 3 };
			const int y{ square / 27 };
			const int center{ 1 + 3 * x + 9 * (1 + 3 * y) };
			const size_t col_start{ size_t(square) % 9 };
			const size_t row_start{ size_t(9) * (square / 9) };
			for (uint8_t digit = 1; digit < 10; ++digit) {
				if (!is_valid_place(square, digit)) continue;
				ret.push_back(std::make_pair(square, digit));
				int places_in_box{ 0 };
				places_in_box += is_valid_place(center, digit);
				places_in_box += is_valid_place(center + 1, digit);
				places_in_box += is_valid_place(center - 1, digit);
				places_in_box += is_valid_place(center + 9, digit);
				places_in_box += is_valid_place(center - 9, digit);
				places_in_box += is_valid_place(center + 8, digit);
				places_in_box += is_valid_place(center - 8, digit);
				places_in_box += is_valid_place(center + 10, digit);
				places_in_box += is_valid_place(center - 10, digit);

				int places_in_row{ 0 };
				places_in_row += is_valid_place(row_start, digit);
				places_in_row += is_valid_place(row_start + 1, digit);
				places_in_row += is_valid_place(row_start + 2, digit);
				places_in_row += is_valid_place(row_start + 3, digit);
				places_in_row += is_valid_place(row_start + 4, digit);
				places_in_row += is_valid_place(row_start + 5, digit);
				places_in_row += is_valid_place(row_start + 6, digit);
				places_in_row += is_valid_place(row_start + 7, digit);
				places_in_row += is_valid_place(row_start + 8, digit);

				int places_in_col{ 0 };
				places_in_col += is_valid_place(col_start, digit);
				places_in_col += is_valid_place(col_start + 9, digit);
				places_in_col += is_valid_place(col_start + 2 * 9, digit);
				places_in_col += is_valid_place(col_start + 3 * 9, digit);
				places_in_col += is_valid_place(col_start + 4 * 9, digit);
				places_in_col += is_valid_place(col_start + 5 * 9, digit);
				places_in_col += is_valid_place(col_start + 6 * 9, digit);
				places_in_col += is_valid_place(col_start + 7 * 9, digit);
				places_in_col += is_valid_place(col_start + 8 * 9, digit);
				scores[uint16_t(square) | uint16_t(digit) << 15] = (uint8_t)std::min(std::min(places_in_col, places_in_box), places_in_row);
			}
		}
		std::sort(ret.begin(), ret.end(), [&](const pair<short, short>& lhs, const pair<short, short>& rhs) {
			const uint8_t lhs_score{ scores[uint16_t(lhs.first) | uint16_t(lhs.second) << 15] };
			const uint8_t rhs_score{ scores[uint16_t(rhs.first) | uint16_t(rhs.second) << 15] };
			if (lhs_score == rhs_score) {
				const uint8_t lhs_placed{ placed_digits[lhs.second] };
				const uint8_t rhs_placed{ placed_digits[rhs.second] };
				if (lhs_placed == rhs_placed) return lhs.second < rhs.second;
				return lhs_placed > rhs_placed;
			}
			return lhs_score < rhs_score;
		});
		return ret;
	}
	bool solve_unique_patterns() {
		bool found{ true };
		while (found) {
			found = false;
			for (int i = 0; i < 9; ++i) {
				for (int num = 1; num < 10; ++num) {
					int places_for_number{ 0 };
					int place{ 0 };
					for(int j = 0; j < 9; ++j) {
						const int index{ 9 * i + j };
						if (is_valid_place(index, num)) {
							place = index;
							++places_for_number;
						}
					}
					if (places_for_number == 1) {
						place_digit(place, num);
						found = true;
					}

					places_for_number = 0;
					for (int j = 0; j < 9; ++j) {
						const int index{ i + 9 * j };
						if (is_valid_place(index, num)) {
							place = index;
							++places_for_number;
						}
					}
					if (places_for_number == 1) {
						place_digit(place, num);
						found = true;
					}
					/*
					places_for_number = 0;
					for (int j = 0; j < 9; ++j) {
						if (is_valid_place(boxes[i][j], num)) {
							place = boxes[i][j];
							++places_for_number;
						}
					}
					if (places_for_number == 1) {
						place_digit(place, num);
						found = true;
					}
					*/
				}
			}
			/*
			*/
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					const int center{ 1 + 3 * i + 9 * (1 + 3 * j) };
					for (int num = 1; num < 10; ++num) {
						int places_for_number{ 0 };
						int place{ 0 };
						int index{center};
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center+1;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center-1;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center+9;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center-9;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center+10;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center-10;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center+8;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						index=center-8;
						if(is_valid_place(index, num)){
							++places_for_number;
							place=index;
						}
						if (places_for_number == 1) {
							place_digit(place, num);
							found = true;
						}
					}
				}
			}
		}
		return solved();
	}
	bool solve() {
#if StepByStep
		string s{};
		cout << "press enter to continue\n";
		std::getline(std::cin, s);
#endif
		if (solved()) return true;
		++nodes;
		if (has_contradiction()) {
			++contradictions;
#if StepByStep
			cout<<"contradiction in"<<*this;
#endif
			return false;
		}
		const auto brdCopy{ board };
		const auto metaCopy{ meta };
		const auto placedCopy{ total_placed };
		const auto placedDigitsCopy{ placed_digits };
		if(solve_unique_patterns()) return true;
		if (has_contradiction()) {
			++contradictions;
#if StepByStep
			cout << "contradiction in" << *this;
#endif
			board = brdCopy;
			meta = metaCopy;
			placed_digits = placedDigitsCopy;
			total_placed = placedCopy;
			return false;
		}
#if StepByStep
		cout << *this;
#endif
		const vector<pair<short, short>> moves{ sorted_moves() };
		for (const pair<short, short>& p : moves) {
			if (!is_valid_place(p.first, p.second)) continue;
#if StepByStep
			cout <<"square " << int(p.first) << " placing " << int(p.second) << "\n";
#endif
			place_digit(p.first, p.second);
			const bool solved{ solve() };
			if (solved) return true;
			remove_digit(p.first, p.second);
			ruled_out_by_force[total_placed][p.first] |= uint16_t(1) << p.second;
#if StepByStep
			cout << "go back to\n" << *this;
#endif
			if (has_contradiction()) {
				break;
			}
			const auto brdCopy{ board };
			const auto metaCopy{ meta };
			const auto placedCopy{ total_placed };
			const auto placedDigitsCopy{ placed_digits };
			if (solve_unique_patterns()) return true;
			if (total_placed != placedCopy) {
#if StepByStep
				cout << "new unique moves were determined after forced rule out\n" << *this;
#endif
				if (has_contradiction()) {
					++contradictions;
#if StepByStep
					cout << "contradiction in" << *this;
#endif
					break;
				}
			}
		}
		board = brdCopy;
		meta = metaCopy;
		placed_digits = placedDigitsCopy;
		total_placed = placedCopy;
		return false;
	}
	friend std::ostream& operator<<(std::ostream& o, const Solver& s) {
#if StepByStep
		o << "\ncurrently placed "<< int(s.total_placed) <<" digits";
#endif
		const static array<string, 10> ascii_chars{ "   "," 1 "," 2 " ," 3 " ," 4 " ," 5 " ," 6 " ," 7 " ," 8 " ," 9 " };
		for (int i = 0; i < s.board.size(); ++i) {
			if (i % 9 == 3 || i % 9 == 6) o << "#";
			else if (i % 9 == 0 && i) o << "|";
			if(i%9==0 && i/9 % 3 == 0) o << "\n+===========+===========+===========+\n";
			else if (i % 9 == 0) o << "\n+-----------+-----------+-----------+\n";
			if(!(i % 9 == 3 || i % 9 == 6)) o << "|";
			o << ascii_chars[s.board[i]];
		}
		return o<< "|\n+===========+===========+===========+"<<endl;
	}
};