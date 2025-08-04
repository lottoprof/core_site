#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <chrono>
#include <dirent.h>
#include <iterator>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <memory>
#include <set>

#include <fcgi_config.h>
#include <fcgiapp.h>
#include <json/json.h>
#include <json/reader.h>

#include <hiredis.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <openssl/provider.h>
#include <cryptopp/base32.h>
#include <cryptopp/algparam.h>
#include <curl/curl.h>

#include "orakel.h"
#include "oraora.h"
#include "bibingo.h"
#include "config_loader.h"
#include "games_id.h"
#include "oraora_errors.h"
#include "oraora_limits.h"
#include "redis_helper.h"
#include "requests.hpp"

using namespace std::chrono;
/**********************
* Secret key for new
* bet system
**********************/
/*
ed.key generation!
  openssl genpkey -algorithm EC  -pkeyopt ec_paramgen_curve:P-256  -pkeyopt ec_param_enc:named_curve > ed.key
*/
__asm(
    ".global eeee_file\n"
    ".global _eeee_file\n"
    "eeee_file:\n"
    "_eeee_file:\n"
    ".incbin \"../ed.key\"\n"
    ".global eeee_file_len\n"
    ".global _eeee_file_len\n"
    "eeee_file_len:\n"
    "_eeee_file_len:\n"
    ".int .-eeee_file \n"
);

extern void * eeee_file;
extern void * eeee_file_len;
static unsigned char * key =(unsigned char *)&eeee_file;
static int * keylen =(int *) &eeee_file_len;
bool working = true;
bool ora_pause = false;


static int socketId;
/*! \brief RNG generator
*    for cold numbers
*  Function return comma
* separated string
*
* @param game_id
* @param session
* @return out
*
*/
void SignalHandler(int sig);

#ifdef _CONSOLE_DEBUG
# define log_err(fmt, args...) fprintf(stderr, fmt "\n", ## args); syslog(LOG_ERR, fmt, ## args);
# define log_info(fmt, args...) fprintf(stdout, fmt "\n", ## args); syslog(LOG_INFO, fmt, ## args);
#else
# define log_err(fmt, args...) syslog(LOG_ERR, fmt, ## args);
# define log_info(fmt, args...) syslog(LOG_INFO, fmt, ## args);
#endif

redisContext* create_redis_ctx() {
    redisContext *r_ctx;
    char *endptr = NULL;
    int redis_port = 6379;
    char *redis_host = secure_getenv("REDIS_HOST");
    char *str_redis_port = secure_getenv("REDIS_PORT");

    if (str_redis_port != NULL)
    {
        redis_port = strtol(str_redis_port,&endptr,10);
    }

    if (redis_host == NULL)
    {
        r_ctx = redisConnectUnix(REDIS_SOCKET);
    }
    else
    {
        r_ctx = redisConnect(redis_host, redis_port);
    }
    int redis_ready = 0;
    std::string redis_err_msg = "";
    if (r_ctx == NULL )
    {
        log_err("err allocate redis ctx host [%s] port [%d]", redis_host, redis_port);

        //return NULL;

    }
    if(r_ctx->err  )
    {
        log_err("reddis conn err %s\nhost [%s] port [%d]",r_ctx->errstr,redis_host, redis_port);
        redisFree(r_ctx);
        redis_err_msg = r_ctx->errstr;
        //  return NULL;
    }
    else
    {
        redis_ready = 1;
    }

    return redis_ready == 1 ? r_ctx : nullptr;
}


/*! \brief Return json formated bet
*
*  Function return jsom formated bet
* Also detect bonus symbol in bet mask
*
* @param _bet - bet from orakel
* @param out - out string
* @param size - actual string len
* @return 0 on success else error code
*
*/
int bet_to_json(_bet &b, char *out, int size)
{
    string mask = "";
    time_t some_date = time(NULL);
    string s_bonus = "false";
    int bonus =0 ;
    int bonus_max = 0;
    int bonus_max_2 = 0;
    int bonus_max_3 = 0;
    int bonus_symbol =0;
    int bonus_counter = 0;
    int bonus_symbol_2 = 0;
    int bonus_counter_2 = 0;
    int bonus_symbol_3 = 0;
    int bonus_counter_3 = 0;

    if (b.game_id == 2110)
    {
        bonus_symbol = 13;
        bonus_max = 3;
    }
    if (b.game_id == 2111)
    {
        bonus_symbol = 10;
        bonus_max = 1;
    }
    if (b.game_id == 2113)
    {
        bonus_symbol = 8;
        bonus_max = 3;
    }
    if (b.game_id == 2114)
    {
        bonus_symbol = 8;
        bonus_max = 3;
        bonus_symbol_2 = 9;
        bonus_max_2 = 19;
        bonus_symbol_3 = 5;
        bonus_max_3 = 16;
    }
    int price = b.price;
    // if (b.game_id == 211401)
    // {
    //     price = 0;
    // }
    if (b.game_id == 2115)
    {
        bonus_symbol = 15;
        bonus_max = 1;
    }


    // Переписать! сумеречный фикс для чукчи удалить __V
    if (b.win == 0)
    {
        b.win_category = 0;
    }
    if (b.game_id == 2114 )
    {
        int bonus_cherry = 0;
        vector <int> mask_values;
        mask_values.resize(7);
        vector <int> good_values;
        for(auto a= b.mask.begin(); a!=b.mask.end();a++)
        {
            if (*a<=7)
            {
                mask_values[*a -1]++;
            }
        }
        int iterator = 0;
        for(auto &a:mask_values)
        {
            if (mask_values[iterator]<2)
                good_values.push_back(iterator + 1);

            iterator++;
        }

        for(auto a= b.mask.begin(); a!=b.mask.end();a++)
        {
            if (*a == 10)
            {
                b.win_category = *a;
            }
            if (*a == 9)
            {
                bonus_cherry ++;
                b.win_category = *a;
            }
            if (b.win_category !=5 && *a ==5 )
            {
                *a = good_values[0];
                good_values.erase(good_values.begin());
            }
        }
    }
    for(auto a= b.mask.begin(); a!=b.mask.end();a++)
    {
        auto aa = a;
        aa++;
        char buf[10] = {0x00};
        // TODO remove it from release!!!!
        if (*a == bonus_symbol)
        {
            bonus++;
        }
        if (b.game_id == 2115 && (*a==14) && b.win)
        {
            *a = 1;
        }
        if (b.game_id == 2115 && *a ==15)
        {
            b.win_category = 15;
        }
        if (b.game_id == 2115 && (*a==14) && b.win==0)
        {
            b.win_category = 14;
        }
        // !!! TODO убрать из продакшна
        //if (b.game_id == 2111 && *a >=11 && !b.win)
        if (b.game_id == 2111 && *a >=11)
        {
            b.win_category = *a;
        }
        if (b.game_id == 2113 && *a ==9)
        {
            b.win_category = *a;
        }
#if 0
    //if (b.game_id == 2114 && *a >=9&& !b.win)
    if (b.game_id == 2114 && *a >=9)
    {
    b.win_category = *a;
    }
#endif
        snprintf(buf, sizeof(buf),"%d",*a);
        mask += buf;
        if(aa != b.mask.end())
        {
            mask += ",";
        }
    } // for mask
    string win_mask;  // Какая-то нереализованная
        // пиздецкая задумка
    if (bonus >= bonus_max)
    {
        s_bonus = "true";
    }
    int result = 0;
    if (b.number != 77777) //DEMO TICKET
    {
        result = snprintf(out,size,"{\"error\":0,\"number\":\"%llu\",\"game_id\":%d,\"game_type\":\"notype\",\"date\":%d,\"win\":%d,\"price\":%d,\"draw\":%d,\"count_digits\":%d,\"mask\":[%s],\"bet_mask\":[%s],\"win_category\":%d,\"win_tile\":%d ,\"bonus\":%s}", b.number,
                          b.game_id,some_date,
                          b.win,
                          price,
                          b.draw,
                          b.count_digits,
                          mask.c_str(),
                          win_mask.c_str(), // Не используется? что за кек
                          b.win_category, b.win_category, s_bonus.c_str()  );
    }
    else
    {
        result = snprintf(out,size,"{\"error\":0,\"number\":\"DEMO-TICKET\",\"game_id\":%d,\"game_type\":\"notype\",\"date\":%d,\"win\":%d,\"price\":%d,\"draw\":%d,\"count_digits\":%d,\"mask\":[%s],\"bet_mask\":[%s],\"win_category\":%d,\"win_tile\":%d ,\"bonus\":%s}",
                          b.game_id,some_date,
                          b.win,
                          b.price,
                          b.draw,
                          b.count_digits,
                          mask.c_str(),
                          win_mask.c_str(), // Не используется? что за кек
                          b.win_category, b.win_category, s_bonus.c_str()  );

    }
    return result;
}
/*! \brief FCGI err parse RSGI
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param code - error code returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_sgi_err_win(FCGX_Request& request, int code)
{

  Json::Value root_out;
	
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = code;
//	root_out["balance"] = 0;
	root_out["currencyId"] = Json::Value::null;
	Json::Value items;
	Json::Value item;
	item["balance"] = "0";
	items.append(item);
	root_out["items"] = items;

    FCGX_PutS("Status: 200\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);


	Json::StreamWriterBuilder sw;
	string out_str = Json::writeString(sw,root_out );
	


    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}


/*! \brief FCGI err parse RSGI
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param code - error code returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_sgi_err_balance(FCGX_Request& request, int code)
{

  Json::Value root_out;
	
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = code;
	root_out["balance"] = 0;
	root_out["currencyId"] = Json::Value::null;

    FCGX_PutS("Status: 200\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);


	Json::StreamWriterBuilder sw;
	string out_str = Json::writeString(sw,root_out );
	


    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}
/*! \brief FCGI err parse RSGI
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param code - error code returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_sgi_err_bet(FCGX_Request& request, int code)
{

  Json::Value root_out;
	
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = code;
	//root_out["balance"] = "0";
	//root_out["currencyId"] = Json::Value::null;
	Json::Value items;
	Json::Value item;
	item["balance"] = "0";
	items.append(item);
	root_out["items"] = items;

    FCGX_PutS("Status: 200\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);


	Json::StreamWriterBuilder sw;
	string out_str = Json::writeString(sw,root_out );
	


    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}


/*! \brief FCGI err parse RSGI
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param code - error code returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_sgi_err(FCGX_Request& request, int code)
{

  Json::Value root_out;
	
	string timestamp_cur;
	sgi_get_time_format_string(timestamp_cur);
	root_out["timestamp"] = timestamp_cur;
	
	string sign;
	rgs_get_sign(timestamp_cur,sign);
	root_out["signature"] = sign;
	root_out["errorCode"] = code;
	root_out["balance"] = "0";
	root_out["currencyId"] = Json::Value::null;
//	Json::Value items;
//	Json::Value item;
//	item["balance"] = "0";
//	items.append(item);
//	root_out["items"] = items;

    FCGX_PutS("Status: 200\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);


	Json::StreamWriterBuilder sw;
	string out_str = Json::writeString(sw,root_out );
	


    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}



/*! \brief FCGI err parse
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param json_error - error string returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_err(FCGX_Request& request, const char* json_error)
{
    FCGX_PutS("Status: 400\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);
    string out_str;
    out_str.append("{\"err\":\"");
    out_str.append(json_error);
    out_str.append("\"}");
    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}
/*! \brief FCGI err parse
*
*  Function return err to FCGI
*
* @param request - pointer to FCGI request
* @param json_error - error string returning to client
* @param code - error code returning to client
* @return 0 on success else error code
*
*/
int finish_request_parse_err(FCGX_Request& request, const char* json_error, int code)
{
    FCGX_PutS("Status: 400\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    FCGX_PutS("\r\n", request.out);
    string out_str;
    out_str.append("{\"err_msg\":\"");
    out_str.append(json_error);
    out_str.append("\", \"error\":");
    out_str.append(to_string(code));
    out_str.append("}");
    FCGX_PutS(out_str.c_str(), request.out);
    FCGX_PutS("\n\n", request.out);

    // закрыть текущее соединение
    FCGX_Finish_r(&request);
    return 0;
}

/*! \brief FCGI thread
*
*  Function process FCGI
*
* @param a - pointer to FCGI thread
* @return  - never return
*
*/
static void *doit(void *a)
{
    openlog("oraora_d", LOG_NOWAIT | LOG_PID, LOG_USER);
    char *ora_host = secure_getenv("ORA_HOST");
    if (ora_host == NULL)
    {
        ora_host = ORA_HOST;
    }
    int rc, i;

    redisReply *r_reply;
    struct timeval timeout = {1, 500000};
    redisContext *r_ctx = create_redis_ctx();
    int redis_ready = 0;
    if (r_ctx != nullptr)
    {
        redis_ready = 1;
    }
    std::string redis_err_msg = "";
    //generate session
OSSL_PROVIDER *provider = OSSL_PROVIDER_load(NULL, "default");		

    EVP_PKEY *pkey = NULL;
    BIO *bio;
#if 0
  char *key="\x0a \
---- PRIVATE KEY----- \x0a \
MC4CYDK2VwBCIEII9XnyEUVaAHw5S1G7I27QpLDamJub6JXhcfup8sWzph \x0a \
----RIVATE KEY-----";
#endif
    bio = BIO_new_mem_buf((void *)key, *keylen);
    if(bio != NULL)
    {
        EVP_PKEY*  private_read = PEM_read_bio_PrivateKey(
            bio,   /* BIO to read the private key from */
            &pkey, /* pointer to EVP_PKEY structure */
            NULL,  /* password callback - can be NULL */
            NULL   /* parameter passed to callback or password if callback is NULL */
        );

        if (pkey == NULL)
        {
            log_err("unable to load key");
            ERR_load_crypto_strings();
            char err[255];
            ERR_error_string(ERR_get_error(), err);
            return 0;
        }
    }
#if 0
  BIO *bp = NULL;
  EVP_PKEY_print_params(bp, pkey,1, NULL);
#endif

    FCGX_Request request;
    GamesConfigLoader *config_loader;
    config_loader = (GamesConfigLoader*)a;
    //  rakel &orakel = config_loader->orakel;
    if(FCGX_InitRequest(&request, socketId, 0) != 0)
    {
        //ошибка при инициализации структуры запроса
        printf("Can not init request\n");
        return NULL;
    }
    printf("Request is inited\n");
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    for(;working;)
    {
        if (ora_pause)
        {
            usleep(500);
            continue;
        }
        int result = check_redis(&r_ctx);
        if (result)
        {
            usleep(500);
            continue;
        }

        char buf_in[MAX_REQUEST_BUF_SIZE] = {0x00};

        Requests::Data data;

        data.ora_host      = ora_host;
        data.config_loader = config_loader;
        data.r_ctx         = r_ctx;
        data.request       = &request;
        data.redis_ready   = redis_ready;
        data.pkey          = pkey;
        data.buf_in        = buf_in;

        Requests::instance().do_it(data);
    }
    redisFree(r_ctx);
    return NULL;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *data)
{
    (static_cast<string*>(data))->append(static_cast<char *>(ptr), size * nmemb);
    // fprintf(stderr, "out curl:%s\n", ptr);
    return size * nmemb;
}
int do_bet(string &session, int game_id, int amount)
{

}
int do_request_bets_win_swerte(string &session, string &player_id, vector<_bet> &b,unsigned int &balance, string &error_message,redisContext *r_ctx, int game_id)
{
   return 0;
}
int do_request_bets_win_sz(string &session, string &player_id, vector<_bet> &b,unsigned int &balance, string &error_message,redisContext *r_ctx, int game_id)
{
    string out;
    string request= "{\"action\":\"win\",\"session_id\":\"";
    request.append(session);
    request.append("\",\"player_id\":");
    request.append(player_id);
    request.append(",\"currency\":\"KZT\",");
    request.append("\"game_id\":");
    request.append(to_string(game_id));
    request.append(",\"data\":[");
    int index = 0;
    for (auto a = begin(b); a<end(b);a++, index++)
    {
        if (index)
            request.append(",");
        request.append("{\"amount\":");
        request.append(to_string(a->win*100));
        request.append(",\"transaction_id\":");
        request.append(to_string(a->number));
        request.append(",\"bet_transaction_id\":");
        request.append(to_string(a->number));
        request.append(",\"extra_transaction_ids\":[],\"win_data\":{}}");
    }
    request.append("]}");
    log_info("req: %s", request.c_str());
    int res  =  do_request(request, out, r_ctx);
    int err_counter = 0;
    if (res != 0)
    {
        int counter = 0;
        while (res !=0 || counter<9)
        {
            res  =  do_request(request, out, r_ctx);
            if (res !=0 )
            {
                //  usleep(1000);
                err_counter++;
            }
            else
            {
                err_counter = 0;
            }
            counter++;
        }
    }
    //err_counter++ ;
    if (err_counter )
    {
        for (auto a = begin(b); a<end(b);a++, index++)
        {
            bet_win_add_sync_list(r_ctx, session.c_str(), *a);
        }

    }
    log_info("get: %s", out.c_str());
    if (res == 0)
    {
        Json::Value root;
        Json::Reader reader;
        int res_parsing = reader.parse(out.c_str(),root);
        if (!res_parsing)
        {
            log_err("unable to parse request %s", out.c_str());
            vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
            for (auto a: vec_err)
            {
                log_err("%s",a.message.c_str());
            }
        }
        Json::Value error = root["error"];
        // check game_id
        if (error == Json::Value::null)
        {
            return -1;
        }

        if (error.asBool() == true)
        {
            log_info("WIN req | err from SZ: %s", root["error_message"].asCString());
            error_message = root["error_message"].asCString();
            return -1;
        }

        Json::Value j_balance = root["balance"];
        if (j_balance == Json::Value::null)
        {
            //  return -1;
            //write out balance
        }
        else
        {
            balance = j_balance.asInt64();
#ifdef _CONSOLE_DEBUG
            fprintf(stdout,"current balance from SZ =%d\n", balance);
#endif
            syslog(LOG_INFO,"current balance from SZ =%d", balance);
            //update balance by session
        }

        Json::Value j_trnid = root["transactions"];


        if (j_trnid == Json::Value::null)
        {
            log_info("err: trnid");

            for (auto a = begin(b); a<end(b);a++)
            {
                a->is_sold=0;
            }
            return ERR_TRNID;
        }
        else
        {
            for(auto a: j_trnid)
            {
                if (a["transaction_id"] != Json::Value::null)
                {
                    const char *trn_id = a["transaction_id"].asCString();
                    log_info("trnid = %s %d", trn_id, balance);
                }
            }
        } // else transactions is null

        // parse json
        return 0;
    }
    closelog();
    return -1;
}
int do_request_bets_win(string &session, string &player_id, vector<_bet> &b,unsigned int &balance, string &error_message,redisContext *r_ctx)
{
    // b.is_sold = 1; //
    if (!b.size())
    {
        fprintf(stderr, "no bets to send to sz\n");
        return -1;
    }
    int game_id = b[0].game_id;
#if 0
  if (game_id == 211301)
  {
  game_id = 2113;
  }
#endif
            int res = do_request_bets_win_swerte(session, player_id, b,balance, error_message,r_ctx, game_id);
            return res;

}
int do_request_bets_swerte(redisContext *r_ctx,
                    string &session,int game_id, string &player_id,
                    vector<_bet> &b,unsigned int &balance,
                    string &error_message)
{
    return 0;
}
int do_request_bets_szkz(redisContext *r_ctx,
                    string &session,int game_id, string &player_id,
                    vector<_bet> &b,unsigned int &balance,
                    string &error_message)
{
    string request= "{\"action\":\"bet_bundle\",\"session_id\":\"";
    request.append(session);
    request.append("\",\"player_id\":");
    request.append(player_id);
    request.append(",\"currency\":\"KZT\",");
    request.append("\"game_id\":");
    request.append(to_string(game_id));
    request.append(",\"bets\":[");
    int index = 0;
    uint64_t trn=time(NULL)*10000000000;
    for (auto a = begin(b); a<end(b);a++, index++)
    {
        a->number += trn; //TODO remove on release
        a->is_sold=1;
        if (index)
            request.append(",");
        request.append("{\"amount\":");
        int price = a->price;
        if (game_id == 211401)
        {
            price = 0;
        }
        if (game_id == 211301)
        {
            price = 0;
        }
        if (game_id == 211500)
        {
            price = 0;
        }
        if (game_id == 211101)
        {
            price = 0;
        }
        request.append(to_string(price*100));
        request.append(",\"transaction_id\":");
        request.append(to_string(a->number));
        request.append(",\"bet_data\":{\"number\":\"");
        request.append(to_string(a->number));
        request.append("\",\"mask\":[");
        int c_index = 0;
        for (auto c:a->mask)
        {
            if(c_index)
                request.append(",");
            request.append(to_string(c));
            c_index++;
        }
        request.append("]}}");
    }
    request.append("]}");
    log_info("req: %s", request.c_str());
    string out;
    int res  =  do_request(request, out, r_ctx);
    log_info("get: %s", out.c_str());
    if (res == 0)
    {
        Json::Value root;
        Json::Reader reader;
        int res_parsing = reader.parse(out.c_str(),root);

        if (!res_parsing)
        {
            log_err("unable to parse request %s", out.c_str());
            vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
            for (auto a: vec_err)
            {
                log_err("%s",a.message.c_str());
            }
        }
        Json::Value error = root["error"];
        // check game_id
        if (error == Json::Value::null)
        {
            return -1;
        }

        if (error.asBool() == true)
        {
            Json::Value j_balance = root["balance"];
            if (j_balance == Json::Value::null)
            {
                //  return -1;
            }
            else
            {
                balance = j_balance.asUInt64();
            }
            log_info("BETS req|err from SZ: %s", root["error_message"].asCString());
            error_message = root["error_message"].asCString();
            char *merchant_key = secure_getenv("LOGYCOM_MERCHANT_KEY");
            char *merchant_id =secure_getenv("LOGYCOM_MERCHANT_ID");
#ifdef _CONSOLE_DEBUG
            fprintf(stdout, "MERCHANT_ID = %s\n MERCHANT_KEY = %s\n", merchant_id, merchant_key);
#endif
            syslog(LOG_INFO, "MERCHANT_ID = %s MERCHANT_KEY = %s", merchant_id, merchant_key);
            return ERR_BALANCE;
        }
        log_info("BALANCE");

        Json::Value j_balance = root["balance"];
        if (j_balance == Json::Value::null)
        {
            //  return -1;
        }
        else
        {
            balance = j_balance.asInt64();
        }

        Json::Value j_trnid = root["transactions"];
        if (j_trnid == Json::Value::null)
        {
            log_info("err: trnid");

            for (auto a = begin(b); a<end(b);a++)
            {
                a->is_sold=0;
            }
            return ERR_TRNID;
        }
        else
        {
            for(auto a: j_trnid)
            {
                if (a["transaction_id"] != Json::Value::null)
                {
                    const char *trn_id = a["transaction_id"].asCString();
                    log_info("trnid = %s %d", trn_id, balance);
                }
            }
        } // else transactions is null

        // parse json
        return 0;
    }
    return -1;

}
int get_game_provider(int game_id, int &game_provider)
{
    int is_sz = 0; // only for SZ.KZ
        is_sz = 0;
        game_provider = GAMEPROVIDER_SWERTE;
    
    return 0;
}

int do_request_bets(redisContext *r_ctx,
                    string &session, string &player_id,
                    vector<_bet> &b,unsigned int &balance,
                    string &error_message)
{
    if (!b.size())
    {
        fprintf(stdout, "bets size 0!!\n");
        return -1;
    }
    int game_id = b[0].game_id;
          int  res = do_request_bets_swerte(r_ctx,
                    session,game_id, player_id,
                    b,balance,
                    error_message);
            return res;
}

int do_request_balance(string &session, string &player_id, int &balance,redisContext *r_ctx)
{
    string out;
    string request= "{\"action\":\"balance\",\"session_id\":\"";
    request.append(session);
    request.append("\",\"player_id\":");
    request.append(player_id);
    request.append(",\"currency\":\"KZT\"}");
    int res  =  do_request(request, out, r_ctx);
    //fprintf(stdout, "get: %s\n", out.c_str());
    if (res == 0)
    {
        Json::Value root;
        Json::Reader reader;
        int res_parsing = reader.parse(out.c_str(),root);
        if (!res_parsing)
        {
            log_err("unable to parse request %s", out.c_str());
            vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
            for (auto a: vec_err)
            {
                log_err("%s",a.message.c_str());
            }
        }
        Json::Value error = root["error"];
        // check game_id
        if (error == Json::Value::null)
        {
            return -1;
        }

        if (error.asBool() == true)
        {
            log_info("BALANCE req|err from SZ: %s", root["error_message"].asCString());
            return -1;
        }

        Json::Value j_balance = root["balance"];
        if (j_balance == Json::Value::null)
        {
            return -1;
        }
        balance = j_balance.asInt64();
#ifdef _CONSOLE_DEBUG
        fprintf(stdout,"current balance from SZ =%d\n", balance);
#endif
        syslog(LOG_INFO,"current balance from SZ =%d", balance);
        // parse json
        return 0;
    }
    return -1;
}

int do_request (string &request, string &out,redisContext *r_ctx)
{
    // time_start
    //
    uint64_t start_ts =  std::chrono::duration_cast<std::chrono::nanoseconds>
                        (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    char *request_url = secure_getenv("LOGYCOM_REQUEST_URL");
    if (request_url == NULL)
    {
        request_url = REQUEST_URL;
    }
    char * merchant_key = secure_getenv("LOGYCOM_MERCHANT_KEY");
    if (merchant_key == NULL)
    {
        merchant_key = MERCHANT_KEY;
    }
    int merchant_key_len = strnlen(merchant_key,MAX_MERCHANT_KEY_LEN);
    char * merchant_id = secure_getenv("LOGYCOM_MERCHANT_ID");
    if (merchant_id == NULL)
    {
        merchant_id = MERCHANT_ID;
    }

    string url_address = request_url;
    //url_address.append(action);
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, url_address.c_str());
#if 1
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEF_CURLOPT_TIMEOUT);

    struct curl_slist *headers = NULL;
    //headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // headers = curl_slist_append(headers, "Charset: utf-8");
    // headers = curl_slist_append(headers, "User-Agent: oraora");
    string xmerchant_header = "X-Merchant-Id: ";
    xmerchant_header.append(merchant_id);
    //headers = curl_slist_append(headers, "X-Merchant-Id: a7cd24bb6123cb44173c0a178511274b12e34fc06e9b8ce5769dbc197098183a");
    headers = curl_slist_append(headers, xmerchant_header.c_str());
    time_t t;
    t = time(NULL);
    char time_buf[100] = {0x00};
    snprintf(time_buf,sizeof(time_buf),"X-Timestamp: %u", t);

    string sign, sign_body;
    sign_body.append(request);
    sign_body.append(to_string(t));
    //  ENGINE_load_builtin_engines();
    //  ENGINE_register_all_complete();
    HMAC_CTX *ctx = HMAC_CTX_new();
    //HMAC_Init_ex(ctx, MERCHANT_KEY,MERCHANT_KEY_LEN - 1 , EVP_sha1(),NULL);
    HMAC_Init_ex(ctx, merchant_key,merchant_key_len , EVP_sha1(),NULL);
    //fprintf(stdout, "key[%d]:[%s]\n",MERCHANT_KEY_LEN,MERCHANT_KEY);
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
    char sign_buf[4096] = {0x00};
    snprintf(sign_buf,sizeof(sign_buf),"X-Sign: %s",  s_buf.c_str());
    //fprintf(stdout, "\ns_buf:[%s] sign_buf:[%s",s_buf.c_str(), sign_buf);


    headers = curl_slist_append(headers, time_buf);
    headers = curl_slist_append(headers, sign_buf);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.length());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

    CURLcode code = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    if (code != CURLE_OK || response_code != 200)
    {
        log_err("code: %d resp_code %d", code, response_code);
        syslog(LOG_ERR, "CURL REQ [%s]", request.c_str());
        return -1;
    }
    uint64_t end_ts =  std::chrono::duration_cast<std::chrono::nanoseconds>
                      (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    string prefix = "rpo_req";
    do_request_log_redis(r_ctx,prefix,start_ts,end_ts);
    return 0;
}

