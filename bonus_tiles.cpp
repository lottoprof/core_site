#include <string.h>
#include <vector>
#include <syslog.h>
#include <hiredis.h>
#include "bonus_tiles.h"
#include "oraora_errors.h"
#include "oraora_limits.h"
#include "redis_helper.h"
int get_bonus_symbols(string s_session,int &symbol1,int &symbol2)
{
 // HM user_id:game_id:symbols
 // ts symbol1 symbol2
	int ts = 0; 
	if (ts > 6*3600 
			&& symbol1 > 0 
			&& symbol2 > 0)
	{
		set_bonus_symbols(s_session, symbol1,symbol2);
	}
	return 0;
}
int set_bonus_symbols(string s_session,int &symbol1,int &symbol2)
{
 // HM user_id:game_id:symbols
 // ts symbol1 symbol2 

	return 0;
}
int start_stars_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,29);
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9,0);
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,10,0);

	return 0;
}

int start_suns_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 5 - is count of stars for game 211301
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,35);

	return 0;
}
int start_fishes_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 5 - is count of stars for game 211301
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,5);
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9,0);

	return 0;
}
int update_fishes_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	int a = 0;
		get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	if (a)
	{
		a--;
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	}
	
	return 0;
}
int update_suns_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	int a = 0;
		get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	if (a)
	{
		a--;
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	}
	
	return 0;
}
int update_stars_for_sale(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	int a = 0;
		get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	if (a)
	{
		a--;
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,a);
	}
	
	return 0;
}
int reset_suns(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		time_t current_time = time(NULL);
		fprintf(stdout, "delete redis key:\n del user:sz:%d:game_id:%d:price:%d\n",
				sz_user_id, game_id, price);
		r_reply =(redisReply*) redisCommand(r_ctx,"del user:sz:%d:game_id:%d:price:%d",
				sz_user_id, game_id, price);
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	return 0;
}

int reset_fishes(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		time_t current_time = time(NULL);
		fprintf(stdout, "delete redis key:\n del user:sz:%d:game_id:%d:price:%d\n",
				sz_user_id, game_id, price);
		r_reply =(redisReply*) redisCommand(r_ctx,"del user:sz:%d:game_id:%d:price:%d",
				sz_user_id, game_id, price);
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	return 0;
}


int reset_stars(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		time_t current_time = time(NULL);
		fprintf(stdout, "delete redis key:\n del user:sz:%d:game_id:%d:price:%d\n",
				sz_user_id, game_id, price);
		r_reply =(redisReply*) redisCommand(r_ctx,"del user:sz:%d:game_id:%d:price:%d",
				sz_user_id, game_id, price);
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
	// 66 - it is virtual tile!!!! MEMO
	// 30 - is count of stars for game 211401
	return 0;
}

int reset_letters(redisContext *r_ctx,
	string &session, int price)
{
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		// TODO syslog event
		return res; // no user. must return error. 
								// save state to redis
	}

		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		time_t current_time = time(NULL);
		fprintf(stdout, "delete redis key:\n del user:sz:%d:game_id:%d:price:%d\n",
				sz_user_id, game_id, price);
		r_reply =(redisReply*) redisCommand(r_ctx,"del user:sz:%d:game_id:%d:price:%d",
				sz_user_id, game_id, price);
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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

int check_fishes_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 66 /*tile_number*/,
														 bonus_counter /*tile_value*/);	
	return 0;
}
int check_suns_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 66 /*tile_number*/,
														 bonus_counter /*tile_value*/);	
	return 0;
}

int check_stars_bonus(redisContext *r_ctx,
	string &session, int price, int &bonus_counter)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 66 /*tile_number*/,
														 bonus_counter /*tile_value*/);	
	return 0;
}


