#ifndef ORAORAH
#define ORAORAH

#define _CONSOLE_DEBUG 1

#define REQUEST_POST "POST"
#define REQUEST_POST_LEN sizeof(REQUEST_POST)
#define REQUEST_GET "GET"
#define REQUEST_GET_LEN sizeof(REQUEST_GET) 

// ADMIN
extern Json::Value get_user_by_id(redisContext *c, uint32_t user_id);
#define GET_USER_BY_ID_REQUEST_ADMIN "/admin/get_user_by_id"
#define GET_USER_BY_ID_REQUEST_ADMIN_LEN sizeof(GET_USER_BY_ID_REQUEST_ADMIN)

extern Json::Value get_user_list(redisContext *c, uint32_t parent_id);
#define GET_USER_LIST_REQUEST_ADMIN "/admin/get_user_list"
#define GET_USER_LIST_REQUEST_ADMIN_LEN sizeof(GET_USER_LIST_REQUEST_ADMIN)

extern Json::Value get_user_list_by_time(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_USER_LIST_BY_TIME_REQUEST_ADMIN "/admin/get_user_list_by_time"
#define GET_USER_LIST_BY_TIME_REQUEST_ADMIN_LEN sizeof(GET_USER_LIST_BY_TIME_REQUEST_ADMIN)

extern Json::Value get_user_list_by_geo(redisContext *c, uint32_t parent_id, uint32_t geo_id);
#define GET_USER_LIST_BY_GEO_REQUEST_ADMIN "/admin/get_user_list_by_geo"
#define GET_USER_LIST_BY_GEO_REQUEST_ADMIN_LEN sizeof(GET_USER_LIST_BY_GEO_REQUEST_ADMIN)

extern Json::Value get_user_list_by_channel(redisContext *c, uint32_t parent_id, uint32_t channel_id);
#define GET_USER_LIST_BY_CHANNEL_REQUEST_ADMIN "/admin/get_user_list_by_channel"
#define GET_USER_LIST_BY_CHANNEL_REQUEST_ADMIN_LEN sizeof(GET_USER_LIST_BY_CHANNEL_REQUEST_ADMIN)

extern Json::Value get_user_role(redisContext *c, uint32_t user_id);
#define GET_USER_ROLE_REQUEST_ADMIN "/admin/get_user_role"
#define GET_USER_ROLE_REQUEST_ADMIN_LEN sizeof(GET_USER_ROLE_REQUEST_ADMIN)

extern void set_role(redisContext *c, uint32_t user_id, uint32_t role);
#define SET_ROLE_REQUEST_ADMIN "/admin/set_role"
#define SET_ROLE_REQUEST_ADMIN_LEN sizeof(SET_ROLE_REQUEST_ADMIN)

extern void update_user_channel(redisContext *c, uint32_t user_id, uint32_t channel_id);
#define UPDATE_USER_CHANNEL_REQUEST_ADMIN "/admin/update_user_channel"
#define UPDATE_USER_CHANNEL_REQUEST_ADMIN_LEN sizeof(UPDATE_USER_CHANNEL_REQUEST_ADMIN)

extern void update_user_parent(redisContext *c, uint32_t user_id, uint32_t parent_id);
#define UPDATE_USER_PARENT_REQUEST_ADMIN "/admin/update_user_parent"
#define UPDATE_USER_PARENT_REQUEST_ADMIN_LEN sizeof(UPDATE_USER_PARENT_REQUEST_ADMIN)

extern Json::Value get_user_status(redisContext *c, uint32_t user_id);
#define GET_USER_STATUS_REQUEST_ADMIN "/admin/get_user_status"
#define GET_USER_STATUS_REQUEST_ADMIN_LEN sizeof(GET_USER_STATUS_REQUEST_ADMIN)

extern void set_user_status(redisContext *c, uint32_t user_id, uint32_t status);
#define SET_USER_STATUS_REQUEST_ADMIN "/admin/set_user_status"
#define SET_USER_STATUS_REQUEST_ADMIN_LEN sizeof(SET_USER_STATUS_REQUEST_ADMIN)

extern Json::Value get_user_sales(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds);
#define GET_USER_SALES_REQUEST_ADMIN "/admin/get_user_sales"
#define GET_USER_SALES_REQUEST_ADMIN_LEN sizeof(GET_USER_SALES_REQUEST_ADMIN)

extern Json::Value get_user_payin(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds);
#define GET_USER_PAYIN_REQUEST_ADMIN "/admin/get_user_payin"
#define GET_USER_PAYIN_REQUEST_ADMIN_LEN sizeof(GET_USER_PAYIN_REQUEST_ADMIN)

extern Json::Value get_user_payout(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds);
#define GET_USER_PAYOUT_REQUEST_ADMIN "/admin/get_user_payout"
#define GET_USER_PAYOUT_REQUEST_ADMIN_LEN sizeof(GET_USER_PAYOUT_REQUEST_ADMIN)

extern Json::Value get_parent_sales(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_SALES_REQUEST_ADMIN "/admin/get_parent_sales"
#define GET_PARENT_SALES_REQUEST_ADMIN_LEN sizeof(GET_PARENT_SALES_REQUEST_ADMIN)

extern Json::Value get_parent_payin(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_PAYIN_REQUEST_ADMIN "/admin/get_parent_payin"
#define GET_PARENT_PAYIN_REQUEST_ADMIN_LEN sizeof(GET_PARENT_PAYIN_REQUEST_ADMIN)

extern Json::Value get_parent_payout(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_PAYOUT_REQUEST_ADMIN "/admin/get_parent_payout"
#define GET_PARENT_PAYOUT_REQUEST_ADMIN_LEN sizeof(GET_PARENT_PAYOUT_REQUEST_ADMIN)

extern Json::Value get_parent_sales_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_SALES_PER_USER_REQUEST_ADMIN "/admin/get_parent_sales_per_user"
#define GET_PARENT_SALES_PER_USER_REQUEST_ADMIN_LEN sizeof(GET_PARENT_SALES_PER_USER_REQUEST_ADMIN)

extern Json::Value get_parent_payin_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_PAYIN_PER_USER_REQUEST_ADMIN "/admin/get_parent_payin_per_user"
#define GET_PARENT_PAYIN_PER_USER_REQUEST_ADMIN_LEN sizeof(GET_PARENT_PAYIN_PER_USER_REQUEST_ADMIN)

extern Json::Value get_parent_payout_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds);
#define GET_PARENT_PAYOUT_PER_USER_REQUEST_ADMIN "/admin/get_parent_payout_per_user"
#define GET_PARENT_PAYOUT_PER_USER_REQUEST_ADMIN_LEN sizeof(GET_PARENT_PAYOUT_PER_USER_REQUEST_ADMIN)

#define TEST_WD_TG_URL "/tg-withdrawal"
// ---

#define MAX_BET_COUNT_PER_REQUEST 300

#define THREAD_COUNT 3
#define SOCKET_PATH "/dev/shm/oraora.socket"
#define ORAORA_USER "oraora"



#define PLATFORM_URL "https://rpo.logycom.kz/tm/threemen.dll/interface2/"
#define PLATFORM_BET PLATFORM_URL "bet"
#define PLATFORM_BALANCE PLATFORM_URL "balance"
#define PLATFORM_SESSION_STOP PLATFORM_URL "session-stop"
#define PLATFORM_REFUND PLATFORM_URL "refund"
#define PLATFORM_WIN PLATFORM_URL "win"
 
#define MIN_SESSION_LEN 20

/*! @brief key for signing requests to logycom
*
*/

#define MERCHANT_ID "a7cd24bb6123cb44173c0a178511274b12e34fc06e9b8ce5769dbc197098183a"
#define MERCHANT_KEY "dda0e7f5a0a29583e4e1ff7e03cc8459694146e4f2b4ceaa9a17bd3f4f8bc436"
#define MERCHANT_KEY_LEN sizeof(MERCHANT_KEY)
#define MAX_MERCHANT_KEY_LEN 255
#define REQUEST_URL  "https://rpo.logycom.kz/threemen/threemen.dll/interface2"

#define DEF_CURLOPT_TIMEOUT 20

#define GAMEPROVIDER_SZKZ 1 
#define GAMEPROVIDER_SWERTE 2

int parse_player_cn(char *player, string &s_player);
int finish_request_parse_err(FCGX_Request& request, const char* json_error, int code);
int finish_request_parse_err(FCGX_Request& request, const char* json_error);

int bet_add_sync_list(redisContext *r_ctx,const char *session,_bet &b);
int session_add_sync_list(redisContext *r_ctx,const char *session);
int do_request (string &request, string &out,redisContext *r_ctx);
int do_request_log_redis(redisContext *r_ctx, string &prefix, uint64_t ts_start,
	uint64_t ts_end);
int do_request_bets_win(string &session, string &player_id, vector<_bet> &b, unsigned int &balance, string &error_message,redisContext *r_ctx);
int do_request_bets_win_swerte(string &session, string &player_id, vector<_bet> &b,unsigned int &balance, string &error_message,redisContext *r_ctx, int game_id);
int do_request_bets_win_sz(string &session, string &player_id, vector<_bet> &b,unsigned int &balance, string &error_message,redisContext *r_ctx, int game_id);
int get_game_provider(int game_id, int &game_provider);
int do_request_bets(redisContext *r_ctx,
		string &session, string &player_id, 
		vector<_bet> &b, unsigned int &balance, string &error_message);
int do_request_bets_swerte(redisContext *r_ctx,
                    string &session,int game_id, string &player_id,
                    vector<_bet> &b,unsigned int &balance,
                    string &error_message);
int do_request_bets_szkz(redisContext *r_ctx,
                    string &session,int game_id, string &player_id,
                    vector<_bet> &b,unsigned int &balance,
                    string &error_message);

int do_request_balance(string &session, string &player_id, unsigned int &balance);
int get_balance(redisContext *r_ctx,const char* session, unsigned int &balance, int &demo, string &player);
int get_balance_senden(redisContext *r_ctx,const char *player,
											unsigned int &balance);
int get_balance_senden(redisContext *r_ctx,const char *session,
											unsigned int &balance,unsigned int &bonus_balance, 
											int &demo, string &player, 
											unsigned long long &gross_bet, unsigned long long &gross_win,
										unsigned long long &gross_bet_bonus, unsigned long long &gross_win_bonus

											);
int get_balance_senden(redisContext *r_ctx,const char *session,
											unsigned int &balance, string &player);

int rgs_get_amount_from_txid(redisContext *r_ctx, const char* player,
	 const char * tx_id, int &bet_amount);
int rgs_get_amount_from_round(redisContext *r_ctx, const char* player_id,
	 const char * round_id, int &bet_amount);
int rgs_get_win_from_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int &bet_amount);
int rgs_set_txid_win(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					 int bet_win);
int rgs_check_rolled_round(redisContext *r_ctx, const char *player_id
			,const char *round_id);
int rgs_set_rolled_round(redisContext *r_ctx, const char *player_id
			,const char *round_id);
int rgs_check_round_id(redisContext *r_ctx, string &player,const char* round_id);
int set_balance(redisContext *r_ctx,const char* session, unsigned int &balance);
int rgs_set_txid_roud_id(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					const char *round_id, int bet_amount);

int set_balance_senden(redisContext *r_ctx,const char *session,const char *tx_id,unsigned int &balance);
int set_balance_senden_player(redisContext *r_ctx,const char *player,unsigned int &balance);
int set_balance_senden_player_notxid(redisContext *r_ctx,const char *player,unsigned int &balance);
int set_balance_senden(redisContext *r_ctx,const char *session,unsigned int &balance);
int set_balance_senden(redisContext *r_ctx,const char *session,unsigned int &balance, unsigned int &bonus_balance,unsigned long long gross_bet, unsigned long long gross_win,
						unsigned long long gross_bet_bonus, unsigned long long gross_win_bonus);

int get_player_data(redisContext *r_ctx,const char *player, string &out);
int get_user_from_session(redisContext *r_ctx, const char *session, string &out);
int get_status_session(redisContext *r_ctx,const char *session, int &status, string &return_url, string &refill_url, string &action_url, string &language, string &myparam, string &player_name, int &demo, string &register_url);
int get_status_session_www(redisContext *r_ctx,const char *session,
                       string &return_url,
                       string &refill_url, string &action_url,
                       string &language, string &player_name,string &register_url);
int set_status_session(redisContext *r_ctx,const char *session, int status);
int renew_session(redisContext *r_ctx,const char *session, const char *session_new, string &url);
int get_player_session(redisContext *r_ctx,const char *session, string &player);
int get_game_id(redisContext *r_ctx, string &session, string &game_id);
int get_banner(redisContext *r_ctx, string &out_json);
int get_game_path(redisContext *r_ctx, const int game_id, string &game_path);
int get_draw(redisContext *r_ctx,const char *session,int game_id, string &out_json);
int get_current_draw(redisContext *r_ctx,int game_id, int &draw_id);
int paybiz_write_callback(redisContext *r_ctx,string &hmset);
int buy_bet_bingo(redisContext *r_ctx,const char *session,
						const char* number, int &price, int game_id, int draw_id);
int buy_bet(redisContext *r_ctx,const char *session,
								 const char* number,const char *mask,
								 const int price, int game_id, 
								 const int draw_id);
int generate_token(redisContext *r_ctx, const char* user, int token_gw = 0);
int generate_session4www(redisContext *r_ctx, const char* user);
int generate_session4www_tmp_with_balance(redisContext *r_ctx, const char* user);

int generate_session4www_tmp_with_balanceV2(redisContext *r_ctx, const char* user, 
string &session);
int register_bet(redisContext *r_ctx,const char *session,
									uint64_t number, char *mask, int price, int game_id, int draw);
int get_my_bets(redisContext *r_ctx,const char *session,
								 int game_id, int draw_id, string &out_json);
int get_all_my_bets(redisContext *r_ctx,const char *session, string &out_json);
int get_game_bonuses(redisContext *r_ctx, string game_id, string &bonuses);
int get_game_prices(redisContext *r_ctx, string game_id, string &prices);
int register_user(redisContext *r_ctx, const char* user);
int register_user_www(redisContext *r_ctx, const char* user, int sms_gate = 0);
int generate_p12(redisContext *r_ctx, const char* user);
int get_user_p12(redisContext *r_ctx, const char* user, string token, string &secret_path);	
int get_user_sesson_after_register(redisContext *r_ctx, const char* user, 
								string token, string &session);
int json_to_int_patch_hujatch(Json::Value &v);
void add_headers(FCGX_Request &request, string &out);
int bet_to_json(_bet &b, char *out, int size);
int bet_to_json_alladin(_bet &b, char *out, int size);
int get_games(redisContext *r_ctx, string &out_json);
int get_numbers_hot(const char* session,int game_id, string &out);
int get_numbers_cold(const char* session,int game_id, string &out);
int check_redis(redisContext **r_ctx);
int bet_win_add_sync_list(redisContext *r_ctx,const char *session, _bet &b);
int sendTelegramMessage(const std::string& botToken, const std::string& chatId, const std::string& message);
int generate_token_www(redisContext *r_ctx, const char* user, string &token_s);
int get_user_sesson_after_register_tg(redisContext *r_ctx, const char* user,
                                   string token, string &session);
int get_tgid_from_phone(redisContext *r_ctx, const char* phone,
                                    string &tg_id);

int set_player_data(redisContext *r_ctx,
			const char *player,
							const char *name,
							const char *family,
							const char *patronymic,
							const char *phone,
							const char *email,
							const char *birth,
							const char *ts,
							const char *document,
							const char *issue_date,
							const char *code,
							const char *address
);
int p2p_write_withdrawal(redisContext *r_ctx,map<string,string> &post_vals);
int p2p_write_payment(redisContext *r_ctx,map<string,string> &post_vals);
					int p2p_create_invoice(redisContext *r_ctx,const char *player, int amount, 
								const char* currency, const char *gw,const char *session, uint64_t &invoice);
int p2p_create_withdrawal(redisContext *r_ctx,const char *player, int amount, 
								const char* currency, const char *gw,const char *session, uint64_t &invoice);
int p2p_generate_form(const char *player, const int amount, 
								const char* currency, const char *gw, const uint64_t invoice, string &out_form);
int p2p_generate_form_withdrowal(const char *player,const int amount, 
								const char* currency, const char *gw, const uint64_t invoice,
								string &out_form);
int rgs_set_refunded_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int amount);
int rgs_is_refunded_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int &refunded);

