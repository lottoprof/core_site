#include <map>
#include <hiredis.h>
#include <fcgiapp.h>
#include <json/value.h>
#include <openssl/evp.h>
#include  <functional>
#include "orakel.h"
#include "oraora.h"
#include "config_loader.h"

#pragma once

#define RGS_ERR_OK 1
#define RGS_ERR_SESS_NOT_FOUND 2
#define RGS_ERR_SESS_EXIRED 3
#define RGS_ERR_PLAYER_ID 4
#define RGS_ERR_LOW_BALANCE 6
#define RGS_ERR_TRN_NOT_FOUND 7
#define RGS_ERR_TRN_EXISTS 8
#define RGS_ERR_GAME_NOT_FOUND 11
#define RGS_ERR_SIGN 12
#define RGS_ERR_TRN_ALREADY_ROLLED 14
#define RGS_ERR_OPERATOR_ID 15
#define RGS_ERR_CURRENCY_ID 16
#define RGS_ERR_PARAM_MISS 17
#define RGS_ERR_DATA 18
#define RGS_ERR_TRN_ALREADY_WON 20
#define RGS_ERR_UNAUTH 23
#define RGS_ERR_GAME_DISABLED 26
#define RGS_ERR_999 999



class Requests {
public:
    class Data {
    public:
        const char* ora_host;
        GamesConfigLoader* config_loader;
        redisContext *r_ctx;
        FCGX_Request* request;
        int redis_ready;
        EVP_PKEY* pkey;
        char* buf_in;
    };

    static Requests& instance() {
        static Requests instance;
        return instance;
    }

    int do_it(Data& data);

private:
    using RequestFunc = std::function<int(Data&)>;

    std::map<std::string, RequestFunc> funcs;

    Requests();
    ~Requests();

    int parse_json(Data& data, Json::Value& root);

    int request_bingo(Data& data);
    int request_banner(Data& data);
    int request_planogram(Data& data);
    int request_games(Data& data);
    int request_draws_info(Data& data);
    int request_draws_data(Data& data);
    int request_ping(Data& data);
    int request_ready(Data& data);
    int request_param(Data& data, const char* query_string);
    int request_alt_param(Data& data, const char* query_string);

    // JSON

    int request_init(Data& data);
    int request_get_winners_top(Data& data);
    int request_cold_numbers(Data& data);
    int request_hot_numbers(Data& data);
    int request_draw(Data& data);
    int request_balance_update(Data& data);
    int request_start(Data& data);
    int request_init_demo(Data& data);
    int request_videoplayer(Data& data);

    int request_get_user(Data& data);
    int request_edit_user(Data& data);
    int request_p2p_create_withdrowal(Data& data);
    int request_p2p_create_invoice(Data& data);
    int request_p2p_process_wd(Data& data);
    int request_p2p_process(Data& data);
    int request_p2p_process_paybiz(Data& data);
    int request_register_www(Data& data);
		int request_renew(Data& data);
    int request_tglogin(Data& data);
    int request_tgloginslot(Data& data);

    int request_register(Data& data);
    int request_balance(Data& data);
    int request_balance_and_bonus(Data& data);
    int request_getmybets(Data& data);
    int request_buybet(Data& data);
    int request_bet(Data& data);

		/*RGS methods*/

		int request_authenticate(Data& data);
		int	request_getbalance(Data& data);
		int	request_refund(Data& data);
		int rgs_request_bet(Data& data,Json::Value &) ;
		int rgs_request_win(Data &data) ;
};
int rgs_check_sign(string &a, string &b, string &sign);
	#define TIMEBUFSIZE 200
int sgi_get_time_format_string (string &str);
int rgs_get_sign(string &a, string &sign);
