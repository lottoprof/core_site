#include "requests.hpp"

#include <string>
#include <functional>
#include <syslog.h>
#include <pthread.h>
#include <chrono>
#include <cstdio>
#include <iomanip>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <cryptopp/base32.h>
#include <cryptopp/algparam.h>
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>

#include "bonus_tiles.h"
#include "oraora_errors.h"
#include "oraora_limits.h"
#include "oraora_demo_rng.h"
#include "games_id.h"
#include "paybiz.h"
#include "bibingo.h"
#include "gen_array/ShipTicketInfo.h"

#ifdef _CONSOLE_DEBUG
# define log_err(fmt, args...) fprintf(stderr, fmt "\n", ## args); syslog(LOG_ERR, fmt, ## args);
# define log_info(fmt, args...) fprintf(stdout, fmt "\n", ## args); syslog(LOG_INFO, fmt, ## args);
#else
# define log_err(fmt, args...) syslog(LOG_ERR, fmt, ## args);
# define log_info(fmt, args...) syslog(LOG_INFO, fmt, ## args);
#endif

// #define bind(funcname) std::bind(&Requests::funcname, this, std::placeholders::_1)
#define bind(funcname) [this] (Data& data) { return funcname(data); }

static const char* GAMES_REQUEST                 = "/games";
static const char* PLANOGRAM_REQUEST             = "/planogram";
static const char* DRAWS_INFO_REQUEST            = "/draws/info";
static const char* DRAWS_DATA_REQUEST            = "/draws/data";
static const char* INIT_REQUEST                  = "/games/init";
static const char* PING_REQUEST                  = "/ping";
static const char* READY_REQUEST                  = "/ready";
static const char* BUYBET_REQUEST                = "/buy_bets";
static const char* GETMYBETS_REQUEST             = "/get_my_bets";
static const char* BALANCE_UPDATE_REQUEST        = "/balance-update";
static const char* START_REQUEST                 = "/start";
static const char* INIT_DEMO_REQUEST             = "/games/init-demo";
static const char* GAMES_BINGO_REQUEST           = "/games-bingo";
static const char* BET_REQUEST                   = "/bet";
static const char* PLAY_BET_REQUEST              = "/play_bet";
static const char* P2P_PROCESS_REQUEST           = "/p2p_process_callback";
static const char* P2P_PROCESS_REQUEST_PAYBIZ    = "/paybiz/webhook";
static const char* P2P_PROCESS_REQUEST_WD        = "/p2p_process_callback_wd";
static const char* P2P_CREATE_INVOICE_REQUEST    = "/p2p_create_invoice";
static const char* P2P_CREATE_WITHDROWAL_REQUEST = "/p2p_create_withdrawal";
static const char* PARAM_REQUEST                 = "/param.js?";
static const char* PARAM_ALT_REQUEST             = "/param?session=";
static const char* BALANCE_REQUEST               = "/balance";
static const char* BALANCE_BONUS_REQUEST         = "/balance_and_bonus";
static const char* BANNER_TOP_REQUEST            = "/banner_main";
static const char* HOT_NUMBERS_REQUEST           = "/hot_numbers";
static const char* COLD_NUMBERS_REQUEST          = "/cold_numbers";
static const char* DRAW_REQUEST                  = "/draw";
static const char* REGISTER_REQUEST              = "/register_user";
static const char* REGISTER_WWW_REQUEST          = "/register_www";
static const char* RENEW_REQUEST          = "/renew";
static const char* TGLOGIN_WWW_REQUEST          = "/tglogin";
static const char* TGLOGINSLOT_WWW_REQUEST          = "/tgloginslot";
static const char* EDIT_USER_REQUEST             = "/edit_user";
static const char* GET_USER_REQUEST              = "/get_user";
static const char* GET_WINNERS_TOP_REQUEST       = "/winners_hot";
static const char* GET_VIDEOPLAYER       = "/ffffff";

/*************************
*	Galaxy games provider
*************************/

static const char* RGS_AUTHENTICATE = "/authenticate";
static const char* RGS_GETBALANCE   = "/getbalance";
static const char* RGS_REFUND       = "/refund";
static const char* RGS_WIN       = "/win";

// ***********************
void fcgx_put_json(FCGX_Request& request, const Json::Value& root) {
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);
    FCGX_PutS(root.toStyledString().c_str(), request.out);
    FCGX_PutS("\n\n", request.out);
}

void fcgx_put_json_ok(FCGX_Request& request) {
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);
    FCGX_PutS("{OK}", request.out);
    FCGX_PutS("\n\n", request.out);
}

void fcgx_put_str_json(FCGX_Request& request, const std::string& str) {
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);
    FCGX_PutS(str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);
}

Requests::Requests() {
    funcs[GAMES_BINGO_REQUEST] = bind(request_bingo);
    funcs[BANNER_TOP_REQUEST]  = bind(request_banner);
    funcs[PLANOGRAM_REQUEST]   = bind(request_planogram);
    funcs[GAMES_REQUEST]       = bind(request_games);
    funcs[DRAWS_INFO_REQUEST]  = bind(request_draws_info);
    funcs[DRAWS_DATA_REQUEST]  = bind(request_draws_data);
    funcs[PING_REQUEST]        = bind(request_ping);
    funcs[READY_REQUEST]        = bind(request_ready);
    // funcs[PARAM_REQUEST]       = request_param;
    // funcs[PARAM_ALT_REQUEST]   = request_alt_param;

    // JSON

    funcs[INIT_REQUEST]            = bind(request_init);
    funcs[GET_WINNERS_TOP_REQUEST] = bind(request_get_winners_top);
    funcs[COLD_NUMBERS_REQUEST]    = bind(request_cold_numbers);
    funcs[HOT_NUMBERS_REQUEST]     = bind(request_hot_numbers);
    funcs[DRAW_REQUEST]            = bind(request_draw);
    funcs[BALANCE_UPDATE_REQUEST]  = bind(request_balance_update);
    funcs[START_REQUEST]           = bind(request_start);
    funcs[INIT_DEMO_REQUEST]       = bind(request_init_demo);

    funcs[GET_USER_REQUEST]              = bind(request_get_user);
    funcs[EDIT_USER_REQUEST]             = bind(request_edit_user);
    funcs[P2P_CREATE_WITHDROWAL_REQUEST] = bind(request_p2p_create_withdrowal);
    funcs[P2P_CREATE_INVOICE_REQUEST]    = bind(request_p2p_create_invoice);
    funcs[P2P_PROCESS_REQUEST_WD]        = bind(request_p2p_process_wd);
    funcs[P2P_PROCESS_REQUEST]           = bind(request_p2p_process);
    funcs[P2P_PROCESS_REQUEST_PAYBIZ]    = bind(request_p2p_process_paybiz);
    funcs[RENEW_REQUEST]          = bind(request_renew);
    funcs[REGISTER_WWW_REQUEST]          = bind(request_register_www);
    funcs[TGLOGIN_WWW_REQUEST]          = bind(request_tglogin);
    funcs[TGLOGINSLOT_WWW_REQUEST]          = bind(request_tgloginslot);
    funcs[GET_VIDEOPLAYER]          = bind(request_videoplayer);

    funcs[REGISTER_REQUEST]  = bind(request_register);
    funcs[BALANCE_REQUEST]   = bind(request_balance);
    funcs[BALANCE_BONUS_REQUEST]   = bind(request_balance_and_bonus);
    funcs[GETMYBETS_REQUEST] = bind(request_getmybets);
    funcs[BUYBET_REQUEST]    = bind(request_buybet);
    funcs[BET_REQUEST]       = bind(request_bet);

		/*RGS BIND*/

    funcs[RGS_AUTHENTICATE] = bind(request_authenticate);
    funcs[RGS_GETBALANCE]   = bind(request_getbalance);
    funcs[RGS_REFUND]       = bind(request_refund);
    funcs[RGS_WIN]       = bind(rgs_request_win);

}

Requests::~Requests() = default;