/*! \brief set player data
*
*  Function update player data
*
* @param r_ctx - Oppened Redis context
* @param player - player ID
* @param name - player name
* @param family - second name
* @param patronik - wow. it is patronik name
* @param phone - user phone number
* @param email - user email address
* @param birth - date of birth
* @param ts - timestamp request

* @return 0 on success else error code
*
*/
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
                    const char *address)
{
    // Request session from reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s name %s family %s patronymic %s phone %s email %s birth %s ts %s document %s issue_date %s code %s address %s",
                                          player, name, family, patronymic, phone, email, birth, ts,
                                          document,issue_date,code, address
                                          );
    if (r_reply == NULL)
    {
        log_err("edit user: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user %s", player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return player data
*
*  Function return player data
*
* @param r_ctx - Oppened Redis context
* @param player - player ID
* @param out - out formatted json
* @return 0 on success else error code
*
*/
int get_player_data(redisContext *r_ctx,
                    const char *player, string &out)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    log_info("HMGET user:%s name family patronymic phone email birth ts", player);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s name family patronymic phone email birth ts document issue_date code address", player);
    if (r_reply == NULL)
    {
        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no player %s", player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        //  fprintf(stderr, "player [%s]\n", session, r_reply->element[0]->str);
        if (r_reply->element[0]->len>0)
        {
            out.append(",");
            out.append("\"name\":\"");
            out.append(r_reply->element[0]->str);
            out.append("\"");
        }
        if (r_reply->element[1]->len>0)
        {
            out.append(",");
            out.append("\"family\":\"");
            out.append(r_reply->element[1]->str);
            out.append("\"");
        }
        if (r_reply->element[2]->len>0)
        {
            out.append(",");
            out.append("\"patronik\":\"");
            out.append(r_reply->element[2]->str);
            out.append("\"");
        }
        if (r_reply->element[3]->len>0)
        {
            out.append(",");
            out.append("\"phone\":\"");
            out.append(r_reply->element[3]->str);
            out.append("\"");
        }
        if (r_reply->element[4]->len>0)
        {
            out.append(",");
            out.append("\"email\":\"");
            out.append(r_reply->element[4]->str);
            out.append("\"");
        }
        if (r_reply->element[5]->len>0)
        {
            out.append(",");
            out.append("\"birth\":\"");
            out.append(r_reply->element[5]->str);
            out.append("\"");
        }
        if (r_reply->element[6]->len>0)
        {
            out.append(",");
            out.append("\"ts\":\"");
            out.append(r_reply->element[6]->str);
            out.append("\"");
        }
        if (r_reply->element[7]->len>0)
        {
            out.append(",");
            out.append("\"document\":\"");
            out.append(r_reply->element[7]->str);
            out.append("\"");
        }
        if (r_reply->element[8]->len>0)
        {
            out.append(",");
            out.append("\"issue_date\":\"");
            out.append(r_reply->element[8]->str);
            out.append("\"");
        }
        if (r_reply->element[9]->len>0)
        {
            out.append(",");
            out.append("\"code\":\"");
            out.append(r_reply->element[9]->str);
            out.append("\"");
        }
        if (r_reply->element[10]->len>0)
        {
            out.append(",");
            out.append("\"address\":\"");
            out.append(r_reply->element[10]->str);
            out.append("\"");
        }
    } // else redis return data ok

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief Return player session
*
*  Function return player
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param player - return player of the session
* @return 0 on success else error code
*
*/
int get_player_session(redisContext *r_ctx,const char *session, string &player)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
#define BAD_SESSION "gbcseiczx7uxeuc7mh2tbyxr3v4hj6wz5u9tepa6tkw34xijwt9napim9wbccae7t639pbc2n8cpi8zhb34u9ymb2k5ghw5z5v6fadbkpsa46hxts6"
#define BAD_SESSION_LEN (sizeof(BAD_SESSION)-1)
    if (strncmp(session,BAD_SESSION, BAD_SESSION_LEN )==0)
    {
        return 1;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_err("SESSION %s player [%s]", session, r_reply->element[0]->str);

        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}
/*! \brief Return user_id and params using session
*
*  Function checking session for websites
*  swertefun, senden
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param return_url -
* @param refill_url -
* @param action_url -
* @param language -
* @return 0 on success else error code
*
*/
int get_status_session_www(redisContext *r_ctx,const char *session,
                       string &return_url,
                       string &refill_url, string &action_url,
                       string &language, string &player_name,string &register_url)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s return_url refill_url action_url language player_name register_url", session);
    if (r_reply == NULL)
    {
        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_err("SESSION %s status [%s]", session, r_reply->element[0]->str);
        if (!r_reply->elements)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REDIS;
        }
        if (r_reply->element[0]->len>0)
        {
            return_url = r_reply->element[0]->str;
        }
        if (r_reply->element[1]->len>0)
        {
            refill_url = r_reply->element[1]->str;
        }
        if (r_reply->element[2]->len>0)
        {
            action_url = r_reply->element[2]->str;
        }
        if (r_reply->element[3]->len>0)
        {
            language = r_reply->element[3]->str;
        }
        if (r_reply->element[4]->len>0)
        {
            player_name = r_reply->element[4]->str;
        }
        if (r_reply->element[5]->len>0)
        {
            register_url = r_reply->element[5]->str;
        }
        else
        {
            player_name = "noname";
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief Return status session
*
*  Function checking session status
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param status - return status of the session
* @param return_url -
* @param refill_url -
* @param action_url -
* @param language -
* @param myparam -
* @return 0 on success else error code
*
*/
int get_status_session(redisContext *r_ctx,const char *session,
                       int &status, string &return_url,
                       string &refill_url, string &action_url,
                       string &language, string &myparam, string &player_name, int &demo, string &register_url)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s status  return_url refill_url action_url language myparam player_name demo register_url", session);
    if (r_reply == NULL)
    {
        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_err("SESSION %s status [%s]", session, r_reply->element[0]->str);
        if (!r_reply->elements)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REDIS;
        }
        if (r_reply->element[0]->len>0)
        {
            char *endptr  = NULL;
            status = strtol(r_reply->element[0]->str, &endptr,10);
        }
        if (r_reply->element[1]->len>0)
        {
            return_url = r_reply->element[1]->str;
        }
        if (r_reply->element[2]->len>0)
        {
            refill_url = r_reply->element[2]->str;
        }
        if (r_reply->element[3]->len>0)
        {
            action_url = r_reply->element[3]->str;
        }
        if (r_reply->element[4]->len>0)
        {
            language = r_reply->element[4]->str;
        }
        if (r_reply->element[5]->len>0)
        {
            myparam = r_reply->element[5]->str;
        }
        if (r_reply->element[6]->len>0)
        {
            player_name = r_reply->element[6]->str;
        }
        if (r_reply->element[7]->len>0)
        {
            char *endptr  = NULL;
            demo = strtol(r_reply->element[7]->str, &endptr,10);
        }
        if (r_reply->element[8]->len>0)
        {
            register_url = r_reply->element[8]->str;
        }
        else
        {
            player_name = "noname";
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief renew status session
*
*  Function getting new session
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param url - return url to user with new session_id
* @return 0 on success else error code
*
*/
int renew_session(redisContext *r_ctx,const char *session, const char *session_new, string &url)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
#define BAD_SESSION "gbcseiczx7uxeuc7mh2tbyxr3v4hj6wz5u9tepa6tkw34xijwt9napim9wbccae7t639pbc2n8cpi8zhb34u9ymb2k5ghw5z5v6fadbkpsa46hxts6"
#define BAD_SESSION_LEN (sizeof(BAD_SESSION)-1)
    if (strncmp(session,BAD_SESSION, BAD_SESSION_LEN )==0)
    {
        return 1;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;

    string player, balance, demo, game_id;

    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s demo player balance game_id", session);
    if (r_reply == NULL)
    {
        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    //read data
    {
        if (r_reply->element[0]->len > 0)
        {
            demo = r_reply->element[0]->str;
        }
        else
        {
            log_err("demo undef len[%d] ", r_reply->element[0]->len);
        }
        player = r_reply->element[1]->str;
        balance = r_reply->element[2]->str;
        game_id = r_reply->element[3]->str;
    }


    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    r_reply = NULL;
    int status = SESSION_INITED;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s demo %s player %s balance %s status %d game_id %s",
                                          session_new, demo.c_str(), player.c_str(), balance.c_str(), status, game_id.c_str());
    if (r_reply == NULL)
    {
        log_err("set new session: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    url.append("https://oraora.winlottery.site/static/index.html?session=");
    url.append(session_new);
    return 0;
}

/*! \brief write log to redis
*
*  Function  write timestamp
* of each request to logycom
* in the redis
*
* @param r_ctx - Oppened Redis context
* @param ts_start - timestamp start of request
* @param ts_end - timestamp end of request
* @return 0 on success else error code
*
*/
int do_request_log_redis(redisContext *r_ctx,string &prefix, uint64_t ts_start, uint64_t ts_end)
{
    return 0;
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("\n-------\nHMSET %s:%llu ts_start %llu ts_end %llu ts_delta [%llu] ",
            prefix.c_str(),
            ts_start, ts_start, ts_end, (ts_end-ts_start)/100000);

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s:%llu ts_start %llu ts_end %llu ts_delta %llu",
                                          prefix.c_str(),
                                          ts_start, ts_start, ts_end, ts_end-ts_start);

    if (r_reply == NULL)
    {
        log_err("do_logs write error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("some shit");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try register withdrawal
*
*  Function  write data from p2p
* payment GW to redis
*
* @param r_ctx - Oppened Redis context
* @param root - json object which send
* us from server
* @return 0 on success else error code
*
*/
int p2p_write_withdrawal(redisContext *r_ctx,map<string,string> &post_vals)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("HMSET p2p_withdrawal:%s merchant_id %s invoice_id %s amount %s amount_currency %s currency %s merchant_amount %s order_desc %s account_info %s payment_system_type %s status %s signature %s",
           post_vals["order_id"].c_str(),
           post_vals["merchant_id"].c_str(),
           post_vals["invoice_id"].c_str(),
           post_vals["amount"].c_str(),
           post_vals["amount_currency"].c_str(),
           post_vals["currency"].c_str(),
           post_vals["merchant_amount"].c_str(),
           post_vals["order_desc"].c_str(),
           post_vals["account_info"].c_str(),
           post_vals["payment_system_type"].c_str(),
           post_vals["status"].c_str(),
           post_vals["signature"].c_str()
           );

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET  p2p_withdrawal:%s merchant_id %s invoice_id %s amount %s amount_currency %d currency %s merchant_amount %s order_desc %s account_info %s payment_system_type %s status %s signature %s",
                                          post_vals["order_id"].c_str(),
                                          post_vals["merchant_id"].c_str(),
                                          post_vals["invoice_id"].c_str(),
                                          post_vals["amount"].c_str(),
                                          post_vals["amount_currency"].c_str(),
                                          post_vals["currency"].c_str(),
                                          post_vals["merchant_amount"].c_str(),
                                          post_vals["order_desc"].c_str(),
                                          post_vals["account_info"].c_str(),
                                          post_vals["payment_system_type"].c_str(),
                                          post_vals["status"].c_str(),
                                          post_vals["signature"].c_str()
                                          );

    if (r_reply == NULL)
    {
        log_err("callback p2p write error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("some shit");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief write paybiz data to redis
*
*  Function  write data from p2p
* payment GW to redis
*
* @param r_ctx - Oppened Redis context
* @param hmset - formatted string, ready to process in redis
* @return 0 on success else error code
*
*/
int paybiz_write_callback(redisContext *r_ctx,string &hmset)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("%s",hmset.c_str());

    r_reply =(redisReply*) redisCommand(r_ctx,hmset.c_str());

    if (r_reply == NULL)
    {
        log_err("callback paybiz write error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("some shit");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try register bet
*
*  Function  write data from p2p
* payment GW to redis
*
* @param r_ctx - Oppened Redis context
* @param root - json object which send
* us from server
* @return 0 on success else error code
*
*/
int p2p_write_payment(redisContext *r_ctx,map<string,string> &post_vals)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("HMSET p2p_payment:%s merchant_id %s invoice_id %s amount %s amount_currency %s currency %s merchant_amount %s order_desc %s account_info %s payment_system_type %s status %s signature %s",
           post_vals["order_id"].c_str(),
           post_vals["merchant_id"].c_str(),
           post_vals["invoice_id"].c_str(),
           post_vals["amount"].c_str(),
           post_vals["amount_currency"].c_str(),
           post_vals["currency"].c_str(),
           post_vals["merchant_amount"].c_str(),
           post_vals["order_desc"].c_str(),
           post_vals["account_info"].c_str(),
           post_vals["payment_system_type"].c_str(),
           post_vals["status"].c_str(),
           post_vals["signature"].c_str()
           );

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET  p2p_payment:%s merchant_id %s invoice_id %s amount %s amount_currency %d currency %s merchant_amount %s order_desc %s account_info %s payment_system_type %s status %s signature %s",
                                          post_vals["order_id"].c_str(),
                                          post_vals["merchant_id"].c_str(),
                                          post_vals["invoice_id"].c_str(),
                                          post_vals["amount"].c_str(),
                                          post_vals["amount_currency"].c_str(),
                                          post_vals["currency"].c_str(),
                                          post_vals["merchant_amount"].c_str(),
                                          post_vals["order_desc"].c_str(),
                                          post_vals["account_info"].c_str(),
                                          post_vals["payment_system_type"].c_str(),
                                          post_vals["status"].c_str(),
                                          post_vals["signature"].c_str()
                                          );

    if (r_reply == NULL)
    {
        log_err("callback p2p write error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("some shit");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try register bet
*
*  Function register bet on draw
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param number - bet number
* @param mask - mask of bet
* @return 0 on success else error code
*
*/
int register_bet(redisContext *r_ctx,const char *session,
                 uint64_t number, char *mask, int price,
                 int game_id, int draw
                 )
{
    // Request DRAW from reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("HMSET %s:%llu mask %s price %d draw %d game_id %d",
           session, number, mask, price,draw, game_id);

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s:%llu mask %s price %d draw %d game_id %d",
                                          session, number, mask, price,draw, game_id);

    if (r_reply == NULL)
    {
        log_err("register bet error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no bet %llu", number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    log_info("repl:%s", r_reply->str);
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Set status session
*
*  Function activate session status
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @return 0 on success else error code
*
*/
int set_status_session(redisContext *r_ctx,const char *session, int status)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
#define BAD_SESSION "gbcseiczx7uxeuc7mh2tbyxr3v4hj6wz5u9tepa6tkw34xijwt9napim9wbccae7t639pbc2n8cpi8zhb34u9ymb2k5ghw5z5v6fadbkpsa46hxts6"
#define BAD_SESSION_LEN (sizeof(BAD_SESSION)-1)
    if (strncmp(session,BAD_SESSION, BAD_SESSION_LEN )==0)
    {
        return 1;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s status %d", session, status);
    if (r_reply == NULL)
    {

        log_err("get status error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief banner list
*
*  Function return banner list
*
* @param r_ctx - Oppened Redis context
* @param out - json formatted text
* @return 0 on success else error code
*
*/
int get_banner(redisContext *r_ctx, string &out_json)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    int iterator = 0;
    redisReply* r_reply = NULL;
    out_json.append("[");
    r_reply =(redisReply*) redisCommand(r_ctx,"LLEN banners:top");
    if (r_reply == NULL)
    {
        log_err("get_banner: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no banners in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_info("ELEM:%ld", r_reply->integer);
        if (r_reply->integer!=0)
        {
            int len = r_reply->integer;
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }

            //fprintf(stdout, "LRANGE games:instant 0 %d\n", len);
            r_reply = (redisReply*) redisCommand(r_ctx,"LRANGE banners:top 0 -1");
            if (r_reply->elements)
            {
                for (int a = 0; a< len;a++)
                {
                    //
                    redisReply* r_repl = NULL;
                    char *endptr = NULL;
                    char* number = r_reply->element[a]->str;
                      fprintf(stdout,"HMGET  banners:top:%s img_url site_url game_id alt text\n",
                        number);
                    r_repl =(redisReply*) redisCommand(r_ctx,"HMGET banners:top:%s img_url site_url game_id alt text",
                                                         number);
                    if (r_repl == NULL)
                    {
                        log_err("get_banner err: %s", r_ctx->errstr );
                        return ERR_SESSION;
                    }
                    if (r_repl->type == REDIS_REPLY_ERROR)
                    {
                        log_err("no banner in the list");
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_REDIS;
                    }
                    else// if (r_repl->type == REDIS_REPLY_STRING)
                    {
												 
                        if (a)
                        {
                            out_json.append(",\n");
                        }
                        char *img_url = r_repl->element[0]->str;
                        char *site_url = r_repl->element[1]->str;
                        char *game_id = r_repl->element[2]->str;
                        char *alt = r_repl->element[3]->str;
                        char *text = r_repl->element[4]->str;
												if (img_url != NULL &&
													site_url  != NULL &&
													game_id != NULL &&
													alt != NULL &&
													text != NULL)
                        out_json.append("{\"game_id\":\"");
                        out_json.append(game_id);
                        out_json.append("\",\"img_url\":\"");
                        out_json.append(img_url);
                        out_json.append("\",\"site_url\":\"");
                        out_json.append(site_url);
                        out_json.append("\",\"alt\":\"");
                        out_json.append(alt);
                        out_json.append("\",\"text\":\"");
                        out_json.append(text);
                        out_json.append("\"}");
                    }
                    if (r_repl != NULL)
                    {
                        freeReplyObject(r_repl);
                    }
                } //for list
                out_json.append("]");

                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return 0;
            } // if len list > 0
        } //else redis return ok
        else
        {
            out_json.append("]");
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

/*! \brief games list
*
*  Function return games list
*
* @param r_ctx - Oppened Redis context
* @param out - json formatted text
* @return 0 on success else error code
*
*/
int get_games(redisContext *r_ctx, string &out_json)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    int iterator = 0;
    redisReply* r_reply = NULL;
    out_json.append("[");
    r_reply =(redisReply*) redisCommand(r_ctx,"LLEN games:instant");
    if (r_reply == NULL)
    {
        log_err("get_games: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no games in list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_info("ELEM:%d", r_reply->integer);
        if (r_reply->integer!=0)
        {
            int len = r_reply->integer;
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }

            //fprintf(stdout, "LRANGE games:instant 0 %d\n", len);
            r_reply = (redisReply*) redisCommand(r_ctx,"LRANGE games:instant 0 -1");
            if (r_reply->elements)
            {
                for (int a = 0; a< len;a++)
                {
                    //
                    redisReply* r_repl = NULL;
                    char *endptr = NULL;
                    char* number = r_reply->element[a]->str;
                    //  fprintf(stdout,"HMGET games:instant:%s img_url site_url prices\n",
                    //    number);
                    r_repl =(redisReply*) redisCommand(r_ctx,"HMGET games:instant:%s img_url site_url prices menu provider",
                                                         number);
                    if (r_repl == NULL)
                    {
                        log_err("get_games err: %s", r_ctx->errstr );
                        return ERR_SESSION;
                    }
                    if (r_repl->type == REDIS_REPLY_ERROR)
                    {
                        log_err("no games in the list");
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_REDIS;
                    }
                    else
                    {
                        if (a)
                        {
                            out_json.append(",\n");
                        }
                        char *img_url = r_repl->element[0]->str;
                        char *site_url = r_repl->element[1]->str;
                        char *prices = r_repl->element[2]->str;
												char *menu = r_repl->element[3]->str;
												char *provider = r_repl->element[4]->str;
                        out_json.append("{\"game_id\":\"");
                        out_json.append(number);
                        out_json.append("\",\"provider\":\"");
                        out_json.append(provider);
                        out_json.append("\",\"img_url\":\"");
                        out_json.append(img_url);
                        out_json.append("\",\"menu\":[");
                        out_json.append(menu);
                        out_json.append("],\"prices\":[");
                        out_json.append(prices);
                        out_json.append("],\"site_url\":\"");
                        out_json.append(site_url);
                        out_json.append("\"}");
                    }
                    if (r_repl != NULL)
                    {
                        freeReplyObject(r_repl);
                    }
                } //for list
                out_json.append("]");

                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return 0;
            } // if len list > 0
        } //else redis return ok
        else
        {
            out_json.append("]");
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

/*! \brief list my bets
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @return 0 on success else error code
*
*/
int get_my_bets(redisContext *r_ctx,const char *session, int game_id,
                int draw_id, string &out_json)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request DRAW from reddis!!!
    //int draw = 0;
    //int res = get_current_draw(r_ctx, game_id,draw);
    out_json.append("\"bets\":[");
    redisReply* r_reply = NULL;
        log_info("LLEN %s:%d:%d:bets",  session,
                                          game_id, draw_id);
    r_reply =(redisReply*) redisCommand(r_ctx,"LLEN %s:%d:%d:bets", session,
                                          game_id, draw_id);
    if (r_reply == NULL)
    {
        log_err("get_my_bets err: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no bets for session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_info("ELEM  :%d", r_reply->integer);
        if (r_reply->integer!=0)
        {
            int len = r_reply->integer;
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }

            log_info("LRANGE %s:%d:%d:bets 0 %d", session,game_id,draw_id, len);
            r_reply = (redisReply*) redisCommand(r_ctx,"LRANGE %s:%d:%d:bets 0 %d",
                                                  session,game_id,draw_id, len);
            if (r_reply->elements)
            {
                for (int a = 0; a< len;a++)
                {
                    //
                    redisReply* r_repl = NULL;
                    char *endptr = NULL;
                    char* number = r_reply->element[a]->str;
                    log_info("HMGET %s:%s mask price",
                           session,number);
                    r_repl =(redisReply*) redisCommand(r_ctx,"HMGET %s:%s mask price",
                                                         session,number);
                    if (r_repl == NULL)
                    {
                        log_err("get_my_bets err: %s", r_ctx->errstr );
                        return ERR_SESSION;
                    }
                    if (r_repl->type == REDIS_REPLY_ERROR)
                    {
                        log_err("no bets for session %s", session);
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_REDIS;
                    }
                    else
                    {
                        if (r_repl->element[0]->len)
                        {
                            if (a)
                            {
                                out_json.append(",");
                            }
                            out_json.append("{\"number\":\"");
                            out_json.append(number);
                            out_json.append("\",\"draw_id\":\"");
                            out_json.append(to_string(draw_id));
                            out_json.append("\",\"mask\":[");
                            out_json.append(r_repl->element[0]->str);
                            out_json.append("],\"price\":\"");
                            out_json.append(r_repl->element[1]->str);
                            if (game_id == BINGO37_GAME_ID || game_id== BINGO38_GAME_ID)
                                out_json.append("\", \"win\":\"10\", \"win_mask\":[4]}");
                            else
                                out_json.append("\", \"win\":\"10\", \"win_mask\":[1,4]}");
                        }
                    }
                    if (r_repl != NULL)
                    {
                        freeReplyObject(r_repl);
                    }
                } //for list
                out_json.append("]");

                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return 0;
            } // if len list > 0
        } //else redis return ok
        else
        {
            out_json.append("]");
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

int append_bets(redisContext * r_ctx, const char* session, char* key, string & out_json, bool & append_comma) {
    fprintf(stdout, "LLEN %s ", key);
    redisReply * r_reply = (redisReply * ) redisCommand(r_ctx, "LLEN %s ", key);
    if (r_reply == NULL) {
        fprintf(stderr, "append_bets err: %s\n", r_ctx -> errstr);
        return ERR_SESSION;
    }
    if (r_reply -> type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "append_bets no bets for session %s\n", session);
        if (r_reply != NULL) {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    } else {
        fprintf(stdout, "ELEM  :%d\n", r_reply -> integer);
        if (r_reply -> integer != 0) {
            int len = r_reply -> integer;
            if (r_reply != NULL) {
                freeReplyObject(r_reply);
            }

            fprintf(stdout, "LRANGE %s 0 %d\n", key, len);
            r_reply = (redisReply * ) redisCommand(r_ctx, "LRANGE %s 0 %d", key, len);
            if (r_reply -> elements) {
                for (int a = 0; a < len; a++) {
                    //
                    redisReply * r_repli = NULL;
                    char * endptr = NULL;
                    char * number = r_reply -> element[a] -> str;
                    fprintf(stdout, "HMGET %s:%s mask price draw game_id\n",
                            session, number);
                    r_repli = (redisReply * ) redisCommand(r_ctx, "HMGET %s:%s mask price draw game_id",
                                                          session, number);
                    if (r_repli == NULL) {
                        fprintf(stderr, "get_my_bets err: %s\n", r_ctx -> errstr);
                        return ERR_SESSION;
                    }
                    if (r_repli -> type == REDIS_REPLY_ERROR) {
                        fprintf(stderr, "no bets for session %s\n", session);
                        if (r_repli != NULL) {
                            freeReplyObject(r_repli);
                        }
                        return ERR_REDIS;
                    } else {
                        if (r_repli -> element[0] -> len) {
                            if (append_comma) {
                                out_json.append(",");
                            }
                            else
                            {
                                append_comma = true;
                            }

                            out_json.append("{\"number\":\"");
                            out_json.append(number);
                            out_json.append("\",\"draw_id\":\"");
                            out_json.append(r_repli -> element[2] -> str);
                            out_json.append("\",\"mask\":[");
                            out_json.append(r_repli -> element[0] -> str);
                            out_json.append("],\"price\":\"");
                            out_json.append(r_repli -> element[1] -> str);
                            int game_id = r_repli->element[3]->integer;
                            out_json.append("\",\"game_id\":\"");
                            out_json.append(r_repli -> element[3]->str);

                            redisReply * r_win_reply = NULL;

                            fprintf(stdout, "HMGET %s:%s:%s win\n", r_repli -> element[3]->str,r_repli -> element[2] -> str, number);

                            r_win_reply = (redisReply * ) redisCommand(r_ctx, "HMGET %s:%s:%s win", r_repli -> element[3]->str,r_repli -> element[2] -> str, number);
                            if (r_win_reply == NULL) {
                                fprintf(stderr, "get_my_bets err: %s\n", r_ctx -> errstr);
                                return ERR_SESSION;
                            }
                            if (r_win_reply -> type == REDIS_REPLY_ERROR) {
                                fprintf(stderr, "no bets for session %s\n", session);
                                if (r_win_reply != NULL) {
                                    freeReplyObject(r_win_reply);
                                }
                                return ERR_REDIS;
                            }

                            fprintf(stdout, "replyType %d", r_win_reply->type);

                            if(r_win_reply->element[0]->type != REDIS_REPLY_NIL)
                            {
                                out_json.append("\", \"win\":\"");
                                if(r_win_reply->element[0]->type == REDIS_REPLY_STRING)
                                    out_json.append(r_win_reply->element[0]->str);
                                else if(r_win_reply->element[0]->type == REDIS_REPLY_INTEGER)
                                    out_json.append(std::to_string(r_win_reply->element[0]->integer));
                            }

                            out_json.append("\"}");

                            if (r_win_reply != NULL) {
                                freeReplyObject(r_win_reply);
                            }
                        }
                    }
                    if (r_repli != NULL) {
                        freeReplyObject(r_repli);
                    }
                } //for list

                if (r_reply != NULL) {
                    freeReplyObject(r_reply);
                }
                return 0;
            } // if len list > 0
        } //else redis return ok
    }

    if (r_reply != NULL) {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief list all my bets
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @return 0 on success else error code
*
*/
int get_all_my_bets(redisContext *r_ctx,const char *session, string &out_json)
{
    out_json.append("\"bets\":[");

    redisReply* r_reply =(redisReply*) redisCommand(r_ctx,"KEYS %s:*:bets ", session);

    fprintf(stdout, "KEYS %s:*:bets ", session);


    if (r_reply == NULL)
    {
        fprintf( stderr, "get_all_my_bets err: %s\n", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        fprintf(stderr, "get_all_my_bets no bets for session %s\n", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        bool append_comma = false;
        for(int x =0;x<r_reply->elements;x+=1)
        {
            int ret = append_bets(r_ctx,session, r_reply->element[x]->str, out_json, append_comma);
            if(ret!=0)
            {
                return ret;
            }
        }
    }

    out_json.append("]");

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief add session
*
*  Function add session to sync with PG
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @return 0 on success else error code
*
*/
int session_add_sync_list(redisContext *r_ctx,const char *session)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }

    log_info("LPUSH session2pg %s",  session);
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH session2pg %s",  session);
    if (r_reply == NULL)
    {
        log_err("set session error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err reply %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief get player_id from session
*
*  Function  return player_id
*
* @param r_ctx - Oppened Redis context
* @param session - user session
* @return 0 on success else error code
*
*/
int session_get_player_id(redisContext *r_ctx,const char *session, string &player_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get player error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {

        log_err("no player_id %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player_id = r_reply->element[0]->str;
#ifdef _CONSOLE_DEBUG
       //     fprintf(stdout , "player id: %s\n", player_id.c_str());
#endif
         //   syslog(LOG_INFO , "player id: %s", player_id.c_str());
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief add bet to sync list
*
*  Function add bet to sync with PG
* add bet to list
*
* @param r_ctx - Oppened Redis context
* @param bet - lottery instant bet
* @return 0 on success else error code
*
*/
int bet_win_add_sync_list(redisContext *r_ctx,const char *session, _bet &b)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    log_info("LPUSH win_bets2sz %lld",  b.number);
    redisReply* r_reply = NULL;
    string player_id;
    int res =  session_get_player_id(r_ctx, session,player_id);

    if(res!=0)
    {
        log_info("unable to get session %s", session);
        return res;
    }

    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH win_bets2sz %llu",  b.number);
    if (r_reply == NULL)
    {
        log_err("set session error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err insert bet 2 sz %lld", b.number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    string mask;
    for (auto a = begin(b.mask); a < end(b.mask);a++)
    {
        mask.append(to_string(*a));
        if (a+1 < end(b.mask))
            mask.append(",");
    }
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET bet2sz:%llu mask %s price %d win %d player_id %s game_id %d draw %d session %s",
                                          b.number, mask.c_str(), b.price, b.win, player_id.c_str(), b.game_id, b.draw, session);
    if (r_reply == NULL)
    {
        log_err("set bet2sz error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis insert bet 2 sz err %llu %s", b.number, player_id.c_str());
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

/*! \brief add bet to sync list
*
*  Function add bet to sync with PG
* add bet to list
*
* @param r_ctx - Oppened Redis context
* @param bet - lottery instant bet
* @return 0 on success else error code
*
*/
int bet_add_sync_list(redisContext *r_ctx,const char *session, _bet &b)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    //log_info("LPUSH instant_bets2pg %lld",  b.number);
    redisReply* r_reply = NULL;
    string player_id;
    int res =  session_get_player_id(r_ctx, session,player_id);

    if(res!=0)
    {
        log_info("unable to get session %s", session);
        return res;
    }

    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH instant_bets2pg %llu",  b.number);
    if (r_reply == NULL)
    {
        log_err("set session error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err insert bet %lld", b.number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    string mask;
    for (auto a = begin(b.mask); a < end(b.mask);a++)
    {
        mask.append(to_string(*a));
        if (a+1 < end(b.mask))
            mask.append(",");
    }
   // fprintf(stderr,"HMSET bet:%llu mask %s price %d win %d player_id %s game_id %d draw %d session %s win_category %d count_digits %d\n",
  //          b.number, mask.c_str(), b.price, b.win, player_id.c_str(), b.game_id, b.draw, session, b.win_category, b.count_digits);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET bet:%llu mask %s price %d win %d player_id %s game_id %d draw %d session %s win_category %d count_digits %d",
                                          b.number, mask.c_str(), b.price, b.win, player_id.c_str(), b.game_id, b.draw, session, b.win_category, b.count_digits);
    if (r_reply == NULL)
    {
        log_err("set insert bet error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis insert bet err %llu %s", b.number, player_id.c_str());
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

/*! \brief try buy bet KENO
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param number - bet number
* @param mask - bet mask
* @param price - return price of bet
* @return 0 on success else error code
*
*/
int buy_bet(redisContext *r_ctx,const char *session,
            const char *number,const char *mask,
            const int price, int game_id, int draw_id)
{
    // update player balance
    //
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }

    // Request actual DRAW from reddis!!!
    // check with draw from player



    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s:%s draw %d mask %s price %d game_id %d",
                                          session, number, draw_id, mask, price, game_id);
    if (r_reply == NULL)
    {
        log_err("set bet error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("unable record bet %s", number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    log_info("LPUSH %s:%d:%d:bets  %s",  session,game_id,draw_id, number);
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH %s:%d:%d:bets %s",  session,game_id,draw_id, number);
    if (r_reply == NULL)
    {
        log_err("push bet error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("unable write bet %s", number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    // add bet to draw list

    log_info("LPUSH %d:draw:%d:bets %s",game_id, draw_id, number);
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH %d:draw:%d:bets %s",game_id, draw_id, number);
    if (r_reply == NULL)
    {
        log_err("get bet error: %s", r_ctx->errstr );
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no draw %d", draw_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

// add bet to draw

    log_info("HMSET %d:draw:%d:%s mask %s price %d",game_id, draw_id, number,
           mask, price);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %d:draw:%d:%s mask %s price %d",
                                          game_id, draw_id, number,
                                          mask, price);
    if (r_reply == NULL)
    {
        log_err("get bet error: %s", r_ctx->errstr );
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no draw %d", draw_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try buy bet
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param number - bet number
* @param price - return price of bet
* @return 0 on success else error code
*
*/
int buy_bet_bingo(redisContext *r_ctx,const char *session,const char* number, int &price, int game_id, int draw_id)
{

    // TODO check bet BINGO CLUB HMGET

    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }

    log_info("-->LPUSH %s:%d:%d:bets %s",  session,game_id,draw_id, number);
    redisReply *r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH %s:%d:%d:bets %s",  session,game_id,draw_id, number);
    if (r_reply == NULL)
    {
        log_err("get bet error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no bet %s", number);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    // add bet to draw

    log_info("--->LPUSH %d:draw:%d:bets %s",game_id, draw_id, number);
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH %d:draw:%d:bets %s",game_id, draw_id, number);
    if (r_reply == NULL)
    {
        log_err("get bet error: %s", r_ctx->errstr );
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no draw %d", draw_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }


    return 0;
}

/*! \brief try to get current DRAW in Redis
*
*  Function return current draw
*
* @param r_ctx - Oppened Redis context
* @param game_id - game identificator.
* @param draw_id - returning draw identificator.
* @return 0 on success else error code
*
*/
int get_current_draw(redisContext *r_ctx,int game_id, int &draw_id)
{
    // Request DRAW from reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    syslog(LOG_INFO, "HMGET %d draw_id status", game_id);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %d draw_id status", game_id);
    if (r_reply == NULL)
    {
        log_err("get current draw error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no game_id %d", game_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        string status = "";
        if(r_reply->element[1]->len>0)
        {
            status = r_reply->element[1]->str;
        }
        if (r_reply->element[0]->len>0)
        {
            char *endptr = NULL;
            draw_id = strtoll(r_reply->element[0]->str, &endptr,10);
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return 0;
        }
        else
        {
            draw_id = -1;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return -1;
}

/*! \brief try to get DRAW in Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param game_id - game identificator.
* @return 0 on success else error code
*
*/
int get_draw(redisContext *r_ctx,const char *session,int game_id, string &out_json)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request DRAW from reddis!!!
    redisReply* r_reply = NULL;
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    syslog(LOG_INFO, "HMGET %d win_combination step draw_id bet_count ts_start ts_end status price  jackpot", game_id);

    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %d win_combination step draw_id bet_count ts_start ts_end status price  jackpot", game_id);
    if (r_reply == NULL)
    {
        log_err("get draw error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no game_id %d", game_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->elements && r_reply->type == REDIS_REPLY_ARRAY)
    {
        string win_combination ;
        if (r_reply->element[0]->type != REDIS_REPLY_NIL)
        if (r_reply->element[0]->len>0)
        {
            win_combination = r_reply->element[0]->str;
        }
        string step;
        if (r_reply->element[1]->type != REDIS_REPLY_NIL)
        if (r_reply->element[1]->len>0)
        {
            step = r_reply->element[1]->str;
        }

        string draw_id;
        if (r_reply->element[2]->type != REDIS_REPLY_NIL)
        if (r_reply->element[2]->len>0)
        {
            draw_id = r_reply->element[2]->str;
        }
        string bet_count;
        if (r_reply->element[3]->type != REDIS_REPLY_NIL)
        if (r_reply->element[3]->len>0)
        {
            bet_count = r_reply->element[3]->str;
        }
        string ts_start;
        if (r_reply->element[4]->type != REDIS_REPLY_NIL)
        if (r_reply->element[4]->len>0)
        {
            ts_start = r_reply->element[4]->str;
        }
        string ts_end;
        if (r_reply->element[5]->type != REDIS_REPLY_NIL)
        if (r_reply->element[5]->len>0)
        {
            ts_end = r_reply->element[5]->str;
        }
        string status;
        if (r_reply->element[6]->type != REDIS_REPLY_NIL)
        if (r_reply->element[6]->len>0)
        {
            status = r_reply->element[6]->str;
        }
        string price;
        if (r_reply->element[7]->type != REDIS_REPLY_NIL)
        if (r_reply->element[7]->len>0)
        {
            price = r_reply->element[7]->str;
        }
        string jackpot;
        if (r_reply->element[8]->type != REDIS_REPLY_NIL)
        if (r_reply->element[8]->len>0)
        {
            jackpot = r_reply->element[8]->str;
        }
        //    string ts_played = r_reply->element[8]->str;
				jackpot = "113000";
        out_json.append("\"draw\":{");
        out_json.append("\"id\":");
        out_json.append(draw_id);
        out_json.append(",\"ts\":");
        time_t ts = time(NULL);
        out_json.append(to_string(ts));
        out_json.append(",\"ts_start\":");
        out_json.append(ts_start);
        out_json.append(",\"ts_end\":");
        out_json.append(ts_end);
        out_json.append(",\"jackpot\":");
        out_json.append(jackpot);
        out_json.append(",\"status\":\"");
        out_json.append(status);
        out_json.append("\",\"price\":[");
        out_json.append(price);
        out_json.append("],\"step\":");
        out_json.append(step);
        out_json.append(",\"win_combination\":[");
        out_json.append(win_combination);
        out_json.append("]}");
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}


int  rgs_get_session(redisContext *r_ctx,const char *session,
										 string &player, string &username, int & balance,
			string &first_name,string &last_name)
{
	int bonus_balance = 0;
	string phone;
  if (check_redis(&r_ctx))
  {
      return ERR_REDIS;
  }
  // Request balance from reddis!!!
  redisReply* r_reply = NULL;
  //log_info("balance req: HMGET %s player", session);
  r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
  if (r_reply == NULL)
  {
      log_err("get balance error: %s", r_ctx->errstr );
      return ERR_REDIS;
  }
  if (r_reply->type == REDIS_REPLY_ERROR)
  {
      log_err("no money session %s", session);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
      return ERR_REDIS;
  }
  else
  {
      if (r_reply->element[0]->len>0)
      {
          player = r_reply->element[0]->str;
      }
			else
			{
				return ERR_SESSION;
			}
  }

  if (r_reply != NULL)
  {
      freeReplyObject(r_reply);
  }
  
  //log_info("session req: HMGET tgusers:%s balance ", player.c_str());
  r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s balance bonus_balance", player.c_str());
  if (r_reply == NULL)
  {
      log_err("get balance error: %s", r_ctx->errstr );
      return ERR_REDIS;
  }
  if (r_reply->type == REDIS_REPLY_ERROR)
  {
      log_err("no money session %s", session);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
      return ERR_REDIS;
  }
  else
  {

      char *endptr  = NULL;
      if (r_reply->element[0]->len > 0)
      {
      	balance = strtol(r_reply->element[0]->str, &endptr,10);
      }
			endptr = NULL;	

      if (r_reply->element[1]->len > 0)
      {
      	bonus_balance = strtol(r_reply->element[1]->str, &endptr,10);
      }

  } //r_reply no error

  if (r_reply != NULL)
  {
      freeReplyObject(r_reply);
  }

  //log_info("session req: HMGET tgusers:%s name phone  ", player.c_str());
  r_reply =(redisReply*) redisCommand(r_ctx,"HMGET tgusers:%s name phone", player.c_str());
  if (r_reply == NULL)
  {
      log_err("get balance error: %s", r_ctx->errstr );
      return ERR_REDIS;
  }
  if (r_reply->type == REDIS_REPLY_ERROR)
  {
      log_err("no money session %s", session);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
      return ERR_REDIS;
  }
  else
  {

      char *endptr  = NULL;
      if (r_reply->element[0]->len > 0)
      {
      	username = r_reply->element[0]->str;
      }
			endptr = NULL;	

      if (r_reply->element[1]->len > 0)
      {
      	phone = r_reply->element[1]->str;
      }

  } //r_reply no error

  if (r_reply != NULL)
  {
      freeReplyObject(r_reply);
  }

	first_name = username;
	last_name = username;	
	return 0;
}
/*! \brief try to get balance swerte from Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param balance - reference. returning after session find
* @param demo - 1 when balance is demo
* @return 0 on success else error code
*
*/
int get_tgid_and_token(redisContext *r_ctx,const char *session,
											string &tg_id,
											string &token) 
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;
    //log_info("balance req: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            tg_id = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    
    log_info("balance req: HMGET user:%s token ", tg_id.c_str());
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s token", tg_id.c_str());
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len > 0)
        {
        	token = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try to get balance swerte from Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param balance - reference. returning after session find
* @param demo - 1 when balance is demo
* @return 0 on success else error code
*
*/
int get_balance_senden(redisContext *r_ctx,const char *session,
											unsigned int &balance,unsigned int &bonus_balance, 
											int &demo, string &player, 
											unsigned long long &gross_bet, unsigned long long &gross_win,
unsigned long long &gross_bet_bonus, unsigned long long &gross_win_bonus

											)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
		bonus_balance = 0;
            balance = 0;
            gross_bet = 0;
            gross_win = 0;
    redisReply* r_reply = NULL;
    //log_info("balance req: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    
    log_info("balance req: HMGET user:%s balance bonus_balance gross_bet gross_win ", player.c_str());
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s balance bonus_balance gross_bet gross_win gross_bet_bonus gross_win_bonus", player.c_str());
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	balance = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	

        if (r_reply->element[1]->len > 0)
        {
        	bonus_balance = strtol(r_reply->element[1]->str, &endptr,10);
        }


				endptr = NULL;	

        if (r_reply->element[2]->len > 0)
        {
        	gross_bet = strtoll(r_reply->element[2]->str, &endptr,10);
        }

				endptr = NULL;	

        if (r_reply->element[3]->len > 0)
        {
  	      gross_win = strtoll(r_reply->element[3]->str, &endptr,10);
	
        }
				endptr = NULL;	
////////////////////
				if (r_reply->element[4]->len > 0)
        {
        	gross_bet_bonus = strtoll(r_reply->element[4]->str, &endptr,10);

        }
				endptr = NULL;	

        if (r_reply->element[5]->len > 0)
        {
        	gross_win_bonus = strtoll(r_reply->element[5]->str, &endptr,10);
        }


    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}
int get_balance_senden(redisContext *r_ctx,const char *player,
											unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
            balance = 0;
    redisReply* r_reply = NULL;
    //log_info("balance req: HMGET %s player", session);
    
    log_info("balance req: HGET user:%s balance", player);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s balance", player);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money player %s", player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->element[0]->type !=REDIS_REPLY_NIL)
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	balance = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	
    }
		else
		{
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
			return -1;
		}

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

int get_balance_senden(redisContext *r_ctx,const char *session,
											unsigned int &balance, string &player)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
            balance = 0;
    redisReply* r_reply = NULL;
    //log_info("balance req: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    
    log_info("balance req: HGET user:%s balance", player.c_str());
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s balance", player.c_str());
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	balance = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try to get balance from Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param balance - reference. returning after session find
* @param demo - 1 when balance is demo
* @return 0 on success else error code
*
*/
int get_balance(redisContext *r_ctx,const char *session,unsigned int &balance, int &demo, string &player)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s demo balance player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_err("SESSION %s balance [%s]", session, r_reply->element[1]->str);

        if (r_reply->element[0]->len>0)
        {
            if (strncmp(r_reply->element[0]->str, "1",1)==0)
            {
                log_err("demo game!!!");
                demo = 1;
            }
        }
        if (r_reply->element[2]->len>0)
        {
            player = r_reply->element[2]->str;
        }
        char *endptr  = NULL;
        if (r_reply->element[1]->len<=0)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            balance = 0;
            return 1;
        }
        balance = strtol(r_reply->element[1]->str, &endptr,10);
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}
/*! \brief try to set balance senden in Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param balance - reference. returning after session find
* @return 0 on success else error code
*
*/

int set_balance_senden(redisContext *r_ctx,const char *session,unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session
    string player;
    //log_info("set_balance: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    


    //log_info("set_balance: HMSET %s balance %d", player.c_str(), balance);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s balance %d", player.c_str(), balance);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis session error %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}
int set_balance_senden_player_notxid(redisContext *r_ctx,const char *player,unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session
    //log_info("set_balance: HMSET %s balance %d", player.c_str(), balance);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s balance %d", player, balance);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis get money error %s", player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		// set txId
    return 0;
}
int set_balance_senden_player(redisContext *r_ctx,const char *player,unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session
    //log_info("set_balance: HMSET %s balance %d", player.c_str(), balance);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s balance %d", player, balance);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis session error %s", player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}
int set_balance_senden(redisContext *r_ctx,const char *session,const char *tx_id,unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session
    string player;
    //log_info("set_balance: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    


    //log_info("set_balance: HMSET %s balance %d", player.c_str(), balance);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s balance %d", player.c_str(), balance);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis session error %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		// set txId
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:txs %s",
					player.c_str(), tx_id);
    if (r_reply == NULL)
    {
        log_err("push txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("cant push tx %s for player %s", player.c_str(), tx_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

int set_balance_senden(redisContext *r_ctx,const char *session,unsigned int &balance, unsigned int &bonus_balance,unsigned long long gross_bet, unsigned long long gross_win, unsigned long long gross_bet_bonus, unsigned long long gross_win_bonus)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session
    string player;
    //log_info("set_balance: HMGET %s player", session);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player", session);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money session %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (r_reply->element[0]->len>0)
        {
            player = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    


    //log_info("set_balance: HMSET %s balance %d", player.c_str(), balance);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:%s balance %d bonus_balance %d gross_bet %llu gross_win %llu gross_bet_bonus %llu gross_win_bonus %llu", player.c_str(), balance,bonus_balance,gross_bet, gross_win, gross_bet_bonus, gross_win_bonus);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis session error %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief try to set balance in Redis
*
*  Function checking session on exists
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param balance - reference. returning after session find
* @return 0 on success else error code
*
*/
int set_balance(redisContext *r_ctx,const char *session,unsigned int &balance)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request balance from reddis!!!
    redisReply* r_reply = NULL;

     // get player from session


    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET %s balance %d", session, balance);
    if (r_reply == NULL)
    {
        log_err("set balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis session error %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return user name
*
*  Function return user name, using session
*
* @param r_ctx - Oppened Redis context
* @param session - www session
* @param out - return json
* @return 0 on success else error code
*
*/
int get_user_from_session(redisContext *r_ctx, const char *session, string &out)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player ", session);
    if (r_reply == NULL)
    {
        log_err("get session error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no player %s", session);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            char *player = r_reply->element[0]->str;
            out = "{\"player\":\"";
            out.append(player);
            out.append("\", \"error\":0}");
        }
        else
        {
            out.append("\"error\":1}");
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return game_path
*
*  Function return path of game (nginx cataloge)
*
* @param r_ctx - Oppened Redis context
* @param game_id - game code
* @param game_path - return path of the game
* @return 0 on success else error code
*
*/
int get_game_path(redisContext *r_ctx, const int game_id, string &game_path)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET games %d ", game_id);
    if (r_reply == NULL)
    {
        log_err("get game path error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no path %d", game_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {

            log_err(" game_id %d path %s",game_id, r_reply->element[0]->str);
            game_path = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return game id
*
*  Function return game code
*
* @param r_ctx - Oppened Redis context
* @param session - User session. Stored in Redis as key
* @param game_id - return player game code
* @return 0 on success else error code
*
*/
int get_game_id(redisContext *r_ctx, string &session, string &game_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s game_id ", session.c_str());
    if (r_reply == NULL)
    {
        log_err("get game_id error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no session %s", session.c_str());
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        if (!r_reply->elements)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REDIS;
        }
        if (r_reply->element[0]->len>0)
        {
            game_id = r_reply->element[0]->str;
            log_err("\n ------- \n SESSION %s GAME_ID [%s]",session.c_str(),game_id.c_str());
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return string bonuses
*
*  Function return game bonuses
*
* @param r_ctx - Oppened Redis context
* @param game_id - player game code
* @param prices  - string array of prices
* @return 0 on success else error code
*
*/
int get_game_bonuses(redisContext *r_ctx, string game_id, string &bonuses)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s:params bonus_games_count bonus_1_game_id bonus_1_prices bonus_2_game_id bonus_2_prices bonus_3_game_id bonus_3_prices", game_id.c_str());
    if (r_reply == NULL)
    {
        log_err("get bonuses error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no bonuses for game_id %s", game_id.c_str());
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (!r_reply->elements)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REDIS;
        }
        if (r_reply->element[0]->len>0)
        {

            char *endptr = NULL;
            int  bonus_games_count = strtol(r_reply->element[0]->str, &endptr, 10);
            bonuses.append(",\"bonus_games_count\":");
            bonuses.append(r_reply->element[0]->str);
            if (bonus_games_count >=1)
            {
                if (r_reply->element[1]->len>0)
                {
                    bonuses.append(",\"bonus_1_game_id\":");
                    bonuses.append(r_reply->element[1]->str);
                    bonuses.append(",");
                }
                if (r_reply->element[2]->len>0)
                {
                    bonuses.append("\"bonus_1_prices\":[");
                    bonuses.append(r_reply->element[2]->str);
                    bonuses.append("]");
                }
            } // bonus_count >= 1
            if (bonus_games_count >1)
            {
                bonuses.append(",");
            }
            if (bonus_games_count >=2)
            {
                if (r_reply->element[3]->len>0)
                {
                    bonuses.append("\"bonus_2_game_id\":");
                    bonuses.append(r_reply->element[3]->str);
                    bonuses.append(",");
                }
                if (r_reply->element[4]->len>0)
                {
                    bonuses.append("\"bonus_2_prices\":[");
                    bonuses.append(r_reply->element[4]->str);
                    bonuses.append("]");
                }
            } // bonus_count >= 2
            if (bonus_games_count >2)
            {
                bonuses.append(",");
            }
            if (bonus_games_count >=3)
            {
                if (r_reply->element[5]->len>0)
                {
                    bonuses.append("\"bonus_3_game_id\":");
                    bonuses.append(r_reply->element[5]->str);
                    bonuses.append(",");
                }
                if (r_reply->element[6]->len>0)
                {
                    bonuses.append("\"bonus_3_prices\":[");
                    bonuses.append(r_reply->element[6]->str);
                    bonuses.append("]");
                }
            } // bonus_count >= 2

        } // if bonus_games_count
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Return string prices
*
*  Function return game prices
*
* @param r_ctx - Oppened Redis context
* @param game_id - player game code
* @param prices  - string array of prices
* @return 0 on success else error code
*
*/
int get_game_prices(redisContext *r_ctx, string game_id, string &prices)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    // Request session from reddis!!!
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s:params prices", game_id.c_str());
    if (r_reply == NULL)
    {
        log_err("get prices error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no prices for game_id %s", game_id.c_str());
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (!r_reply->elements)
        {
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REDIS;
        }
        if (r_reply->element[0]->len>0)
        {
            prices = r_reply->element[0]->str;
        }
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

int json_to_int_patch_hujatch(Json::Value &v)
{
    if (v.isInt64())
        return v.asInt64();
    if (v.isInt())
        return v.asInt();
    if (v.isDouble())
        return int(v.asDouble());
    char *end = NULL;
    if (v.isString())
    {
        uint64_t val = strtoll(v.asCString(), &end,10);
        return val;
    }
    return -1;
}

void add_headers(FCGX_Request &request, string &out)
{
    char * merchant_key = secure_getenv("LOGYCOM_MERCHANT_KEY");
    if (merchant_key == NULL)
    {
        merchant_key = MERCHANT_KEY;
        //  fprintf(stdout,"DEFAULT MERCHANT_KEY\n");
    }
    int merchant_key_len = strnlen(merchant_key,MAX_MERCHANT_KEY_LEN);
    char * merchant_id = secure_getenv("LOGYCOM_MERCHANT_ID");
    if (merchant_id == NULL)
    {
        merchant_id = MERCHANT_ID;
        //  fprintf(stdout,"DEFAULT MERCHANT_ID\n");
    }
    //  fprintf(stdout, "LOGYCOM_MERCHANT_KEY: %s\n LOGYCOM_MERCHANT_ID: %s\n",
    //        merchant_key,merchant_id);
    out.append("\n\n");
    char buf[1024] ={0x00};
    snprintf(buf,sizeof(buf),"X-Merchant-Id: %s\r\n",merchant_id);
    FCGX_PutS(buf, request.out);
        //FCGX_PutS("Content-type: text/javascript\r\n", request.out);
    FCGX_PutS("Content-type: application/json\r\n", request.out);
    // calculate X-Sign

    time_t t;
    t = time(NULL);
    char time_buf[100] = {0x00};
    snprintf(time_buf,sizeof(time_buf),"X-Timestamp: %u\r\n", t);

    string sign, sign_body;
    sign_body.append(out);
    sign_body.append(to_string(t));
    //  ENGINE_load_builtin_engines();
    //  ENGINE_register_all_complete();
    HMAC_CTX *ctx = HMAC_CTX_new();
    //HMAC_CTX_init(ctx);
    //HMAC_Init_ex(ctx, MERCHANT_KEY,MERCHANT_KEY_LEN - 1 , EVP_sha1(),NULL);
    HMAC_Init_ex(ctx, merchant_key,merchant_key_len , EVP_sha1(),NULL);
    //fprintf(stdout, "key[%d]:[%s]\n",MERCHANT_KEY_LEN,MERCHANT_KEY);
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
    char sign_buf[4096] = {0x00};
    snprintf(sign_buf,sizeof(sign_buf),"X-Sign: %s\r\n",  s_buf.c_str());
    //fprintf(stdout, "\ns_buf:[%s] sign_buf:[%s",s_buf.c_str(), sign_buf);

    FCGX_PutS(time_buf, request.out);
    FCGX_PutS(sign_buf, request.out);
    FCGX_PutS("\r\n", request.out);
    FCGX_PutS(out.c_str(), request.out);

    return;
}


/*! \brief The main function
*
*  Function load bets
* load configs for bets
*
* @return 0 on success else error code
*
*/
int main(void)
{
    pid_t pid = 0;
    pid_t sid = 0;

#ifdef _ORAORA_DAEMONIZE
    pid = fork();
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    openlog("oraora_d", LOG_NOWAIT | LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "Successfully started oraora");

    umask(0);

    sid = setsid();
    if (sid < 0)
    {
        syslog(LOG_ERR, "Could not generate session ID for child process");
        closelog();
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Could not change working directory to /");
        closelog();
        exit(EXIT_FAILURE);
    }
    closelog();

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
    signal(SIGSTOP, SignalHandler);
    signal(SIGCONT, SignalHandler);
#endif



    // hardcore read files from CDN
    char *configs_path =secure_getenv("BF_PATH");

    auto* r_ctx = create_redis_ctx();
    GamesConfigLoader config_loader;
    redisFree(r_ctx);

    /*
        Load games

    */
    
    pthread_t id[THREAD_COUNT];

    FCGX_Init();
    printf("Lib is inited\n");

    //открываем новый сокет
  char *socket_path =secure_getenv("FCGI_SOCKET_PATH");
    if (socket_path == NULL)
    {
        socket_path = SOCKET_PATH;
    }
    socketId = FCGX_OpenSocket(socket_path, 20);
    if(socketId < 0)
    {
        log_err("unable open %s %s", socket_path, strerror(errno));
        return -1;
    }

    printf("Socket is opened\n");

// chmod 770
#if 0
  int chmod_res = chmod(SOCKET_PATH, S_IRWXU| S_IRWXG);
  if (chmod_res !=0 )
  {
    fprintf(stderr, "unable chmod socket %s %s\n", SOCKET_PATH, strerror(errno));
  }

  struct group *gr  = NULL;
  gr = getgrnam (ORAORA_USER);
  if (gr == NULL)
  {
    fprintf(stderr, "unable find user [%s] %s\n", ORAORA_USER, strerror(errno));
    return -1;
  }
#endif
#if 0
  int chown_res = chown(ORAORA_USER, -1, gr->gr_gid);
  if (chown_res != 0)
  {
    fprintf(stderr, "unable chown group [%s] %s\n", ORAORA_USER, strerror(errno));
    return -1;

  }
#endif
    for(int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&id[i], NULL, doit, (void *)&config_loader);
    }

    for(int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(id[i], NULL);
    }

    return 0;
}
/*! \brief generate session for www tg user
*
*  Function generate new session for user
*  authorized from tg API
*
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int generate_session4www_tmp_with_balanceV2(redisContext *r_ctx, const char* user, 
string &session)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    time_t current_time = time(NULL);

    // INIT SESSION
    //generate session
OSSL_PROVIDER *provider = OSSL_PROVIDER_load(NULL, "default");		
    EVP_PKEY *pkey = NULL;
    BIO *bio;
#if 0
  char *key="\x0a \
---- PRIVATE KEY----- \x0a \
MC4CYDK2VwBCIEII9XnyEUVaAHw5S1G7I27QpLDamJub6JXhcfup8sWzph \x0a \
----RIVATE KEY-----";
#endif
    bio = BIO_new_mem_buf((void *)key, *keylen);
    if(bio != NULL)
    {
        EVP_PKEY*  private_read = PEM_read_bio_PrivateKey(
            bio,   /* BIO to read the private key from */
            &pkey, /* pointer to EVP_PKEY structure */
            NULL,  /* password callback - can be NULL */
            NULL   /* parameter passed to callback or password if callback is NULL */
            );

        if (pkey == NULL)
        {
            log_err("unable to load key");
            ERR_load_crypto_strings();
            char err[255];
            ERR_error_string(ERR_get_error(), err);
            return 0;
        }
    }
#if 0
  BIO *bp = NULL;
  EVP_PKEY_print_params(bp, pkey,1, NULL);
#endif

    // SESSION


    EVP_MD_CTX* mdctx = NULL;
    mdctx = EVP_MD_CTX_create();
    if(mdctx == NULL)
    {
        log_err("unable create md ctx");
    }
    int res_md = EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey);
    if (res_md != 1)
    {
        log_err("unable init md");
        ERR_load_crypto_strings();
        char err[255];
        ERR_error_string(ERR_get_error(), err);
        return -1;
    }
    res_md = EVP_DigestSignUpdate(mdctx, user,strlen(user));
    if (res_md != 1)
    {
        log_err("err Sign message");
    }
    size_t sig_len;
    res_md = EVP_DigestSignFinal(mdctx, NULL,&sig_len);
    char *sig = (char*)OPENSSL_malloc(sig_len);
    res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
    CryptoPP::Base32Encoder encoder(NULL,false);
    log_err("SIG: [%d] bytes", sig_len);
    encoder.Put((byte*) sig, sig_len);
    encoder.MessageEnd();
//    FILE *f = fopen("/dev/shm/s", "w+");
//    fwrite(sig,sig_len,1,f);
//    fclose(f);
    char encoded[1024]={0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((byte*)encoded, len);
    log_err("SIG[%d]:%s",len, encoded);
    if(sig !=NULL)
    {
        OPENSSL_free((void*)sig);
    }
    if (mdctx!=NULL)
    {
        EVP_MD_CTX_destroy(mdctx);
    }
OSSL_PROVIDER_unload(provider); 
		session  = encoded;
    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET %s session_ts %d player %s demo 0 balance 0 status 0",
                                          encoded,  current_time , user);

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

int generate_session4www_tmp_with_balance(redisContext *r_ctx, const char* user)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    time_t current_time = time(NULL);
    string session;

    // INIT SESSION
    //generate session
OSSL_PROVIDER *provider = OSSL_PROVIDER_load(NULL, "default");		
    EVP_PKEY *pkey = NULL;
    BIO *bio;
#if 0
  char *key="\x0a \
---- PRIVATE KEY----- \x0a \
MC4CYDK2VwBCIEII9XnyEUVaAHw5S1G7I27QpLDamJub6JXhcfup8sWzph \x0a \
----RIVATE KEY-----";
#endif
    bio = BIO_new_mem_buf((void *)key, *keylen);
    if(bio != NULL)
    {
        EVP_PKEY*  private_read = PEM_read_bio_PrivateKey(
            bio,   /* BIO to read the private key from */
            &pkey, /* pointer to EVP_PKEY structure */
            NULL,  /* password callback - can be NULL */
            NULL   /* parameter passed to callback or password if callback is NULL */
            );

        if (pkey == NULL)
        {
            log_err("unable to load key");
            ERR_load_crypto_strings();
            char err[255];
            ERR_error_string(ERR_get_error(), err);
            return 0;
        }
    }
#if 0
  BIO *bp = NULL;
  EVP_PKEY_print_params(bp, pkey,1, NULL);
#endif

    // SESSION


    EVP_MD_CTX* mdctx = NULL;
    mdctx = EVP_MD_CTX_create();
    if(mdctx == NULL)
    {
        log_err("unable create md ctx");
    }
    int res_md = EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey);
    if (res_md != 1)
    {
        log_err("unable init md");
        ERR_load_crypto_strings();
        char err[255];
        ERR_error_string(ERR_get_error(), err);
        return -1;
    }
    res_md = EVP_DigestSignUpdate(mdctx, user,strlen(user));
    if (res_md != 1)
    {
        log_err("err Sign message");
    }
    size_t sig_len;
    res_md = EVP_DigestSignFinal(mdctx, NULL,&sig_len);
    char *sig = (char*)OPENSSL_malloc(sig_len);
    res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
    CryptoPP::Base32Encoder encoder(NULL,false);
    log_err("SIG: [%d] bytes", sig_len);
    encoder.Put((byte*) sig, sig_len);
    encoder.MessageEnd();
//    FILE *f = fopen("/dev/shm/s", "w+");
//    fwrite(sig,sig_len,1,f);
//    fclose(f);
    char encoded[1024]={0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((byte*)encoded, len);
    log_err("SIG[%d]:%s",len, encoded);
    if(sig !=NULL)
    {
        OPENSSL_free((void*)sig);
    }
    if (mdctx!=NULL)
    {
        EVP_MD_CTX_destroy(mdctx);
    }
OSSL_PROVIDER_unload(provider); 

    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET user:%s session_ts %d session %s token 1111 ts %d demo 0",
                                          user, current_time, encoded, current_time);

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    // TODO !!! ATTENTION. NEED REQUEST BALANCE
    //          from user:[tg_id] balance
    //          BEFORE INIT SESSION
//write session to redis with player_name
    log_info("HMSET %s session_ts %d player %s",
           encoded,  current_time , user);
    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET %s session_ts %d player %s demo 0 balance 0 status 0",
                                          encoded,  current_time , user);

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief generate session for www user
*
*  Function generate new session for user
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int generate_session4www(redisContext *r_ctx, const char* user)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    time_t current_time = time(NULL);
    string session;
OSSL_PROVIDER *provider = OSSL_PROVIDER_load(NULL, "default");		

    // INIT SESSION
    //generate session
    EVP_PKEY *pkey = NULL;
    BIO *bio;
#if 0
  char *key="\x0a \
---- PRIVATE KEY----- \x0a \
MC4CYDK2VwBCIEII9XnyEUVaAHw5S1G7I27QpLDamJub6JXhcfup8sWzph \x0a \
----RIVATE KEY-----";
#endif
    bio = BIO_new_mem_buf((void *)key, *keylen);
    if(bio != NULL)
    {
        EVP_PKEY*  private_read = PEM_read_bio_PrivateKey(
            bio,   /* BIO to read the private key from */
            &pkey, /* pointer to EVP_PKEY structure */
            NULL,  /* password callback - can be NULL */
            NULL   /* parameter passed to callback or password if callback is NULL */
            );

        if (pkey == NULL)
        {
            log_err("unable to load key");
            ERR_load_crypto_strings();
            char err[255];
            ERR_error_string(ERR_get_error(), err);
            return 0;
        }
    }
#if 0
  BIO *bp = NULL;
  EVP_PKEY_print_params(bp, pkey,1, NULL);
#endif

    // SESSION
    EVP_MD_CTX* mdctx = NULL;
    mdctx = EVP_MD_CTX_create();
    if(mdctx == NULL)
    {
        log_err("unable create md ctx");
    }
    int res_md = EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey);
    if (res_md != 1)
    {
        log_err("unable init md");
        ERR_load_crypto_strings();
        char err[255];
        ERR_error_string(ERR_get_error(), err);
        return -1;
    }
    res_md = EVP_DigestSignUpdate(mdctx, user,strlen(user));
    if (res_md != 1)
    {
        log_err("err Sign message");
    }
    size_t sig_len;
    res_md = EVP_DigestSignFinal(mdctx, NULL,&sig_len);
    char *sig = (char*)OPENSSL_malloc(sig_len);
    res_md = EVP_DigestSignFinal(mdctx,(unsigned char*) sig, &sig_len);
    CryptoPP::Base32Encoder encoder(NULL,false);
    log_err("SIG: [%d] bytes", sig_len);
    encoder.Put((byte*) sig, sig_len);
    encoder.MessageEnd();
    FILE *f = fopen("/dev/shm/s", "w+");
    fwrite(sig,sig_len,1,f);
    fclose(f);
    char encoded[1024]={0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((byte*)encoded, len);
    log_err("SIG[%d]:%s",len, encoded);
    if(sig !=NULL)
    {
        OPENSSL_free((void*)sig);
    }
    if (mdctx!=NULL)
    {
        EVP_MD_CTX_destroy(mdctx);
    }

OSSL_PROVIDER_unload(provider); 

    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET user:%s session_ts %d session %s",
                                          user, current_time, encoded);

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

//write session to redis with player_name
    log_info("HMSET %s session_ts %d player %s",
           encoded,  current_time , user);
    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET %s session_ts %d player %s",
                                          encoded,  current_time , user);

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}
/*! \brief generate token for user
*
*  Function generate new token for user
* and changes old state
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int generate_token_www(redisContext *r_ctx, const char* user, string &token_s)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    time_t current_time = time(NULL);
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    vector<int> token={1,2,3,4,5,6,7,8,9,9,2,3,4,5};
    std::mt19937 random;
    random = std::mt19937(current_time);
    shuffle(begin(token), end(token), random);
    token.resize(TOKEN_SIZE);
    for (auto a:token)
    {
        token_s.append(to_string(a));
    }
    log_info("new token %s generated", token_s.c_str());
    // create token
    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET user:%s ts %d token %s",
                                          user, current_time, token_s.c_str());

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;
}

/*! \brief generate token for user
*
*  Function generate new token for user
* and changes old state
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int generate_token(redisContext *r_ctx, const char* user, int token_gw)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    time_t current_time = time(NULL);
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    vector<int> token={1,2,3,4,5,6,7,8,9,9,2,3,4,5};
    std::mt19937 random;
    random = std::mt19937(current_time);
    shuffle(begin(token), end(token), random);
    token.resize(TOKEN_SIZE);
    string token_s;
    for (auto a:token)
    {
        token_s.append(to_string(a));
    }
    log_info("new token %s generated", token_s.c_str());
    // create token
    r_reply =(redisReply*) redisCommand(r_ctx,
                                          "HMSET user:%s ts %d token %s",
                                          user, current_time, token_s.c_str());

    if (r_reply == NULL)
    {
        log_err("ERR NULL reply %s for user %s ",
               r_ctx->errstr,   user);
        return REDIS_REPLY_ERROR;
    }

    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("ERR HMSET %s for user %s ts %d ",
               r_ctx->errstr,user, current_time);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return REDIS_REPLY_ERROR;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    //
    //
    //
    // LPUSH
    char* token_gw_list = DEFAULT_TOKEN_GW_TG_LIST;
    if (token_gw == DEFAULT_SMS_TWILLO)
    {
        token_gw_list = DEFAULT_TOKEN_GW_SMS_TWILLO_LIST;
    }
    log_info("LPUSH %s %s [%d]",token_gw_list,  user, token_gw);
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH %s %s",
                                          token_gw_list,  user);
    if (r_reply == NULL)
    {
        log_err("set session error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err reply %s", user);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Check and send SMS to user
*
*  Function check in redis user registration
* status. If user are new - his phone write
* in redis list registration_www2pg.
* If user already registered - should check
* last time of SMS.
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int register_user_www(redisContext *r_ctx, const char* tg_id, int sms_gate)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    log_info("hmget users:%s ts session tg_id", tg_id);
    r_reply =(redisReply*) redisCommand(r_ctx,"hmget user:%s ts session", tg_id);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            //if (current_time >= SMS_REQUEST_TIMEOUT)
            if (current_time >= 5)
            {
								string token;
                int res = generate_token_www(r_ctx, tg_id, token);
								string message = "Token: ";
								message.append(token);
								std::string tg_key = secure_getenv("TGAPI_KEY");
								res = sendTelegramMessage(tg_key, tg_id, message);
								
                // should ceck result of tocken generate
                // if err - return token generation error
                if (r_reply->element[1]->len==0)
                {
                    // no elements. first registration
                    log_info("first registration %s", tg_id);
                    int res = generate_session4www(r_ctx, tg_id);
                }

                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_SESSION_READY_TOKEN;
                //return ERR_REGISTER_RESEND;
            }
            else
            {
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_REGISTER_TIMEOUT;
            }
        } // if element len
        else
        {
            // no elements. first registration
            // create token and session
						string token;
            int res = generate_token_www(r_ctx, tg_id, token);
						// send message directly to tg
						string message = "Token: ";
						message.append(token);
						std::string tg_key = secure_getenv("TGAPI_KEY");
						res = sendTelegramMessage(tg_key, tg_id, message);
						
            // should ceck result of tocken generate
            // if err - return token generation error
            if (r_reply->element[1]->len==0)
            {
                // no elements. first registration
                log_info("first registration %s", tg_id);
                int res = generate_session4www(r_ctx, tg_id);
            }

            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_SESSION_READY_TOKEN;

        }// if element->len
    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Check and send SMS to user
*
*  Function check in redis user registration
* status. If user are new - his phone write
* in redis list registration2pg.
* If user already registered - should check
* last time of SMS.
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int register_user(redisContext *r_ctx, const char* user)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s ts", user);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            if (current_time >= SMS_REQUEST_TIMEOUT)
            {
                int res = generate_token(r_ctx, user);
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_REGISTER_RESEND;
            }
            else
            {
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_REGISTER_TIMEOUT;
            }
        } // if element len
        else
        {
            // no elements. first registration
            log_info("first registration %s", user);
            int res = generate_token(r_ctx, user);
            res = generate_p12(r_ctx,user);

        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}


/*! \brief get tg_id
*
*  Function check user token in redis
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param token  - token from SMS (or tg bot)
* @param session  - session from redis
* @return 0 on success else error code
*
*/
int get_tgid_from_phone(redisContext *r_ctx, const char* phone,
                                    string &tg_id)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET users:%s tg_id", phone);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            tg_id = r_reply->element[0]->str;
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
						return 0;
        } // if element len
        else
        {
            // no token. return err
            log_info("not found registration %s", tg_id.c_str());
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REGISTER_USER;
        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}


/*! \brief Check token and send SESSION to user
*
*  Function check user token in redis
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param token  - token from SMS (or tg bot)
* @param session  - session from redis
* @return 0 on success else error code
*
*/
int get_user_sesson_after_register_tg(redisContext *r_ctx, const char* user,
                                   string token, string &session)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s ts token session", user);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            if (current_time >= SMS_REQUEST_TIMEOUT)
            {
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_TOKEN_TIMEOUT;
            }// if token is too old
            else if (r_reply->element[2]->len == 0)
            {
                // link is not ready
                // generate session
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_SESSION_GENERATE;
            }
            else
            {
                // time is ok, need check token
                if (r_reply->element[1]->len>0 &&
                    r_reply->element[2]->len>0
                    )
                {
                    char *endptr = NULL;
                    int token_original = strtol(r_reply->element[1]->str, &endptr, 10);
                    endptr = NULL;
                    int token_to_check = strtol(token.c_str(), &endptr, 10);
                    log_info("\n=======\n check %d original %d",
                           token_original, token_to_check);
                    if (token_original != token_to_check)
                    {
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_TOKEN;
                    }
                    session = r_reply->element[2]->str;
                    if (r_reply != NULL)
                    {
                        freeReplyObject(r_reply);
                    }
                    return 0;
                } // if user have token and p12
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }

                return ERR_TOKEN;
            }// else token time is ok
        } // if element len
        else
        {
            // no token. return err
            log_info("not found registration %s", user);
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REGISTER_USER;
        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}
int get_user_sesson_after_registerV2(redisContext *r_ctx, const char* user,
                                   string token, string &session)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s ts token session", user);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            if (r_reply->element[2]->len == 0)
            {
                // link is not ready
                // generate session
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_SESSION_GENERATE;
            }
            else
            {
                // time is ok, need check token
                if (r_reply->element[1]->len>0 &&
                    r_reply->element[2]->len>0
                    )
                {
                    char *endptr = NULL;
                    int token_original = strtol(r_reply->element[1]->str, &endptr, 10);
                    endptr = NULL;
                    int token_to_check = strtol(token.c_str(), &endptr, 10);
                    log_info("\n=======\n check %d original %d",
                           token_original, token_to_check);
                    if (token_original != token_to_check)
                    {
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_TOKEN;
                    }
                    session = r_reply->element[2]->str;
                    if (r_reply != NULL)
                    {
                        freeReplyObject(r_reply);
                    }
                    return 0;
                } // if user have token and p12
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }

                return ERR_TOKEN;
            }// else token time is ok
        } // if element len
        else
        {
            // no token. return err
            log_info("not found registration %s", user);
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REGISTER_USER;
        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}


/*! \brief Check token and send SESSION to user
*
*  Function check user token in redis
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param token  - token from SMS (or tg bot)
* @param session  - session from redis
* @return 0 on success else error code
*
*/
int get_user_sesson_after_register(redisContext *r_ctx, const char* user,
                                   string token, string &session)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s ts token session", user);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            if (current_time >= SMS_REQUEST_TIMEOUT)
            {
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_TOKEN_TIMEOUT;
            }// if token is too old
            else if (r_reply->element[2]->len == 0)
            {
                // link is not ready
                // generate session
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_SESSION_GENERATE;
            }
            else
            {
                // time is ok, need check token
                if (r_reply->element[1]->len>0 &&
                    r_reply->element[2]->len>0
                    )
                {
                    char *endptr = NULL;
                    int token_original = strtol(r_reply->element[1]->str, &endptr, 10);
                    endptr = NULL;
                    int token_to_check = strtol(token.c_str(), &endptr, 10);
                    log_info("\n=======\n check %d original %d",
                           token_original, token_to_check);
                    if (token_original != token_to_check)
                    {
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_TOKEN;
                    }
                    session = r_reply->element[2]->str;
                    if (r_reply != NULL)
                    {
                        freeReplyObject(r_reply);
                    }
                    return 0;
                } // if user have token and p12
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }

                return ERR_TOKEN;
            }// else token time is ok
        } // if element len
        else
        {
            // no token. return err
            log_info("not found registration %s", user);
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REGISTER_USER;
        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Check token and send p12 to user
*
*  Function check user token in redis
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param token  - token from SMS
* @param secret_path  - direct url (p12)
* @return 0 on success else error code
*
*/
int get_user_p12(redisContext *r_ctx, const char* user,
                 string token, string &secret_path)
{
    // find user in reddis!!!
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:%s ts token secret_path", user);
    if (r_reply == NULL)
    {
        log_err("get user error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user ");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        if (r_reply->element[0]->len>0)
        {
            time_t current_time = time(NULL);
            char *endptr = NULL;
            long long t= strtoll(r_reply->element[0]->str, &endptr,10);
            current_time -= t;
            log_info("current time diff=%d", current_time);
            if (current_time >= SMS_REQUEST_TIMEOUT)
            {
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_TOKEN_TIMEOUT;
            }// if token is too old
            else if (r_reply->element[2]->len == 0)
            {
                // link is not ready
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }
                return ERR_P12_GENERATE;
            }
            else
            {
                // time is ok, need check token
                if (r_reply->element[1]->len>0 &&
                    r_reply->element[2]->len>0
                    )
                {
                    char *endptr = NULL;
                    int token_original = strtol(r_reply->element[1]->str, &endptr, 10);
                    endptr = NULL;
                    int token_to_check = strtol(token.c_str(), &endptr, 10);
                    log_info("\n=======\n check %d original %d",
                           token_original, token_to_check);
                    if (token_original != token_to_check)
                    {
                        if (r_reply != NULL)
                        {
                            freeReplyObject(r_reply);
                        }
                        return ERR_TOKEN;
                    }
                    secret_path = r_reply->element[2]->str;
                    if (r_reply != NULL)
                    {
                        freeReplyObject(r_reply);
                    }
                    return 0;
                } // if user have token and p12
                if (r_reply != NULL)
                {
                    freeReplyObject(r_reply);
                }

                return ERR_TOKEN;
            }// else token time is ok
        } // if element len
        else
        {
            // no token. return err
            log_info("not found registration %s", user);
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }
            return ERR_REGISTER_USER;
        }// if element->len

    } // if no REDIS_REPLY_ERROR

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

int p2p_generate_form_withdrowal(const char *player,const int amount,
                                 const char* currency, const char *gw, const uint64_t invoice,
                                 string &out_form)
{
    string body_sign;
    body_sign.append(PAYME_TEST_API5KEY);
    map <string,string> form_map;
    string order_desc = "Payout for user ";
    order_desc.append(player);
    order_desc.append (" is ");
    order_desc.append(to_string(amount));
    order_desc.append(currency);

    form_map.emplace(std::make_pair(std::string("order_desc"),order_desc));

    form_map.emplace(std::make_pair(
        std::string("server_url"),
        std::string(PAYME_TEST_CALLBACK_WD_URL)));

    form_map.emplace(std::make_pair(
        std::string("response_url"),
        std::string(PAYME_TEST_RESP_URL)));

    form_map.emplace(std::make_pair(
        std::string("user_id"),
        std::string(player)));

    form_map.emplace(std::make_pair(
        std::string("order_id"),
        std::string(to_string(invoice))));

    form_map.emplace(std::make_pair(
        std::string("currency"),
        std::string(currency)));

    form_map.emplace(std::make_pair(
        std::string("amount"),
        std::string(to_string(amount))));

    form_map.emplace(std::make_pair(
        std::string("merchant_id"),
        std::string(to_string(PAYME_TEST_MERCHANT_ID))));
    log_info("Sort:");
    for (auto a: form_map)
    {
#ifdef _CONSOLE_DEBUG
        fprintf(stdout,"|%s", a.first.c_str());
#endif
        syslog(LOG_INFO,"|%s", a.first.c_str());
        body_sign.append("|");
        body_sign.append(a.second);
    }
    log_info("\n");

    //sha1sum
    unsigned char sha_out_buf[20];

    log_info("body_sign:[%s]", body_sign.c_str());
    SHA1((const unsigned char*) body_sign.c_str(),
         body_sign.length(),
         sha_out_buf);
    string s_buf = "";
    for (int a = 0; a< 20;a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", sha_out_buf[a]);
        s_buf.append(tmp_buf);
    }
    form_map.emplace(std::make_pair(
        std::string("signature"),
        s_buf));
    string form_str = "<form method=\"POST\" action=\"";
    form_str.append(PAYME_TEST_WD_URL);
    form_str.append("\">\n");

    for (auto a: form_map)
    {
        form_str.append("<input type=\"hidden\" name=\"");
        form_str.append(a.first);
        form_str.append("\" value=\"");
        form_str.append(a.second);
        form_str.append("\" />\n");
    }
    form_str.append("<input class=\"account-form__button active-tab\"  type=\"submit\" value=\"Pay out <--\">");
    form_str.append("</form>\n");

    CryptoPP::Base32Encoder encoder(NULL,false);
    const byte ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::EncodingLookupArray(),(const byte *)ALPHABET);
    encoder.IsolatedInitialize(params);

    log_err("form: [%d] bytes [%s]",
           form_str.length(), form_str.c_str());
    encoder.Put((byte*) form_str.c_str(),
                form_str.length());
    encoder.MessageEnd();
    unsigned char encoded[4096*10]={0x00};
    unsigned char decoded[4096*10]={0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((byte*)encoded, len);
    log_err("encoded [%d]: %s",len, encoded);
#if 0
  CryptoPP::Base32Decoder decoder;
  decoder.Put((byte*) encoded, len);
  decoder.MessageEnd();
  const long len_2 = decoder.MaxRetrievable();
  decoder.Get((byte*)decoded,len_2);
  fprintf(stderr, "decoded [%d]:\n %s\n",len_2, decoded);
#endif
    out_form.append((const char*)encoded);
    return 0;
}

int
p2p_generate_form_withdrowal_tg(
    const char *player,
    const int amount,
    const char* currency,
    const char *gw,
    const uint64_t invoice,
    string &out_form) {
    string form_str = "<form method=\"POST\" action=\"" TEST_WD_TG_URL "\">\n";

    form_str.reserve(4096);

    map<string,string> form_map;

    form_map.emplace("user_id" , player            );
    form_map.emplace("order_id", to_string(invoice));
    form_map.emplace("currency", currency          );
    form_map.emplace("amount"  , to_string(amount) );

    for (const auto& a: form_map) {
        form_str.append("<input type=\"hidden\" name=\"");
        form_str.append(a.first);
        form_str.append("\" value=\"");
        form_str.append(a.second);
        form_str.append("\" />\n");
    }

    form_str.append("<input class=\"account-form__button active-tab\"  type=\"submit\" value=\"Pay out <--\">");
    form_str.append("</form>\n");

    CryptoPP::Base32Encoder encoder(nullptr, false);
    const byte ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::EncodingLookupArray(),(const byte *)ALPHABET);
    encoder.IsolatedInitialize(params);

    encoder.Put((const unsigned char*) form_str.c_str(), form_str.size());
    encoder.MessageEnd();

    char encoded[encoder.MaxRetrievable() + 1];

    encoder.Get((unsigned char*) encoded, encoder.MaxRetrievable());

    out_form.append((const char*) encoded);

    return 0;
}

int p2p_generate_form(const char *player,const int amount,
                      const char* currency, const char *gw, const uint64_t invoice,
                      string &out_form)
{

    string body_sign;
    body_sign.append(PAYME_TEST_API5KEY);
    map <string,string> form_map;
    string order_desc = "Payin for user ";
    order_desc.append(player);
    order_desc.append (" is ");
    order_desc.append(to_string(amount));
    order_desc.append(currency);

    form_map.emplace(std::make_pair(std::string("order_desc"),order_desc));
    form_map.emplace(std::make_pair(
        std::string("server_url"),
        std::string(PAYME_TEST_CALLBACK_URL)));

    form_map.emplace(std::make_pair(
        std::string("response_url"),
        std::string(PAYME_TEST_RESP_URL)));

    form_map.emplace(std::make_pair(
        std::string("user_id"),
        std::string(player)));

    form_map.emplace(std::make_pair(
        std::string("order_id"),
        std::string(to_string(invoice))));

    form_map.emplace(std::make_pair(
        std::string("currency"),
        std::string(currency)));

    form_map.emplace(std::make_pair(
        std::string("amount"),
        std::string(to_string(amount))));

    form_map.emplace(std::make_pair(
        std::string("merchant_id"),
        std::string(to_string(PAYME_TEST_MERCHANT_ID))));
    log_info("Sort:");
    for (auto a: form_map)
    {
#ifdef _CONSOLE_DEBUG
        fprintf(stdout,"|%s", a.first.c_str());
#endif
        syslog(LOG_INFO,"|%s", a.first.c_str());
        body_sign.append("|");
        body_sign.append(a.second);
    }
    log_info("\n");

    //sha1sum
    unsigned char sha_out_buf[20];

    log_info("body_sign:[%s]", body_sign.c_str());
    SHA1((const unsigned char*) body_sign.c_str(),
         body_sign.length(),
         sha_out_buf);
    string s_buf = "";
    for (int a = 0; a< 20;a++)
    {
        char tmp_buf[10] = {0x00};
        snprintf(tmp_buf, sizeof(tmp_buf), "%.2x", sha_out_buf[a]);
        s_buf.append(tmp_buf);
    }
    form_map.emplace(std::make_pair(
        std::string("signature"),
        s_buf));
    string form_str = "<form method=\"POST\" action=\"";
    form_str.append(PAYME_TEST_URL);
    form_str.append("\">\n");

    for (auto a: form_map)
    {
        form_str.append("<input type=\"hidden\" name=\"");
        form_str.append(a.first);
        form_str.append("\" value=\"");
        form_str.append(a.second);
        form_str.append("\" />\n");
    }
    form_str.append("<input class=\"account-form__button active-tab\"  type=\"submit\" value=\"Pay in <--\">");
    form_str.append("</form>\n");

    CryptoPP::Base32Encoder encoder(NULL,false);
    const byte ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::EncodingLookupArray(),(const byte *)ALPHABET);
    encoder.IsolatedInitialize(params);

    log_err("form: [%d] bytes [%s]",
           form_str.length(), form_str.c_str());
    encoder.Put((byte*) form_str.c_str(),
                form_str.length());
    encoder.MessageEnd();
    unsigned char encoded[4096*10]={0x00};
    unsigned char decoded[4096*10]={0x00};
    const long len = encoder.MaxRetrievable();
    encoder.Get((byte*)encoded, len);
    log_err("encoded [%d]: %s",len, encoded);
#if 0
  CryptoPP::Base32Decoder decoder;
  decoder.Put((byte*) encoded, len);
  decoder.MessageEnd();
  const long len_2 = decoder.MaxRetrievable();
  decoder.Get((byte*)decoded,len_2);
  fprintf(stderr, "decoded [%d]:\n %s\n",len_2, decoded);
#endif
    out_form.append((const char*)encoded);
    return 0;
}

/*! \brief Write p2p withdrowal
*
* Function write p2p withdrowal to list
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param amount  - money in cents
* @param currency
* @return invoice number, else error code
*
*/
int p2p_create_withdrawal(redisContext *r_ctx,const char *player, int amount,
                          const char* currency, const char *gw,const char *session, uint64_t &invoice)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    invoice =  std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    // 1. write to redis p2p:withdrowals list
    // 2. write to invoice:[number]

    log_info("LPUSH p2p:withdrowals %lld",  invoice);
    redisReply* r_reply = NULL;
    string player_id;

    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH p2p:withdrowals %lld",  invoice);
    if (r_reply == NULL)
    {
        log_err("set list withdrowals error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err push withdrowal %lld", invoice);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    // push invoice in user list
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH p2p:user:%s:withdrowals %lld",
                                          player, invoice);
    if (r_reply == NULL)
    {
        log_err("set list user %s withdrowal error: %s",player,  r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err push user %s withdrowal %lld",
               player, invoice);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }


    // set ts to current
    time_t ts = time(NULL);

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET withdrowal:%llu player %s amount %d currency %s ts %d gw %s session %s",
                                          invoice, player, amount, currency, ts, gw, session);
    if (r_reply == NULL)
    {
        log_err("set withdrowal err: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis err set withdrowal %llu %s", invoice, player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Write p2p invoice
*
* Function write p2p invoice to list
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @param amount  - money in cents
* @param currency
* @return invoice number, else error code
*
*/
int p2p_create_invoice(redisContext *r_ctx,const char *player, int amount,
                       const char* currency, const char *gw,const char *session, uint64_t &invoice)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    invoice =  std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    // 1. write to redis p2p:invoices list
    // 2. write to invoice:[number]

    log_info("LPUSH p2p:invoices %lld",  invoice);
    redisReply* r_reply = NULL;
    string player_id;

    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH p2p:invoices %lld",  invoice);
    if (r_reply == NULL)
    {
        log_err("set list invoice error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err push invoice %lld", invoice);

        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    // push invoice in user list
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH p2p:user:%s:invoices %lld",
                                          player, invoice);
    if (r_reply == NULL)
    {
        log_err("set list user %s invoice error: %s",player,  r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err push user %s invoice %lld",
               player, invoice);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }


    // set ts to current
    time_t ts = time(NULL);

    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET invoice:%llu player %s amount %d currency %s ts %d gw %s session %s",
                                          invoice, player, amount, currency, ts, gw, session);
    if (r_reply == NULL)
    {
        log_err("set invoice err: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("redis err set invoice %lld %s", invoice, player);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

/*! \brief Write user to redis list for generate p12
*
*  Function write user to list for future
* pg synchronization and generate p12
* container
* @param r_ctx - Oppened Redis context
* @param user  - player phone
* @return 0 on success else error code
*
*/
int generate_p12(redisContext *r_ctx, const char* user)
{
    log_info("LPUSH player2pg2p12 %s",  user);
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH player2pg2p12 %s",  user);
    if (r_reply == NULL)
    {
        log_err("set session error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err reply %s", user);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

    return 0;
}

int parse_player_cn(char *player, string &s_player)
{
    int len = strnlen(player, MAX_PLAYER_LEN);
    log_info("parse player=%s", player);
    char *start = NULL;
    char *end = NULL;
    for (int a=0;a< len;a++)
    {
        if (player[a]=='=' && start==NULL)
        {
            start = player+a+1;
        }
        if (player[a]==',' &&start !=NULL)
        {
            end = player+a;
            break;
        }
    }
    if (start != NULL && end != NULL && end-start<MAX_PLAYER_LEN)
    {
        char buf[MAX_PLAYER_LEN] = {0x00};
        strncpy(buf, start, end-start);
        s_player = buf;
        return 0;
    }
    return -1;
}

void SignalHandler(int sig)
{
    switch (sig) {
    case SIGTERM:
        syslog(LOG_NOTICE, "Terminate signal catched: %i", sig);
        working = false;
        break;
    case SIGUSR1:
        syslog(LOG_NOTICE, "STOP signal catched: %i", sig);
        working = false;
        break;
    case SIGINT:
        syslog(LOG_NOTICE, "Terminate signal catched: %i", sig);
        working = false;
        break;
    case SIGSTOP:
        syslog(LOG_NOTICE, "Terminate signal catched: %i", sig);
        ora_pause = true;
        break;
    case SIGCONT:
        syslog(LOG_NOTICE, "Terminate signal catched: %i", sig);
        ora_pause = false;
        break;
    default:
        syslog(LOG_NOTICE, "Signal code: %i", sig);
    }
}



int rgs_check_tx_in_round(redisContext *r_ctx, const char *player_id,
			const char* tx_id,const char *round_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPOS rgs:player:%s:round:%s %s",
					player_id, round_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->type == REDIS_REPLY_NIL)
		{
      log_info("rgs null trn %s", tx_id);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
			return 0;
		}
		else
    {
        log_info("rgs found trn %s in list %d",tx_id, r_reply->integer);
         int len = r_reply->integer;
         if (r_reply != NULL)
         {
             freeReplyObject(r_reply);
         }
				 return len+1;
		}
	return 0;
}
int rgs_check_round_id(redisContext *r_ctx, string &player,const char* round_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LLEN rgs:player:%s:round:%s",
					player.c_str(), round_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->type == REDIS_REPLY_NIL)
		{
      log_info("rgs null trn %s", round_id);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
			return -1;
		}
		else
    {
        log_info("rgs found trn %s in list %d",round_id, r_reply->integer);
         int len = r_reply->integer;
				 if (len == 0)
				 {
						return -1;
				 }
         if (r_reply != NULL)
         {
             freeReplyObject(r_reply);
         }
				 return len+1;
		}
	return 0;
}

int rgs_check_txId(redisContext *r_ctx, string &player,const char* tx_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPOS rgs:player:%s:txs %s",
					player.c_str(), tx_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->type == REDIS_REPLY_NIL)
		{
      log_info("rgs null trn %s", tx_id);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
			return 0;
		}
		else
    {
        log_info("rgs found trn %s in list %d",tx_id, r_reply->integer);
         int len = r_reply->integer;
         if (r_reply != NULL)
         {
             freeReplyObject(r_reply);
         }
				 return len+1;
		}
	return 0;
}


int rgs_set_txid_roud_id(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					const char *round_id, int bet_amount)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:txs %s",
					player_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

	// hmset balace 
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET rgs:player:%s:txs:%s amount %d",
                                          player_id, tx_id, bet_amount
                                          );
    if (r_reply == NULL)
    {
        log_err("edit user: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user %s", player_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }


		// check tx_id in round.
		fprintf(stdout, "rgs_check_tx_in_round!!!\n");
		int res = rgs_check_tx_in_round(r_ctx, player_id, tx_id, round_id);
		fprintf(stdout, "res = %d\n",res);
	  if (res == 0)	
		{
			// LPUSH it and hmset
    	r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:round:%s %s",
					player_id, round_id, tx_id);
    	if (r_reply == NULL)
    	{
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    	}
    	if (r_reply->type == REDIS_REPLY_ERROR)
    	{
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    	}
  		if (r_reply != NULL)
    	{
        freeReplyObject(r_reply);
    	}
		}  

	return 0;
}
int rgs_set_txid_win(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					 int bet_win)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:txs %s",
					player_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

	// hmset balace 
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET rgs:player:%s:txs:%s win %d",
                                          player_id, tx_id, bet_win
                                          );
    if (r_reply == NULL)
    {
        log_err("edit user: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no user %s", player_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

	return 0;
}


int rgs_add_roud_id(redisContext *r_ctx, const char* player_id,const char*	tx_id,
					const char *round_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
		// check tx_id in round.
		int res = rgs_check_tx_in_round(r_ctx, player_id, tx_id, round_id);
	  if (res == 0)	
		{
			// LPUSH it and hmset
    	r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:round:%s %s",
					player_id, round_id, tx_id);
    	if (r_reply == NULL)
    	{
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    	}
    	if (r_reply->type == REDIS_REPLY_ERROR)
    	{
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    	}
		}  
  	if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }

	return 0;
}

int rgs_get_win_from_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int &bet_amount)
{
	// hmset balace 
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET rgs:player:%s:txs:%s win",
                                          player_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money in txid %s", tx_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	bet_amount = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		
	return 0;
}
int rgs_set_refunded_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int amount)
{
	// hmset balace 
		int refunded = 0;
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
		fprintf(stdout, "HMSET rgs:player:%s:txs:%s refunded %d\n",
				player_id, tx_id, amount);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMSET rgs:player:%s:txs:%s refunded %d",
                                          player_id, tx_id, amount);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money in txid %s", tx_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
			// ok
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		
	return 0;
}

int rgs_is_refunded_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int &refunded)
{
	// hmset balace 
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
		fprintf(stdout, "refund check \n\nHMGET rgs:player:%s:txs:%s refunded \n\n",
										player_id, tx_id);
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET rgs:player:%s:txs:%s refunded",
                                          player_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money in txid %s", tx_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	refunded = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		
	return 0;
}

int rgs_get_amount_from_txid(redisContext *r_ctx, const char* player_id,
	 const char * tx_id, int &bet_amount)
{
	// hmset balace 
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
		int res = rgs_is_refunded_txid(r_ctx, player_id, tx_id, bet_amount);
		fprintf(stdout, "check refunded %s res is %d amount is %d\n", tx_id, res,
																		bet_amount);
		if (bet_amount > 0)
		{
			bet_amount = 0;
			return 0;
		}
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"HMGET rgs:player:%s:txs:%s amount",
                                          player_id, tx_id);
    if (r_reply == NULL)
    {
        log_err("get balance error: %s", r_ctx->errstr );
        return ERR_REDIS;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no money in txid %s", tx_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {

        char *endptr  = NULL;
        if (r_reply->element[0]->len > 0)
        {
        	bet_amount = strtol(r_reply->element[0]->str, &endptr,10);
        }
				endptr = NULL;	
    }

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
		
	return 0;
}
int rgs_get_amount_from_round(redisContext *r_ctx, const char* player_id,
	 const char * round_id, int &bet_amount)
{
	// hmset balace 
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
		// LRANGE 0 -1
            //fprintf(stdout, "LRANGE games:instant 0 %d\n", len);
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LLEN rgs:player:%s:round:%s",
		 player_id, round_id);
    if (r_reply == NULL)
    {
        log_err("get_banner: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no banners in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else
    {
        log_info("ELEM:%ld", r_reply->integer);
        if (r_reply->integer!=0)
        {
            int len = r_reply->integer;
            if (r_reply != NULL)
            {
                freeReplyObject(r_reply);
            }

  r_reply = (redisReply*) redisCommand(r_ctx,"LRANGE rgs:player:%s:round:%s 0 -1",
		player_id, round_id);
  if (r_reply->elements)
  {
      for (int a = 0; a < len;a++)
      {
          //
          char* tx_id = r_reply->element[a]->str;
					int current_bet_amount =0 ;
					int win_amount =0 ;
					rgs_get_amount_from_txid(r_ctx, player_id, tx_id, current_bet_amount);
					fprintf(stdout, "add amount %d from txid %s\n", current_bet_amount, tx_id);
					bet_amount += current_bet_amount;
					rgs_get_win_from_txid(r_ctx, player_id, tx_id, win_amount);
					if (current_bet_amount > 0)
					{
						bet_amount -= win_amount;
					}
      } //for list
		}
	}
	}

		

    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
    return 0;

}
int rgs_check_rolled_round(redisContext *r_ctx, const char *player_id
			,const char *round_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    r_reply =(redisReply*) redisCommand(r_ctx,"LPOS rgs:player:%s:rolled_round %s",
					player_id, round_id);
    if (r_reply == NULL)
    {
        log_err("rgs check txid: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("no tx in the list");
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    else if (r_reply->type == REDIS_REPLY_NIL)
		{
      log_info("rgs null trn %s", round_id);
      if (r_reply != NULL)
      {
          freeReplyObject(r_reply);
      }
			return 0;
		}
		else
    {
        log_info("rgs found rolled round [%d]%s in list", 
					r_reply->integer, round_id);
         int len = r_reply->integer;
         if (r_reply != NULL)
         {
             freeReplyObject(r_reply);
         }
				 return len+1;
		}
	return 0;
}



int rgs_set_rolled_round(redisContext *r_ctx, const char *player_id
			,const char *round_id)
{
    if (check_redis(&r_ctx))
    {
        return ERR_REDIS;
    }
    redisReply* r_reply = NULL;
    
	r_reply =(redisReply*) redisCommand(r_ctx,"LPUSH rgs:player:%s:rolled_round %s",
			 player_id,round_id);
    if (r_reply == NULL)
    {
        log_err("set round error: %s", r_ctx->errstr );
        return ERR_SESSION;
    }
    if (r_reply->type == REDIS_REPLY_ERROR)
    {
        log_err("err reply %s", round_id);
        if (r_reply != NULL)
        {
            freeReplyObject(r_reply);
        }
        return ERR_REDIS;
    }
    if (r_reply != NULL)
    {
        freeReplyObject(r_reply);
    }
	return 0;	
}



int sendTelegramMessage(const std::string& botToken, const std::string& chatId, const std::string& message) {
    CURL* curl;
    CURLcode res;

    // Initialize curl
    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
        std::string postFields = "chat_id=" + chatId + "&text=" + curl_easy_escape(curl, message.c_str(), message.length());

        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up curl
        curl_easy_cleanup(curl);
			return res;
    }
	return -1;
}
