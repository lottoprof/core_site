#ifndef ORAORA_RNG_DEMO_H
#define ORAORA_RNG_DEMO_H
#include  <fstream>
#include <json/json.h>
  volatile static const int PROBABILITY_MULTIPLIER = 1073741824;

int get_numbers_cold(const char* session,int game_id, string &out);

int get_bet_aksha_bar(int game_id, int price, _bet &b, int &error);
int get_numbers_hot(const char* session,int game_id, string &out);
int get_bet_fruit_and_ice(int game_id, int price, _bet &b, int &error);
int get_bet_chukcha(int game_id, int price, _bet &b, int &error);
int get_bet_frutoboom(int game_id, int price, _bet &b, int &error);
int get_bet_crazy_lemon(int game_id, int price, _bet &b, int &error);
int get_bet_lucky_queen_bonus(int game_id, int price, _bet &b, int &error);
int get_bet_lucky_queen(int game_id, int price, _bet &b, int &error);
int get_bet(int game_id, int price, _bet &b, int &error);
int get_winners_top( string &out);
int get_bet_poker(vector<string> &cards_mask,vector<string> &cards, string &result, int &win);
template <typename T>
class Point {
public:
    Point() = default;
    Point(T x, T y) : x(x), y(y) {}

    static Point<T> fromJsonArr(Json::Value arr) {
        if (!arr.isArray() || arr.size() != 2 || !arr[0].isNumeric() || !arr[1].isNumeric()) {
            throw std::runtime_error("Invalid JSON array for creating a point");
        }

        /// Todo change asInt to make it work with other types
        return Point<T>(arr[0].asInt(), arr[1].asInt());
    }

    Point<T> operator+(Point<T> other) {
        return Point<T>(x + other.x, y + other.y);
    }

    Point<T> operator-(Point<T> other) {
        return Point<T>(x - other.x, y - other.y);
    }

    /// Todo more operators

    T x;
    T y;
};


class Config {
public:
    int rows;
    int reels;
    int num_symbols;
    int** probability_matrix_int;
    Point<int>** win_lines;
    int num_win_lines;
    int win_line_length;
		int wild_number;
		int wild_order;
    std::vector<int> winning_matches;
    std::map<std::string, std::vector<double>> paytable;


    static Config from_file(const std::string& filename, float rtp) {
        Config self;

        std::ifstream ifs(filename);

        Json::Reader reader;
        Json::Value config;

        if (!reader.parse(ifs, config)) {
            printf("JSON parsing error.");

            std::exit(-1);
        }

        self.rows = config["initial_reel_matrix"]["rows"].asInt();
        self.reels = config["initial_reel_matrix"]["reels"].asInt();
        self.num_symbols = config["symbols"].asInt();

        static const std::string probability_matrix_names[] = {
            "matrix_probabilities_low",
            "matrix_probabilities_normal",
            "matrix_probabilities_high",
        };

        std::string probability_matrix_name;
				if (rtp > 0.95f)
				{
					probability_matrix_name = probability_matrix_names[0];
				}
				if (rtp > 0.9f && rtp < 0.95f)
				{
					probability_matrix_name = probability_matrix_names[1];
				}
				if (rtp < 0.9f)
				{
					probability_matrix_name = probability_matrix_names[2];
				}
				 
        //    probability_matrix_names[rtp < 0.9f ? 0 : (rtp > 0.95f ? 2 : 1)];

        self.probability_matrix_int = new int*[self.reels];

        for (int i = 0; i < self.reels; i++) {
            self.probability_matrix_int[i] = new int[self.num_symbols];

            for (int j = 0; j < self.num_symbols; j++) {
                self.probability_matrix_int[i][j] =
                    config[probability_matrix_name]["reel" + std::to_string(i + 1)][j].asFloat()
                    * PROBABILITY_MULTIPLIER;
            }
        }
				// set param from args
        self.num_win_lines = config["win_lines"].size();
			
        self.win_line_length = config["win_lines"]["line1"].size();
        self.win_lines = new Point<int>*[self.num_win_lines];
				self.wild_number = config["additional_symbols"]["Wild"].asInt();
				self.wild_order = config["additional_symbols"]["Wild_order"].asInt();
        for (int i = 0; i < self.num_win_lines; i++) {
            self.win_lines[i] = new Point<int>[self.reels];

            Json::Value win_line_json = config["win_lines"]["line" + std::to_string(i + 1)];

            for (int j = 0; j < self.win_line_length; j++) {
                self.win_lines[i][j] = Point<int>::fromJsonArr(win_line_json[j]);
            }
        }

        for (const auto& winning_match: config["winning_matches"]) {
            self.winning_matches.push_back(std::stoi(winning_match.asString()));
        }

        for (const auto& symbol_line_name: config["paytable"].getMemberNames()) {
            self.paytable[symbol_line_name] = {};

            for (const auto& coeff: config["paytable"][symbol_line_name]) {
                self.paytable[symbol_line_name].push_back(coeff.asFloat());
            }
        }

        return self;
    }
};
int slot_rng(Config &config, int bet_count, int game_id, int price,vector<_bet> &bb);
int get_bet_rng(Config& config, int game_id, int price, 
								vector<_bet> &bb,int bet_count, int &error);
#endif