int
Requests::do_it(Data& data) {
    int rc;
    std::string redis_err_msg = "";

    static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
    // static pthread_mutex_t bet_mutex = PTHREAD_MUTEX_INITIALIZER;

    //попробовать получить новый запрос
    //printf("Try to accept new request\n");
    int res = pthread_mutex_lock(&accept_mutex);
    if (res!=0 )
    {
        log_err("unable lock mutex reason: %s", strerror(errno));
    }
    rc = FCGX_Accept_r(data.request);
    res = pthread_mutex_unlock(&accept_mutex);
    if(res !=0)
    {
        log_err("unable unlock mutex reason: %s", strerror(errno));
        if (res == EDEADLK)
        {
            log_err("dead lock");
        }
    }

    if(rc < 0)
    {
        log_err("Can not accept new request");
        return 1;
    }
    //printf("request is accepted\n");
    char *method =  FCGX_GetParam("REQUEST_METHOD", data.request->envp);

    if (strncmp(method, REQUEST_POST, REQUEST_POST_LEN-1)!=0 &&
            strncmp(method, REQUEST_GET, REQUEST_GET_LEN-1)!=0)
    {
//finish
#if  1
        string out ="{\"method\":\"";
        out.append(method);
        out.append("\"}");

        fcgx_put_str_json(*data.request, out);
#endif
        return 0;
    }
    char *query_string = FCGX_GetParam("REQUEST_URI", data.request->envp);
    char *content_type = FCGX_GetParam("CONTENT_TYPE", data.request->envp);
    int query_len = strnlen(query_string,50);
    log_info("query_string:%s [%d] type:[%s]", query_string, query_len,content_type);

    std::string query_string2 = query_string;

    Json::Value root;

    res = 0;

    if (funcs.count(query_string2)) {
        res = funcs[query_string2](data);
    } else if (strncmp(query_string, PARAM_REQUEST, sizeof(PARAM_REQUEST) - 1) == 0) {
        res = request_param(data, query_string);
    } else if (strncmp(query_string, PARAM_ALT_REQUEST, sizeof(PARAM_ALT_REQUEST) - 1) == 0) {
        res = request_alt_param(data, query_string);
    }

    else if (strncmp(query_string, GET_USER_BY_ID_REQUEST_ADMIN, GET_USER_BY_ID_REQUEST_ADMIN_LEN) == 0) {
        //int res_parsing = reader.parse(buf_in,root);
        //if (res_parsing) {

        //Json::Value player_id_j = root["player_id"];
        int user_id = json_to_int_patch_hujatch(root["user_id"]);

        Json::Value jvalue = get_user_by_id(data.r_ctx, user_id);

        //jvalue["in"] = buf_in;
        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_LIST_REQUEST_ADMIN, GET_USER_LIST_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);

        Json::Value jvalue = get_user_list(data.r_ctx, parent_id);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_LIST_BY_TIME_REQUEST_ADMIN, GET_USER_LIST_BY_TIME_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_user_list_by_time(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_LIST_BY_GEO_REQUEST_ADMIN, GET_USER_LIST_BY_GEO_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int geo_id    = json_to_int_patch_hujatch(root["geo_id"]);

        Json::Value jvalue = get_user_list_by_geo(data.r_ctx, parent_id, geo_id);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_LIST_BY_CHANNEL_REQUEST_ADMIN, GET_USER_LIST_BY_CHANNEL_REQUEST_ADMIN_LEN) == 0) {
        int parent_id  = json_to_int_patch_hujatch(root["parent_id"]);
        int channel_id = json_to_int_patch_hujatch(root["channel_id"]);

        Json::Value jvalue = get_user_list_by_channel(data.r_ctx, parent_id, channel_id);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_ROLE_REQUEST_ADMIN, GET_USER_ROLE_REQUEST_ADMIN_LEN) == 0) {
        int user_id = json_to_int_patch_hujatch(root["user_id"]);

        Json::Value jvalue = get_user_role(data.r_ctx, user_id);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, SET_ROLE_REQUEST_ADMIN, SET_ROLE_REQUEST_ADMIN_LEN) == 0) {
        int user_id = json_to_int_patch_hujatch(root["user_id"]);
        int role    = json_to_int_patch_hujatch(root["role"]);

        set_role(data.r_ctx, user_id, role);

        fcgx_put_json_ok(*data.request);
    } else if (strncmp(query_string, UPDATE_USER_CHANNEL_REQUEST_ADMIN, UPDATE_USER_CHANNEL_REQUEST_ADMIN_LEN) == 0) {
        int user_id    = json_to_int_patch_hujatch(root["user_id"]);
        int channel_id = json_to_int_patch_hujatch(root["channel_id"]);

        update_user_channel(data.r_ctx, user_id, channel_id);

        fcgx_put_json_ok(*data.request);
    } else if (strncmp(query_string, UPDATE_USER_PARENT_REQUEST_ADMIN, UPDATE_USER_PARENT_REQUEST_ADMIN_LEN) == 0) {
        int user_id   = json_to_int_patch_hujatch(root["user_id"]);
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);

        update_user_parent(data.r_ctx, user_id, parent_id);

        fcgx_put_json_ok(*data.request);
    } else if (strncmp(query_string, GET_USER_STATUS_REQUEST_ADMIN, GET_USER_STATUS_REQUEST_ADMIN_LEN) == 0) {
        int user_id = json_to_int_patch_hujatch(root["user_id"]);

        Json::Value jvalue = get_user_status(data.r_ctx, user_id);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, SET_USER_STATUS_REQUEST_ADMIN, SET_USER_STATUS_REQUEST_ADMIN_LEN) == 0) {
        int user_id = json_to_int_patch_hujatch(root["user_id"]);
        int status  = json_to_int_patch_hujatch(root["status"]);

        set_user_status(data.r_ctx, user_id, status);

        fcgx_put_json_ok(*data.request);
    } else if (strncmp(query_string, GET_USER_SALES_REQUEST_ADMIN, GET_USER_SALES_REQUEST_ADMIN_LEN) == 0) {
        int user_id  = json_to_int_patch_hujatch(root["user_id"]);
        int ts_start = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds  = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_user_sales(data.r_ctx, user_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_PAYIN_REQUEST_ADMIN, GET_USER_PAYIN_REQUEST_ADMIN_LEN) == 0) {
        int user_id  = json_to_int_patch_hujatch(root["user_id"]);
        int ts_start = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds  = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_user_payin(data.r_ctx, user_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_USER_PAYOUT_REQUEST_ADMIN, GET_USER_PAYOUT_REQUEST_ADMIN_LEN) == 0) {
        int user_id  = json_to_int_patch_hujatch(root["user_id"]);
        int ts_start = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds  = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_user_payout(data.r_ctx, user_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_SALES_REQUEST_ADMIN, GET_PARENT_SALES_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_sales(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_PAYIN_REQUEST_ADMIN, GET_PARENT_PAYIN_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_payin(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_PAYOUT_REQUEST_ADMIN, GET_PARENT_PAYOUT_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_payout(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_SALES_PER_USER_REQUEST_ADMIN, GET_PARENT_SALES_PER_USER_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_sales_per_user(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_PAYIN_PER_USER_REQUEST_ADMIN, GET_PARENT_PAYIN_PER_USER_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_payin_per_user(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    } else if (strncmp(query_string, GET_PARENT_PAYOUT_PER_USER_REQUEST_ADMIN, GET_PARENT_PAYOUT_PER_USER_REQUEST_ADMIN_LEN) == 0) {
        int parent_id = json_to_int_patch_hujatch(root["parent_id"]);
        int ts_start  = json_to_int_patch_hujatch(root["ts_start"]);
        int seconds   = json_to_int_patch_hujatch(root["seconds"]);

        Json::Value jvalue = get_parent_payout_per_user(data.r_ctx, parent_id, ts_start, seconds);

        fcgx_put_json(*data.request, jvalue);
    }

    //закрыть текущее соединение
    FCGX_Finish_r(data.request);

    //завершающие действия - запись статистики, логгирование ошибок и т.п.

    return res;
}

int
Requests::parse_json(Data& data, Json::Value& root) {
    const char *content_type = FCGX_GetParam("CONTENT_TYPE", data.request->envp);
    /************
    * Parse JSON
    ************/
    int read_bytes = FCGX_GetStr(data.buf_in, MAX_REQUEST_BUF_SIZE,data.request->in);
    if (read_bytes == 0)
    {
        finish_request_parse_err(*data.request,"json");
        return 1;
        //return nullptr;
    }
    Json::Reader reader;
    //fprintf(stdout, "\n--------\n%s\n-------------\n",buf_in);
    int res_parsing  = 0;
    if(strncmp(content_type,CONTENT_TYPE_JSON, CONTENT_TYPE_JSON_LEN-1)==0)
    {
        res_parsing= reader.parse(data.buf_in,root);

        if (!res_parsing)
        {
            log_err("unable to parse request %s", data.buf_in);
            vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
            for (auto a: vec_err)
            {
                log_err("%s",a.message.c_str());
            }
            finish_request_parse_err(*data.request,"json");
            return 1;
        }
    }

    return 0;
}

int
Requests::request_bingo(Data& data) {
    std::string out;
    out.append("[");

    out.append("{\"game_id\":7770,\"name\":\"KENO\",\"logo\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7770_logo.png\",\"image_url\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7770.png\",");
    out.append("\"stream_url\":\"rtmp://");
    out.append(data.ora_host);
    out.append("/keno\"}");

    out.append(",{\"game_id\":7771,\"name\":\"Naval Battle\",\"logo\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7771_logo.png\",\"image_url\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7771.png\",");
    out.append("\"stream_url\":\"rtmp://");
    out.append(data.ora_host);
    out.append("/naval_battle\"}");

    out.append(",{\"game_id\":7775,\"name\":\"Bingo 37\",\"logo\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7775_logo.png\",\"image_url\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7775.png\",");
    out.append("\"stream_url\":\"rtmp://");
    out.append(data.ora_host);
    out.append("/bingo_37\"}");

    out.append(",{\"game_id\":7776,\"name\":\"Bingo 38\",\"logo\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7776_logo.png\",\"image_url\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7776.png\",");
    out.append("\"stream_url\":\"rtmp://");
    out.append(data.ora_host);
    out.append("/bingo_38\"}");

    out.append(",{\"game_id\":7777,\"name\":\"Bingo Club\",\"logo\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7777_logo.png\",\"image_url\":\"https://");
    out.append(data.ora_host);
    out.append("/static/img/7777.png\",");
    out.append("\"stream_url\":\"rtmp://");
    out.append(data.ora_host);
    out.append("/bingo_club\"}");


    out.append("]");

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_banner(Data& data) {
    std::string out;
    // 1. request games from redis
    // 2. prepare response
    int res = get_banner(data.r_ctx, out);
    if (res != 0)
    {
        // return error
        finish_request_parse_err(*data.request,"banner err");
        return 0;
    }

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_planogram(Data& data) {
    std::string out;
    // 1. request games from redis
    // 2. prepare response
    int res = get_games(data.r_ctx, out);
    if (res != 0)
    {
        // return error
        finish_request_parse_err(*data.request,"planogram err");
        return 0;
    }

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_games(Data& data) {
    std::string out;
    // 1. request games from redis
    // 2. prepare response
    //int res = get_games(r_ctx, out);

    //out.append(R"(,{"game_id":211000,"name":"Lucky Queen Bonus","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/lucky-queen.png"})");

    out.append(R"([
{"game_id":2111,"name":"Crazy Lemon","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/crazy-lemon.png"},
{"game_id":211101,"name":"Crazy Lemon Bonus letters","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/crazy-lemon.png"},
{"game_id":2110,"name":"Lucky Queen","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/lucky-queen.png"},
{"game_id":222200,"name":"Bonus game double","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/lucky-queen.png"},
{"game_id":2117,"name":"Satty Taga","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/taga.png"},
{"game_id":2112,"name":"Fruto Boom","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/fruto-boom.png"},
{"game_id":211200,"name":"Fruto Boom Bonus","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/fruto-boom.png"},
{"game_id":2114,"name":"Fruit and Ice","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/fruit-n-ice.png"},
{"game_id":211400,"name":"Fruit and Ice Bonus 1","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/fruit-n-ice.png"},
{"game_id":211401,"name":"Fruit and Ice Bonus 2","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/fruit-n-ice.png"},
{"game_id":2115,"name":"Aksha Bar","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/aksha-bar.png"},
{"game_id":211500,"name":"Aksha Bar bonus 3 parrots","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/aksha-bar.png"},
{"game_id":211501,"name":"Aksha Bar bonus 4 parrots","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/aksha-bar.png"},
{"game_id":211502,"name":"Aksha Bar bonus 5 parrots","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/aksha-bar.png"},
{"game_id":2113,"name":"Chukcha","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/chukcha.png"},
{"game_id":211300,"name":"Chukcha Bonus","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/chukcha.png"},
{"game_id":211301,"name":"Chukcha Bonus fishes","demo":true,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/chukcha.png"},
{"game_id":2118,"name":"3 Diamonds","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/diamonds.png"},
{"game_id":2123,"name":"Egypt Secrets","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/egypt-secrets.png"},
{"game_id":2116,"name":"Aladdin","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/aladdin.png"},
{"game_id":2121,"name":"Altyn","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/altyn.png"},
{"game_id":2122,"name":"Cleopatra","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/cleopatra.png"},
{"game_id":2119,"name":"Mega Casino","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/mega-casino.png"},
{"game_id":2120,"name":"Magic Lamp","demo":false,"mobile":true, "category":{"application":["web"],"class":"rng","type":"on-demand","subtype":"loto"},"image_url":"https://games-stage.kube.indev.expert/static/img/magic-lamp.png"}
])");

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_draws_info(Data& data) {
    std::string out;
    out.append("[");
    out.append("]");

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_draws_data(Data& data) {
    std::string out;
    out.append("[");
    out.append("]");

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_ready(Data& data) {
    int res = 0;
    std::string redis_err_msg = "";
    std::string out;
    out.append("{\"ts\":");
    time_t t;
    t = time(nullptr);
    out.append(to_string(t));
    out.append(",\"bf\":1");
    out.append("}");
    FCGX_PutS("Content-type: text/javascript\r\n", data.request->out);
    FCGX_PutS("\r\n", data.request->out);
    FCGX_PutS(out.c_str(), data.request->out);
    FCGX_PutS("\n\n", data.request->out);

    add_headers(*data.request,out);
    return 0;
}
int
Requests::request_ping(Data& data) {
    int res = 0;
    std::string redis_err_msg = "";
    std::string out;
    out.append("{\"ts\":");
    time_t t;
    t = time(nullptr);
    out.append(to_string(t));
    out.append(",\"redis\":");
    //check ctx
    if (data.redis_ready!=0)
    {
        if (check_redis(&data.r_ctx))
        {
            finish_request_parse_err(*data.request,"redis");
            return 0;
        }
        redisReply* r_reply =(redisReply*) redisCommand(data.r_ctx, "PING");

        if (r_reply == nullptr)
        {
            log_err("ERR nullptr reply PING %s",
                    data.r_ctx->errstr);
            data.redis_ready = 0;
            redis_err_msg = data.r_ctx->errstr;
            return 0;
        }

        if (r_reply->type == REDIS_REPLY_ERROR)
        {
            data.redis_ready = 0;
            redis_err_msg = data.r_ctx->errstr;
            return 0;
        }

        //clear redis reply
        if (r_reply != nullptr)
        {
            freeReplyObject(r_reply);
        }

    }// if data.redis_ready
    out.append(to_string(data.redis_ready));
    out.append(",\"redis_msg\":\"");
    out.append(redis_err_msg);
    out.append("\"");
    out.append("}");
    FCGX_PutS("Content-type: text/javascript\r\n", data.request->out);
    FCGX_PutS("\r\n", data.request->out);
    FCGX_PutS(out.c_str(), data.request->out);
    FCGX_PutS("\n\n", data.request->out);

    add_headers(*data.request,out);
    return 0;
}

int
Requests::request_param(Data& data, const char* query_string) {
    int res = 0;
    std::string redis_err_msg = "";
    char session[1024] = {0x00};
    // check query_string len
    int session_len = strlen(query_string) - sizeof(PARAM_REQUEST) + 1;
    log_err("session_len:%d read_byte: %d PARAM_REQUEST_LEN: %d",
            session_len, strlen(query_string), sizeof(PARAM_REQUEST));
    if (session_len >= MIN_SESSION_LEN)
    {
        std::string return_url;
        std::string refill_url;
        std::string action_url;
        std::string language;
        std::string myparam ;
        std::string player_name;
        std::string register_url ;
        int session_status = 0;
        int demo = 0;
        int result = get_status_session(data.r_ctx, session, session_status,
                                        return_url, refill_url, action_url, language, myparam,player_name, demo, register_url);

        char *params_mask = "const params = {\x0a return_url: \"%s\",\x0a refill_url: \"%s\",\x0a action_url: \"%s\",\x0a session: \"%s\",\x0a register_url:\"%s\",\x0a myparam:\"%s\",\x0a language:\"%s\" }";
        char buf_out[4096]= {0x00};
        snprintf(buf_out,sizeof(buf_out), params_mask,
                 return_url.c_str(), refill_url.c_str(), action_url.c_str(), session, register_url.c_str(),
                 myparam.c_str(), language.c_str()
                );
#ifdef _CONSOLE_DEBUG
        fprintf(stderr, buf_out);
#endif
        syslog(LOG_INFO, buf_out);
        FCGX_PutS("Content-type: text/javascript\r\n", data.request->out);
        FCGX_PutS("\r\n", data.request->out);
        FCGX_PutS(buf_out, data.request->out);
        FCGX_PutS("\n\n", data.request->out);
    }

    return 0;
}

int
Requests::request_alt_param(Data& data, const char* query_string) {
    std::string redis_err_msg = "";
    char session[1024] = {0x00};
    // check query_string len
    int res = 0;
    std::string post_data = query_string+7; //7 because i can
    log_info("try to parse === [%s]",post_data.c_str());
    map <string, std::string> post_vals;
    int pos = 0;
    int is_sz = 1; // only for SZ.KZ
        is_sz = 0;

    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            log_info("[%s=%s]", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);
        }
        else
        {
            pos = std::string::npos;
        }

    } //while npos
    int session_len = 0;
    if(post_vals.find("session")!=post_vals.end())
    {
        session_len = post_vals["session"].length();
        strncpy(session, post_vals["session"].c_str(),session_len);
        log_info("session =%s len=%d",
                 post_vals["session"].c_str(),
                 session_len);
    }
    // check host.
    // for sendnen this check not need
    //
    if( !is_sz && post_vals.find("game_id")==post_vals.end())
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
    }
    //----------------

    if (session_len >= MIN_SESSION_LEN)
    {
        std::string bet_url = "https://";
        std::string start_url = "https://";
        std::string register_url = "https://";

        bet_url.append(data.ora_host);
        start_url.append(data.ora_host);
        register_url.append(data.ora_host);

        bet_url.append("/bet");
        start_url.append("/start");
        register_url.append("/register");

        std::string balance_url = "https://";
        balance_url.append(data.ora_host);
        balance_url.append("/balance_and_bonus");


        std::string draw_info_url = "https://";
        draw_info_url.append(data.ora_host);
        draw_info_url.append("/draw");

        std::string get_my_bets_url = "https://";
        get_my_bets_url.append(data.ora_host);
        get_my_bets_url.append("/get_my_bets");

        std::string buy_bets_url = "https://";
        buy_bets_url.append(data.ora_host);
        buy_bets_url.append("/buy_bets");

        std::string return_url;
        std::string refill_url;
        std::string action_url;
        std::string register_url_redis;
        std::string language;
        std::string myparam ;
        std::string player_name;
        std::string currency ="💰";
        int session_status = 0;
        int demo = 0;
        int result = 0;
        if (is_sz)
        {
            result = get_status_session(data.r_ctx, session, session_status,
                                        return_url, refill_url, action_url,
                                        language, myparam, player_name,
                                        demo, register_url_redis);
        }
        else
        {
            result = get_status_session_www(data.r_ctx, session,
                                            return_url, refill_url, action_url,
                                            language, player_name,
                                            register_url_redis);
        }
        if(result != 0)
        {
            //finish

            finish_request_parse_err(*data.request,"session term 1");
            return 0;
        }
        //if (return_url.length()==0 && (strcmp(data.ora_host,"senden.pro")==0 ||strcmp(data.ora_host,"swertefun.com")==0||strcmp(data.ora_host,"senden.swertefun.com")==0))
        //{
            return_url.append("https://");
            return_url.append(data.ora_host);
            return_url.append("/static/index.html");
        //}
        if (register_url_redis.length()>0)
        {
            register_url = register_url_redis;
        }
        std::string  game_id;
        std::string prices;
        std::string bonuses;
        std::string s_session = session;
        if(post_vals.find("game_id")!=post_vals.end())
        {
            game_id = post_vals["game_id"];
            if (strncmp(game_id.c_str(), "null", game_id.length())==0)
            {
                game_id="";
            }
        }
        if (!game_id.length())
        {
            int res =  get_game_id(data.r_ctx, s_session, game_id);
            if(res != 0)
            {
                // finish
                finish_request_parse_err(*data.request,"session term 2");
                return 0;
            }
        }
        res = get_game_prices(data.r_ctx, game_id, prices);
        if(res != 0)
        {
            // finish
            finish_request_parse_err(*data.request,"session term 3");
            return 0;
        }
        res = get_game_bonuses(data.r_ctx, game_id, bonuses);
        if(res != 0)
        {
            // finish
            finish_request_parse_err(*data.request,"session term 4");
            return 0;
        }
        const char *senden_host = "senden.pro";
        std::string logo;
        logo.append("https://");
        logo.append(data.ora_host);
        logo.append("/static/img/logos/");

        logo.append("senden.png");
        language = "[\"cn\",\"en\",\"tg\"]";

        std::string bonuses_json;
        char *params_mask = nullptr;
        char buf_out[4096]= {0x00};
        char *endptr = nullptr;

        int i_game_id = strtoll(game_id.c_str(), &endptr,10);
        if (i_game_id == 2113
                || i_game_id == 2114
                || i_game_id == 2111
           )
        {
            if (i_game_id == 2111)
            {
                res = get_all_lemon_letters_count(data.r_ctx, s_session, bonuses_json);
            }
            if (i_game_id == 2113)
            {
                res = get_all_chuckcha_tile_count(data.r_ctx, s_session, bonuses_json);
            }
            if (i_game_id == 2114)
            {
                res = get_all_fruit_n_ice_tile_count(data.r_ctx, s_session, bonuses_json);
            }
            params_mask = "{\"return_url\": \"%s\",\x0a \"refill_url\": \"%s\",\x0a \"action_url\": \"%s\",\x0a \"session\": \"%s\",\x0a \"register_url\":\"%s\",\x0a \"bet_url\":\"%s\",\x0a \"balance_url\":\"%s\",\x0a \"start_url\":\"%s\" ,\x0a \"game_id\":%s,\"prices\":[%s],\x0a \"myparam\":\"%s\",\x0a \"language\":%s,\x0a \"player_name\":\"%s\", \x0a \"draw_info_url\":\"%s\", \x0a \"get_my_bets_url\":\"%s\", \x0a \"buy_bets_url\":\"%s\", \"demo\":%d,\x0a \"logo\":\"%s\",\x0a \"currency\":\"%s\" \x0a %s, %s}";
            snprintf(buf_out,sizeof(buf_out), params_mask,
                     return_url.c_str(), refill_url.c_str(), action_url.c_str(), session, register_url.c_str(),
                     bet_url.c_str(), balance_url.c_str(), start_url.c_str(), game_id.c_str(), prices.c_str(),
                     myparam.c_str(), language.c_str(),player_name.c_str(),draw_info_url.c_str(), get_my_bets_url.c_str(), buy_bets_url.c_str(), demo, logo.c_str(), currency.c_str(), bonuses.c_str(), bonuses_json.c_str()

                    );

        }
        else
        {
            params_mask = "{\"return_url\": \"%s\",\x0a \"refill_url\": \"%s\",\x0a \"action_url\": \"%s\",\x0a \"session\": \"%s\",\x0a \"register_url\":\"%s\",\x0a \"bet_url\":\"%s\",\x0a \"balance_url\":\"%s\",\x0a \"start_url\":\"%s\" ,\x0a \"game_id\":%s,\"prices\":[%s],\x0a \"myparam\":\"%s\",\x0a \"language\":%s,\x0a \"player_name\":\"%s\", \x0a \"draw_info_url\":\"%s\", \x0a \"get_my_bets_url\":\"%s\", \x0a \"buy_bets_url\":\"%s\", \"demo\":%d,\x0a \"logo\":\"%s\",\x0a \"currency\":\"%s\" \x0a %s}";
            snprintf(buf_out,sizeof(buf_out), params_mask,
                     return_url.c_str(), refill_url.c_str(), action_url.c_str(), session, register_url.c_str(),
                     bet_url.c_str(), balance_url.c_str(), start_url.c_str(), game_id.c_str(), prices.c_str(),
                     myparam.c_str(), language.c_str(),player_name.c_str(),draw_info_url.c_str(), get_my_bets_url.c_str(), buy_bets_url.c_str(), demo, logo.c_str(), currency.c_str(), bonuses.c_str()

                    );
        }
#ifdef _CONSOLE_DEBUG
        fprintf(stderr, buf_out);
#endif \
//syslog(LOG_INFO, buf_out);
        fcgx_put_str_json(*data.request, std::string(buf_out));
    }

    return 0;
}

int
Requests::request_init(Data& data) {
    int res = 0;
    std::string redis_err_msg = "";
    Json::Value root;
		int bonus_balance = 0;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    log_info("INIT!");
    Json::Value game_id_j = root["game_uuid"];
    int game_id = json_to_int_patch_hujatch(game_id_j);
    // check game_id
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    if (game_id >9999 || game_id < 1000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    Json::Value player_j = root["player_id"];
    if (player_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }

    uint64_t player = json_to_int_patch_hujatch(player_j);
    if (player <1)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
#if 0
    else
        fprintf(stdout, "\n*********\nplayer: %llu %llu\n",
                player, player_j.asInt64());
#endif
    Json::Value player_name_j = root["player_name"];
    std::string player_name ="";

    if (player_name_j != Json::Value::null)
    {
        player_name = player_name_j.asString();
    }

    Json::Value application_j = root["application"];
    std::string application = "";

    if (application_j != Json::Value::null)
    {
        application = application_j.asString();
    }


    Json::Value email_j = root["email"];
    std::string email = "";

    if (email_j != Json::Value::null)
    {
        email = email_j.asString();
    }

    Json::Value return_url_j = root["return_url"];
    std::string return_url = "";


    if (return_url_j != Json::Value::null)
    {
        return_url = return_url_j.asString();
    }


    Json::Value register_url_j = root["register_url"];
    std::string register_url = "";
    if (register_url_j != Json::Value::null)
    {
        register_url = register_url_j.asString();
    }
    else
    {
        register_url="empty from logycom";
    }

    Json::Value refill_url_j = root["refill_url"];
    std::string refill_url = "";
    if (refill_url_j != Json::Value::null)
    {
        refill_url = refill_url_j.asString();
    }

    if (!refill_url.length())
    {
        refill_url = register_url;
    }

    Json::Value action_url_j = root["action_url"];
    std::string action_url = "";

    if (action_url_j != Json::Value::null)
    {
        action_url = action_url_j.asString();
    }

    Json::Value language_j = root["language"];
    std::string language = "";

    if (language_j != Json::Value::null)
    {
        language = language_j.asString();
    }


    Json::Value myparam_j = root["my_param"];
    std::string myparam = myparam_j.asString();
    if (myparam_j != Json::Value::null)
    {
        myparam = myparam_j.asString();
    }

    // SESSION
    EVP_MD_CTX* mdctx = nullptr;
    mdctx = EVP_MD_CTX_create();
    if(mdctx == nullptr)
    {
        log_err("unable create md ctx");
    }
    int res_md = EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, data.pkey);
    if (res_md != 1)
    {
        log_err("unable init md");
        ERR_load_crypto_strings();
        char err[255];
        ERR_error_string(ERR_get_error(), err);
        return 0;
    }
    res_md = EVP_DigestSignUpdate(mdctx, player_j.asString().c_str(),player_j.asString().length());
    //res_md = EVP_DigestSignUpdate(mdctx, "79096321010",strlen("79096321010"));
    if (res_md != 1)
    {
        log_err("err Sign message");
    }
    size_t sig_len;
    res_md = EVP_DigestSignFinal(mdctx, nullptr,&sig_len);
    char *sig = (char*)OPENSSL_malloc(sig_len);
    res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
    CryptoPP::Base32Encoder encoder(nullptr,false);
    syslog(LOG_INFO, "SIG: [%d] bytes", sig_len);
    encoder.Put((unsigned char*) sig, sig_len);
    encoder.MessageEnd();
    std::FILE *f = fopen("/dev/shm/s", "w+");
    fwrite(sig,sig_len,1,f);
    fclose(f);
    char encoded[1024]= {0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((unsigned char*)encoded, len);
    syslog(LOG_INFO, "SIG[%d]:%s",len, encoded);
    if(sig !=nullptr)
    {
        OPENSSL_free((void*)sig);
    }
    if (mdctx!=nullptr)
    {
        EVP_MD_CTX_destroy(mdctx);
    }

    // *data.request balance from SZ
    /*====== REDIS WRITE ========*/

    std::string out_resp;
    std::string demo_session = "gbcseic7csf9an3ps9p6a7ar4978knmjyhyygksxsbbjfdeqzqgfmyp8nnbccag784kztjfm4f75u9p3pbeqc4hf6jw8eyq53m7ejwpunspfbukiq6";
    unsigned int balance = 0  ;
    Json::Value balance_j = root["balance"];
    // check balance
    if (balance_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"balance");
        return 0;
        //return nullptr;
    }
    balance = json_to_int_patch_hujatch(balance_j);
    time_t t;
    t = time(nullptr);
    if (check_redis(&data.r_ctx))
    {
        finish_request_parse_err(*data.request,"redis");
        return 0;
    }

    redisReply* r_reply =(redisReply*) redisCommand(data.r_ctx,
                         "HMSET %s player %lld game_id %d balance %d status 0 ts_start %u player_name %s application %s email %s return_url %s refill_url %s action_url %s language %s myparam %s register_url %s",
                         encoded, player, game_id, balance, t,
                         player_name.c_str(),
                         application.c_str(),
                         email.c_str(),
                         return_url.c_str(),
                         refill_url.c_str(),
                         action_url.c_str(),
                         language.c_str(),
                         myparam.c_str(), register_url.c_str()
                                                   );

    if (r_reply == nullptr)
    {
        log_err("ERR nullptr reply %s for SESSION %s ",
                data.r_ctx->errstr,encoded);
        finish_request_parse_err(*data.request,"session");
        return 0;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for SESSION %s ",
                data.r_ctx->errstr,encoded);
        if (r_reply != nullptr)
        {
            freeReplyObject(r_reply);
        }
        finish_request_parse_err(*data.request,"session");
        return 0;
    }

    //clear redis reply
    if (r_reply != nullptr)
    {
        freeReplyObject(r_reply);
    }

    int ret_result = session_add_sync_list(data.r_ctx, encoded);
    if (ret_result != 0)
    {
        log_err("unable to write session for sync");
    }

    /*====== END REDIS WRITE ========*/
    // return message with link


    std::string  game_path;
    res =  get_game_path(data.r_ctx,  game_id, game_path);
    if(res!=0)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
    }
    //string url_link = "https://oraora.winlottery.site/static/index.html?session=";
    std::string url_link = "https://";
    url_link.append(data.ora_host);
    url_link.append("/static/");
    url_link.append(game_path);
    url_link.append("/index.html?session=");
    url_link.append(encoded);
    url_link.append("&host=");
    url_link.append(data.ora_host);
    std::string response = "{\"url\":\"";
    response.append(url_link);
    response.append("\",\"session\":\"");
    response.append(encoded);
    response.append("\",\"balance\":");
    response.append(to_string(balance+bonus_balance));
    response.append("}");
    log_info("init sess: %s", response.c_str());
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_get_winners_top(Data& data) {
    int res = 0;
    Json::Value root;
		
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    std::string out;
    res = get_winners_top(out);
    std::string response ;
    response.append("[");
    response.append(out);
    response.append("]");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_cold_numbers(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    //fprintf(stdout, "*data.request draw\n");
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    Json::Value game_id_j = root["game_id"];
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    int game_id = json_to_int_patch_hujatch(game_id_j);
    // check game_id
    if (game_id >9999 || game_id < 1000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }

    std::string out;
    res = get_numbers_cold(session.asCString(), game_id, out);
    std::string response = "{\"game_id\":\"";
    response.append(to_string(game_id));
    response.append("\",");
    response.append("\"cold\":[");
    response.append(out);
    response.append("]");
    response.append(",\"action\":\"cold_numbers\"}");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_hot_numbers(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    //fprintf(stdout, "*data.request draw\n");
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    Json::Value game_id_j = root["game_id"];
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    int game_id = json_to_int_patch_hujatch(game_id_j);
    // check game_id
    if (game_id >9999 || game_id < 1000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }

    std::string out;
    res = get_numbers_hot(session.asCString(), game_id, out);
    std::string response = "{\"game_id\":\"";
    response.append(to_string(game_id));
    response.append("\",");
    response.append("\"hot\":[");
    response.append(out);
    response.append("]");
    response.append(",\"action\":\"hot_numbers\"}");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_draw(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    //fprintf(stdout, "*data.request draw\n");
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    Json::Value game_id_j = root["game_id"];
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    int game_id = json_to_int_patch_hujatch(game_id_j);
    // check game_id
    if (game_id >9999 || game_id < 1000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }

    std::string out;
    int result = get_draw(data.r_ctx, session.asCString(), game_id, out);

    // return message with link
    std::string error;
    if (result == 0 )
    {
        error="false";
    }
    else
    {
        error="true";
    }

    std::string message="";
    std::string response = "{\"game_id\":\"";
    response.append(to_string(game_id));
    response.append("\",");
    response.append(out);
    response.append(",\"action\":\"draw\"}");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_balance_update(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    log_info("BALANCE UPDATE!");
    Json::Value balance_j = root["balance"];
    // check game_id
    if (balance_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"balance");
        return 0;
        //return nullptr;
    }
    unsigned int balance = json_to_int_patch_hujatch(balance_j);
    if (balance <1 || balance > 1000000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"balance");
        return 0;
        //return nullptr;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    log_info("UPDATE balance SESSION: %s", session.asCString());

    int result = set_balance(data.r_ctx, session.asCString(),balance);


    // return message with link
    std::string error;
    if (result == 0 )
    {
        error="false";
    }
    else
    {
        error="true";
    }

    std::string message="";
    std::string response = "{\"error\":\"";
    response.append(error);
    response.append("\",\"message\":\"");
    response.append(message);
    response.append("\"}");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_start(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    std::string res2= "{\"error\":0";
    res2.append(",\"message\":\"");
    res2.append("\"}");
    fcgx_put_str_json(*data.request, res2);
    return 0;

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    //
    // get session status
    int status = 0;
    std::string error;

    std::string return_url;
    std::string refill_url;
    std::string action_url;
    std::string register_url;
    std::string player_name;
    std::string language;
    std::string myparam ;
    int demo = 0;
    int result = get_status_session(data.r_ctx, session.asCString(), status,
                                    return_url, refill_url, action_url, language, myparam, player_name, demo, register_url);
    if (0)
        if (status > 0)
        {
            // set new session

            std::string player;
            get_player_session(data.r_ctx, session.asCString(), player);

            if (player.length()<5)
            {
                // finish *data.request
                finish_request_parse_err(*data.request,"session");
                return 0;
                //return nullptr;
            }

            log_err("session already started should to shutdown session");
            int result = set_status_session(data.r_ctx, session.asCString(),SESSION_STOPPED);
            if (result !=0 )
            {
                log_err("some err when try to sop session %s ",session.asCString() );
            }

            /*!
            * generate new UID
            *
            */

            // SESSION
            EVP_MD_CTX* mdctx = nullptr;
            mdctx = EVP_MD_CTX_create();
            if(mdctx == nullptr)
            {
                log_err("unable create md ctx");
            }
            int res_md = EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, data.pkey);
            if (res_md != 1)
            {
                log_err("unable init md");
                ERR_load_crypto_strings();
                char err[255];
                ERR_error_string(ERR_get_error(), err);
                return 0;
            }
            res_md = EVP_DigestSignUpdate(mdctx, player.c_str(),player.length());
            //res_md = EVP_DigestSignUpdate(mdctx, "79096321010",strlen("79096321010"));
            if (res_md != 1)
            {
                log_err("err sign message");
            }
            size_t sig_len;
            res_md = EVP_DigestSignFinal(mdctx, nullptr,&sig_len);
            char *sig = (char*)OPENSSL_malloc(sig_len);
            res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
            CryptoPP::Base32Encoder encoder(nullptr,false);
            log_err("SIG: [%d] bytes", sig_len);
            encoder.Put((unsigned char*) sig, sig_len);
            encoder.MessageEnd();
            std::FILE *f = fopen("/dev/shm/s", "w+");
            fwrite(sig,sig_len,1,f);
            fclose(f);
            char encoded[1024]= {0x00};
            const long len = encoder.MaxRetrievable();
            encoder.Get((unsigned char*) encoded, len);
            log_err("SIG[%d]:[%s]",len, encoded);
            if(sig !=nullptr)
            {
                OPENSSL_free((void*)sig);
            }
            if (mdctx!=nullptr)
            {
                EVP_MD_CTX_destroy(mdctx);
            }

            log_err("finish generate new sig --------------");
            /********************/


            std::string url;
            result = renew_session(data.r_ctx, session.asCString(), encoded,url);
            std::string response;
            response.append("{\"error\":");
            response.append(to_string(ERR_SESS_DOUBLE));
            response.append(",\"url\":\"");
            response.append(url);
            response.append("\"}");
            fcgx_put_str_json(*data.request, response);

            return 0;
        }
    status=0;
    if (status == 0)
    {
        error = "0";
        // start session
        log_err("session is 0. update to 1");
        set_status_session(data.r_ctx, session.asCString(),SESSION_STARTED);
    }
    // return message with link
    std::string message="";
    std::string response = "{\"error\":";
    response.append(error);
    response.append(",\"message\":\"");
    response.append(message);
    response.append("\"}");
    fcgx_put_str_json(*data.request, response);

    return 0;
}

int
Requests::request_init_demo(Data& data) {
    int res = 0;
    Json::Value root;
	int bonus_balance = 0;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    log_info("INIT DEMO!");
    Json::Value game_id_j = root["game_uuid"];
    int game_id = json_to_int_patch_hujatch(game_id_j);
    // check game_id
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    if (game_id >9999 || game_id < 1000)
    {
        // return 400  ERR game_id
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    Json::Value player_j = root["player_id"];
    if (player_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }

    uint64_t player = json_to_int_patch_hujatch(player_j);

    Json::Value return_url_j = root["return_url"];
    std::string return_url = "";


    if (return_url_j != Json::Value::null)
    {
        return_url = return_url_j.asString();
    }
    Json::Value register_url_j = root["register_url"];
    std::string register_url = "";
    if (register_url_j != Json::Value::null)
    {
        register_url = register_url_j.asString();
    }

    Json::Value refill_url_j = root["refill_url"];
    std::string refill_url = "";
    if (refill_url_j != Json::Value::null)
    {
        refill_url = refill_url_j.asString();
    }

    if (!refill_url.length())
    {
        refill_url = register_url;
    }

    Json::Value action_url_j = root["action_url"];
    std::string action_url = "";

    if (action_url_j != Json::Value::null)
    {
        action_url = action_url_j.asString();
    }

    Json::Value language_j = root["language"];
    std::string language = "";

    if (language_j != Json::Value::null)
    {
        language = language_j.asString();
    }


    Json::Value myparam_j = root["my_param"];
    std::string myparam = myparam_j.asString();
    if (myparam_j != Json::Value::null)
    {
        myparam = myparam_j.asString();
    }

    // SESSION
    EVP_MD_CTX* mdctx = nullptr;
    mdctx = EVP_MD_CTX_create();
    if(mdctx == nullptr)
    {
        log_err("unable create md ctx");
    }
    int res_md = EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, data.pkey);
    if (res_md != 1)
    {
        log_err("unable init md");
        ERR_load_crypto_strings();
        char err[255];
        ERR_error_string(ERR_get_error(), err);
        return 0;
    }
    res_md = EVP_DigestSignUpdate(mdctx, player_j.asString().c_str(),player_j.asString().length());
    if (res_md != 1)
    {
        log_err("err Sign message");
    }
    size_t sig_len;
    res_md = EVP_DigestSignFinal(mdctx, nullptr,&sig_len);
    char *sig = (char*)OPENSSL_malloc(sig_len);
    res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
    CryptoPP::Base32Encoder encoder(nullptr,false);
    log_err("SIG: [%d] bytes", sig_len);
    encoder.Put((unsigned char*) sig, sig_len);
    encoder.MessageEnd();
    std::FILE *f = fopen("/dev/shm/s", "w+");
    fwrite(sig,sig_len,1,f);
    fclose(f);
    char encoded[1024]= {0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((unsigned char*)encoded, len);
    log_err("SIG[%d]:%s",len, encoded);
    if(sig !=nullptr)
    {
        OPENSSL_free((void*)sig);
    }
    if (mdctx!=nullptr)
    {
        EVP_MD_CTX_destroy(mdctx);
    }

    // *data.request balance from SZ
    /*====== REDIS WRITE ========*/

    std::string out_resp;
    unsigned int balance = 500000;

    time_t t;
    t = time(nullptr);
    if (check_redis(&data.r_ctx))
    {
        finish_request_parse_err(*data.request,"redis");
        return 0;
    }
    redisReply* r_reply =(redisReply*) redisCommand(data.r_ctx,
                         "HMSET %s player %lld game_id %d balance %d status 0 ts_start %u return_url %s refill_url %s action_url %s language %s myparam %s demo 1 register_url %s",
                         encoded, player, game_id, balance, t,
                         return_url.c_str(),
                         refill_url.c_str(),
                         action_url.c_str(),
                         language.c_str(),
                         myparam.c_str(), register_url.c_str()
                                                   );

    if (r_reply == nullptr)
    {
        log_err("ERR nullptr reply %s for SESSION %s ",
                data.r_ctx->errstr,encoded);
        finish_request_parse_err(*data.request,"redis");
        return 0;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for SESSION %s ",
                data.r_ctx->errstr,encoded);
        if (r_reply != nullptr)
        {
            freeReplyObject(r_reply);
        }
        finish_request_parse_err(*data.request,"redis");
        return 0;
    }

    //clear redis reply
    if (r_reply != nullptr)
    {
        freeReplyObject(r_reply);
    }

    /*====== END REDIS WRITE ========*/
    // return message with link

    std::string  game_path;
    res =  get_game_path(data.r_ctx,  game_id, game_path);
    if(res!=0)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
    }
    std::string url_link = "https://";
    url_link.append(data.ora_host);
    url_link.append("/static/");
    url_link.append(game_path);
    url_link.append("/index.html?session=");
    url_link.append(encoded);
    url_link.append("&host=");
    url_link.append(data.ora_host);
    std::string response = "{\"url\":\"";
    response.append(url_link);
    response.append("\",\"session\":\"");
    response.append(encoded);
    response.append("\",\"balance\":");
    response.append(to_string(balance+bonus_balance));
    response.append("}");
    log_info("init sess: %s", response.c_str());
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_get_user(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    int session_len = session.asString().length();
    if (session_len<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    std::string out;

    res = get_user_from_session (data.r_ctx, session.asCString(), out);
    add_headers(*data.request,out);

    return 0;
}

int
Requests::request_edit_user(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    char *player = FCGX_GetParam("DN", data.request->envp);
    std::string s_player;
    if (player == nullptr)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
    else
    {
        // parse DN
        int res = parse_player_cn(player, s_player);
        log_info("detect player ssl %s", s_player.c_str());

    }
    // get variables from response
    // if no variables - return current values


    Json::Value name_j = root["name"];
    if (name_j == Json::Value::null)
    {
        // return current values
        std::string out;
        int res = get_player_data(data.r_ctx, s_player.c_str(),out);
        std::string response = "{\"error\":\"";
        response.append(to_string(res));
        response.append("\"");
        response.append(out);
        response.append("}");
        add_headers(*data.request,response);
        return 0;
    }
    if (!name_j.isString())
    {
        finish_request_parse_err(*data.request,"name");
        return 0;
    }
    if (name_j.asString().length() > MAX_NAME_LEN)
    {
        finish_request_parse_err(*data.request,"name");
        return 0;
        //return nullptr;
    }

    Json::Value family_j = root["family"];
    if (family_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"family");
        return 0;
        //return nullptr;
    }

    Json::Value patronik_j = root["patronik"];
    if (patronik_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"patronik");
        return 0;
        //return nullptr;
    }
    Json::Value phone_j = root["phone"];
    if (phone_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"phone");
        return 0;
        //return nullptr;
    }

    Json::Value email_j = root["email"];
    if (email_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"email");
        return 0;
        //return nullptr;
    }
    Json::Value birth_j = root["birth"];
    if (birth_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"birth");
        return 0;
        //return nullptr;
    }
    Json::Value ts_j = root["ts"];
    if (ts_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"ts");
        return 0;
        //return nullptr;
    }
    Json::Value document_j = root["document"];
    if (document_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"document");
        return 0;
        //return nullptr;
    }
    Json::Value issue_date_j = root["issue_date"];
    if (issue_date_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"issue_date");
        return 0;
        //return nullptr;
    }
    Json::Value code_j = root["code"];
    if (code_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"code");
        return 0;
        //return nullptr;
    }
    Json::Value address_j = root["address"];
    if (address_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"address");
        return 0;
        //return nullptr;
    }
    std::string out;
    res = set_player_data(
              data.r_ctx, s_player.c_str(),
              name_j.asCString(),
              family_j.asCString(),
              patronik_j.asCString(),
              phone_j.asCString(),
              email_j.asCString(),
              birth_j.asCString(),
              ts_j.asCString(),
              document_j.asCString(),
              issue_date_j.asCString(),
              code_j.asCString(),
              address_j.asCString()
          );
    std::string response = "{\"error\":\"";
    response.append(to_string(res));
    response.append("\"");
    response.append(out);
    response.append("}");
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_p2p_create_withdrowal(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    int session_len = session.asString().length();
    if (session_len<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    //
    // get session status
    std::string return_url;
    std::string refill_url;
    std::string action_url;
    std::string register_url;
    std::string language;
    std::string myparam ;
    std::string player_name;
    int session_status = 0;
    int demo = 0;
    if (session_len >= MIN_SESSION_LEN)
    {
        int result = get_status_session(data.r_ctx, session.asCString(), session_status,
                                        return_url, refill_url, action_url, language, myparam,player_name, demo, register_url);
    }
    Json::Value currency_j = root["currency"];
    if (currency_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"currency");
        return 0;
        //return nullptr;
    }
    const char *currency = currency_j.asCString();
    Json::Value gw_j = root["gw"];
    if (gw_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"gw");
        return 0;
        //return nullptr;
    }
    const char *gw  = gw_j.asCString();

    Json::Value amount_j = root["amount"];
    if (amount_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"amount");
        return 0;
        //return null;
    }
    int amount = json_to_int_patch_hujatch(amount_j);


    // write p2p to redis data.r_ctx, Json::Value &
    std::string out; //b64 encoded form
    std::string player;
    get_player_session(data.r_ctx, session.asCString(), player);

    if (player.length()<5)
    {
        // finish *data.request
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }

    uint64_t invoice;
    res = p2p_create_withdrawal(data.r_ctx, player.c_str(), amount,
                                currency, gw, session.asCString(), invoice);
    if (res != 0 )
    {
        // invoice ok, generate html out
        // json{"error":0, "b32form":"b32"}
        // if gw== P2P payme
        finish_request_parse_err(*data.request,"create_withdrowal");
        return 0;
    }

    std::string out_form;

    if (gw_j.asString() == "p2p:telegram") {
        res = p2p_generate_form_withdrowal_tg(
                  player.c_str(),
                  amount,
                  currency,
                  gw,
                  invoice,
                  out_form
              );
    } else {
        res = p2p_generate_form_withdrowal(
                  player.c_str(),
                  amount,
                  currency,
                  gw,
                  invoice,
                  out_form
              );
    }

    std::string response = "{\"error\":";
    response.append(to_string(res));
    response.append(",\"action\":\"p2p_withdrawal\",");
    response.append("\"invoice\":\"");
    response.append(to_string(invoice));
    response.append("\",\"encoded\":\"");
    response.append(out_form);
    response.append("\"}");
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_p2p_create_invoice(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    int session_len = session.asString().length();
    if (session_len<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    //
    // get session status
    std::string return_url;
    std::string refill_url;
    std::string action_url;
    std::string register_url;
    std::string language;
    std::string myparam ;
    std::string player_name;
    int session_status = 0;
    int demo = 0;
    if (session_len >= MIN_SESSION_LEN)
    {
        int result = get_status_session(data.r_ctx, session.asCString(), session_status,
                                        return_url, refill_url, action_url, language, myparam,player_name, demo, register_url);
    }
    Json::Value currency_j = root["currency"];
    if (currency_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"currency");
        return 0;
        //return nullptr;
    }
    const char *currency = currency_j.asCString();
    Json::Value gw_j = root["gw"];
    if (gw_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"gw");
        return 0;
        //return nullptr;
    }
    const char *gw  = gw_j.asCString();

    Json::Value amount_j = root["amount"];
    if (amount_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"amount");
        return 0;
        //return null;
    }
    int amount = json_to_int_patch_hujatch(amount_j);


    // write p2p to redis data.r_ctx, Json::Value &
    std::string out; //b64 encoded form
    std::string player;
    get_player_session(data.r_ctx, session.asCString(), player);

    if (player.length()<5)
    {
        // finish *data.request
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }

    uint64_t invoice;
    res = p2p_create_invoice(data.r_ctx, player.c_str(), amount,
                             currency, gw, session.asCString(), invoice);
    if (res != 0 )
    {
        // invoice ok, generate html out
        // json{"error":0, "b32form":"b32"}
        // if gw== P2P payme
        finish_request_parse_err(*data.request,"create_invoice");
        return 0;
    }
    std::string out_form;
    res = p2p_generate_form(  player.c_str(), amount,
                              currency, gw, invoice, out_form);


    std::string response = "{\"error\":";
    response.append(to_string(res));
    response.append(",\"action\":\"p2p_invoice\",");
    response.append("\"invoice\":\"");
    response.append(to_string(invoice));
    response.append("\",\"encoded\":\"");
    response.append(out_form);
    response.append("\"}");
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_p2p_process_wd(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    // write p2p to redis data.r_ctx, Json::Value &
    res = 0;
    std::string post_data = data.buf_in;
    log_info("try to parse === [%s]",data.buf_in);
    map <string, std::string> post_vals;
    int pos = 0;
    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            log_info("[%s=%s]", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);
        }
        else
        {
            pos = std::string::npos;
        }

    }
    res = p2p_write_withdrawal(data.r_ctx, post_vals);
    std::string response = "{\"error\":";
    response.append(to_string(res));
    response.append("}");
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_p2p_process(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    // write p2p to redis data.r_ctx, Json::Value &
    std::string post_data = data.buf_in;
    log_info("try to parse === [%s]",data.buf_in);
    map <string, std::string> post_vals;
    int pos = 0;
    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            log_info("[%s=%s]", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);
        }
        else
        {
            pos = std::string::npos;
        }

    }
    res = p2p_write_payment(data.r_ctx, post_vals);
    std::string response = "{\"error\":";
    response.append(to_string(res));
    response.append("}");
    add_headers(*data.request,response);

    return 0;
}

