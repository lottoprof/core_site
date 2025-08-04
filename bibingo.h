#include <vector>
#include <string>
#ifndef BIBINGOH
#define BIBINGOH

using namespace std;
#define MAX_MASK 5
#define MAX_LINES 3
#define MAX_CARDS 6

struct Line
{
	int mask[MAX_MASK]; // 
// *****************
// draw playing 
	int mult;
	int win;
	int win_mask[MAX_MASK]; 
	int win_size; // 
	int win_draw_step; // fix win when line feeling
// *****************
	Line();
	~Line();
	int set_line_index(int index){line_index= index;}
	int get_line_index(){return line_index;}
	int set_card_index(int index){card_index = index;}
	int get_card_index(){return card_index;}
private:
	int line_index;
	int card_index;
};

struct Card
{
	Line lines[MAX_LINES];
	int win_lines[MAX_LINES];
	// win_lines[0] = win;
	// win_lines[1] = 0;

// *****************
// draw playing 
	int win;
	int mult;
	int is_bingo;
	int win_draw_step;
	int total_win_lines;
	int win_line_1;
	int win_line_2;
	int win_bingo;
// *****************
	Card();
};
class Bibingo
{
public:
	Bibingo();
	Bibingo(const char* buf_in, const int len,
					const uint64_t number, const unsigned int price,
			 		uint32_t _draw);
	~Bibingo();
	void dump_cards();
	void dump_json();
	int import_json(char *buf, const int size);
	int copy_json(char*, int);
	int copy_mask(char*, int);
	int copy_win_mask(char* buf, int size);
	uint64_t get_number(){return number;}
	uint64_t get_price(){return price;}
	uint64_t set_price(int price){this->price=price;}
	uint32_t get_draw(){return draw;}
	uint32_t set_draw(int draw){this->draw=draw;}
	int get_win();
	int draw_step(int ball);
	int price;
	int draw;
private:
	Card cards[MAX_CARDS];
	vector<int> v_mask;
	uint64_t number;
	int huingo();
	int examine();
	int examine_line();
	int process_card(Card &card, const int ball);
	int process_line(Line &line, const int ball);
	void clear();
	int total_win;
	int step;
};

#endif
