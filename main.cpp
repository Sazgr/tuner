#include <bits/stdc++.h>
#include "bit_operations.h"
#include "lookup_arrays.h"

using namespace std;

/*******************
* INPUT AND OUTPUT *
*******************/

ifstream fin ("C:\\kyle\\chess2\\texel\\tuningsets\\lichess-big3-resolved.epd");
//ifstream fin1 ("C:\\kyle\\chess2\\texel\\tuningsets\\pk-80k-r12-quiet.epd");
//ifstream fin2 ("C:\\kyle\\chess2\\texel\\tuningsets\\pk-60k-r10-set1-quiet.epd");
//ifstream fin3 ("C:\\kyle\\chess2\\texel\\tuningsets\\xxx.epd");
ofstream fout ("C:\\git\\tuner\\tables_raw.txt");
ofstream e_out ("C:\\git\\tuner\\e.txt");
ostream& out = cout;

/**********************
* TYPEDEFS AND CONSTS *
**********************/

typedef int8_t s8;
typedef uint64_t u64; //for bitboards

constexpr double k = 0.0092;
constexpr int king_len = 2 * 32 * 5 * 64;
constexpr int mob_len = 66 + 36;
constexpr int pawn_len = 6 * 8;
constexpr int bishop_len = 1;
constexpr int half = king_len + mob_len + pawn_len + bishop_len;
constexpr int full = 2 * half;

constexpr bool no_draws = false;
constexpr double draw_mult = 1.5;
constexpr bool prejudice = false;
constexpr double prejudice_margin = 0.6;

constexpr int thread_batch_size = 100000;
constexpr int batch_size = 6 * thread_batch_size;
constexpr int max_iterations = 1000;
int num_pos = 100000000;
double lr = 1;
constexpr double decay_rate = 0.9977;
int wdl_data[3][25];

/*************************
* INITIAL PST PARAMETERS *
*************************/

const int mg_value[7] = {100, 335, 369, 485, 1051, 20000, 0};
const int eg_value[7] = {123, 270, 300, 529, 993, 20000, 0};

const int mg_table[6][64] = {{
    0, 0, 0, 0, 0, 0, 0, 0,
    63, 93, 66, 89, 62, 56, -22, -45,
    -20, -8, 23, 23, 35, 63, 42, 4,
    -40, -28, -15, -12, 8, 3, -2, -13,
    -44, -35, -20, -4, -4, -6, -14, -22,
    -42, -38, -22, -18, -4, -7, 3, -15,
    -44, -38, -31, -29, -17, 2, 11, -24,
    0, 0, 0, 0, 0, 0, 0, 0,
}, {
    -182, -108, -56, -33, 32, -74, -36, -121,
    -22, -2, 23, 48, 22, 93, -5, 23,
    0, 32, 49, 60, 103, 112, 59, 32,
    -1, 9, 33, 53, 34, 60, 22, 37,
    -13, -2, 15, 18, 27, 22, 19, -2,
    -33, -12, 1, 11, 24, 7, 9, -15,
    -41, -29, -17, -2, -2, 2, -11, -18,
    -96, -32, -33, -18, -16, -9, -30, -58,
}, {
    -31, -42, -56, -80, -60, -60, -15, -52,
    -16, 2, -4, -23, 10, 1, -2, -13,
    -2, 19, 15, 33, 23, 63, 39, 30,
    -9, 2, 18, 30, 25, 19, 5, -7,
    -8, -6, -4, 22, 17, -1, -6, 6,
    -1, 3, 6, 1, 5, 8, 8, 14,
    6, 7, 9, -6, 3, 15, 23, 11,
    -1, 8, -1, -8, 0, -8, 15, 12,
}, {
    20, 6, 8, 7, 26, 35, 36, 65,
    2, -5, 17, 40, 26, 56, 50, 85,
    -15, 7, 1, 3, 37, 49, 100, 73,
    -23, -14, -15, -12, -8, 4, 20, 19,
    -41, -42, -31, -25, -23, -28, -4, -18,
    -44, -38, -31, -28, -20, -20, 9, -14,
    -44, -37, -19, -21, -15, -11, 5, -27,
    -30, -24, -15, -9, -4, -11, 1, -27,
}, {
    -33, -28, 2, 23, 32, 45, 59, 13,
    -5, -35, -23, -29, -22, 15, 0, 52,
    -2, -6, -13, 1, 11, 63, 70, 69,
    -19, -15, -13, -17, -14, 0, 8, 11,
    -15, -19, -19, -12, -12, -11, -2, 5,
    -14, -8, -12, -13, -9, -4, 10, 4,
    -13, -10, -1, 0, -1, 9, 15, 26,
    -10, -20, -13, -5, -8, -15, 1, 1,
}, {
    -37, 37, 36, -7, -35, -9, 29, 26,
    16, 9, -14, 26, 12, 17, 6, -1,
    -23, 45, -2, -20, 1, 51, 49, 2,
    -22, -31, -42, -80, -80, -45, -39, -66,
    -44, -36, -73, -106, -102, -61, -70, -100,
    3, 22, -38, -51, -43, -40, 1, -17,
    95, 53, 37, 3, -1, 19, 63, 73,
    91, 116, 91, -6, 57, 21, 93, 93,
}};