int
Requests::request_p2p_process_paybiz(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    std::string paybiz_hmset = "HMSET paybiz:";
    // write p2p to redis data.r_ctx, Json::Value &
    Json::Value response_code_j = root["response_code"];
    if (response_code_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"response_code");
        return 0;
        //return nullptr;
    }
    if (!response_code_j.isString())
    {
        finish_request_parse_err(*data.request,"response_code");
        return 0;
    }

    Json::Value pay_reference_j = root["pay_reference"];
    if (pay_reference_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"pay_reference");
        return 0;
        //return nullptr;
    }

    paybiz_hmset.append(pay_reference_j.asString());

    paybiz_hmset.append(" response_code ");
    paybiz_hmset.append(response_code_j.asString());

    Json::Value response_advise_j = root["response_advise"];
    if (response_advise_j != Json::Value::null)
    {
        paybiz_hmset.append(" response_advise \"");
        paybiz_hmset.append(response_advise_j.asString());
        paybiz_hmset.append("\"");
    }

    Json::Value response_message_j = root["response_message"];
    if (response_message_j != Json::Value::null)
    {
        paybiz_hmset.append(" response_message \"");
        paybiz_hmset.append(response_message_j.asString());
        paybiz_hmset.append("\"");
    }

    Json::Value signature_j = root["signature"];
    if (signature_j != Json::Value::null)
    {
        paybiz_hmset.append(" signature ");
        paybiz_hmset.append(signature_j.asString());
    }

    Json::Value response_id_j = root["response_id"];
    if (response_id_j != Json::Value::null)
    {
        paybiz_hmset.append(" response_id ");
        paybiz_hmset.append(response_id_j.asString());
    }

    Json::Value merchant_id_j = root["merchant_id"];
    if (merchant_id_j != Json::Value::null)
    {
        paybiz_hmset.append(" merchant_id ");
        paybiz_hmset.append(merchant_id_j.asString());
    }

    Json::Value pchannel_j = root["pchannel"];
    if (pchannel_j != Json::Value::null)
    {
        paybiz_hmset.append(" pchannel ");
        paybiz_hmset.append(pchannel_j.asString());
    }

    Json::Value request_id_j = root["request_id"];
    if (request_id_j != Json::Value::null)
    {
        paybiz_hmset.append(" request_id ");
        paybiz_hmset.append(request_id_j.asString());
    }

    Json::Value processor_response_id_j = root["processor_response_id"];
    if (processor_response_id_j != Json::Value::null)
    {
        paybiz_hmset.append(" processor_response_id ");
        paybiz_hmset.append(processor_response_id_j.asString());
    }

    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j != Json::Value::null)
    {
        paybiz_hmset.append(" timestamp ");
        paybiz_hmset.append(timestamp_j.asString());
    }

    if (strncmp(PAYBIZ_RESPONSE_CODE_OK, response_code_j.asCString(),
                sizeof(PAYBIZ_RESPONSE_CODE_OK)-1)!=-1)
    {
        // ok. write to redis
        Json::Value customer_info_j = root["customer_info"];
        if (customer_info_j == Json::Value::null)
        {
            finish_request_parse_err(*data.request,"customer_info");
            return 0;
            //return nullptr;
        }


        Json::Value name_j = customer_info_j["name"];
        if (name_j != Json::Value::null)
        {
            // add to hmset
            paybiz_hmset.append(" name \"");
            paybiz_hmset.append(name_j.asString());
            paybiz_hmset.append("\"");
        }

        Json::Value address_j = customer_info_j["address"];
        if (address_j != Json::Value::null)
        {
            // add to hmset
            paybiz_hmset.append(" address \"");
            paybiz_hmset.append(address_j.asString());
            paybiz_hmset.append("\"");
        }
        Json::Value zip_j = customer_info_j["zip"];
        if (zip_j != Json::Value::null)
        {
            // add to hmset
            paybiz_hmset.append(" zip ");
            paybiz_hmset.append(zip_j.asString());
        }

        Json::Value province_j = customer_info_j["province"];
        if (province_j != Json::Value::null)
        {
            // add to hmset
            paybiz_hmset.append(" province \"");
            paybiz_hmset.append(province_j.asString());
            paybiz_hmset.append("\"");
        }

        Json::Value email_j = customer_info_j["email"];
        if (email_j != Json::Value::null)
        {
            // add to hmset
            paybiz_hmset.append(" email ");
            paybiz_hmset.append(email_j.asString());
        }
        Json::Value mobile_j = customer_info_j["mobile"];
        if (mobile_j == Json::Value::null)
        {
            finish_request_parse_err(*data.request,"mobile");
            return 0;
            //return nullptr;
        }

        paybiz_hmset.append(" mobile ");
        paybiz_hmset.append(mobile_j.asString());

        Json::Value amount_j = customer_info_j["amount"];
        if (amount_j == Json::Value::null)
        {
            finish_request_parse_err(*data.request,"amount");
            return 0;
            //return nullptr;
        }

        paybiz_hmset.append(" amount ");
        paybiz_hmset.append(amount_j.asString());

    }

    res = paybiz_write_callback(data.r_ctx,paybiz_hmset);
    std::string response = "{\"error\":0}";
    add_headers(*data.request,response);

    return 0;
}
std::string urldecode(const std::string& input) {
    std::ostringstream decoded;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            // Decode hexadecimal representation
            char hex[3] = { input[i + 1], input[i + 2], '\0' };
            int decodedChar;
            std::istringstream(hex) >> std::hex >> decodedChar;
            decoded << static_cast<char>(decodedChar);
            i += 2; // Move to the next character after '%'
        } else if (input[i] == '+') {
            // Replace '+' with space
            decoded << ' ';
        } else {
            // Copy other characters as is
            decoded << input[i];
        }
    }
    return decoded.str();
}
int calculateHMAC_SHA256( const std::string& data, const unsigned char * key,
                          const int key_len,
                          unsigned char *hmac_buf,
                          unsigned int &hmac_len) {
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key, key_len, EVP_sha256(), NULL);
    HMAC_Update(ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
    HMAC_Final(ctx, hmac_buf, &hmac_len);
    HMAC_CTX_free(ctx);
    return 0;
}