int check_fishes(redisContext *r_ctx,
	string &session, int price, int &bonus_amount)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	int fishes_count = 0;
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 9 /*tile_number*/,
														 fishes_count /*tile_value*/);	
	if (fishes_count >= 5)
	{
		bonus_amount = 1;
	}	
	fprintf(stdout, "win_counter: fishes %d; \n", 
		fishes_count);
	return 0;
}
int check_stars(redisContext *r_ctx,
	string &session, int price, int &bonus_amount)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	int stars_count = 0;
	int cherry_count = 0;
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 9 /*tile_number*/,
														 stars_count /*tile_value*/);	
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 10 /*tile_number*/,
														 cherry_count /*tile_value*/);	
	if (stars_count >= 19 && cherry_count >= 16)
	{
		bonus_amount = 1;
	}	
	fprintf(stdout, "win_counter: cherry %d; stars %d\n", 
		cherry_count, stars_count);
	return 0;
}

int check_letters(redisContext *r_ctx,
	string &session, int price, int &bonus_amount)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	vector<int> letters;
	letters.resize(10);
	for (auto &a: letters)
	{
		a = 0;
	}
	
	int letter_it = 0;
	int letter_win_counter = 0;
	int a = 11;
	for (auto &l:letters)
	{
		letter_it++;
	 		int res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 a /*tile_number*/,
														 l /*tile_value*/);	

		if (letter_it==3 && l>=2)
		{
			letter_win_counter+=2;
			fprintf(stdout, "win_counter: %d letter_it=%d l=%d\n",
			 letter_win_counter, letter_it, l);
		}
		if( letter_it!=3 && l > 0)
		{
			letter_win_counter++;
			fprintf(stdout, "win_counter: %d letter_it=%d l=%d\n",
			 letter_win_counter, letter_it, l);
		}
		a++;
	} // check all letters. is it win or not
	fprintf(stdout, "win_counter: %d\n", letter_win_counter);
	if (letter_win_counter >= SATTYJULDYZ_WIN_LETTERS_COUNT)
	{
		// so, it is win. absoluteley
		bonus_amount = price * LETTERS_WIN_MULT;	
		fprintf(stdout, "bonus amount: %d\n", bonus_amount);
	}
	else
	{
		
	}
	return 0;
}
int set_and_check_letters(redisContext *r_ctx,
	string &session, vector<_bet> &bets, int &bonus_amount, string &_letters)
{
	// get letters
	// 
	int sz_user_id = 0;	// load from session
	int game_id = 0; // load from session
	int price = bets[0].price;
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	vector<int> letters;
	letters.resize(10);
	for (auto &a: letters)
	{
		a = 0;
	}
	for (const auto &b:bets)
	{		
		for(const auto a: b.mask) 
		{
			if (a <= 10)
			{
				continue;
    	} // if tile 
			// init letters
			// need to get letters from current 
			// user (of course we are chech the ts)
			int tile_count = 0;
	 		res = get_bonus_tile_count(r_ctx, sz_user_id,game_id, price,
														 a /*tile_number*/,
														 tile_count /*tile_value*/);	
			if (a > 10 && a <= 20)
			{
				if (a == 13 && letters[2]<2)
					letters[2]++;
				else if (letters[a-11]<1)
					letters[a-11]++;
			}
	 		if (res == ERR_TILE_TIMEOUT)
	 		{
				// set new value for tile
				
	 		} // if timeout		
			if (res!=0)
			{
				// set new value for tile
				set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,a, 	
							letters[a-11] );
			} // 
			else
			{
				set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,a,letters[a-11]+ tile_count);
			}
		} // for mask	
	} // for bets		
	int letter_it = 0;
	int letter_win_counter = 0;
	for (const auto &l:letters)
	{
		letter_it++;
		if (letter_it==3 && l==2)
		{
			letter_win_counter++;
		}
		if( letter_it!=3 && l == 1)
		{
			letter_win_counter++;
		}
	} // check all letters. is it win or not

	if (letter_win_counter == SATTYJULDYZ_WIN_LETTERS_COUNT)
	{
		// so, it is win. absoluteley
		bonus_amount = bets[0].price * LETTERS_WIN_MULT;	
	}
	else
	{
		
	}
	
	return 0;
}