int p2p_generate_form_withdrowal_tg(
    const char *player,
    const int amount,
    const char* currency,
    const char *gw,
    const uint64_t invoice,
    string &out_form
);
int finish_request_parse_sgi_err(FCGX_Request& request, int code);
int finish_request_parse_sgi_err_bet(FCGX_Request& request, int code);
int finish_request_parse_sgi_err_balance(FCGX_Request& request, int code);
int finish_request_parse_sgi_err_win(FCGX_Request& request, int code);
int  rgs_get_session(redisContext *r_ctx,const char *session,
										 string &player, string &username, int & balance,
			string &first_name,string &last_name);
			int rgs_check_txId(redisContext *r_ctx, string &player,const char* tx_id);
int rgs_add_roud_id(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					const char *round_id);
int get_tgid_and_token(redisContext *r_ctx,const char *session,
											string &tg_id,
											string &token);
int get_user_sesson_after_registerV2(redisContext *r_ctx, const char* user,
                                   string token, string &session);

/***********************
 P2P payme defines !!!!
***********************/
#define PAYME_TEST_MERCHANT_ID 298
#define PAYME_TEST_RESP_URL "https://senden.pro/static/index.html"
#define PAYME_TEST_CALLBACK_URL "https://senden.pro/p2p_process_callback"
#define PAYME_TEST_CALLBACK_WD_URL "https://senden.pro/p2p_process_callback_wd"
#define PAYME_TEST_API5KEY "XWJ1IV73JWum3yv8323mzmd0ab1vUYSuMVJImr9qE1ANBXYVeLiVkIzH9cs3ZwIsE3dcxcWhTnKLpPGj7uj2hGL4m6cQTje0RyQKiZsAnMxAiat5U3Rva1fn5fohX00Q"
#define PAYME_TEST_URL "https://sandbox.p2p-transfer.live/payment/invoice/post/add"
#define PAYME_TEST_WD_URL "https://sandbox.p2p-transfer.live/payment/withdrawal/post/add"
//**********************