int
Requests::request_videoplayer(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value initData = root["tglogin"];
    std::string initData_s ;
    if (initData != Json::Value::null)
    {
        if (initData.isString())
        {
            initData_s = initData.asString();
            log_info("initData: %s \n",
                     initData.asString().c_str());

        }
    }
// ------- PROCESS POSTFIELDS---------------

    std::string post_data = urldecode(initData_s);
    log_info("try to parse === [%s]",post_data.c_str());
    map <string, std::string> post_vals;
    std::vector<std::string> dataToCheck;
    int pos = 0;

    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            if (name != "hash")
                dataToCheck.push_back(name + "=" + value);
            log_info("[%s=%s]", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);

        }
        else
        {
            pos = std::string::npos;
        }

    } //while npos
    int user_len = 0;
    char user_json[4049] = {0x00};
    if(post_vals.find("user")!=post_vals.end())
    {
        user_len = post_vals["user"].length();
        strncpy(user_json, post_vals["user"].c_str(),user_len);
        log_info("user =%s len=%d",
                 post_vals["user"].c_str(),
                 user_len);
    }
    Json::Value rroot;
    Json::Reader reader;
    int res_parsing = reader.parse(user_json,rroot);
    if (!res_parsing)
    {
        log_err("unable to parse request %s",user_json);
        vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
        for (auto a: vec_err)
        {
            log_err("%s",a.message.c_str());
        }
    }
    Json::Value tgid = rroot["id"];
    int tg_idval = 0;
    if (tgid  != Json::Value::null)
    {
        if (tgid.isInt64())
        {
            tg_idval = tgid.asInt64();
        }
    }
    std::string hash =  post_vals["hash"];
    std::sort(dataToCheck.begin(), dataToCheck.end());
    std::string dataToCheckStr ;
    int it=0;
    for (auto &a :dataToCheck)
    {
#if 1
        if (it)
        {
            dataToCheckStr += "\n";
            dataToCheckStr +=a ;
        }
        else
        {
            dataToCheckStr +=a ;

        }
        it++;
#else
        dataToCheckStr +=a ;
        dataToCheckStr += "\n";
#endif

    }
    // Calculate HMAC-SHA256
    //swertefun std::string tg_key = "6397870057:AAFzc7GXw8-L9ByhfV-znZooTtr3nyIG_rw";  // Replace with your actual Telegram Bot Token
    std::string tg_key = secure_getenv("TGAPI_KEY");  // Replace with your actual Telegram Bot Token