const int eg_table[6][64] = {{
    0, 0, 0, 0, 0, 0, 0, 0,
    142, 122, 128, 74, 76, 90, 136, 151,
    24, 20, -18, -56, -60, -39, -4, 4,
    8, -5, -20, -40, -40, -31, -18, -18,
    -15, -18, -29, -36, -35, -31, -28, -32,
    -18, -20, -27, -25, -25, -27, -32, -35,
    -19, -19, -23, -24, -17, -26, -35, -37,
    0, 0, 0, 0, 0, 0, 0, 0,
}, {
    -71, -25, -2, -9, -16, -24, -36, -95,
    -20, -3, 5, 4, 1, -20, -7, -38,
    -9, 8, 33, 33, 14, 6, -3, -20,
    4, 24, 45, 46, 46, 41, 23, -8,
    2, 17, 45, 46, 49, 36, 15, -6,
    -13, 9, 25, 39, 37, 21, 3, -11,
    -21, -6, 8, 7, 5, 2, -16, -14,
    -25, -35, -12, -12, -8, -18, -28, -33,
}, {
    0, 8, 6, 14, 8, 0, -4, -4,
    -8, -3, 0, 3, -9, -7, -1, -12,
    8, -2, 3, -7, -3, -1, -4, 2,
    4, 8, 3, 16, 10, 8, 5, 4,
    0, 7, 14, 13, 11, 9, 5, -13,
    0, 5, 8, 9, 13, 6, -5, -10,
    -3, -7, -13, 1, -1, -9, -3, -21,
    -13, -3, -4, -3, -5, 5, -18, -26,
}, {
    12, 18, 25, 20, 13, 9, 9, 0,
    13, 24, 26, 14, 15, 0, -3, -14,
    11, 11, 13, 9, -6, -13, -23, -23,
    13, 11, 17, 14, 0, -7, -8, -12,
    9, 11, 10, 8, 5, 2, -9, -9,
    3, 1, 0, 1, -4, -11, -29, -24,
    -5, 0, -3, -2, -10, -15, -24, -14,
    -4, -2, 2, -1, -8, -7, -13, -14,
}, {
    4, 14, 24, 22, 14, 8, -28, -1,
    -15, 21, 46, 63, 78, 35, 23, 7,
    -13, -4, 32, 35, 49, 19, -13, -16,
    1, 9, 18, 41, 52, 40, 33, 22,
    -10, 11, 14, 31, 28, 24, 12, 7,
    -25, -16, 2, -1, 3, 0, -18, -22,
    -29, -29, -37, -27, -25, -52, -76, -90,
    -33, -28, -31, -37, -35, -36, -53, -59,
}, {
    -78, -51, -32, -4, -5, 1, 0, -81,
    -23, 13, 23, 18, 31, 42, 40, 9,
    -7, 18, 34, 44, 47, 44, 38, 11,
    -12, 21, 40, 52, 53, 48, 37, 11,
    -19, 10, 35, 51, 50, 36, 24, 9,
    -30, -7, 17, 29, 28, 19, 0, -12,
    -53, -22, -11, 0, 4, -5, -23, -42,
    -91, -69, -48, -28, -51, -31, -59, -89,
}};

const int mg_knight_mobility[9] = {-1, -1, 2, 0, 3, 6, 8, 10, 5};
const int mg_bishop_mobility[14] = {-41, -19, -11, -9, -2, 6, 9, 15, 15, 18, 19, 25, 46, 61};
const int mg_rook_mobility[15] = {14, -12, -6, -7, -6, -5, -1, 3, 11, 18, 23, 27, 31, 43, 47};
const int mg_queen_mobility[28] = {0, 21, 63, 64, 58, 56, 56, 57, 58, 59, 60, 63, 63, 62, 63, 64, 62, 62, 63, 63, 68, 68, 84, 93, 105, 130, 100, 78};

const int eg_knight_mobility[9] = {12, 59, 65, 66, 65, 67, 60, 54, 52};
const int eg_bishop_mobility[14] = {5, 9, 14, 31, 41, 49, 56, 60, 66, 65, 65, 61, 49, 54};
const int eg_rook_mobility[15] = {26, 58, 55, 64, 75, 81, 84, 86, 85, 88, 90, 94, 98, 96, 97};
const int eg_queen_mobility[28] = {0, 0, -4, -5, 29, 72, 93, 107, 114, 128, 126, 126, 130, 136, 140, 143, 150, 154, 158, 163, 163, 168, 156, 158, 160, 152, 138, 128};

