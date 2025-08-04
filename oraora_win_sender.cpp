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

#include <json/json.h>
#include <json/reader.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <cryptopp/base32.h>
#include <cryptopp/algparam.h>
#include <curl/curl.h>
#include "oraora_errors.h"
using namespace std;
#define DEF_CURLOPT_TIMEOUT 20
#define MERCHANT_ID "a7cd24bb6123cb44173c0a178511274b12e34fc06e9b8ce5769dbc197098183a"
#define MERCHANT_KEY "dda0e7f5a0a29583e4e1ff7e03cc8459694146e4f2b4ceaa9a17bd3f4f8bc436"
#define MERCHANT_KEY_LEN sizeof(MERCHANT_KEY)
#define MAX_MERCHANT_KEY_LEN 255
#define REQUEST_URL  "https://rpo.logycom.kz/threemen/threemen.dll/interface2"
#define _CONSOLE_DEBUG 1

typedef std::vector<int> Mask;

/*
	class for loading tickets into memory
*/
using namespace std;
struct _bet
{
	uint64_t	number = 0;
	int				game_id = 0;
	int				date = 0;
	// When we call get_bet_keno() negative win value mean
	// that we do not specify it explicitly.
	// If we set it (for example) to zero it means that we need zero-bet.
	int				win = 0;
	int				price = 0;
	int				draw = 0;
	Mask			mask = {};
	int			count_digits = 0;
	int			win_category = 0;
	int 		is_played = 0;
	int 		is_sold = 0;
  ~_bet()
  {
    mask.clear();
  }
};
struct _win_generator:public vector<int>
{
 _win_generator()
	{
	for(int a = 1; a <= 80;a++)
		push_back(a);
	} 
};



int do_request_bets_win(string &session, string &player_id, vector<_bet> &b, int &balance, string &error_message);
int do_request (string &request, string &out);
size_t write_data(void *ptr, size_t size, size_t nmemb, void *data)
{
    (static_cast<string*>(data))->append(static_cast<char *>(ptr), size * nmemb);
		//fprintf(stderr, "out curl:%s\n", ptr);
    return size * nmemb;
}

int do_request_bets_win(string &session, string &player_id, vector<_bet> &b, int &balance, string &error_message)
{
// b.is_sold = 1; //
	if (!b.size())
	{
		return -1;
	}
	int game_id = b[0].game_id;
	if (game_id == 211101)
	{
		game_id = 2111;
	}
	if (game_id == 211301)
	{
		game_id = 2113;
	}
	string out;
	string request= "{\"action\":\"win\"";
			//		request.append(",\"session_id\":\"");
				//	request.append(session);
			//		request.append("\"");
					request.append(",\"session_id\":\"null\"");
					request.append(",\"player_id\":");
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
#ifdef _CONSOLE_DEBUG
	fprintf(stdout, "req: %s\n", request.c_str());
#endif
	syslog(LOG_INFO, "req: %s", request.c_str());
	//return 0;
	int res  =	do_request(request, out);
	if (res != 0)
	{
		int counter = 0;
		while (res !=0 && counter<9)
		{
			res  =	do_request(request, out);
			if (res !=0 )
			{
			//	usleep(1000);
			}
			counter++;
		}
	}
#ifdef _CONSOLE_DEBUG
	fprintf(stdout, "get: %s\n", out.c_str());	
#endif
	syslog(LOG_INFO, "get: %s", out.c_str());	
	if (res == 0)
	{
				Json::Value root;
    		Json::Reader reader;
				int res_parsing = reader.parse(out.c_str(),root);
				if (!res_parsing)
				{
#ifdef _CONSOLE_DEBUG
					fprintf(stderr, "unable to parse request %s\n", out.c_str());
#endif
					syslog(LOG_ERR, "unable to parse request %s", out.c_str());
					vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
					for (auto a: vec_err)
					{
#ifdef _CONSOLE_DEBUG
						fprintf(stderr, "%s\n",a.message);
#endif
						syslog(LOG_ERR, "%s",a.message);
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
#ifdef _CONSOLE_DEBUG
						fprintf(stdout, "WIN req | err from SZ: %s\n", root["error_message"].asCString());
#endif
						syslog(LOG_INFO, "WIN req | err from SZ: %s", root["error_message"].asCString());
						error_message = root["error_message"].asCString();
						return -1;
					}
					
					Json::Value j_balance = root["balance"];
					if (j_balance == Json::Value::null)
					{
					//	return -1;
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
#ifdef _CONSOLE_DEBUG
						fprintf(stdout, "err: trnid\n");
#endif
						syslog(LOG_INFO, "err: trnid");
						
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
#ifdef _CONSOLE_DEBUG
								fprintf(stdout, "trnid = %s %d\n", trn_id, balance);
#endif
								syslog(LOG_INFO, "trnid = %s %d", trn_id, balance);
							}
						}
					} // else transactions is null

		// parse json	
					return 0;
	}	
		closelog();
	    return -1;	

}

int do_request (string &request, string &out)
{
		// time_start
		//  
	uint64_t start_ts =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		char *request_url = secure_getenv("LOGYCOM_REQUEST_URL");
		if (request_url == NULL)
		{
			request_url = "https://play.szhuldyz.kz/interface2";
		}
		char * merchant_key = secure_getenv("LOGYCOM_MERCHANT_KEY");
		if (merchant_key == NULL)
		{
			merchant_key = "0c098525e3fafb90155433606487d95510157739232509ca500ac253e497f6cb";
		}
		int merchant_key_len = strnlen(merchant_key,MAX_MERCHANT_KEY_LEN);	
		char * merchant_id = secure_getenv("LOGYCOM_MERCHANT_ID");
		if (merchant_id == NULL)
		{
			merchant_id = "6b263d369fea928183e709bbc00e4f908357cf14d04aca427b27c0947d04419c";
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
//		ENGINE_load_builtin_engines();
//		ENGINE_register_all_complete();
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
#ifdef _CONSOLE_DEBUG
				fprintf(stderr, "code: %d resp_code %d\n", code, response_code);
				fprintf(stderr, "CURL REQ [%s]\n", request.c_str());
#endif
				syslog(LOG_ERR, "code: %d resp_code %d", code, response_code);
				syslog(LOG_ERR, "CURL REQ [%s]", request.c_str());
				return -1;
    }
	uint64_t end_ts =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    return 0;
}

int main(int argc, char **argv) 
{ 
	if (argc<4)
	{
		fprintf (stderr, "use: \n oraora_win_sender [game_id] [number] [win] [player_id]\n");
		return -1;
	}
	if (strlen(argv[1]) < 4)
	{
		fprintf(stderr, "game_id must be 4 digits or more \n");
		return -1;
	}
	vector<_bet> bv;
	_bet b;
	int game_id = 0;
	uint64_t number = 0UL;
	int win = 0;
	string session;
	string player_id;
	string error_message;
	int balance = 0;



	char *endptr = NULL;
	player_id = argv[4];
	game_id = strtoll(argv[1], &endptr,10);
	number = strtoull(argv[2], &endptr,10);
	win = strtoll(argv[3], &endptr,10);

	b.game_id = game_id;
	b.win = win;
	b.number = number;
	b.price = 0;
	bv.push_back(b);
//	session = argv[5];
	session="";
	fprintf(stdout, "game_id %d number %llu win %d player %s sess: %s\n",
	b.game_id, b.number, b.win, player_id.c_str(), session.c_str()
);
	int res = do_request_bets_win(session, player_id, bv, balance, error_message);
			
  return res; 
} 