// ----------------------------
    unsigned int hmac_len = 0;
    unsigned char hmac_buf[4096]= {0x00};
    const char * webappkey = "WebAppData";
    unsigned int webappkey_len = strlen (webappkey);
    calculateHMAC_SHA256(tg_key,(const unsigned char *)webappkey,webappkey_len,
                         hmac_buf, hmac_len);
    //std::string secret = calculateHMAC_SHA256(tg_key,);
    unsigned int hmac_len_result = 0;
    unsigned char hmac_buf_result[4096]= {0x00};

    calculateHMAC_SHA256(dataToCheckStr, hmac_buf, hmac_len,
                         hmac_buf_result, hmac_len_result);

    string s_buf = "";
    for (int a = 0; a< hmac_len_result; a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", hmac_buf_result[a]);
        s_buf.append(tmp_buf);
    }

    fprintf(stdout, "checked: [%s]\n",dataToCheckStr.c_str());
    fprintf(stdout, "calculated:\t%s\nbased:\t\t%s\n", s_buf.c_str(), hash.c_str());
    std::string session;
    if (hash == s_buf)
    {
        fprintf(stdout, "SUCCESS TG %d\n",tg_idval);

    }
// ----------------------------

    int error=0;

    // create session
    std::string video;
    std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"video\":\"");
        response.append(video);
        response.append("\"}");
        fprintf(stdout, "response to client:[%s]\n", response.c_str());
        add_headers(*data.request,response);

        return 0;
    } // if no error

    response.append("\"}");
    add_headers(*data.request,response);

    return 0;
}



int
Requests::request_tgloginslot(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value user_id_j = root["user_id"];
    std::string user_id_s ;
    if (user_id_j != Json::Value::null)
    {
        if (user_id_j.isString())
        {
            user_id_s = user_id_j.asString();
            log_info("initData: %s \n",
                     user_id_j.asString().c_str());

        }
    }
    res = generate_session4www_tmp_with_balance(data.r_ctx, user_id_s.c_str());
		string session;
		string token = "1111";
    res = get_user_sesson_after_register_tg(data.r_ctx, user_id_s.c_str(),
                                             token, session);


    int error=0;

    // create session

    std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"session\":\"");
        response.append(session);
        response.append("\"}");
        fprintf(stdout, "response to client:[%s]\n", response.c_str());
        add_headers(*data.request,response);

        return 0;
    } // if no error

    response.append("\"}");
    add_headers(*data.request,response);

    return 0;
}


int
Requests::request_tglogin(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value initData = root["tglogin"];
    std::string initData_s ;
    if (initData != Json::Value::null)
    {
        if (initData.isString())
        {
            initData_s = initData.asString();
            log_info("initData: %s \n",
                     initData.asString().c_str());

        }
    }
// ------- PROCESS POSTFIELDS---------------

    std::string post_data = urldecode(initData_s);
    log_info("try to parse === [%s]",post_data.c_str());
    map <string, std::string> post_vals;
    std::vector<std::string> dataToCheck;
    int pos = 0;

    while (pos!=string::npos)
    {
        int name_pos = post_data.find_first_not_of('&',pos);
        int value_pos = post_data.find('=',name_pos);
        if(name_pos != std::string::npos && value_pos != std::string::npos)
        {
            std::string name = post_data.substr(name_pos,value_pos - name_pos);
            std::string value = post_data.substr(value_pos+1,post_data.find('&',value_pos+1)-value_pos -1);
            post_vals[name] = value;
            if (name != "hash")
                dataToCheck.push_back(name + "=" + value);
            log_info("[%s=%s]", name.c_str(), value.c_str());
            pos = post_data.find('&',value_pos);

        }
        else
        {
            pos = std::string::npos;
        }

    } //while npos
    int user_len = 0;
    char user_json[4049] = {0x00};
    if(post_vals.find("user")!=post_vals.end())
    {
        user_len = post_vals["user"].length();
        strncpy(user_json, post_vals["user"].c_str(),user_len);
        log_info("user =%s len=%d",
                 post_vals["user"].c_str(),
                 user_len);
    }
    Json::Value rroot;
    Json::Reader reader;
    int res_parsing = reader.parse(user_json,rroot);
    if (!res_parsing)
    {
        log_err("unable to parse request %s",user_json);
        vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
        for (auto a: vec_err)
        {
            log_err("%s",a.message.c_str());
        }
    }
    Json::Value tgid = rroot["id"];
    uint64_t tg_idval = 0;
    if (tgid  != Json::Value::null)
    {
        if (tgid.isInt64())
        {
            tg_idval = tgid.asInt64();
        }
    }
    std::string hash =  post_vals["hash"];
    std::sort(dataToCheck.begin(), dataToCheck.end());
    std::string dataToCheckStr ;
    int it=0;
    for (auto &a :dataToCheck)
    {
#if 1
        if (it)
        {
            dataToCheckStr += "\n";
            dataToCheckStr +=a ;
        }
        else
        {
            dataToCheckStr +=a ;

        }
        it++;
#else
        dataToCheckStr +=a ;
        dataToCheckStr += "\n";
#endif

    }
    // Calculate HMAC-SHA256
    std::string tg_key = secure_getenv("TGAPI_KEY");  // Replace with your actual Telegram Bot Token
    // std::string tg_key = "7186133948:AAEFezmNWjlcEBkCAS3MKbkYSzvQZBoVCNw";  // Replace with your actual Telegram Bot Token
    //senden.swertefun.com std::string tg_key = "6454350485:AAF4eC91dAyOIhDk89i66-IW7TsEgkfsmLE";  // Replace with your actual Telegram Bot Token


// ----------------------------
    unsigned int hmac_len = 0;
    unsigned char hmac_buf[4096]= {0x00};
    const char * webappkey = "WebAppData";
    unsigned int webappkey_len = strlen (webappkey);
    calculateHMAC_SHA256(tg_key,(const unsigned char *)webappkey,webappkey_len,
                         hmac_buf, hmac_len);
    //std::string secret = calculateHMAC_SHA256(tg_key,);
    unsigned int hmac_len_result = 0;
    unsigned char hmac_buf_result[4096]= {0x00};

    calculateHMAC_SHA256(dataToCheckStr, hmac_buf, hmac_len,
                         hmac_buf_result, hmac_len_result);

    string s_buf = "";
    for (int a = 0; a< hmac_len_result; a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", hmac_buf_result[a]);
        s_buf.append(tmp_buf);
    }

    fprintf(stdout, "checked: [%s]\n",dataToCheckStr.c_str());
    fprintf(stdout, "calculated:\t%s\nbased:\t\t%s\n", s_buf.c_str(), hash.c_str());
    std::string session;
    if (hash == s_buf)
    {
        fprintf(stdout, "SUCCESS TG %d\n",tg_idval);
        string token = "1111";
        string user = to_string(tg_idval);
        res = generate_session4www_tmp_with_balance(data.r_ctx, to_string(tg_idval).c_str());
        res = get_user_sesson_after_register_tg(data.r_ctx, to_string(tg_idval).c_str(),
                                             token, session);

    }
// ----------------------------

    int error=0;
    //int error = get_user_sesson_after_register(data.r_ctx, player_j.asCString(),
    //                                           token_j.asCString(), session);



    // create session

    std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"session\":\"");
        response.append(session);
        response.append("\"}");
        fprintf(stdout, "response to client:[%s]\n", response.c_str());
        add_headers(*data.request,response);

        return 0;
    } // if no error

    response.append("\"}");
    add_headers(*data.request,response);

    return 0;
}
int
Requests::request_renew(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session_j = root["session"];
    if (session_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }

    std::string session;
		// get tg_id and token
		string tg_id;
        string token;
		int error = get_tgid_and_token(data.r_ctx,session_j.asCString(), tg_id, token);	
		if (error == 0)
		{	
			error = generate_session4www_tmp_with_balanceV2(data.r_ctx, tg_id.c_str(),
                                             session);
    }	
		std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"session\":\"");
        response.append(session);
        response.append("\"}");
        add_headers(*data.request,response);
        return 0;
    } // if no error

    if (error == ERR_REGISTER_USER)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_USER_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN_TIMEOUT)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_TIMEOUT_MESSAGE);
        response.append("\"}");
    } // if token wrong

    if (error == ERR_REGISTER_RESEND)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_RESEND_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_SESSION_GENERATE)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_SESSION_GENERATE_MESSAGE);
        response.append("\"}");
    } // if token wrong

    add_headers(*data.request,response);

    return 0;
}


int
Requests::request_register_www(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value sms_gate_j = root["sms_gate"];
    int sms_gate = DEFAULT_TELEGRAM_GATE;
    if (sms_gate_j != Json::Value::null)
    {
        if (sms_gate_j.isString())
        {
            std::string sms_gate_s = sms_gate_j.asString();

            if (strncmp(sms_gate_s.c_str(),
                        PHP_TWILLO_GW,
                        PHP_TWILLO_GW_LEN)
                    ==0)
            {
                sms_gate = DEFAULT_SMS_TWILLO;
            }
        }
    }
    log_info("sms gate: %s [%d]",
             sms_gate_j.asString().c_str(), sms_gate);
    Json::Value player_j = root["player"];
    if (player_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }

    if (player_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
    if (!player_j.isString())
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
    }
    if (player_j.asString().length() > MAX_PLAYER_LEN)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
    Json::Value token_j = root["token"];

		//
		// get user tg_id
		//
		string tg_id;
		res = get_tgid_from_phone(data.r_ctx, player_j.asCString(), tg_id);
    if (token_j == Json::Value::null )
    {
        // register user (send message with registration code)
        int error = res;
				if (error == 0)
				{
					error =  register_user_www(data.r_ctx, tg_id.c_str(), sms_gate);
        }
				std::string response = "{\"error\":\"";
        response.append(to_string(error));
    		if (error == ERR_REGISTER_USER)
   		 	{
       		response.append("\",\"message\":\"");
        	response.append(ERR_REGISTER_USER_MESSAGE);
    		} // strange user 
        if (error== ERR_REGISTER_TIMEOUT)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_REGISTER_TIMEOUT_MESSAGE);
        }
        if (error== ERR_SESSION_READY_TOKEN)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_SESSION_READY_TOKEN_MESSAGE);
        }
        if (error== ERR_REGISTER_RESEND_TG)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_REGISTER_RESEND_TG_MESSAGE);
        }
        if (error== ERR_REGISTER_RESEND)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_REGISTER_RESEND_MESSAGE);
        }
        response.append("\"}");
        add_headers(*data.request,response);
        return 0;
    } // if token_j is null
    // get session from redis
    if (!token_j.isString())
    {
        finish_request_parse_err(*data.request,"token");
        return 0;
        //return nullptr;
    }
    if (token_j.asString().length() > MAX_TOKEN_LEN)
    {
        finish_request_parse_err(*data.request,"token");
        return 0;
        //return nullptr;
    }
    std::string session;
		int	error =  get_user_sesson_after_register(data.r_ctx, tg_id.c_str(),
                token_j.asCString(), session);
    std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"session\":\"");
        response.append(session);
        response.append("\"}");
        add_headers(*data.request,response);
        return 0;
    } // if no error

    if (error == ERR_REGISTER_USER)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_USER_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN_TIMEOUT)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_TIMEOUT_MESSAGE);
        response.append("\"}");
    } // if token wrong

    if (error == ERR_REGISTER_RESEND)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_RESEND_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_SESSION_GENERATE)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_SESSION_GENERATE_MESSAGE);
        response.append("\"}");
    } // if token wrong

    add_headers(*data.request,response);

    return 0;
}
  // 