const int mg_knight_fmobility[5] = {0, 0, 0, 0, 0};
const int mg_bishop_fmobility[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int mg_rook_fmobility[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int mg_queen_fmobility[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const int eg_knight_fmobility[5] = {0, 0, 0, 0, 0};
const int eg_bishop_fmobility[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int eg_rook_fmobility[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int eg_queen_fmobility[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const int mg_passed[8] = {0, -4, 16, 14, -2, -6, 1, 0};
const int mg_passed_free[8] = {0, -10, 0, 11, -8, -8, -1, 0};
const int mg_doubled[8] = {0, 0, -2, 5, -7, -8, -3, 0};
const int mg_isolated[8] = {0, 15, 3, -3, -17, -23, -15, 0};
const int mg_supported[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int mg_phalanx[8] = {0, 0, 0, 0, 0, 0, 0, 0};

const int eg_passed[8] = {0, -37, 59, 43, 29, 16, 6, 0};
const int eg_passed_free[8] = {0, 43, 137, 65, 38, 13, 9, 0};
const int eg_doubled[8] = {0, 0, 4, 0, -9, -10, -16, 0};
const int eg_isolated[8] = {0, -7, -23, -20, -10, -12, -6, 0};
const int eg_supported[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const int eg_phalanx[8] = {0, 0, 0, 0, 0, 0, 0, 0};

const int mg_bishop_pair = 0;
const int eg_bishop_pair = 0;

/*******************
* BITBOARD ATTACKS *
********************/

u64 positive_ray_attacks(u64 occ, int direction, int square) {
    u64 attacks = rays[direction][square];
    square = get_lsb((attacks & occ) | 0x8000000000000000ull);
    attacks ^= rays[direction][square];
    return attacks;
}

u64 negative_ray_attacks(u64 occ, int direction, int square) {
    u64 attacks = rays[direction][square];
    square = get_msb((attacks & occ) | 0x0000000000000001ull);
    attacks ^= rays[direction][square];
    return attacks;
}

u64 diagonal_attacks(u64 occ, int square) {
    return positive_ray_attacks(occ, southeast, square) | negative_ray_attacks(occ, northwest, square);
}

u64 antidiagonal_attacks(u64 occ, int square) {
    return positive_ray_attacks(occ, southwest, square) | negative_ray_attacks(occ, northeast, square);
}

u64 rank_attacks(u64 occ, int square) {
    return positive_ray_attacks(occ, east, square) | negative_ray_attacks(occ, west, square);
}

u64 file_attacks(u64 occ, int square) {
    return positive_ray_attacks(occ, south, square) | negative_ray_attacks(occ, north, square);
}

u64 rook_attacks(u64 occ, int square) {
    return file_attacks(occ, square) | rank_attacks(occ, square);
}

u64 bishop_attacks(u64 occ, int square) {
    return diagonal_attacks(occ, square) | antidiagonal_attacks(occ, square);
}

u64 queen_attacks(u64 occ, int square) {
    return rook_attacks(occ, square) | bishop_attacks(occ, square);
}

/**********************
* STRUCTS AND CLASSES *
**********************/

class Timer {
private:
	using clock_t = std::chrono::steady_clock;
	using second_t = std::chrono::duration<double, std::ratio<1>>;	
	std::chrono::time_point<clock_t> m_beg;
public:
	Timer() : m_beg(clock_t::now()) {}
	void reset() {
		m_beg = clock_t::now();
	}
	double elapsed() const {
		return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
	}
};

struct Data {
    vector<pair<s8, s8>> pieces; //square, piece_type
    vector<pair<s8, s8>> mob;    //piece_type, mobility
    vector<pair<s8, s8>> pawns;   //square, piece_type
    vector<array<s8, 4>> king;   //king_color, king_square, piece_type, piece_square
    int b_p;
    int gp; //gamephase
    int tempo_bonus;
    double score;
    bool load_fen(vector<double>&, string, string, string, string, string, string);
    double eval(const vector<double>& params);
};

/************
* FUNCTIONS *
************/

inline double sigmoid(int s) {
    return 1.0/(1.0+exp(-k*s));
}

bool Data::load_fen(vector<double>& params, string fen_pos, string fen_stm, string fen_castling, string fen_ep, string fen_hmove_clock = "0", string fen_fmove_counter = "1") {
    int sq = 0;
    gp = 0;
    bool wf, bf;
    u64 bb[15];
    for (int i{}; i<15; ++i) bb[i] = 0ull;
    for (auto pos = fen_pos.begin(); pos != fen_pos.end(); ++pos) {
        switch (*pos) {
            case 'p': (bb[ 0] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 0}); break;
            case 'n': (bb[ 2] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 2}); gp += 1; break;
            case 'b': (bb[ 4] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 4}); gp += 1; break;
            case 'r': (bb[ 6] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 6}); gp += 2; break;
            case 'q': (bb[ 8] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 8}); gp += 4; break;
            case 'k': (bb[10] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 10}); break;
            case 'P': (bb[ 1] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 1}); break;
            case 'N': (bb[ 3] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 3}); gp += 1; break;
            case 'B': (bb[ 5] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 5}); gp += 1; break;
            case 'R': (bb[ 7] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 7}); gp += 2; break;
            case 'Q': (bb[ 9] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[13] |= 1ull << sq); pieces.push_back({sq, 9}); gp += 4; break;
            case 'K': (bb[11] |= 1ull << sq); (bb[12] |= 1ull << sq); (bb[14] |= 1ull << sq); pieces.push_back({sq, 11}); break;
            case '/': --sq; break;
            case '1': break;
            case '2': ++sq; break;
            case '3': sq += 2; break;
            case '4': sq += 3; break;
            case '5': sq += 4; break;
            case '6': sq += 5; break;
            case '7': sq += 6; break;
            case '8': sq += 7; break;
            default: return false;
        }
        ++sq;
    }
    s8 ksq[2] = {static_cast<s8>(get_lsb(bb[10])), static_cast<s8>(get_lsb(bb[11]))};
    u64 atts;
    for (pair<s8, s8> piece : pieces) {
        switch (piece.second / 2) {
            case 0: //pawn
                if (!(passed[piece.second][piece.first] & bb[piece.second ^ 1]) && !(doubled[piece.second][piece.first] & bb[piece.second])) {
                    if (!(doubled[piece.second][piece.first] & bb[13 + (piece.second ^ 1)])) {
                        pawns.push_back(make_pair(piece.first >> 3, piece.second + 2));
                    } else {
                        pawns.push_back(make_pair(piece.first >> 3, piece.second));
                    }
                }
                if (doubled[piece.second][piece.first] & bb[piece.second]) {
                    pawns.push_back(make_pair(piece.first >> 3, piece.second + 4));
                }
                if (!(isolated[piece.first & 7] & bb[piece.second])) {
                    pawns.push_back(make_pair(piece.first >> 3, piece.second + 6));
                }
                if (pawn_attacks[piece.second ^ 1][piece.first] & bb[piece.second]) {
                    pawns.push_back(make_pair(piece.first >> 3, piece.second + 8));
                }
                if ((piece.first & 7) != 7 && (bb[piece.second] & (2ull << piece.first))) {
                    pawns.push_back(make_pair(piece.first >> 3, piece.second + 10));
                }
                king.push_back(array<s8, 4>{0, ksq[0], piece.second, piece.first});
                king.push_back(array<s8, 4>{1, ksq[1], piece.second, piece.first});
                break;
            case 1: //knight
                atts = knight_attacks[piece.first] & ~bb[13 + (piece.second & 1)];
                mob.push_back(make_pair(piece.second, popcount(atts)));
                mob.push_back(make_pair(piece.second, 66 + popcount(atts &  forward_m[piece.second & 1][piece.first >> 3])));
                king.push_back(array<s8, 4>{0, ksq[0], piece.second, piece.first});
                king.push_back(array<s8, 4>{1, ksq[1], piece.second, piece.first});
                break;
            case 2: //bishop
                atts = bishop_attacks(bb[12] & ~bb[8 + (piece.second & 1)], piece.first) & ~bb[13 + (piece.second & 1)];
                mob.push_back(make_pair(piece.second, 9 + popcount(atts)));
                mob.push_back(make_pair(piece.second, 66 + 5 + popcount(atts &  forward_m[piece.second & 1][piece.first >> 3])));
                king.push_back(array<s8, 4>{0, ksq[0], piece.second, piece.first});
                king.push_back(array<s8, 4>{1, ksq[1], piece.second, piece.first});
                break;
            case 3: //rook
                atts = rook_attacks(bb[12] & ~bb[8 + (piece.second & 1)], piece.first) & ~bb[13 + (piece.second & 1)];
                mob.push_back(make_pair(piece.second, 9 + 14 + popcount(atts)));
                mob.push_back(make_pair(piece.second, 66 + 5 + 8 + popcount(atts &  forward_m[piece.second & 1][piece.first >> 3])));
                king.push_back(array<s8, 4>{0, ksq[0], piece.second, piece.first});
                king.push_back(array<s8, 4>{1, ksq[1], piece.second, piece.first});
                break;
            case 4: //queen
                atts = queen_attacks(bb[12], piece.first) & ~bb[13 + (piece.second & 1)];
                mob.push_back(make_pair(piece.second, 9 + 14 + 15 + popcount(atts)));
                mob.push_back(make_pair(piece.second, 66 + 5 + 8 + 8 + popcount(atts &  forward_m[piece.second & 1][piece.first >> 3])));
                king.push_back(array<s8, 4>{0, ksq[0], piece.second, piece.first});
                king.push_back(array<s8, 4>{1, ksq[1], piece.second, piece.first});
                break;
            case 5: //king
                break;
        }
    }
    b_p = (popcount(bb[5]) >= 2) - (popcount(bb[4]) >= 2);
    if (fen_stm == "w") tempo_bonus = gp / 2;
    else tempo_bonus = -gp / 2; 
    return true;
}