//#define API_URL "https://oraora.winlottery.site"
#define ORA_HOST "games-stage.kube.indev.expert"
#define API_URL SITE_URL
#define RETURN_URL API_URL "/return"
#define REFIL_URL  API_URL "/refill"
#define ACTION_URL  API_URL "/action"
#define REGISTER_URL  API_URL "/register"

#define BET_URL  API_URL "/bet"
#define BALANCE_URL  API_URL "/balance"
#define START_URL  API_URL "/start"

/*! Session statuses
*		
*/
#define SESSION_UNDEF NULL
#define SESSION_INITED 0
#define SESSION_STARTED 1
#define SESSION_STOPPED 2


/*! Game ID
*		
*/

#define ALADDIN_GAME_ID 2116
#define TAGA_GAME_ID 2117
#define THREEDIAMONDS_GAME_ID 2118 
#define MEGACASINO_GAME_ID 2119
#define MAGICLAMP_GAME_ID 2120
#define ALTYNKUN_GAME_ID 2121
#define CLEOPATRA_GAME_ID 2122
#define EGYPTSECRETS_GAME_ID 2123 


#if CRYPTOPP_VERSION >564
	#define byte CryptoPP::byte
#endif

#define DEFAULT_TELEGRAM_GATE 0
#define DEFAULT_SMS_TWILLO 1
#define PHP_TWILLO_GW "twillo"
#define PHP_TWILLO_GW_LEN sizeof(PHP_TWILLO_GW)
#define DEFAULT_TOKEN_GW_TG_LIST "users2tg"
#define DEFAULT_TOKEN_GW_SMS_TWILLO_LIST "users2twillo"
#endif