/*

operatorId string
Identifier of the website or mobile app provided by
RGS Required

timestamp string
UTC time of the request, formatted as
yyyyMMddHHmmssffff

signature string
HMACSHA256 hash message, calculated using the
secret key and the accompanying operatorId and
timestamp values

token string
Session identifier provided by the Partner
Platform. It can be any string (hashed or not)
that will be used to identify the particular
session of the Player in the following format:
90dbbb443c9b4b3fbcfc59643206a123 

*/
/*
{
"operatorId":"H6654D11",
"timestamp":"202003092113371560",
"signature":"4712bf92ffc2917d15a2f5a273e39f0116667419aa4b6ac0b3baaf26fa3c4d20",
"token":"90dbbb443c9b4b3fbcfc59643206a123"
}
*/
/*
Response Parameters

timestamp string 
UTC time of the request, formatted as yyyyMMddHHmmssffff

signature string
HMACSHA256 hash message, calculated using the secret key and the
accompanying operatorId and timestamp values

errorCode small int
Enumerator of the error code for the Authenticate method request

playerId string 
Identifier of the Player assigned by the Operator

userName string
User name of the Player

currencyId string
Identifier of the player-preferred currency

balance decimal
Balance of the Player in the specified currency

birthDate DateTime
Birthday of the Player

firstName string 
First name of the Player

lastName string
Last name of the Player

gender int
Gender of the Player:
male = 1
female = 2

email string
Email address of the Player

isReal bool 
True, if the Player account is not a test account; otherwise, false


{
"timestamp":"202003092113371560",
"signature":"4712bf92ffc2917d15a2f5a273e39f0116667419aa4b6ac0b3baaf26fa3c4d20",
"errorCode":1,
"playerId":"18232",
"userName":"BlackKnight",
"currencyId":"USD",
"balance":121.3,
"birthDate":"1991-03-09 00:00:00.000",
"firstName":"John",
"lastName":"Doe",
"gender":1,
"email":"johndoe@email.com",
"isReal":true
}
*/
int
Requests::request_authenticate(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value operator_id_j = root["operatorId"];
    if (operator_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
		if (operator_id_j != "52C06E2F")
		{
        finish_request_parse_sgi_err(*data.request, RGS_ERR_OPERATOR_ID);
        return 0;

		}
    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j == Json::Value::null)
    {
        finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value signature_j = root["signature"];
    if (signature_j == Json::Value::null)
    {
        finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
/*
	check RGS signature
	
*/
		string timestamp = timestamp_j.asString();
		string operator_id = operator_id_j.asString();
		// Check operator_id

				

		string signature = signature_j.asString();
		int res_sign = rgs_check_sign (timestamp, operator_id,
													signature);
		if (res_sign != 0)
		{
      finish_request_parse_sgi_err(*data.request, RGS_ERR_SIGN);
			return 0;
		}

    Json::Value token_j = root["token"];
    if (token_j == Json::Value::null)
    {
        finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
	
	/*
		check token
	*/
	 
#if 1
	string player;
	string username;
	int balance;
	string first_name;
	string last_name;

  int result  = rgs_get_session(data.r_ctx,	 token_j.asCString(),
										 player, username, balance, first_name, last_name);
	if (result != 0)
	{
		
    finish_request_parse_sgi_err(*data.request, RGS_ERR_SESS_NOT_FOUND);
		return 0;
		// return
	}
    Json::Value root_out;
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = 1;	
	string email = "user";
	email.append(player);
	email.append("@wincasinogame.online");
	int gender = 1;	

	root_out["playerId"] = player;
	root_out["userName"] = username;
	root_out["currencyId"] = "KZT";
	root_out["balance"] = balance;
	root_out["birthDate"] = "1986-01-01";
	root_out["firstName"] = first_name;
	root_out["lastName"] = last_name;
	root_out["gender"] = gender;
	root_out["email"] = email;
	root_out["isReal"] = true;

	Json::StreamWriterBuilder sw;
	string out = Json::writeString(sw,root_out );
	//fprintf(stdout, "out: %s\n", out.c_str());
  add_headers(*data.request,out);
#endif	

	return 0;
}


    // 
int Requests::request_getbalance(Data& data) 
{
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value operator_id_j = root["operatorId"];
    if (operator_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
		if (operator_id_j != "52C06E2F")
		{
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_OPERATOR_ID);
        return 0;

		}
    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value signature_j = root["signature"];
    if (signature_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
/*
	check RGS signature
	
*/
		string timestamp = timestamp_j.asString();
		string operator_id = operator_id_j.asString();
		// Check operator_id

				

		string signature = signature_j.asString();
		int res_sign = rgs_check_sign (timestamp, operator_id,
													signature);
		if (res_sign != 0)
		{
      finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_SIGN);
			return 0;
		}

    Json::Value token_j = root["token"];
    if (token_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }

    Json::Value player_id_j = root["playerId"];
    if (player_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }

    Json::Value currency_id_j = root["currencyId"];
    if (currency_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
		if (currency_id_j != "KZT")
		{
        finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_CURRENCY_ID);
        return 0;

		}
	/*
		check token
	*/
	 
#if 1
	string player;
	string username;
	int balance;
	string first_name;
	string last_name;

  int result  = rgs_get_session(data.r_ctx,	 token_j.asCString(),
										 player, username, balance, first_name, last_name);
	if (result != 0)
	{
		
    finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_SESS_NOT_FOUND);
		return 0;
		// return
	}
	if (player_id_j!=player)
	{
    finish_request_parse_sgi_err_balance(*data.request, RGS_ERR_PLAYER_ID);
		return 0;
	}
    Json::Value root_out;
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = 1;	
	root_out["currencyId"] = "KZT";
	char buf_balance [100] = {0x00};
	snprintf(buf_balance,100,"%.4f",((float)balance)/100);
	root_out["balance"] = buf_balance;

	Json::StreamWriterBuilder sw;
	string out = Json::writeString(sw,root_out );
	//fprintf(stdout, "out: %s\n", out.c_str());
  add_headers(*data.request,out);
#endif	

	return 0;

}
int Requests::rgs_request_win(Data &data) 
{
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value operator_id_j = root["operatorId"];
    if (operator_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
		char *operator_id_s = secure_getenv("RGS_OPERATOR_ID");
		if (operator_id_j != operator_id_s)
		{
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_OPERATOR_ID);
        return 0;

		}
    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value signature_j = root["signature"];
    if (signature_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
/*
	check RGS signature
	
*/
		string timestamp = timestamp_j.asString();
		string operator_id = operator_id_j.asString();
		// Check operator_id

				

		string signature = signature_j.asString();
		int res_sign = rgs_check_sign (timestamp, operator_id,
													signature);
		if (res_sign != 0)
		{
      finish_request_parse_sgi_err_win(*data.request, RGS_ERR_SIGN);
			return 0;
		}

    Json::Value provider_id_j = root["providerId"];
    if (provider_id_j == Json::Value::null)
    {
      finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }

    Json::Value all_or_none_j = root["allOrNone"];
    if (all_or_none_j == Json::Value::null)
    {
      finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }
		
// get values and process each one

    Json::Value root_out;
		Json::Value global_player_id_j;
		int tmp_balance = 0;
		int items_error_flag = 1;
		for(auto &element: root["items"])
		{
			Json::Value json_item;
			int error_code = 1;

    	Json::Value player_id_j = element["playerId"];
			if (player_id_j == Json::Value::null)
			{
      	error_code = RGS_ERR_PLAYER_ID;
    		//finish_request_parse_sgi_err(*data.request, RGS_ERR_PLAYER_ID);
				//return 0;
			}
		
			unsigned int balance = 0;
			if (error_code == 1)
			{
				get_balance_senden(data.r_ctx,player_id_j.asCString(), balance);
			}
    	Json::Value currency_id_j = element["currencyId"];
    	if (currency_id_j == Json::Value::null)
    	{
      	error_code = RGS_ERR_PARAM_MISS;
        //finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        //return 0;
    	}
			if (currency_id_j != "KZT")
			{
      	error_code = RGS_ERR_CURRENCY_ID;
       // finish_request_parse_sgi_err(*data.request, RGS_ERR_CURRENCY_ID);
        return 0;

			}

    	Json::Value game_id_j = element["gameId"];
    	if (game_id_j == Json::Value::null)
    	{
      	error_code = RGS_ERR_PARAM_MISS;
        //finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        //return 0;
        //return nullptr;
    	}
			
			if (!game_id_j.isInt())
			{
      	error_code = RGS_ERR_GAME_NOT_FOUND;
			}
			else
			{	
				if (game_id_j.asInt()<=0)
				{
      		error_code = RGS_ERR_GAME_NOT_FOUND;
				}
			}
			// get ticket amount	
    	Json::Value bet_amount_j = element["winAmount"];
			unsigned int bet_amount = (int)(bet_amount_j.asFloat()*100);
			// check session, get balance
    	Json::Value tx_id_j = element["txId"];
    	Json::Value bet_tx_id_j = element["betTxId"];
			int jopa_win_trn = 0;
			if (error_code == 1)
			{
				if (bet_tx_id_j ==  Json::Value::null)
				{
					jopa_win_trn = 1;
					bet_tx_id_j = tx_id_j;
      	//	error_code = RGS_ERR_TRN_NOT_FOUND;
				}
			}
    	Json::Value round_id_j = element["roundId"];
#if 0
			if (round_id_j ==  Json::Value::null)
			{
      	error_code = RGS_ERR_TRN_NOT_FOUND;
			}
#endif		
			
			string player;
			if (player_id_j != Json::Value::null)
			{
				player = player_id_j.asCString();
			}

			int is_valid_bet_tx_id =0;
			int new_txId = 0;
			if (error_code == 1)
			{
				if (jopa_win_trn == 0)
				{
					is_valid_bet_tx_id = rgs_check_txId(data.r_ctx, player, bet_tx_id_j.asCString());
			 	}
				else
				{
					is_valid_bet_tx_id = jopa_win_trn;
				}
				if (is_valid_bet_tx_id == 0)
				{
					error_code = RGS_ERR_TRN_NOT_FOUND; // 7
				}
				else
				{
					new_txId = rgs_check_txId(data.r_ctx, player, tx_id_j.asCString());

					if (new_txId)
					{
						error_code = RGS_ERR_TRN_EXISTS;
					}
				} // is_valid_bet_tx_id
			}
			items_error_flag = error_code;
			if (all_or_none_j.asBool() && error_code !=1)
			{
				break;
			}
			fprintf(stdout, "txid checks is done\n");
			if (!new_txId && error_code == 1)
			{
				global_player_id_j = player_id_j;
				// change balance
					balance += bet_amount;
					tmp_balance += bet_amount;
					set_balance_senden_player(data.r_ctx, player_id_j.asCString(), 
							 balance);
					if (round_id_j !=  Json::Value::null)
					{
			fprintf(stdout, "round checks\n");

							rgs_add_roud_id(data.r_ctx, player_id_j.asCString(),
								bet_tx_id_j.asCString(),
										round_id_j.asCString());
						fprintf(stdout, "set win %d for tx %s\n", bet_amount,
									bet_tx_id_j.asCString());
						rgs_set_txid_win(data.r_ctx, player_id_j.asCString(),
									bet_tx_id_j.asCString(), bet_amount);
						fprintf(stdout, "set rgs_set_txid_roud_id\n");	
						rgs_set_txid_roud_id(data.r_ctx, player_id_j.asCString(),
									tx_id_j.asCString(),round_id_j.asCString(), 0);
						fprintf(stdout, "set rgs_set_txid_win\n");	
						rgs_set_txid_win(data.r_ctx, player_id_j.asCString(),
									tx_id_j.asCString(), bet_amount);
					}
					else
					{
						// add tx
						
						rgs_set_txid_win(data.r_ctx, player_id_j.asCString(),
									bet_tx_id_j.asCString(), bet_amount);
						rgs_set_txid_win(data.r_ctx, player_id_j.asCString(),
									tx_id_j.asCString(), bet_amount);

					}					
			}
		
			char buf_balance [100] = {0x00};
			if (error_code != 1 && error_code != 8)
			//if (error_code != 1)
			{
				balance = 0;
			}
			snprintf(buf_balance,100,"%.4f",((float)balance)/100);
			json_item["balance"] = buf_balance;
			json_item["errorCode"] = error_code;
			//json_item["info"] = tx_id_j;
			json_item["info"] = element["info"];
			json_item["externalTxId"] = tx_id_j;

			root_out["items"].append(json_item);	
			
		}
	/*
		check token
	*/
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		// reset balance
		root_out["items"].resize(0);
		unsigned int balance = 0;
		string player;
		get_balance_senden(data.r_ctx,global_player_id_j.asCString(), balance);
		balance -= tmp_balance;
		set_balance_senden_player_notxid(data.r_ctx,global_player_id_j.asCString(), balance);
	}
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = 1;	
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		root_out["errorCode"] = items_error_flag;	
	}
	
	//root_out["currencyId"] = "KZT";

	Json::StreamWriterBuilder sw;
	string out = Json::writeString(sw,root_out );
	//fprintf(stdout, "out: %s\n", out.c_str());
  add_headers(*data.request,out);

	return 0;

}



int Requests::rgs_request_bet(Data &data, Json::Value & root) 
{
    int res = 0;
    Json::Value operator_id_j = root["operatorId"];
    if (operator_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_bet(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
	char *operator_id_s = secure_getenv("RGS_OPERATOR_ID");
		if (operator_id_j != operator_id_s)
		{
        finish_request_parse_sgi_err_bet(*data.request, RGS_ERR_OPERATOR_ID);
        return 0;

		}
    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j == Json::Value::null)
    {
        finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value signature_j = root["signature"];
    if (signature_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_bet(*data.request, RGS_ERR_SIGN);
        return 0;
        //return nullptr;
    }
/*
	check RGS signature
	
*/
		string timestamp = timestamp_j.asString();
		string operator_id = operator_id_j.asString();
		// Check operator_id

				

		string signature = signature_j.asString();
		int res_sign = rgs_check_sign (timestamp, operator_id,
													signature);
		if (res_sign != 0)
		{
      finish_request_parse_sgi_err_bet(*data.request, RGS_ERR_SIGN);
			return 0;
		}

    Json::Value provider_id_j = root["providerId"];
    if (provider_id_j == Json::Value::null)
    {
      finish_request_parse_sgi_err_bet(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }

    Json::Value all_or_none_j = root["allOrNone"];
    if (all_or_none_j == Json::Value::null)
    {
      finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }
		
// get values and process each one

    Json::Value root_out;
		Json::Value global_token_j;
		int tmp_balance = 0;
		int items_error_flag = 1;
		for(auto &element: root["items"])
		{
			Json::Value json_item;
			int error_code = 1;

    	Json::Value token_j = element["token"];
    	if (token_j == Json::Value::null)
    	{
      	error_code = RGS_ERR_PARAM_MISS;
    	}
			string player;
			unsigned int balance = 0;
			get_balance_senden(data.r_ctx, token_j.asCString(),balance,player);
    	Json::Value player_id_j = element["playerId"];
			if (player.length()<5)
			{
      	error_code = RGS_ERR_SESS_NOT_FOUND;
			}
			else
			{
				if (player_id_j != player)
				{
      		error_code = RGS_ERR_PLAYER_ID;
    			//finish_request_parse_sgi_err(*data.request, RGS_ERR_PLAYER_ID);
					//return 0;
				}
			}

			global_token_j = token_j;
    	Json::Value currency_id_j = element["currencyId"];
    	if (currency_id_j == Json::Value::null)
    	{
      	error_code = RGS_ERR_PARAM_MISS;
        //finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        //return 0;
    	}
			if (currency_id_j != "KZT")
			{
      	error_code = RGS_ERR_CURRENCY_ID;
       // finish_request_parse_sgi_err(*data.request, RGS_ERR_CURRENCY_ID);
			}

    	Json::Value game_id_j = element["gameId"];
    	if (game_id_j == Json::Value::null)
    	{
      	error_code = RGS_ERR_PARAM_MISS;
        //finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
        //return 0;
    	}
			
			if (!game_id_j.asInt())
			{
      	error_code = RGS_ERR_GAME_NOT_FOUND;
			}
			
			// get ticket amount	
    	Json::Value bet_amount_j = element["betAmount"];
			unsigned int bet_amount = (int)(bet_amount_j.asFloat()*100);
			// check session, get balance
    	Json::Value change_balance = element["changeBalance"];
    	Json::Value tx_id_j = element["txId"];
    	Json::Value round_id_j = element["roundId"];
			if (round_id_j ==  Json::Value::null)
			{
      	error_code = RGS_ERR_PARAM_MISS;
			}
			int rolled_round_id = rgs_check_rolled_round(data.r_ctx, player.c_str(),
														round_id_j.asCString());
			if (rolled_round_id)
			{
				error_code = RGS_ERR_TRN_ALREADY_ROLLED;
			}
			int new_txId = rgs_check_txId(data.r_ctx, player, tx_id_j.asCString());
			if (new_txId)
			{
				error_code = RGS_ERR_TRN_EXISTS;
			}
			items_error_flag = error_code;
			if (error_code != RGS_ERR_TRN_EXISTS)
			{
				if (all_or_none_j.asBool() && error_code !=1)
				{
					break;
				}
			}
			if (change_balance.asBool() && !new_txId && error_code == 1)
			{
				// change balance
				if (balance < bet_amount)
				{
					// insufficient balance
					error_code = 	RGS_ERR_LOW_BALANCE;
				}	
				else
				{
					balance -= bet_amount;
					tmp_balance += bet_amount;
					rgs_set_txid_roud_id(data.r_ctx, player_id_j.asCString(),
									tx_id_j.asCString(),round_id_j.asCString(), bet_amount);
					set_balance_senden_player(data.r_ctx, player_id_j.asCString(), balance);
				}
			}
		
			char buf_balance [100] = {0x00};
			if (error_code != 1 && error_code != 8)
			//if (error_code != 1)
			{
				balance = 0;
			}
			snprintf(buf_balance,100,"%.4f",((float)balance)/100);
			json_item["balance"] = buf_balance;
			json_item["errorCode"] = error_code;
			//json_item["info"] = tx_id_j;
			json_item["info"] = element["info"];
			if (error_code == 1 || error_code == RGS_ERR_TRN_EXISTS)
			{
				json_item["externalTxId"] = tx_id_j;
			}
			root_out["items"].append(json_item);	

			
		}
	/*
		check token
	*/
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		// reset balance
		root_out["items"].resize(0);
		unsigned int balance = 0;
		string player;
		get_balance_senden(data.r_ctx, global_token_j.asCString(),balance,player);
		balance += tmp_balance;
		set_balance_senden(data.r_ctx, global_token_j.asCString(), balance);
	}
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = 1;	
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		root_out["errorCode"] = items_error_flag;	
	}
	
	//root_out["currencyId"] = "KZT";

	Json::StreamWriterBuilder sw;
	string out = Json::writeString(sw,root_out );
	//fprintf(stdout, "out: %s\n", out.c_str());
  add_headers(*data.request,out);

	return 0;

}


    // RGS Refund 
int
Requests::request_refund(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value operator_id_j = root["operatorId"];
    if (operator_id_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value timestamp_j = root["timestamp"];
    if (timestamp_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
    Json::Value signature_j = root["signature"];
    if (signature_j == Json::Value::null)
    {
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_PARAM_MISS);
        return 0;
        //return nullptr;
    }
/*
	check RGS signature
	
*/
		string timestamp = timestamp_j.asString();
		string operator_id = operator_id_j.asString();
		// Check operator_id
		char *operator_id_s = secure_getenv("RGS_OPERATOR_ID");

		if (operator_id_j != operator_id_s)
		{
        finish_request_parse_sgi_err_win(*data.request, RGS_ERR_OPERATOR_ID);
        return 0;

		}
				

		string signature = signature_j.asString();
		int res_sign = rgs_check_sign (timestamp, operator_id,
													signature);
		if (res_sign != 0)
		{
      finish_request_parse_sgi_err_win(*data.request, RGS_ERR_SIGN);
			return 0;
		}

    Json::Value provider_id_j = root["providerId"];
    if (provider_id_j == Json::Value::null)
    {
      finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }

    Json::Value all_or_none_j = root["allOrNone"];
    if (all_or_none_j == Json::Value::null)
    {
      finish_request_parse_sgi_err(*data.request, RGS_ERR_PARAM_MISS);
      return 0;
      //return nullptr;
    }
		
// get values and process each one

    Json::Value root_out;
		Json::Value global_player_id_j;
		int tmp_balance = 0;
		int items_error_flag = 1;
		for(auto &element: root["items"])
		{
			Json::Value json_item;
			int error_code = 1;

    	Json::Value player_id_j = element["playerId"];
		
			unsigned int balance = 0;
			int get_balance_res = get_balance_senden(data.r_ctx,player_id_j.asCString(), balance);
			fprintf(stdout,"return get balance %d\n", get_balance_res);
			if (get_balance_res !=0 )
			{
      	error_code = RGS_ERR_PLAYER_ID;
    		//finish_request_parse_sgi_err(*data.request, RGS_ERR_PLAYER_ID);
				//return 0;
			}

			// check session, get balance
    	Json::Value tx_id_j = element["txId"];
	
			// check txId

    	Json::Value original_tx_id_j = element["originalTxId"];
    	Json::Value is_refund_round_js = element["refundRound"];

    	Json::Value round_id_j = element["roundId"];
#if 0
			if (round_id_j ==  Json::Value::null)
			{
      	error_code = RGS_ERR_TRN_NOT_FOUND;
			}
#endif		
			string player = player_id_j.asCString();
			int is_valid_bet_tx_id =0;
			int is_have_already_win = 0;
			int is_have_already_bet = 0;
			if (error_code == 1)
			{
				if (is_refund_round_js.asBool())
				{
			 		is_valid_bet_tx_id = rgs_check_round_id(data.r_ctx, player, round_id_j.asCString());
					fprintf(stdout, "check round id %s = %d\n",
							round_id_j.asCString(),is_valid_bet_tx_id);
					if (is_valid_bet_tx_id == -1)
					{
						
						error_code =RGS_ERR_TRN_NOT_FOUND; 
					}
				}
				else
				{
			 			is_valid_bet_tx_id = rgs_check_txId(data.r_ctx, player, original_tx_id_j.asCString());
						rgs_get_win_from_txid(data.r_ctx, player_id_j.asCString(), 
							original_tx_id_j.asCString(),
									 is_have_already_win);
					fprintf(stdout, "already win for trn %s is %d\n",
					original_tx_id_j.asCString(), is_have_already_win);
				}
			}
	

				
			int new_txId =0;
			if (error_code == 1)
			{
				new_txId =  rgs_check_txId(data.r_ctx, player, tx_id_j.asCString());
				if (new_txId)
				{
					error_code = RGS_ERR_TRN_EXISTS;
				}
				if (!is_valid_bet_tx_id)
				{
					//fprintf(stdout, "not valid bet trn %s\n", original_tx_id_j.asCString());
      		error_code = RGS_ERR_TRN_NOT_FOUND;
				}
			}
			
				if (original_tx_id_j != Json::Value::null)
				{
				rgs_get_amount_from_txid(data.r_ctx, player_id_j.asCString(), 
							original_tx_id_j.asCString(),
									 is_have_already_bet);
				}
	
			if (is_refund_round_js.asBool()== false && is_have_already_win)
			{
				// check may be trn have not bet. it is just win
				if (is_have_already_bet >0 )	
				{	
					error_code = RGS_ERR_TRN_ALREADY_WON;
				}
				else
				{
					is_have_already_win = 0;
				}
				//fprintf(stdout, "check is_have_already_win: %s bet is %d\n", 
				//	original_tx_id_j.asCString(),is_have_already_bet);
			}

			items_error_flag = error_code;
			if (all_or_none_j.asBool() && error_code !=1)
			{
				break;
			}
			if (!new_txId && error_code == 1)
			{
				global_player_id_j = player_id_j;
				// change balance
					int bet_amount = 0;
			if (is_refund_round_js.asBool())
			{
				int res = rgs_get_amount_from_round(data.r_ctx, player_id_j.asCString(),
									  round_id_j.asCString(),
									 	bet_amount);
				
				rgs_set_rolled_round(data.r_ctx, player_id_j.asCString(),
									  round_id_j.asCString());
				fprintf(stdout, "refound in round: %d\n", bet_amount);
			}
			else
			{		
					int res =0;
				if (is_have_already_bet == 0)
				{
				 rgs_get_win_from_txid(data.r_ctx, player_id_j.asCString(), 
							original_tx_id_j.asCString(),
									 bet_amount);
					bet_amount *=-1;
				}
				else
				{
				 rgs_get_amount_from_txid(data.r_ctx, player_id_j.asCString(), 
							original_tx_id_j.asCString(),
									 bet_amount);
				}
							rgs_set_refunded_txid(data.r_ctx, player_id_j.asCString(), 
							original_tx_id_j.asCString(), bet_amount);
					// add txid to history
					if (round_id_j != Json::Value::null)
					{
						rgs_set_txid_roud_id(data.r_ctx, player_id_j.asCString(),
									tx_id_j.asCString(),round_id_j.asCString(), bet_amount*(-1));
					}
		
			}
					balance += bet_amount;
					tmp_balance += bet_amount;
					set_balance_senden_player(data.r_ctx, player_id_j.asCString(), 
							 balance);
					
					if (round_id_j !=  Json::Value::null)
					{
						//	rgs_set_txid_roud_id_win(data.r_ctx, player_id_j.asCString(),
						//				tx_id_j.asCString(),round_id_j.asCString(), bet_amount);
						
					}
			}
		
			char buf_balance [100] = {0x00};
			if (error_code != 1 && error_code != 8)
			{
				balance = 0;
			}
			snprintf(buf_balance,100,"%.4f",((float)balance)/100);
			json_item["balance"] = buf_balance;
			json_item["errorCode"] = error_code;
			//json_item["info"] = tx_id_j;
			json_item["info"] = element["info"];
			json_item["externalTxId"] = tx_id_j;

			root_out["items"].append(json_item);	
			
		}
	/*
		check token
	*/
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		// reset balance
		root_out["items"].resize(0);
		unsigned int balance = 0;
		string player;
		get_balance_senden(data.r_ctx,global_player_id_j.asCString(), balance);
		balance -= tmp_balance;
		set_balance_senden_player_notxid(data.r_ctx,global_player_id_j.asCString(), balance);
	}
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = 1;	
	if (all_or_none_j.asBool() && items_error_flag !=1)
	{
		root_out["errorCode"] = items_error_flag;	
	}
	
	//root_out["currencyId"] = "KZT";

	Json::StreamWriterBuilder sw;
	string out = Json::writeString(sw,root_out );
	//fprintf(stdout, "out: %s\n", out.c_str());
  add_headers(*data.request,out);

	return 0;


}


int
Requests::request_register(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value player_j = root["player"];
    if (player_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
    if (!player_j.isString())
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
    }
    if (player_j.asString().length() > MAX_PLAYER_LEN)
    {
        finish_request_parse_err(*data.request,"player");
        return 0;
        //return nullptr;
    }
    Json::Value token_j = root["token"];
    if (token_j == Json::Value::null)
    {
        // register user (send message with registration code)
        int error = register_user(data.r_ctx, player_j.asCString());
        std::string response = "{\"error\":\"";
        response.append(to_string(error));
        if (error== ERR_REGISTER_TIMEOUT)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_REGISTER_TIMEOUT_MESSAGE);
        }
        if (error== ERR_REGISTER_RESEND)
        {
            response.append("\",\"message\":\"");
            response.append(ERR_REGISTER_RESEND_MESSAGE);
        }
        response.append("\"}");
        add_headers(*data.request,response);
        return 0;
    } // if token_j is null
    // get p12 from redis
    if (!token_j.isString())
    {
        finish_request_parse_err(*data.request,"token");
        return 0;
        //return nullptr;
    }
    if (token_j.asString().length() > MAX_TOKEN_LEN)
    {
        finish_request_parse_err(*data.request,"token");
        return 0;
        //return nullptr;
    }
    std::string secret_path;
    int error = get_user_p12(data.r_ctx, player_j.asCString(), token_j.asCString(), secret_path);
    std::string response = "{\"error\":\"";
    response.append(to_string(error));
    if (error == 0)
    {
        // ok, send secret path
        response.append("\",\"p12\":\"");
        response.append(secret_path);
        response.append("\"}");
        add_headers(*data.request,response);
        return 0;
    } // if no error

    if (error == ERR_REGISTER_USER)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_USER_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_TOKEN_TIMEOUT)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_TOKEN_TIMEOUT_MESSAGE);
        response.append("\"}");
    } // if token wrong

    if (error == ERR_REGISTER_RESEND)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_REGISTER_RESEND_MESSAGE);
        response.append("\"}");
    } // if token wrong
    if (error == ERR_P12_GENERATE)
    {
        response.append("\",\"message\":\"");
        response.append(ERR_P12_GENERATE_MESSAGE);
        response.append("\"}");
    } // if token wrong

    add_headers(*data.request,response);
    return 0;
}
int
Requests::request_balance_and_bonus(Data& data) {
    int res = 0;
		unsigned long long gross_bet = 0, gross_win = 0;
		unsigned long long gross_bet_bonus = 0, gross_win_bonus = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }

    unsigned int balance;
    int demo = 0;
    std::string player;

    int result = 0;
		unsigned int bonus_balance = 0;
    //  int game_id = 0;
    //  log_info("Balance SESSION: %s gameprov %d", session.asCString(), game_provider);
        result  = get_balance_senden(data.r_ctx, session.asCString(),
			balance,bonus_balance, demo, player, gross_bet, gross_win, gross_bet_bonus, gross_win_bonus);
    //do_request_balance(session.asCString(), balance);
    // Request balance from reddis!!!
    if (result == 0)
    {
        std::string out_json = "{\"balance\":";
        out_json.append(to_string((balance+bonus_balance)/100));
        out_json.append(",\"bonus_balance\":");
        out_json.append(to_string(bonus_balance/100));
        out_json.append(",\"demo\":");
        out_json.append(to_string(demo));
        out_json.append("}");
        fcgx_put_str_json(*data.request, out_json);
    }
    //----- end balance *data.request ------

    return 0;
}