s8 compress(s8 a) {
    return ((a & 56) >> 1) + (a & 3);
}

double Data::eval(const vector<double>& params) {
    double mg{}, eg{};
    /*for (pair<s8, s8> piece : pieces) {
        int param_id = (piece.second / 2) * 64 + ((piece.second & 1) ? piece.first : (piece.first ^ 56));
        mg += ((piece.second & 1) * 2 - 1) * params[param_id]; //middlegame parameter
        eg += ((piece.second & 1) * 2 - 1) * params[half + param_id];//endgame parameter
    }*/
    for (array<s8, 4> piece : king) {
        int param_id = ((piece[1] & 7) >= 4)
            ? (32 * 5 * 64 * (piece[0] ^ (piece[2] & 1)) + 5 * 64 * ((piece[2] & 1) ? compress(piece[1] ^ 7) : (compress(piece[1] ^ 7 ^ 56))) + 64 * (piece[2] / 2) + ((piece[2] & 1) ? (piece[3] ^ 7) : (piece[3] ^ 7 ^ 56)))
            : (32 * 5 * 64 * (piece[0] ^ (piece[2] & 1)) + 5 * 64 * ((piece[2] & 1) ? compress(piece[1] ^ 0) : (compress(piece[1] ^ 0 ^ 56))) + 64 * (piece[2] / 2) + ((piece[2] & 1) ? (piece[3] ^ 0) : (piece[3] ^ 0 ^ 56)));
        mg += ((piece[2] & 1) * 2 - 1) * params[param_id]; //middlegame parameter
        eg += ((piece[2] & 1) * 2 - 1) * params[half + param_id];//endgame parameter
    }
    for (pair<s8, s8> mobile : mob) {
        int param_id = king_len + mobile.second;
        mg += ((mobile.first & 1) * 2 - 1) * params[param_id]; //middlegame parameter
        eg += ((mobile.first & 1) * 2 - 1) * params[half + param_id];//endgame parameter
    }
    for (pair<s8, s8> piece : pawns) {
        int param_id = king_len + mob_len + (piece.second / 2) * 8 + ((piece.second & 1) ? piece.first : (piece.first ^ 7));
        mg += ((piece.second & 1) * 2 - 1) * params[param_id]; //middlegame parameter
        eg += ((piece.second & 1) * 2 - 1) * params[half + param_id];//endgame parameter
    }
    mg += b_p * params[king_len + mob_len + pawn_len];
    eg += b_p * params[half + king_len + mob_len + pawn_len];
    return (gp * mg + (24 - gp) * eg) / 24 + tempo_bonus;
}

