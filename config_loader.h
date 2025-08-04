#ifndef CONFIGLOADERH
#define CONFIGLOADERH

#include "oraora_demo_rng.h"
struct ConfigJson
{
	// [price][Json]
	map<int,Json::Value> config; 
	map<int,Config> config_rng; 
};
struct GamesConfigLoader
{

public:
	//[game_id][ConfigJson]
	map<int,ConfigJson> games; 
	GamesConfigLoader();
	int get_bet_and_win(_bet &b,
							string &bet_str);
	Config & get_config_rng(int game_id, float rtp, int balance, int price, int bet_count);
private:
int load_rng_config(int game_id);
	Json::Value &get_config(int game_id,int price);
	int load(int game_id, int price);
	int gen_bet_aladdin(Json::Value &val, string &bet_str);
	int gen_bet_tripler(Json::Value &val, string &bet_str);
	int gen_bet_altyn(Json::Value &val, string &bet_str);
	int gen_bet_cleopatra(Json::Value &val, string &bet_str);
	int gen_bet_mega_casino(Json::Value &val, string &bet_str);
};

#define ALADDIN_GAME_ID 2116U
#define TAGA_GAME_ID 2117
#define THREEDIAMONDS_GAME_ID 2118 
#define MEGACASINO_GAME_ID 2119
#define MAGICLAMP_GAME_ID 2120
#define ALTYNKUN_GAME_ID 2121U
#define CLEOPATRA_GAME_ID 2122
#define EGYPTSECRETS_GAME_ID 2123 
#define MAX_BET_SIZE 6
#define MAX_MASK_CASINO1 4
#define MAX_MASK_CASINO2 3
#define MAX_MASK_CASINO3 3
#define MAX_MASK_CASINO4 4
#define MAX_BET_TRIPLER_SIZE 10
#define MAX_BET_CLEO_SIZE 8
#define MAX_MASK_SIZE 2
#define MAX_MASK_TRIPLER_SIZE 3
#define MAX_MASK_CLEO_SIZE 8
#define MAX_MASK_ALTYN_SIZE 4
#define MAX_BET_ALTYN_SIZE MAX_MASK_ALTYN_SIZE 
#define MAX_BONUS_SIZE 5
#define MAX_FILE_PATH_LEN 2048
#endif