int
Requests::request_balance(Data& data) {
    int res = 0;
		unsigned long long gross_bet = 0, gross_win = 0;
		unsigned long long gross_bet_bonus = 0, gross_win_bonus = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }

    unsigned int balance;
    int demo = 0;
    std::string player;

    int result = 0;
		unsigned int bonus_balance = 0;
    //  int game_id = 0;
    //  log_info("Balance SESSION: %s gameprov %d", session.asCString(), game_provider);
        result  = get_balance_senden(data.r_ctx, session.asCString(),
			balance,bonus_balance, demo, player, gross_bet, gross_win, gross_bet_bonus, gross_win_bonus);
    //do_request_balance(session.asCString(), balance);
    // Request balance from reddis!!!
    if (result == 0)
    {
        std::string out_json = "{\"error\":0,\"balance\":";
        out_json.append(to_string(balance/100));
        out_json.append(",\"bonus_balance\":");
        out_json.append(to_string(bonus_balance/100));
        out_json.append(",\"demo\":");
        out_json.append(to_string(demo));
        out_json.append("}");
				fprintf(stdout, "%s\n", out_json.c_str());
        fcgx_put_str_json(*data.request, out_json);
    }
    //----- end balance *data.request ------

    return 0;
}

int
Requests::request_getmybets(Data& data) {
    int res = 0;
    Json::Value root;
		unsigned long long gross_bet = 0, gross_win = 0;
		unsigned long long gross_bet_bonus = 0, gross_win_bonus = 0;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    bool allBets = false;
    std::string out;

    Json::Value action = root["action"];
    if (action == Json::Value::null)
    {
        return 0;
        //return nullptr;
    }
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }

    Json::Value game_id_j = root["game_id"];
    if(game_id_j == Json::Value::null)
    {
        allBets = true;
        res = get_all_my_bets(data.r_ctx, session.asCString(), out);
        if(res!=0)
        {
            finish_request_parse_err(*data.request,"all_bets");
            return 0;
        }
    }
    else
    {
        int game_id = json_to_int_patch_hujatch(game_id_j);
        if (game_id <7770 ||game_id>7783 )
        {
            finish_request_parse_err(*data.request,">game_id");
            return 0;
        }

        Json::Value draw_id_j = root["draw_id"];
        int draw_id = 0;
        if(draw_id_j == Json::Value::null)
        {
            //  finish_request_parse_err(*data.request,"draw_id null");
            //  continue;
            int res = get_current_draw(data.r_ctx,game_id,draw_id);
            fprintf(stdout, "!! *data.request bets without draw. Current is %d\n",
                    draw_id);
            if (res !=0 )
            {
                // finish *data.request
                // no opened draw
                finish_request_parse_err(*data.request,"turned off draw");
                return 0;
            }
        }
        else
        {
            draw_id = json_to_int_patch_hujatch(draw_id_j);
        }

        res = get_my_bets(data.r_ctx, session.asCString(),game_id,draw_id, out);
        if(res!=0)
        {
            finish_request_parse_err(*data.request,"bets");
            return 0;
        }
    }

    unsigned int balance = 0;
    int demo = 0;
    int err_code = 0;
    std::string player;
    //int game_id = 0;
    int result = 0;
		unsigned int bonus_balance = 0;
    //  log_info("Balance SESSION: %s gameprov %d", session.asCString(), game_provider);
        result  = get_balance_senden(data.r_ctx, session.asCString(),
					balance,bonus_balance, demo, player, gross_bet, gross_win,
					  gross_bet_bonus, gross_win_bonus );
    //if (result !=0 || !balance)
    if (result !=0)
    {
        err_code = result;
        finish_request_parse_err(*data.request,"balance");
        return 0;
    }


    std::string out_json = "{\"action\":\"";
    if(allBets)
    {
        out_json.append("get_all_my_bets\", ");
    }
    else
    {
        out_json.append("get_my_bets\", ");
    }
    out_json.append(out);
    out_json.append(",\"balance\":");
    out_json.append(to_string(balance));
    out_json.append(",\"bonus_balance\":");
    out_json.append(to_string(bonus_balance));
    out_json.append(",\"demo\":");
    out_json.append(to_string(demo));
    out_json.append(",\"err\":0}");
    fcgx_put_str_json(*data.request, out_json);

    return 0;
}

int
Requests::request_buybet(Data& data) {
    int res = 0;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) {
        return 0;
    }

    Json::Value action = root["action"];
    if (action == Json::Value::null)
    {
        return 0;
        //return nullptr;
    }
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err(*data.request,"session (|)");
        return 0;
    }
    Json::Value game_id_j = root["game_id"];
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"game_id");
        return 0;
        //return nullptr;
    }
    Json::Value bets = root["bets"];
    if (bets == Json::Value::null)
    {
        finish_request_parse_err(*data.request,"bets");
        return 0;
    }
    if (!bets.isArray())
    {
        finish_request_parse_err(*data.request,"bets");
        return 0;
    }

    unsigned int balance = 0;
		unsigned long long gross_bet = 0, gross_win = 0;
		unsigned long long gross_bet_bonus = 0, gross_win_bonus = 0;
    int demo = 0;
    int err_code = 0;
    int game_id = 0;
    int price = 0;
    int bet_count = 0;
		unsigned int bonus_balance = 0;
    std::string player;
    //  log_info("Balance SESSION: %s gameprov %d", session.asCString(), game_provider);
    int result = 0;
        result  = get_balance_senden(data.r_ctx, session.asCString(),
					balance,bonus_balance, demo, player, gross_bet, gross_win,
				 gross_bet_bonus, gross_win_bonus);
    //int result = get_balance(data.r_ctx, session.asCString(),balance, demo,player);
    std::string out_json;
    if (result !=0 || !balance)
    {
        err_code = result;
        finish_request_parse_err(*data.request,"balance");
        return 0;
    }
    game_id = json_to_int_patch_hujatch(game_id_j);
    int err_bet = 0;
    for (auto a : bets)
    {
        int price;
				price = a["price"].asInt();
        if (a.empty() || !a.isObject())
        {
            err_bet++;
            break;
        }
            int draw_id = 0;
            if (a["draw_id"] != Json::Value::null)
            {
                draw_id = json_to_int_patch_hujatch(a["draw_id"]);
            }
            if (a["draw"] != Json::Value::null)
            {
                draw_id = json_to_int_patch_hujatch(a["draw"]);
            }
            if (a["draw_id"] == Json::Value::null &&
                    a["draw"] == Json::Value::null
               )
            {
                finish_request_parse_err(*data.request,"draw_id");
                err_bet++;
                break;
            }
            // no wai
            if (draw_id<1)
            {
                finish_request_parse_err(*data.request,"draw_id suck");
                err_bet++;
                break;

                // no wai
            }
            //fprintf(stdout, "bet:%d %llu \n",json_to_int_patch_hujatch(a["draw"]),json_to_int_patch_hujatch(a["number"]));
            if (game_id == BINGO_CLUB_GAME_ID||
                    game_id == NAVAL_BATTLE_GAME_ID)
            {
                res = buy_bet_bingo(data.r_ctx, session.asCString(),a["number"].asCString(),
                                    price, game_id,draw_id);
            } //BINGO
            if (game_id == KENO_GAME_ID ||
                    game_id == BINGO37_GAME_ID||
                    game_id == BINGO38_GAME_ID ||
										game_id == LOTO_5x36_GAME_ID ||
										game_id == LOTO_6x45_GAME_ID ||
										game_id == LOTO_7x49_GAME_ID ||
										game_id == POWERBALL_GAME_ID 
											

               )// KENO
            {
                if (a["mask"].empty() || !a["mask"].isArray())
                {
                    log_err("no mask");
                    finish_request_parse_err(*data.request,"mask0");
                    err_bet++;
                    break;
                }
                else
                {
                    std::string mask;
                    int x=0;
                    if (!a["mask"].size())
                    {
                        log_err("no mask");
                        finish_request_parse_err(*data.request,"mask1");
                        err_bet++;
                        break;
                    }
                    for (auto ff:a["mask"])
                    {
                        if(x>9 && game_id==KENO_GAME_ID)
                            break;
                        if(x)
                            mask.append(",");
                        mask.append(to_string(ff.asInt()));
                        x++;
                    }
										long number = 2500000000 + time(nullptr);
										string s_number = to_string(number);
                    res = buy_bet(data.r_ctx, session.asCString(),s_number.c_str(),
                                  mask.c_str(), price, game_id,draw_id);
                }
            } // if KENO
						if (bonus_balance > price*100)
						{
							bonus_balance -= price*100;
						}
						else
						{
            	balance -= price*100;
						}
    }
    if (err_bet)
    {
        //finish_request_parse_err(*data.request,"number");
        return 0;
    }
    //log_info("read %d bytes", read_bytes);
		
  	res  = set_balance_senden(data.r_ctx,session.asCString() ,balance,bonus_balance, gross_bet, gross_win, gross_bet_bonus, gross_win_bonus);
    syslog(LOG_INFO, "%s", data.buf_in);
    out_json = "{\"action\":\"buy_bet\", ";

    out_json.append("\"balance\":\"");
    out_json.append(to_string(balance+bonus_balance));
    out_json.append("\",\"err\":0}");
    fcgx_put_str_json(*data.request, out_json);

    return 0;
}