/*******************
* GRADIENT DESCENT *
*******************/

vector<Data> test_set;

void e_thread(int n_threads, int offset, const vector<double>& params, double& result) {
    for (int i{offset}; i<test_set.size(); i += n_threads) {
        double diff = (test_set[i].score - sigmoid(test_set[i].eval(params)));
        result += diff * diff;
    }
}

double e(const vector<double>& params) {
    double total_e_0{}, total_e_1{}, total_e_2{}, total_e_3{}, total_e_4{}, total_e_5{};
    std::thread ethread0 (e_thread, 6, 0, std::ref(params), std::ref(total_e_0));
    std::thread ethread1 (e_thread, 6, 1, std::ref(params), std::ref(total_e_1));
    std::thread ethread2 (e_thread, 6, 2, std::ref(params), std::ref(total_e_2));
    std::thread ethread3 (e_thread, 6, 3, std::ref(params), std::ref(total_e_3));
    std::thread ethread4 (e_thread, 6, 4, std::ref(params), std::ref(total_e_4));
    std::thread ethread5 (e_thread, 6, 5, std::ref(params), std::ref(total_e_5));
    ethread0.join();
    ethread1.join();
    ethread2.join();
    ethread3.join();
    ethread4.join();
    ethread5.join();
    return (total_e_0 + total_e_1 + total_e_2 + total_e_3 + total_e_4 + total_e_5) / test_set.size();
}

void compute_gradient(int n_threads, int offset, int start, vector<double>& gradient, vector<double>& params) {
    for (int i{start + offset}; i<start + batch_size; i += n_threads) {
        Data& this_pos = test_set[i % num_pos];
        double linear_eval = this_pos.eval(params);
        double sig = sigmoid(linear_eval);
        double res = (this_pos.score - sig) * sig * (1 - sig);
        double mg_base = res * this_pos.gp;
        double eg_base = res * (24 - this_pos.gp);
        //looping through features
        /*for (pair<s8, s8> piece : this_pos.pieces) {
            int param_id = (piece.second / 2) * 64 + ((piece.second & 1) ? piece.first : (piece.first ^ 56));
            gradient[param_id] += ((piece.second & 1) * 2 - 1) * mg_base; //middlegame parameter
            gradient[half + param_id] += ((piece.second & 1) * 2 - 1) * eg_base; //endgame parameter
        }*/
        for (array<s8, 4> piece : this_pos.king) {
            int param_id = ((piece[1] & 7) >= 4)
                ? (32 * 5 * 64 * (piece[0] ^ (piece[2] & 1)) + 5 * 64 * ((piece[2] & 1) ? compress(piece[1] ^ 7) : (compress(piece[1] ^ 7 ^ 56))) + 64 * (piece[2] / 2) + ((piece[2] & 1) ? (piece[3] ^ 7) : (piece[3] ^ 7 ^ 56)))
                : (32 * 5 * 64 * (piece[0] ^ (piece[2] & 1)) + 5 * 64 * ((piece[2] & 1) ? compress(piece[1] ^ 0) : (compress(piece[1] ^ 0 ^ 56))) + 64 * (piece[2] / 2) + ((piece[2] & 1) ? (piece[3] ^ 0) : (piece[3] ^ 0 ^ 56)));
            gradient[param_id] += ((piece[2] & 1) * 2 - 1) * mg_base; //middlegame parameter
            gradient[half + param_id] += ((piece[2] & 1) * 2 - 1) * eg_base; //endgame parameter
        }
        for (pair<s8, s8> mobile : this_pos.mob) {
            int param_id = king_len + mobile.second;
            gradient[param_id] += ((mobile.first & 1) * 2 - 1) * mg_base; //middlegame parameter
            gradient[half + param_id] += ((mobile.first & 1) * 2 - 1) * eg_base; //endgame parameter
        }
        for (pair<s8, s8> piece : this_pos.pawns) {
            int param_id = king_len + mob_len + (piece.second / 2) * 8 + ((piece.second & 1) ? piece.first : (piece.first ^ 7));
            gradient[param_id] += ((piece.second & 1) * 2 - 1) * mg_base; //middlegame parameter
            gradient[half + param_id] += ((piece.second & 1) * 2 - 1) * eg_base; //endgame parameter
        }
        gradient[king_len + mob_len + pawn_len] += this_pos.b_p * mg_base;
        gradient[half + king_len + mob_len + pawn_len] += this_pos.b_p * eg_base;
    }
}

