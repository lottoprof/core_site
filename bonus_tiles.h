#ifndef _BONUSTILESH
#define _BONUSTILESH
#include <string.h>
#include <string>
#include "orakel.h"

/************************
*defines area
************************/

#define SATTYJULDYZ_WIN_LETTERS_COUNT 11
#define LETTERS_WIN_MULT 500
#define CHERRY_MAX_COUNT 16
#define ESTRELLA_MAX_COUNT 17
using namespace std;
int get_letters_tile(redisContext *r_ctx,
		int sz_user_id, int game_id, 
		int tile_number, int &letter_value);
int set_letters_tile(redisContext *r_ctx,
		int sz_user_id, int game_id, 
		int tile_number, int &letter_value);
int reset_letters(redisContext *r_ctx,
	string &session, int price);
int set_and_check_letters(redisContext *r_ctx,
string &session, vector<_bet> &b, int &bonus_amount, 
string &_letters);
int set_and_check_suns(redisContext *r_ctx,
	string &session, vector<_bet> &bets,
	int &bonus_amount,
  int &bonus_counter_1) ;
int check_letters(redisContext *r_ctx,
	string &session, int price, int &bonus_amount);
int set_and_check_stars(redisContext *r_ctx,
	string &session, vector<_bet> &bets, int &bonus_amount,int &bonus_counter_1, int &bonus_counter_2);
int get_bonus_symbols(string s_session,int &symbol1,int &symbol2);
int set_bonus_symbols(string s_session,int &symbol1,int &symbol2);
int get_bonus_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int price, int tile, int &count);
int set_bonus_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int price, int tile, int count);
int get_stars_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int price, int &stars);
int set_stars_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int price, int stars);
int check_stars(redisContext *r_ctx,
	string &session, int price, int &bonus_amount);
int reset_stars(redisContext *r_ctx,
	string &session, int price);
int update_stars_for_sale(redisContext *r_ctx,
	string &session, int price);
int start_stars_for_sale(redisContext *r_ctx,
	string &session, int price);
int check_stars_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter);

int get_all_fruit_n_ice_tile_count(redisContext *r_ctx,string &s_session, string &out_json);
int get_all_chuckcha_tile_count(redisContext *r_ctx,string &s_session, string &out_json);
int get_all_lemon_letters_count(redisContext *r_ctx,string &s_session, string &out_json);
int set_and_check_fishes(redisContext *r_ctx,
	string &session, vector<_bet> &bets,
	int &bonus_amount,
  int &bonus_counter_1);
int check_fishes(redisContext *r_ctx,
	string &session, int price, int &bonus_amount);
int start_fishes_for_sale(redisContext *r_ctx,
	string &session, int price);
int reset_fishes(redisContext *r_ctx,
	string &session, int price);
int check_fishes_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter);
int update_fishes_for_sale(redisContext *r_ctx,
	string &session, int price);
int reset_suns(redisContext *r_ctx,
	string &session, int price);
int start_suns_for_sale(redisContext *r_ctx,
	string &session, int price);
int update_suns_for_sale(redisContext *r_ctx,
	string &session, int price);
int check_suns_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter);

#endif