int set_letters_tile(redisContext *r_ctx,
		int sz_user_id, int game_id, 
		int tile_number, int &letter_value)
{
		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		time_t current_time = time(NULL);
		r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:sz:%d:game_id:%d ts %d tile_%d %d",
				sz_user_id, game_id,current_time, tile_number, letter_value);	
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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


int get_letters_tile(redisContext *r_ctx,
		int sz_user_id, int game_id, 
		int tile_number, int &letter_value)
{
		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
		r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:sz:%d:game_id:%d ts tile_%d",
				sz_user_id, game_id, tile_number);	
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
			
			// check element_count in reply
			if (r_reply->element[0]->len>0)
			{
				time_t current_time = time(NULL);
				char *endptr = NULL;
				long long t= strtoll(r_reply->element[0]->str, &endptr,10);
				current_time -= t;
#ifdef _CONSOLE_DEBUG
				fprintf(stdout, "current time diff=%d\n", current_time);
#endif
				syslog(LOG_INFO, "current time diff=%d", current_time);
				if (current_time >= BONUS_TILE_TIMEOUT)
				{
					// ERASE all tiles
					// TODO
					if (r_reply != NULL)
					{
						freeReplyObject(r_reply);
					}
					return ERR_TILE_TIMEOUT;
				}// if bonus tiles is too old
				else
				{
					// time is ok, need check token
					if (r_reply->element[1]->len>0 )
					{

							char *endptr  = NULL;
							letter_value = strtol(r_reply->element[1]->str, &endptr,10); 
						
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
					
					return -1;
				}// else token time is ok 
			} // if element len
			else
			{
				// no token. return err
#ifdef _CONSOLE_DEBUG
				fprintf(stdout, "not found registration %d\n", sz_user_id);
#endif
				syslog(LOG_INFO, "not found registration %d", sz_user_id);
				if (r_reply != NULL)
				{
					freeReplyObject(r_reply);
				}
				return -1;
			}// if element->len

		} // if no REDIS_REPLY_ERROR

		if (r_reply != NULL)
		{
			freeReplyObject(r_reply);
		}

	return 0;
}
int set_and_check_fishes(redisContext *r_ctx,
	string &session, vector<_bet> &bets,
	int &bonus_amount,
  int &bonus_counter_1)
{
	// get letters
	// 
	bonus_counter_1 = 0;
	int sz_user_id = 0;	// load from session
	int game_id = 0; 		// load from session
	int fishes = 0;
	int cherry = 0;
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	for (const auto &b:bets)
	{		
		for(const auto &a: b.mask) 
		{
			if (a == 9)
			{
				bonus_counter_1++;
			}
		} // for mask	
	} // for bets		
	// need to get letters from current 
	// user (of course we are chech the ts)
	int price = bets[0].price;
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9,fishes);
	//if (res == ERR_TILE_TIMEOUT)
	if (res!=0)
	{
		// set new value for tile
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9, bonus_counter_1);
	} // if timeout		
	else
	{
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9, bonus_counter_1 + fishes);
	}


	return 0;
}
int set_and_check_suns(redisContext *r_ctx,
	string &session, vector<_bet> &bets,
	int &bonus_amount,
  int &bonus_counter_1) 
{
	// get letters
	// 
	bonus_counter_1 = 0;
	int sz_user_id = 0;	// load from session
	int game_id = 0; 		// load from session
	int suns = 0;
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	for (const auto &b:bets)
	{		
		for(const auto &a: b.mask) 
		{
			if (a == 15)
			{
				bonus_counter_1++;
			}
		} // for mask	
	} // for bets		
	// need to get letters from current 
	// user (of course we are chech the ts)
	int price = bets[0].price;
	suns = bonus_counter_1;
	if (suns >= 6)
	{
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,66,40 );
	}
	else 
	{
		fprintf(stdout, "\nno bonus suns! suns=%d\n",suns);
	}
	return 0;
}
int set_and_check_stars(redisContext *r_ctx,
	string &session, vector<_bet> &bets,
	int &bonus_amount,
  int &bonus_counter_1, 
  int &bonus_counter_2) 
{
	// get letters
	// 
	bonus_counter_2 = 0;
	bonus_counter_1 = 0;
	int sz_user_id = 0;	// load from session
	int game_id = 0; 		// load from session
	int stars = 0;
	int cherry = 0;
	
	int res = get_user_game_id_session(r_ctx, session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	for (const auto &b:bets)
	{		
		for(const auto &a: b.mask) 
		{
			if (a == 10)
			{
				bonus_counter_2++;
			}
			if (a == 9)
			{
				bonus_counter_1++;
			}
		} // for mask	
	} // for bets		
	// need to get letters from current 
	// user (of course we are chech the ts)
	int price = bets[0].price;
	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9,stars);
	//if (res == ERR_TILE_TIMEOUT)
	if (res!=0)
	{
		// set new value for tile
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9, bonus_counter_1);
	} // if timeout		
	else
	{
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,9, bonus_counter_1 + stars);
	}

	res = get_bonus_tile_count(r_ctx, sz_user_id,game_id,price,10,cherry);
	//if (res == ERR_TILE_TIMEOUT)
	if (res!=0)
	{
		// set new value for tile
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,10, bonus_counter_2);
	} // if timeout		
	else
	{
		set_bonus_tile_count(r_ctx, sz_user_id,game_id,price,10, bonus_counter_2 + cherry);
	}

	return 0;
}
int get_all_lemon_letters_count(redisContext *r_ctx,string &s_session, string &out_json)
{
	int sz_user_id = 0;
	 
	int game_id = 0;
	int res = get_user_game_id_session(r_ctx, s_session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	vector<int> prices = {20,40,100,200,500,1000};

	int iterator=0;
	out_json.append("\"bonuses\":[");
	for (auto price: prices)
	{
		if (iterator)
			out_json.append(",");
		out_json.append("{\"price\":");
		out_json.append(to_string(price));
		out_json.append(",");
		out_json.append("\"letters\":[");
		for (int tile = 11; tile <=20; tile++)
		{
			int tile_counter = 0;	

			if (tile > 11)
				out_json.append(",");
			int res = get_bonus_tile_count(r_ctx, sz_user_id, game_id, 
															price, tile, tile_counter);
			out_json.append(to_string(tile_counter));
		}
		out_json.append("]}");
		
		iterator++;
	}
	out_json.append("]");
	return 0;
}
int get_all_chuckcha_tile_count(redisContext *r_ctx,string &s_session, string &out_json)
{
	int sz_user_id = 0;
	 
	int game_id = 0;
	int res = get_user_game_id_session(r_ctx, s_session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	vector<int> prices = {4,10,20,50,100,200,300,500,1000};

	int iterator=0;
	out_json.append("\"bonuses\":[");
	for (auto price: prices)
	{
		if (iterator)
			out_json.append(",");
		out_json.append("{\"price\":");
		out_json.append(to_string(price));
		out_json.append(",");
		out_json.append("\"bonus_counter_1\":");
		int tile_counter = 0;	
		int tile = 9; //fish 
		int res = get_bonus_tile_count(r_ctx, sz_user_id, game_id, 
															price, tile, tile_counter);
		out_json.append(to_string(tile_counter));
		out_json.append("}");
		iterator++;
	}
	out_json.append("]");
	return 0;
}

int get_all_fruit_n_ice_tile_count(redisContext *r_ctx,string &s_session, string &out_json)
{
	int sz_user_id = 0;
	 
	int game_id = 0;
	int res = get_user_game_id_session(r_ctx, s_session.c_str(),
					sz_user_id, game_id);
	if (res != 0)
	{
		return res; // no user. must return error. 
								// save state to redis
	}
	vector<int> prices = {10,20,30,50,100,200,300,500,1000};

	int iterator=0;
	out_json.append("\"bonuses\":[");
	for (auto price: prices)
	{
		if (iterator)
			out_json.append(",");
		out_json.append("{\"price\":");
		out_json.append(to_string(price));
		out_json.append(",");
		out_json.append("\"bonus_counter_2\":");
		int tile_counter = 0;	
		int tile = 10; //cherry
		int res = get_bonus_tile_count(r_ctx, sz_user_id, game_id, 
															price, tile, tile_counter);
		out_json.append(to_string(tile_counter));
		out_json.append(",");
		tile = 9; //star
		res = get_bonus_tile_count(r_ctx, sz_user_id, game_id, 
															price, tile, tile_counter);	
		out_json.append("\"bonus_counter_1\":");
		out_json.append(to_string(tile_counter));
		out_json.append("}");
		iterator++;
	}
	out_json.append("]");
	return 0;
}

int get_bonus_tile_count(redisContext *r_ctx,int sz_user_id,
												int game_id,int price,int tile, int &count)
{
		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		//fprintf(stdout, "HMGET user:sz:%d:game_id:%d:price:%d ts tile_%d\n",
		//		sz_user_id, game_id, price,tile);
		redisReply* r_reply = NULL;
		r_reply =(redisReply*) redisCommand(r_ctx,"HMGET user:sz:%d:game_id:%d:price:%d ts tile_%d",
				sz_user_id, game_id, price,tile);	
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get cherry  error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get cherry error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
			if (r_reply->elements<2)
			{
					if (r_reply != NULL)
					{
						freeReplyObject(r_reply);
					}
				return 0;
			}	
			// check element_count in reply
			if (r_reply->element[0]->len>0)
			{
				time_t current_time = time(NULL);
				char *endptr = NULL;
				long long t= strtoll(r_reply->element[0]->str, &endptr,10);
				current_time -= t;
#ifdef _CONSOLE_DEBUG
				fprintf(stdout, "current time diff=%d\n", current_time);
#endif
				syslog(LOG_INFO, "current time diff=%d", current_time);
				if (current_time >= BONUS_TILE_TIMEOUT)
				{
					// ERASE all tiles
					// TODO
					if (r_reply != NULL)
					{
						freeReplyObject(r_reply);
					}
					return ERR_TILE_TIMEOUT;
				}// if bonus tiles is too old
				else
				{
					// time is ok, need check token
					if (r_reply->element[1]->len>0 )
					{

							char *endptr  = NULL;
							count = strtol(r_reply->element[1]->str, &endptr,10); 
						
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
					
					return -1;
				}// else token time is ok 
			} // if element len
			else
			{
				// no token. return err
#ifdef _CONSOLE_DEBUG
				fprintf(stdout, "not found registration %d\n", sz_user_id);
#endif
				syslog(LOG_INFO, "not found registration %d", sz_user_id);
				if (r_reply != NULL)
				{
					freeReplyObject(r_reply);
				}
				return -1;
			}// if element->len

		} // if no REDIS_REPLY_ERROR

		if (r_reply != NULL)
		{
			freeReplyObject(r_reply);
		}

	return 0;
}
int set_bonus_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int price, int tile, int count)
{
		if (check_redis(&r_ctx))
		{
			return ERR_REDIS;
		}
		redisReply* r_reply = NULL;
				time_t current_time = time(NULL);
		fprintf(stdout,"HMSET user:sz:%d:game_id:%d:price:%d ts %d tile_%d %d\n",
				sz_user_id, game_id,price,current_time, tile, count);	
		r_reply =(redisReply*) redisCommand(r_ctx,"HMSET user:sz:%d:game_id:%d:price:%d ts %d tile_%d %d",
				sz_user_id, game_id,price,current_time, tile, count);	
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "get bonus letters error: %s\n", r_ctx->errstr );
#endif
			syslog(LOG_ERR, "get bonus letters error: %s", r_ctx->errstr );
			return ERR_REDIS;
		}	
		if (r_reply->type == REDIS_REPLY_ERROR)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "no sz user \n");
#endif
			syslog(LOG_INFO, "no sz user ");
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
int get_stars_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int &stars)
{
	return 0;
}
int set_stars_tile_count(redisContext *r_ctx,int sz_user_id,int game_id,int stars)
{
	return 0;
}