vector<double> gradient_descent(const vector<double>& initial_guess) {
    Timer timer;
    double best_e = e(initial_guess);
    cout << "start e " << best_e << endl;
    e_out << 0 << ' ' << best_e << endl;
    best_e = 1.0;
    const int n_params = initial_guess.size();
    vector<double> best_params = initial_guess;
    vector<double> all_best_params = initial_guess;
    array<vector<double>, 6> thread_gradient{};
    vector<double> gradient{};
    thread_gradient[0].resize(n_params);
    thread_gradient[1].resize(n_params);
    thread_gradient[2].resize(n_params);
    thread_gradient[3].resize(n_params);
    thread_gradient[4].resize(n_params);
    thread_gradient[5].resize(n_params);
    gradient.resize(n_params);
    int start = 0;
    vector<double> momentum(n_params, 0);
    vector<double> velocity(n_params, 0);
    for (int iter{}; iter < max_iterations; ++iter) {
        //lr *= decay_rate;
        timer.reset();
        for (int i{}; i<n_params; ++i) {
            gradient[i] = 0;
        }
        for (int j{}; j<thread_gradient.size(); ++j) {
            for (int i{}; i<n_params; ++i) {
                thread_gradient[j][i] = 0;
            }
        }
        start %= num_pos;
        std::thread tuning0 (compute_gradient, 6, 0, start, std::ref(thread_gradient[0]), std::ref(best_params));
        std::thread tuning1 (compute_gradient, 6, 1, start, std::ref(thread_gradient[1]), std::ref(best_params));
        std::thread tuning2 (compute_gradient, 6, 2, start, std::ref(thread_gradient[2]), std::ref(best_params));
        std::thread tuning3 (compute_gradient, 6, 3, start, std::ref(thread_gradient[3]), std::ref(best_params));
        std::thread tuning4 (compute_gradient, 6, 4, start, std::ref(thread_gradient[4]), std::ref(best_params));
        std::thread tuning5 (compute_gradient, 6, 5, start, std::ref(thread_gradient[5]), std::ref(best_params));
        tuning0.join();
        tuning1.join();
        tuning2.join();
        tuning3.join();
        tuning4.join();
        tuning5.join();
        for (int j{}; j<thread_gradient.size(); ++j) {
            for (int i{}; i<n_params; ++i) {
                gradient[i] += thread_gradient[j][i];
            }
        }
        vector<double> new_params = best_params;
        constexpr double beta1 = 0.9;
        constexpr double beta2 = 0.999;

        for (int i = 0; i < n_params; ++i) {
            const double grad = -k * gradient[i] / batch_size;
            momentum[i] = beta1 * momentum[i] + (1 - beta1) * grad;
            velocity[i] = beta2 * velocity[i] + (1 - beta2) * pow(grad, 2);
            new_params[i] -= lr * momentum[i] / (1e-8 + sqrt(velocity[i]));
        }
        /*for (int i{}; i<n_params; ++i) {
            new_params[i] += gradient[i] / batch_size * lr;
        }*/
        best_params = new_params;
        cout << "iteration " << iter << " time " << timer.elapsed() << "s speed " << batch_size/timer.elapsed()/1000000 << "mps step " << lr << "x" << endl;
        start += batch_size;
        if (iter % 10 == 9) {
            double new_e = e(best_params);
            cout << "current e " << new_e << endl;
            e_out << iter + 1 << ' ' << new_e << endl;
            if (new_e < best_e) {
                best_e = new_e;
                all_best_params = best_params;
            }
        }
    }
    cout << "end e " << best_e << endl;
    return all_best_params;
}