int
Requests::request_bet(Data& data) {
    int res = 0;
    pthread_mutex_t bet_mutex;
    Json::Value root;
    if ((res = parse_json(data, root)) != 0) 
		{
			fprintf(stdout, "some shit bet json \n");
      return 0;
    }
		unsigned long long gross_bet = 0, gross_win = 0;
		unsigned long long gross_bet_bonus = 0, gross_win_bonus = 0;
    uint64_t start_ts =  std::chrono::duration_cast<std::chrono::nanoseconds>
                         (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::string prefix = "main_req";
    vector<_bet> bets;
    std::string error_message;
    Json::Value signature_j = root["signature"];
    if (signature_j != Json::Value::null)
    {
      rgs_request_bet(data, root);
      return 0;
    }

		
    Json::Value action = root["action"];
    if (action == Json::Value::null)
    {
        finish_request_parse_err((*data.request),"action");
        return 0;
        //return nullptr;
    }
    Json::Value session = root["session"];
    if (session == Json::Value::null)
    {
        finish_request_parse_err((*data.request),"session");
        return 0;
        //return nullptr;
    }
    if (session.asString().length()<50)
    {
        finish_request_parse_err((*data.request),"session (|)");
        return 0;
    }
    Json::Value game_id_j = root["game_id"];
    Json::Value price_j = root["price"];
    Json::Value bet_count_j = root["bet_count"];
    int game_id = json_to_int_patch_hujatch(game_id_j);
    if (game_id_j == Json::Value::null)
    {
        finish_request_parse_err((*data.request),"game_id");
        return 0;
        //return nullptr;
    }
    if (price_j == Json::Value::null)
    {
        finish_request_parse_err((*data.request),"price");
        return 0;
        //return nullptr;
    }
    if (bet_count_j == Json::Value::null)
    {
        finish_request_parse_err((*data.request),"bet_count");
        return 0;
        //return nullptr;
    }
		if (game_id==211301)
		{
			game_id=2134;
		}
		if (game_id==211200)
		{
			game_id=2134;
		}
		if (game_id==222200)
		{
			game_id=2134;
		}
		if (game_id==2125)
		{
			game_id=2134;
		}
		if (game_id==2120)
		{
			game_id=2134;
		}
		if (game_id==2133)
		{
			game_id=2134;
		}
		if (game_id==2135)
		{
			game_id=2131;
		}
		if (game_id==2136)
		{
			game_id=2130;
		}
		if (game_id==2137)
		{
			game_id=2130;
		}
		if (game_id==2138)
		{
			game_id=2130;
		}
		if (game_id==2139)
		{
			game_id=2134;
		}
		if (game_id==2140)
		{
			game_id=2130;
		}
		if (game_id==2141)
		{
			game_id=2130;
		}
		if (game_id==2142)
		{
			game_id=2130;
		}
		if (game_id==2143)
		{
			game_id=2134;
		}
		if (game_id==2144)
		{
			game_id=2134;
		}
		if (game_id==2145)
		{
			game_id=2134;
		}
		if (game_id==2146)
		{
			game_id=2134;
		}
		if (game_id==2147)
		{
			game_id=2130;
		}
		if (game_id==2148)
		{
			game_id=2130;
		}
		if (game_id==2149)
		{
			game_id=2130;
		}
		if (game_id==2127)
		{
			game_id=2134;
		}
		if (game_id==2600)
		{
			game_id=2134;
		}
		vector <int> game_list = {2110,2111, 2112,2113,2114,2126, 2128,2129, 2130,2131,2132,2134, 2500,7770,7771,7775,7776,7777, 7780,7781,7782,7783};
		//for (auto g_id: game_list)
		//{
		//	fprintf(stdout, "g[%d]: %d\n",game_id, g_id);
		//} 
		auto it_find = find(game_list.begin(), game_list.end(), game_id);
		
    if (it_find == game_list.end()) 
    {
        finish_request_parse_err((*data.request),"game_id");
        return 0;
    }



    int price = json_to_int_patch_hujatch(price_j);
    if (price<=0) // TODO better check price
    {
        finish_request_parse_err((*data.request),"price");
        return 0;
        //return nullptr;
    }
    int bet_count = json_to_int_patch_hujatch(bet_count_j);
    if(bet_count<1)
        bet_count=1;
    if (bet_count>97)
    {
        finish_request_parse_err((*data.request),"bet_count");
        return 0;
        //return nullptr;
    }
    if (game_id ==211101 )
    {
        bet_count=1;
    }
    if (game_id ==2115 )
    {
        bet_count=8;
    }
    if (game_id ==211500 )
    {
        bet_count=40;
    }

    log_info("bet_count= %d action=%s game_id= %d price =%d",bet_count, action.asCString(),
             game_id,price);

    //log_info("read %d bytes", read_bytes);
    syslog(LOG_INFO, "%s", data.buf_in);
    const char* server_name = FCGX_GetParam("SERVER_NAME", (*data.request).envp);
    std::string out_json = "{\"action\":\"get_bet\", ";
    int demo_bet_error;
    unsigned int balance = 0;
    int demo = 0;
    int err_code = 0;
    std::string player;
    int bonus_amount = 0;
		unsigned int bonus_balance = 0;
    int result = 0;
        result  = get_balance_senden(data.r_ctx, session.asCString(),
										balance,bonus_balance, demo, player, gross_bet, 
											gross_win, gross_bet_bonus, gross_win_bonus);
        demo = 0; // TODO!!!! ALARM!!! remove on PRODUCTION!!!!!!
// !!!!!!! ^^^^^^ ALARM !!!! only for test!!!
    //int result = get_balance(data.r_ctx, session.asCString(),balance, demo,player);
		fprintf(stdout, "current balance = %d price = %d\n", balance, price*bet_count);
    std::string s_session = session.asString();
    if (result !=0  || ((balance+bonus_balance)/100 < bet_count*price))
    {
        std::string out_json = "{\"balance\":";
        out_json.append(to_string(balance/100));
        out_json.append(",\"demo\":");
        out_json.append(to_string(demo));
        out_json.append(", \"error\":4}");
        fcgx_put_str_json(*data.request, out_json);
        return 0;
    }

    out_json.append("\"bets\":[");
    if (game_id >= 7770 && game_id<=7783)
    {
        // TODO get_current draw
        int draw = 0;
        res = get_current_draw(data.r_ctx,game_id,draw);
        if (res !=0 )
        {
            // finish (*data.request)
            // no opened draw
            finish_request_parse_err((*data.request),"turned off draw");
            return 0;
        }

        if(game_id == 7771)
        {
            vector<ShipTicketInfo> bibi;
            bibi.resize(bet_count);
            for (int a = 0; a< bet_count; a++)
            {
                char bet_char[1024] = {0x00};
                char bet_out[1024*10] = {0x00};
                bibi[a].set_draw(draw);
                bibi[a].set_price(price);
                int res = bibi[a].copy_mask(bet_char, 1024);
                register_bet(data.r_ctx,session.asCString(),bibi[a].get_number(), bet_char, bibi[a].get_price(), game_id, bibi[a].get_draw());
                res = bibi[a].copy_json(bet_out, 1024);
                if (a)
                    out_json.append(",");
                out_json.append(bet_out);
            }
        }
        else
        {
            //BINGO CLUB DUMMY generator
            vector<Bibingo> bibi;
            bibi.resize(bet_count);
            for (int a = 0; a< bet_count; a++)
            {
                char bet_char[1024] = {0x00};
                char bet_out[1024*10] = {0x00};
                bibi[a].set_draw(draw);
                bibi[a].set_price(price);
                int res = bibi[a].copy_mask(bet_char, 1024);
                register_bet(data.r_ctx,session.asCString(),bibi[a].get_number(), bet_char, bibi[a].get_price(), game_id, bibi[a].get_draw());
                res = bibi[a].copy_json(bet_out, 1024);
                if (a)
                    out_json.append(",");
                out_json.append(bet_out);
            }
        }
    }
    else
    {
        if (bet_count < MAX_BET_COUNT_PER_REQUEST && err_code==0)
        {
            int bet_err = 0;
            if (game_id !=2500)
            {
								int tmp_gross_bet = bet_count*price;
								float rtp = ((float)(gross_win+gross_win_bonus))/((float)(tmp_gross_bet + gross_bet_bonus));
								fprintf(stdout, "calculated rtp is %f win=%d bet=%d\n",
												 rtp, gross_win, tmp_gross_bet);
								if (bet_count<=9)
								{
                	get_bet_rng(data.config_loader->get_config_rng(game_id, rtp, balance/100, price, bet_count),
									game_id,price,bets,bet_count, demo_bet_error);
								}
								else
								{
									for (int a= 0; a<bet_count; a++)
									{
										vector<_bet> bets_single;
                	get_bet_rng(data.config_loader->get_config_rng(game_id, rtp, balance/100, price, bet_count),
									game_id,price,bets_single,1, demo_bet_error);
									bets.insert(bets.end(), bets_single.begin(), bets_single.end());
									}
								}

            }
						int gross_bet = 0;	
            //balance/=100;
            int bets_iterator = 0;
            for (auto &bet: bets)
            {
                fprintf(stdout,"json: %lld\n", bet.number);
                char bet_char[BET_JSON_SIZE] = {0x00};
                    bet_to_json(bet,bet_char,BET_JSON_SIZE);
                    if(bets_iterator)
                        out_json.append(",");
                    out_json.append(bet_char);

                bets_iterator++;

            } // for auto bets
                // TODO uncomment there
                for (auto a :bets)
                {
										a.number = 2500000000 + time(nullptr);
                    //  if (demo == 0)
                    //  {
                      bet_add_sync_list(data.r_ctx, session.asCString(), a);
                    //  }
                        //disable balance calculate
#if 1
												if (bonus_balance> a.price*100)
												{
													bonus_balance -= a.price*100;
													bonus_balance += a.win*100;
													gross_win_bonus += a.win;
													gross_bet_bonus += a.price;
													// set bonus bet and win

												
												}
												else
												{
                        	balance -= a.price*100;
                        	balance += a.win*100;
													gross_win += a.win;
													gross_bet += a.price;
												}
#endif
                        if (balance < 0)
                        {
                            balance = 0;
                        }
                }

            result  = set_balance_senden(data.r_ctx, s_session.c_str(),
											balance,bonus_balance, gross_bet, gross_win,
										 gross_bet_bonus, gross_win_bonus);
    }// else bet_count!=1
} // if game_id
	if (game_id == 2500)
	{
		vector<string> cards;
		string result;
		int win;
		// send array of current mask
		// each round 
		// 
		vector<string> cards_mask;
    Json::Value cards_mask_j = root["cards"];
    if (cards_mask_j == Json::Value::null)
    {
			for (int a = 0; a<5;a++)
			{
				cards_mask.push_back("0");
			}
    }
		else
		{
			for (auto &card:cards_mask_j)
			{
				cards_mask.push_back(card.asString());
			}		
		}	

		res = get_bet_poker(cards_mask, cards, result, win);
		_bet a;
		a.game_id = 2500;
		a.win = win;
		a.price = price;
		a.number = 2500000000 + time(nullptr);
    bet_add_sync_list(data.r_ctx, session.asCString(), a);
		int iterator =0;
		if (bonus_balance>price*100)
		{
			bonus_balance -= price*100;
    	bonus_balance += win*100;
			gross_win_bonus += a.win;
			gross_bet_bonus += a.price;
		}
		else
		{
    	balance -= price*100;
    	balance += win*100;
			gross_bet += a.price;
			gross_win += a.win;
		}
    if (balance < 0)
    {
        balance = 0;
    }
 //  res= set_balance_senden(data.r_ctx, s_session.c_str(),balance);
  res  = set_balance_senden(data.r_ctx, s_session.c_str(),balance,bonus_balance,
						 gross_bet, gross_win, gross_bet_bonus, gross_win_bonus);

		out_json.append("{\"mask\":[");
	
		for (auto card:cards)
		{
			if (iterator>0)
			{
				out_json.append(",");
			}
			out_json.append("\"");
			out_json.append(card);
			out_json.append("\"");
			iterator++;
		}
		out_json.append("],\"win\":");
		out_json.append(to_string(win));
		out_json.append(",\"result\":\"");
		out_json.append(result);
		out_json.append("\"}");
	}// game 2500 windjammer
	out_json.append("],\"balance\":");
out_json.append(to_string((balance+bonus_balance)/100));
out_json.append(",\"bonuses_amount\":");
out_json.append(to_string(bonus_amount));
if(demo && (game_id <7770 || game_id>7783))
{
    out_json.append(",\"demo_error\":");
    out_json.append(to_string(demo_bet_error));
    if (demo_bet_error ==4)
        err_code = 4;
    else
        err_code = 0;

}
out_json.append(",\"error\":");


/*! @brief update balance in Redis
* should check result after!!!
*/
//  result = set_balance(data.r_ctx, session.asCString(),balance);

out_json.append(to_string(err_code));
out_json.append("}");

fcgx_put_str_json(*data.request, out_json);

return 0;
}

int rgs_get_sign(string &a, string &sign)
{
	char *operator_id = secure_getenv("RGS_OPERATOR_ID");
	if (! operator_id)
	{
		operator_id = "52C06E2F";
	}
	
  char * merchant_key = secure_getenv("RGS_MERCHANT_KEY");
	if (! merchant_key)
	{
		merchant_key = "Zi2s6GtiIl";
	}
	string sign_body;
	sign_body.append(a);
	sign_body.append(operator_id);
	int merchant_key_len = strnlen(merchant_key, 50);
  HMAC_CTX *ctx = HMAC_CTX_new();
  HMAC_Init_ex(ctx, merchant_key,merchant_key_len , EVP_sha256(),NULL);
 // fprintf(stdout, "key[%d]:[%s]\n",MERCHANT_KEY_LEN,MERCHANT_KEY);
  HMAC_Update(ctx, (const unsigned char*)sign_body.c_str(),sign_body.length());
  //fprintf(stdout, "sign body:[%s] len %d\n",sign_body.c_str(), sign_body.length());
  unsigned char hmac_buf[4096]={0x00};
  unsigned int hmac_len = 0;
  HMAC_Final(ctx, hmac_buf, &hmac_len );
  HMAC_CTX_free(ctx);
  string s_buf = "";
  for (int a = 0; a< hmac_len;a++)
  {
      char tmp_buf[10] = {0x00};
      snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", hmac_buf[a]);
      s_buf.append(tmp_buf);
  }
	//fprintf(stdout, "result: %s\n", s_buf.c_str());
	sign = s_buf;
	return 0;
}


int rgs_check_sign(string &a, string &b, string &sign)
{
    char * merchant_key = secure_getenv("RGS_MERCHANT_KEY");
		if (! merchant_key)
		{
			merchant_key = "Zi2s6GtiIl";
		}
		string sign_body;
		sign_body.append(a);
		sign_body.append(b);
		int merchant_key_len = strnlen(merchant_key, 50);
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, merchant_key,merchant_key_len , EVP_sha256(),NULL);
 //   fprintf(stdout, "key[%d]:[%s]\n",MERCHANT_KEY_LEN,MERCHANT_KEY);
    HMAC_Update(ctx, (const unsigned char*)sign_body.c_str(),sign_body.length());
  //  fprintf(stdout, "sign body:[%s] len %d\n",sign_body.c_str(), sign_body.length());
    unsigned char hmac_buf[4096]={0x00};
    unsigned int hmac_len = 0;
    HMAC_Final(ctx, hmac_buf, &hmac_len );
    HMAC_CTX_free(ctx);
    string s_buf = "";
    for (int a = 0; a< hmac_len;a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", hmac_buf[a]);
        s_buf.append(tmp_buf);
    }
		//fprintf(stdout, "check: %s\n", s_buf.c_str());
		//fprintf(stdout, "sign: %s\n", sign.c_str());
	if (s_buf != sign)
	{
		return 1;
	}
	return 0;
}

int sgi_get_time_format_string (string &str)
{
    uint64_t nanoseconds =  std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	char buf[TIMEBUFSIZE] = {0x00};
	time_t t = time(NULL);
	struct tm *tmp;
	tmp = localtime(&t);
	strftime(buf, TIMEBUFSIZE,"%Y%m%d%H%M%S",tmp);
	str.append(buf);
	str.append(to_string(nanoseconds % 10000));
	return 0;
}