int main() {
    Timer timer;
    string command;
    array<string, 8> tokens;
    timer.reset();
    vector<double> params;
    /*for (int pc{}; pc<6; ++pc) {
        for (int sq{}; sq<64; ++sq) {
            params.push_back(mg_value[pc] + mg_table[pc][sq ^ 7]);
        }
    }*/
    for (int i{}; i<64; ++i) {
        for (int pc{}; pc<5; ++pc) {
            for (int sq{}; sq<64; ++sq) {
                params.push_back(0.5 * (mg_value[pc] + mg_table[pc][sq ^ 7]));
            }
        }
    }
    for (int i{}; i<9; ++i) {params.push_back(mg_knight_mobility[i]);}
    for (int i{}; i<14; ++i) {params.push_back(mg_bishop_mobility[i]);}
    for (int i{}; i<15; ++i) {params.push_back(mg_rook_mobility[i]);}
    for (int i{}; i<28; ++i) {params.push_back(mg_queen_mobility[i]);}
    for (int i{}; i<5; ++i) {params.push_back(mg_knight_fmobility[i]);}
    for (int i{}; i<8; ++i) {params.push_back(mg_bishop_fmobility[i]);}
    for (int i{}; i<8; ++i) {params.push_back(mg_rook_fmobility[i]);}
    for (int i{}; i<15; ++i) {params.push_back(mg_queen_fmobility[i]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_passed[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_passed_free[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_doubled[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_isolated[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_supported[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(mg_phalanx[sq]);}
    params.push_back(mg_bishop_pair);
    /*for (int pc{}; pc<6; ++pc) {
        for (int sq{}; sq<64; ++sq) {
            params.push_back(eg_value[pc] + eg_table[pc][sq]);
        }
    }*/
    for (int i{}; i<64; ++i) {
        for (int pc{}; pc<5; ++pc) {
            for (int sq{}; sq<64; ++sq) {
                params.push_back(0.5 * (eg_value[pc] + eg_table[pc][sq]));
            }
        }
    }
    for (int i{}; i<9; ++i) {params.push_back(eg_knight_mobility[i]);}
    for (int i{}; i<14; ++i) {params.push_back(eg_bishop_mobility[i]);}
    for (int i{}; i<15; ++i) {params.push_back(eg_rook_mobility[i]);}
    for (int i{}; i<28; ++i) {params.push_back(eg_queen_mobility[i]);}
    for (int i{}; i<5; ++i) {params.push_back(eg_knight_fmobility[i]);}
    for (int i{}; i<8; ++i) {params.push_back(eg_bishop_fmobility[i]);}
    for (int i{}; i<8; ++i) {params.push_back(eg_rook_fmobility[i]);}
    for (int i{}; i<15; ++i) {params.push_back(eg_queen_fmobility[i]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_passed[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_passed_free[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_doubled[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_isolated[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_supported[sq]);}
    for (int sq{}; sq<8; ++sq) {params.push_back(eg_phalanx[sq]);}
    params.push_back(eg_bishop_pair);
    while ((fin >> tokens[0]) && (fin >> tokens[1]) && (fin >> tokens[2]) && (fin >> tokens[3]) && (fin >> tokens[4]) && (fin >> tokens[5]) && (fin >> tokens[6])) {
        test_set.push_back(Data{});
        test_set.back().load_fen(params, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        fin >> tokens[7];
        if (tokens[7] == "\"1-0\";") {
            test_set.back().score = 1;
            wdl_data[0][test_set.back().gp]++;
        } else if (tokens[7] == "\"0-1\";") {
            test_set.back().score = 0;
            wdl_data[2][test_set.back().gp]++;
        } else {
            test_set.back().score = 0.5;
            wdl_data[1][test_set.back().gp]++;
            if constexpr (no_draws) {wdl_data[1][test_set.back().gp]--; test_set.pop_back(); continue;}
        }
        if constexpr (prejudice) if (abs(sigmoid(test_set.back().eval(params)) - test_set.back().score) > prejudice_margin) {test_set.pop_back(); continue;}
        if (test_set.size() >= num_pos) break;
        if (test_set.size() % 1000000 == 0) {
            cout << test_set.size() << " positions parsed in " << timer.elapsed() << "s" << endl;
        }
    }
    /*while ((fin1 >> tokens[0]) && (fin1 >> tokens[1]) && (fin1 >> tokens[2]) && (fin1 >> tokens[3]) && (fin1 >> tokens[4]) && (fin1 >> tokens[5]) && (fin1 >> tokens[6])) {
        test_set.push_back(Data{});
        test_set.back().load_fen(params, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        fin1 >> tokens[7];
        if (tokens[7] == "\"1-0\";") {
            test_set.back().score = 1;
            ++num_wins;
        } else if (tokens[7] == "\"0-1\";") {
            test_set.back().score = 0;
            ++num_losses;
        } else {
            test_set.back().score = 0.5;
            ++num_draws;
            if constexpr (no_draws) {test_set.pop_back(); --num_draws; continue;}
        }
        if constexpr (prejudice) if (abs(sigmoid(test_set.back().eval(params)) - test_set.back().score) > prejudice_margin) {test_set.pop_back(); continue;}
        if (test_set.size() % 1000000 == 0) {
            cout << test_set.size() << " positions parsed in " << timer.elapsed() << "s" << endl;
        }
    }
    while ((fin2 >> tokens[0]) && (fin2 >> tokens[1]) && (fin2 >> tokens[2]) && (fin2 >> tokens[3]) && (fin2 >> tokens[4]) && (fin2 >> tokens[5]) && (fin2 >> tokens[6])) {
        test_set.push_back(Data{});
        test_set.back().load_fen(params, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        fin2 >> tokens[7];
        if (tokens[7] == "\"1-0\";") {
            test_set.back().score = 1;
            ++num_wins;
        } else if (tokens[7] == "\"0-1\";") {
            test_set.back().score = 0;
            ++num_losses;
        } else {
            test_set.back().score = 0.5;
            ++num_draws;
            if constexpr (no_draws) {test_set.pop_back(); --num_draws; continue;}
        }
        if constexpr (prejudice) if (abs(sigmoid(test_set.back().eval(params)) - test_set.back().score) > prejudice_margin) {test_set.pop_back(); continue;}
        if (test_set.size() % 1000000 == 0) {
            cout << test_set.size() << " positions parsed in " << timer.elapsed() << "s" << endl;
        }
    }*/
    /*while ((fin3 >> tokens[0]) && (fin3 >> tokens[1]) && (fin3 >> tokens[2]) && (fin3 >> tokens[3]) && (fin3 >> tokens[4]) && (fin3 >> tokens[5]) && (fin3 >> tokens[6])) {
        test_set.push_back(Data{});
        test_set.back().load_fen(params, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        fin3 >> tokens[7];
        if (tokens[7] == "\"1-0\";") {
            test_set.back().score = 1;
            ++num_wins;
        } else if (tokens[7] == "\"0-1\";") {
            test_set.back().score = 0;
            ++num_losses;
        } else {
            test_set.back().score = 0.5;
            ++num_draws;
            if constexpr (no_draws) {test_set.pop_back(); --num_draws; continue;}
        }
        if constexpr (prejudice) if (abs(sigmoid(test_set.back().eval(params)) - test_set.back().score) > prejudice_margin) {test_set.pop_back(); continue;}
        if (test_set.size() % 1000000 == 0) {
            cout << test_set.size() << " positions parsed in " << timer.elapsed() << "s" << endl;
        }
    }*/
    cout << "position parsing finished successfully, " << test_set.size() << " positions parsed in " << timer.elapsed() << "s" << endl;
    cout << "gp           w           d           l\n";
    for (int i{24}; i>=0; --i) cout << setw(2) << i << setw(12) << wdl_data[0][i] << setw(12) << wdl_data[1][i] << setw(12) << wdl_data[2][i] << endl;
    num_pos = min(num_pos, static_cast<int>(test_set.size()));
    while (test_set.size() > num_pos) {
        test_set.pop_back();
    }
    /*for (k=0.0025; k<0.01; k += 0.0001) {
        cout << "k: " << k << " e: " << e(params) << endl;
    }
    cin >> k;*/
    vector<double> final_params = gradient_descent(params);
    /*for (int i{}; i < final_params.size(); ++i) {
        fout << static_cast<int>(final_params[i]) << ",";
        if (i % 64 == 63) fout << '\n';
        else fout << ' ';
    }
    fout << flush;
    cin >> command;
    return 0;*/
    for (int i{}; i < king_len; ++i) {
        fout << static_cast<int>(final_params[i]) /*-mg_value[(i)/64]*/ << ",";
        if (i % 64 == 63) fout << '\n';
        else fout << ' ';
    }
    for (int i{king_len}; i < king_len + mob_len; ++i) {
        fout << static_cast<int>(final_params[i]) << ",";
        if (i - king_len == 101) fout << '\n';
        else fout << ' ';
    }
    for (int i{king_len + mob_len}; i < king_len + mob_len + pawn_len; ++i) {
        fout << static_cast<int>(final_params[i]) << ",";
        if ((i - king_len - mob_len) % 8 == 7) fout << '\n';
        else fout << ' ';
    }
    fout << static_cast<int>(final_params[king_len + mob_len + pawn_len]) << "," << '\n';
    cout << '\n';
    for (int i{half}; i < half + king_len; ++i) {
        fout << static_cast<int>(final_params[i]) /*-eg_value[(i - half)/64]*/ << ",";
        if ((i - half) % 64 == 63) fout << '\n';
        else fout << ' ';
    }
    for (int i{half + king_len}; i < half + king_len + mob_len; ++i) {
        fout << static_cast<int>(final_params[i]) << ",";
        if (i - half - king_len == 101) fout << '\n';
        else fout << ' ';
    }
    for (int i{half + king_len + mob_len}; i < half + king_len + mob_len + pawn_len; ++i) {
        fout << static_cast<int>(final_params[i]) << ",";
        if ((i - half - king_len - mob_len) % 8 == 7) fout << '\n';
        else fout << ' ';
    }
    fout << static_cast<int>(final_params[half + king_len + mob_len + pawn_len]) << "," << '\n';
    fout << flush;
    cin >> command;
    return 0;
}
