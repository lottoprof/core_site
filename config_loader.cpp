#include <stdio.h> 
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>		
#include <time.h>
#include <algorithm>


#include <json/json.h>
#include <json/reader.h>

#include "orakel.h"
#include "config_loader.h"


/*! \brief constructor of config loader
*
*	Function load configs from BF_PATH
* and set tickets database
*
* @return 
*
*/

GamesConfigLoader::GamesConfigLoader()
{
	load_rng_config(2110);
	load_rng_config(2111);
	load_rng_config(2112);
	load_rng_config(2113);
	load_rng_config(2114);
	load_rng_config(2126);
	load_rng_config(2128);
	load_rng_config(2129);
	load_rng_config(2130);
	load_rng_config(2131);
	load_rng_config(2132);
	load_rng_config(2133);
	load_rng_config(2134);
	load_rng_config(2135);
	load_rng_config(2136);
	load_rng_config(2137);
	load_rng_config(2138);
	load_rng_config(2139);
	load_rng_config(2140);
	load_rng_config(2141);
	load_rng_config(2142);
	load_rng_config(2143);
	load_rng_config(2144);
	load_rng_config(2145);
	load_rng_config(2146);
	load_rng_config(2147);
	load_rng_config(2148);
	load_rng_config(2149);
	load_rng_config(2600);
	
	return;
}
/*! \brief Function load json configs  for rng
*
*	Function load json configs
* from files and parse it
*
* @return 0 if ok
*
*/
int GamesConfigLoader::load_rng_config(int game_id)
{
	auto game_cfg = games.find(game_id);
	if (game_cfg == end(games))
	{
		games.emplace(game_id, ConfigJson() )	;
	}
	auto &b = games.at(game_id);
	string filename;
	filename.append("CDN/slot_");
	filename.append(to_string(game_id));	
	filename.append(".json");
	b.config_rng.emplace(0,Config::from_file(filename, 0.8f));
	b.config_rng.emplace(1,Config::from_file(filename,0.92f));
	b.config_rng.emplace(2,Config::from_file(filename, 0.99f));
		
	return 0;
}

/*! \brief Function load json configs 
*
*	Function load json configs
* from files and parse it
*
* @return 0 if ok
*
*/
int GamesConfigLoader::load(int game_id, int price)
{
	char *path = NULL;
	char *configs_path =secure_getenv("BF_PATH");
	if (configs_path == NULL)
	{
		fprintf(stderr, "unable read ENV BF_PATH\n");
		return -1;
	}
	string config_path = configs_path;
	char buf_path[MAX_FILE_PATH_LEN] = {0x00};
	int res = snprintf(buf_path,MAX_FILE_PATH_LEN, "%s/CDN/%d_win_%d.json",
	configs_path, game_id, price);
	if (res < 0)
	{
		fprintf(stderr, "snprintf err %s/%d_win_%d.json %d %s\n",
		configs_path, game_id, price, errno, strerror(errno));
		return -1;
	}
	path = buf_path;
	FILE *f = fopen(path, "r");
	if (!f)
	{
		fprintf(stderr, "unable load %s [%d] %s\n",
			path,errno,
			strerror(errno)
		);
		return -1;
	}

	/*********************
		finish. try to close
		 file and exit
	**********************/
	int res_seek = fseek(f, 0L, SEEK_END);
	if (res_seek != 0)
	{
			fprintf(stderr, "unable fseek %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		int res_close = fclose(f);
		if (res_close != 0)
		{
			fprintf(stderr, "unable fclose %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		}
		return -1;
	}
	long file_len = ftell(f);
	if (file_len < 0)
	{
			fprintf(stderr, "unable ftell %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		int res_close = fclose(f);
		if (res_close != 0)
		{
			fprintf(stderr, "unable fclose %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		}
		return -1;
	}
	res_seek = 	fseek(f, 0L, SEEK_SET);
	if (res_seek != 0)
	{
			fprintf(stderr, "unable fseek %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		int res_close = fclose(f);
		if (res_close != 0)
		{
			fprintf(stderr, "unable fclose %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		}
		return -1;
	}

	char *buf = (char*) malloc(file_len);
	if (!buf)
	{
		fprintf(stderr, "unable malloc %d bytes\n");
		int res_close = fclose(f);
		if (res_close != 0)
		{
			fprintf(stderr, "unable fclose %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		}
		
		return -1;
	}
	int result_read = fread (buf, file_len, 1,f);
	if (result_read!=1)
	{
			fprintf(stderr, "unable fread %s bytes %d [%d] %s\n",
				path, file_len, errno,
				strerror(errno)
			);
		int res_close = fclose(f);
		if (res_close != 0)
		{
			fprintf(stderr, "unable fclose %s [%d] %s\n",
				path,errno,
				strerror(errno)
			);
		}
		return -1;
		
	}
	int res_close = fclose(f);
	if (res_close != 0)
	{
		fprintf(stderr, "unable fclose %s [%d] %s\n",
			path,errno,
			strerror(errno)
		);
	}

	Json::Value root;
  Json::Reader reader;
	int res_parsing = reader.parse(buf,root);
	int bet_count = 0;
	if (!res_parsing)
	{
		fprintf(stderr, "unable to parse request %s\n", buf);
		vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
		for (auto a: vec_err)
		{
			fprintf(stderr, "%s\n",a.message);
		}
	}
	
	auto game_cfg = games.find(game_id);
	if (game_cfg == end(games))
	{
		games.emplace(game_id, ConfigJson() )	;
	}
	auto &b = games.at(game_id);
	b.config.emplace(price, root);
	
	free(buf);
	return 0;
}



/*! \brief method return win and bet
*
*	Function return win and bet for game
*
* @param game_id - player game code
* @param price  - bet price
* @param category  - bet price
* @param bet_str  - generated bet 
* @return >0 on success else 0
*
*/
int GamesConfigLoader::get_bet_and_win(_bet &b, string &bet_str)
{
	int win = 0;
	const int bet_win = b.win;
	const int game_id = b.game_id;
	const int price = b.price;
	const int category = b.win_category;
	const int subcat = b.count_digits;
	fprintf(stdout, "game_id:%d price: %d cat:%d sub:%d\n",
	game_id, price, category, subcat);
	bet_str.append("{\"number\":\"");
	bet_str.append(to_string(b.number));
	bet_str.append("\", \"price\":");
	bet_str.append(to_string(b.price));
	bet_str.append(", \"win\":");
	bet_str.append(to_string(b.win));
	bet_str.append(", \"game_id\":");
	bet_str.append(to_string(b.game_id));
	bet_str.append(", \"mask\":");
	
	auto val = get_config(game_id, price);
	if (val == Json::Value::null)
	{
		fprintf(stderr, "Couldnt find config %d price %d \n",
		game_id, price);
		return 0;// no win
	}
	if(val["tile_config"] == Json::Value::null)	
	{
		fprintf(stderr, "no tile config for game %d price %d\n",
		game_id, price);
		return 0;
	}
	
	/*************
	* check game_id
	* and price
	**************/

	if(val["game_id"] == Json::Value::null)	
	{
		fprintf(stderr, "no tile config for game %d price %d\n",
		game_id, price);
		return 0;
	}
	if (val["game_id"].asInt()!= game_id)
	{
		fprintf(stderr, "err game %d. in config is \n",
		game_id, val["game_id"].asInt());
		return 0;
	}
	if (val["price"].asInt()!= price)
	{
		fprintf(stderr, "err price %d in game_id. in config is %d need %d\n",
		game_id, val["price"].asInt(), price);
		return 0;
	}


	/*************
	* choose win
	**************/
	
	for (auto a: val["tile_config"])
	{
		if (!a.isObject())
		{
			fprintf(stderr, "some err with object for game %d price %d\n",	game_id, price);
			return 0;
		}
		// check elements
			
	 if ((a["cat"] == Json::Value::null)	 ||
				a["subcat"] == Json::Value::null ||
				a["mult"] == Json::Value::null	)	
		{
			fprintf(stderr, "some err with cat subcat for game %d price %d\n",game_id, price);
			return 0;

		}
		
		if (a["cat"].asInt() == category &&
			  a["subcat"].asInt() == subcat)
		{
			win = price * a["mult"].asInt();
			int res_gen = 0;
			switch (game_id)
			{
				case MEGACASINO_GAME_ID:
					res_gen = gen_bet_mega_casino(a, bet_str);
					break;
				case ALADDIN_GAME_ID:
					res_gen = gen_bet_aladdin(a, bet_str);
					break;
				case CLEOPATRA_GAME_ID:
					res_gen = gen_bet_cleopatra(a, bet_str);
					break;
				case THREEDIAMONDS_GAME_ID:
					res_gen = gen_bet_tripler(a, bet_str);
					break;
				case ALTYNKUN_GAME_ID:
					res_gen = gen_bet_altyn(a, bet_str);
					break;
				case TAGA_GAME_ID:
					res_gen = gen_bet_altyn(a, bet_str);
					break;
			}
			// generate bet
			break;
		}
	}	



	/**************/
	
	return win;
}
Json::Value &GamesConfigLoader::get_config(int game_id,
																					int price)
{
	Json::Value empt = Json::Value::null;
	
	// *****************
	// try to find game_id
	// *****************
	auto game_cfg = games.find(game_id);
	if (game_cfg == end(games))
	{
		return empt;
	}
		
	auto &b = games.at(game_id);
		
	// *****************
	// try to find price
	// *****************
	auto price_cfg = b.config.find(price);
	if (price_cfg == end(b.config))
	{
		return empt;
	}
	// *****************
	auto &json_value = b.config.at(price);
		
	return json_value;

}

int  GamesConfigLoader::gen_bet_tripler(
							Json::Value &val, string &bet_str)
{
	int total_win = 0;	

	vector<int> bet_elements;
	map<int,int> game1_elements;
	map<int,int> game2_elements;
	vector<int> win_mask;
	string bet;
	string mask;
	int total_tile_count = 0;
	int mask_tile_count = 0;
	/****************
	* generate mask 
	* and bonus mask
	*****************/

	for (int a=1;a<=20;a++)
		win_mask.push_back(a);

	time_t t = time(NULL);
  std::mt19937 random;
  random = std::mt19937(t);
	shuffle(begin(win_mask), end(win_mask),random);
	auto win_mask_it = begin(win_mask);

	/*****************/

	// *****************
	// check elements
	// *****************
	 if (val["tripler"] == Json::Value::null)
	{
		bet_str.append("{\"err\":\"config\"}");
		return 0;
	}

	bool tripler = val["tripler"].asBool();
	int tripler_once=tripler;

 	if (val["bet"].isArray())
	{
		int bet_len = val["bet"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		//win_mask.resize(bet_len);
		int mask_count = 0;	
		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *win_mask_it;
			mask.append(to_string(tile));
			if (mask_tile_count < MAX_MASK_TRIPLER_SIZE - 1)
			{
				mask.append(",");
			}
			mask_tile_count++;
			win_mask_it++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++,total_tile_count ++)
			{
				if(tripler_once)
				{
					tile = 666;
					bet.append("{\"tile\":");
					bet.append(to_string(tile));
					bet.append(",\"tripler\":");
					bet.append(to_string(tripler));
					bet.append(",\"win\":");
					bet.append(to_string(win));
					bet.append("},");
					tripler_once = 0;
					total_win += win*count;
					total_tile_count++;
					break;
				} // if tripler_once
				if (total_tile_count)	
					bet.append(",");
				bet.append("{\"tile\":");
				bet.append(to_string(tile));
				bet.append(",\"win\":");
				bet.append(to_string(win));
				bet.append("}");
				total_win += win;
			} // for tile

		}// for bet
		for(int b =total_tile_count; b<10;b++ )
		{
			int tile  = *win_mask_it;
			win_mask_it++;
			int win = 0;
			if (b)	
				bet.append(",");
			bet.append("{\"tile\":");
			bet.append(to_string(tile));
			bet.append(",\"win\":");
			bet.append(to_string(win));
			bet.append("}");
		} // for win_mask_it	
	}	// if bet Array()

	
	for (int x = mask_tile_count; x<MAX_MASK_TRIPLER_SIZE;x++)
	{
		int tile = *win_mask_it;
		mask.append(to_string(tile));
		if (x < MAX_MASK_TRIPLER_SIZE - 1)
			mask.append(",");
	}
	bet_str.append("[");	
	bet_str.append(mask);	
	bet_str.append("],");	
	bet_str.append("\"tripler\":");
	bet_str.append(to_string(tripler));
	bet_str.append(",\"bet\":[");	
	bet_str.append(bet);	
	bet_str.append("]}");	
	fprintf(stdout, "bet: %s\n", bet_str.c_str());
	return total_win;	
}

int  GamesConfigLoader::gen_bet_cleopatra(
							Json::Value &val, string &bet_str)
{
	int total_win = 0;	

	vector<int> bet_elements;
	map<int,int> game1_elements;
	map<int,int> game2_elements;
	vector<int> win_mask;
	string bet;
	string mask;
	string bonus;
	int total_tile_count = 0;
	int mask_tile_count = 0;
	/****************
	* generate mask 
	* and bonus mask
	*****************/

	for (int a=1;a<=20;a++)
		win_mask.push_back(a);

	time_t t = time(NULL);
  std::mt19937 random;
  random = std::mt19937(t);
	shuffle(begin(win_mask), end(win_mask),random);
	auto win_mask_it = begin(win_mask);

	/*****************/

 	if (val["bet"].isArray())
	{
		int bet_len = val["bet"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		//win_mask.resize(bet_len);
		int mask_count = 0;	
		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *win_mask_it;
			mask.append(to_string(tile));
			if (mask_tile_count < MAX_MASK_CLEO_SIZE - 1)
			{
				mask.append(",");
			}
			mask_tile_count++;
			win_mask_it++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			// generate a win line
			for (int b = 0; (b < count) && win; b++,total_tile_count ++)
			{
				if (total_tile_count)	
					bet.append(",");
				bet.append("{\"tile\":[");
				for(int t_n = 0; t_n <3; t_n++)
				{
					
					bet.append(to_string(tile));
					if (t_n<3-1)
					{
						bet.append(",");
					}
				}
				bet.append("],\"win\":");
				bet.append(to_string(win));
				bet.append("}");
				total_win += win;
			} // for tile
			
			

		}// for bet
		for(int b =total_tile_count; b<8;b++ )
		{
			int tile  = *win_mask_it;
			auto win_mask_itit = win_mask_it;
			win_mask_it++;
			int win = 0;
				if (b)	
					bet.append(",");
			bet.append("{\"tile\":[");
			for(int t_n = 0; t_n <3; t_n++)
			{
				
				bet.append(to_string(tile));
				if (t_n<3-1)
				{
					bet.append(",");
				}
				win_mask_itit++;
				tile = *win_mask_itit;	
			}
			bet.append("],\"win\":");
			bet.append(to_string(win));
			bet.append("}");
		} // for win_mask_it	
	}	// if bet Array()

	// generate bonus
 	if (val["bonus"].isArray())
	{
		int bet_len = val["bonus"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}


		//win_mask.resize(bet_len);
		//auto bonus_mask_it = begin(bonus_mask);
		int mask_count = 0;	
		// append bet
		total_tile_count = 0;
		for (auto a: val["bonus"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			//int tile = *bonus_mask_it;
			int tile = 1;// bonus tile
			//bonus_mask_it++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
			if(win)
				tile = 66;
		  // random number
			for (int b = 0; b < count; b++,total_tile_count ++)
			{
				if (b)	
					bonus.append(",");
				bonus.append("{\"tile\":");
				bonus.append(to_string(tile));
				bonus.append(",\"win\":");
				bonus.append(to_string(win));
				bonus.append("}");
				total_win += win;
			} // for tile

		}// for bet

	}// if bonus
	
	for (int x = mask_tile_count; x<MAX_MASK_CLEO_SIZE;x++)
	{
		int tile = *win_mask_it;
		mask.append(to_string(tile));
		if (x < MAX_MASK_CLEO_SIZE - 1)
			mask.append(",");
	}
	bet_str.append("[");	
	bet_str.append(mask);	
	bet_str.append("],");	
	bet_str.append("\"bet\":[");	
	bet_str.append(bet);	
	bet_str.append("],\"bonus\":[");	
	bet_str.append(bonus);	
	bet_str.append("]}");	
	fprintf(stdout, "bet: %s\n", bet_str.c_str());
	return total_win;	
}
int  GamesConfigLoader::gen_bet_mega_casino(
							Json::Value &val, string &bet_str)
{
	int total_win = 0;	

	vector<int> mask_bet1;
	vector<int> mask_bet2;
	vector<int> mask_bet3;
	vector<int> mask_bet4;
	string bet1;
	string bet2;
	string bet3;
	string bet4;
	int total_tile_bet1_count = 0;
	int total_tile_bet2_count = 0;
	int total_tile_bet3_count = 0;
	int total_tile_bet4_count = 0;
	/****************
	* generate mask 
	* and bonus mask
	*****************/

	for (int a=1;a<=20;a++)
		mask_bet1.push_back(a);
	for (int a=1;a<=6;a++)
		mask_bet2.push_back(a);
	for (int a=1;a<=8;a++)
		mask_bet3.push_back(a);
	for (int a=1;a<=9;a++)
		mask_bet4.push_back(a);

	time_t t = time(NULL);
  std::mt19937 random;
  random = std::mt19937(t);
	
	shuffle(begin(mask_bet1), end(mask_bet1),random);
	shuffle(begin(mask_bet2), end(mask_bet2),random);
	shuffle(begin(mask_bet3), end(mask_bet3),random);
	shuffle(begin(mask_bet4), end(mask_bet4),random);

	auto bet1_it = begin(mask_bet1);
	auto bet2_it = begin(mask_bet2);
	auto bet3_it = begin(mask_bet3);
	auto bet4_it = begin(mask_bet4);

	/*****************/

 	if (val["bet1"].isArray())
	{
		int bet_len = val["bet1"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet1"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *bet1_it;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++)
			{
				total_tile_bet1_count ++;
				bet1.append("{\"tile\":");
				bet1.append(to_string(tile));
				bet1.append(",\"win\":");
				bet1.append(to_string(win));
				bet1.append("}");
				if (b < MAX_MASK_CASINO1 - 1)	
					bet1.append(",");
				total_win += win;
			} // for tile
		}// for bet

		for(int b =total_tile_bet1_count; b<MAX_MASK_CASINO1;b++ )
		{
			int tile  = *bet1_it;
			bet1_it++;
			int win = 0;
			bet1.append("{\"tile\":");
			bet1.append(to_string(tile));
			bet1.append(",\"win\":");
			bet1.append(to_string(win));
			bet1.append("}");
			if (b < MAX_MASK_CASINO1 - 1)	
				bet1.append(",");
		} // for win_mask_it	
	}	// if bet1 Array()
/*********************************************/

 	if (val["bet2"].isArray())
	{
		int bet_len = val["bet2"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet2"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *bet2_it;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++)
			{
				if (b||total_tile_bet2_count)	
					bet2.append(",");
				total_tile_bet2_count ++;
				bet2.append("{\"tile\":[");
				bet2.append(to_string(tile));
				bet2.append(",");
				bet2.append(to_string(tile));
				bet2.append("],\"win\":");
				bet2.append(to_string(win));
				bet2.append("}");
				total_win += win;
			} // for tile
		}// for bet

		for(int b =total_tile_bet2_count; b<MAX_MASK_CASINO2;b++ )
		{
			int tile  = *bet2_it;
			bet2_it++;
			int win = 0;
			if (b)	
				bet2.append(",");
			bet2.append("{\"tile\":[");
			bet2.append(to_string(tile));
			tile  = *bet2_it;
			bet2_it++;
			bet2.append(",");
			bet2.append(to_string(tile));
			bet2.append("],\"win\":");
			bet2.append(to_string(win));
			bet2.append("}");
		} // for win_mask_it	
	}	// if bet2 Array()


/*********************************************/

 	if (val["bet3"].isArray())
	{
		int bet_len = val["bet3"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet3"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *bet3_it;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++)
			{
				total_tile_bet3_count ++;
				bet3.append("{\"tile\":[");
				bet3.append(to_string(tile));
				bet3.append(",");
				bet3.append(to_string(tile));
				bet3.append(",");
				bet3.append(to_string(tile));
				bet3.append("],\"win\":");
				bet3.append(to_string(win));
				bet3.append("}");
				if (b < MAX_MASK_CASINO3 - 1)	
					bet3.append(",");
				total_win += win;
			} // for tile
		}// for bet

		for(int b =total_tile_bet3_count; b<MAX_MASK_CASINO3;b++ )
		{
			int tile  = *bet3_it;
			bet3_it++;
			int win = 0;
			bet3.append("{\"tile\":[");
			bet3.append(to_string(tile));
			tile  = *bet3_it;
			bet3_it++;
			bet3.append(",");
			bet3.append(to_string(tile));
			bet3.append(",");
			bet3.append(to_string(tile));
			bet3.append("],\"win\":");
			bet3.append(to_string(win));
			bet3.append("}");
			if (b < MAX_MASK_CASINO3 - 1)	
				bet3.append(",");
		} // for win_mask_it	
	}	// if bet3 Array()



	/*****************************/

 	if (val["bet4"].isArray())
	{
		int bet_len = val["bet4"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet4"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *bet4_it;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++)
			{
				total_tile_bet4_count ++;
				bet4.append("{\"tile\":");
				bet4.append(to_string(tile));
				bet4.append(",\"win\":");
				bet4.append(to_string(win));
				bet4.append("}");
				if (b < MAX_MASK_CASINO4 - 1)	
					bet4.append(",");
				total_win += win;
			} // for tile
		}// for bet

		for(int b =total_tile_bet4_count; b<MAX_MASK_CASINO4;b++ )
		{
			int tile  = *bet4_it;
			bet4_it++;
			int win = 0;
			bet4.append("{\"tile\":");
			bet4.append(to_string(tile));
			bet4.append(",\"win\":");
			bet4.append(to_string(win));
			bet4.append("}");
			if (b < MAX_MASK_CASINO4 - 1)	
				bet4.append(",");
		} // for win_mask_it	
	}	// if bet4 Array()
/*********************************************/

	bet_str.append("[],\"bet1\":[");	
	bet_str.append(bet1);	
	bet_str.append("],");	
	bet_str.append("\"bet2\":[");	
	bet_str.append(bet2);	
	bet_str.append("],");	
	bet_str.append("\"bet3\":[");	
	bet_str.append(bet3);	
	bet_str.append("],");	
	bet_str.append("\"bet4\":[");	
	bet_str.append(bet4);	
	bet_str.append("]}");	
	fprintf(stdout, "bet: %s\n", bet_str.c_str());
	return total_win;	
}

int  GamesConfigLoader::gen_bet_aladdin(
							Json::Value &val, string &bet_str)
{
	int total_win = 0;	

	vector<int> bet_elements;
	map<int,int> game1_elements;
	map<int,int> game2_elements;
	vector<int> win_mask;
	vector<int> bonus_mask;
	string bet;
	string mask;
	string bonus;
	int total_tile_count = 0;
	int mask_tile_count = 0;
	/****************
	* generate mask 
	* and bonus mask
	*****************/

	for (int a=1;a<=20;a++)
		win_mask.push_back(a);

	for (int a=1;a<10;a++)
		bonus_mask.push_back(a);
	
	time_t t = time(NULL);
  std::mt19937 random;
  random = std::mt19937(t);
	shuffle(begin(win_mask), end(win_mask),random);
	shuffle(begin(bonus_mask), end(bonus_mask),random);
	bonus_mask.resize(5);
	auto win_mask_it = begin(win_mask);

	/*****************/

	// *****************
	// check elements
	// *****************
	 if ((val["box"] == Json::Value::null)	 ||
				val["horseshoe"] == Json::Value::null )
	{
		bet_str.append("{\"err\":\"config\"}");
		return 0;
	}

	bool box = val["box"].asBool();
	bool horseshoe =val["horseshoe"].asBool(); 

 	if (val["bet"].isArray())
	{
		int bet_len = val["bet"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		//win_mask.resize(bet_len);
		int mask_count = 0;	
		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *win_mask_it;
			mask.append(to_string(tile));
			if (mask_tile_count < MAX_MASK_SIZE - 1)
			{
				mask.append(",");
			}
			mask_tile_count++;
			win_mask_it++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++,total_tile_count ++)
			{
				if (total_tile_count)	
					bet.append(",");
				bet.append("{\"tile\":");
				bet.append(to_string(tile));
				bet.append(",\"win\":");
				bet.append(to_string(win));
				bet.append("}");
				total_win += win;
			} // for tile
			
			

		}// for bet
		for(int b =total_tile_count; b<6;b++ )
		{
			int tile  = *win_mask_it;
			win_mask_it++;
			int win = 0;
			if (b)	
				bet.append(",");
			bet.append("{\"tile\":");
			bet.append(to_string(tile));
			bet.append(",\"win\":");
			bet.append(to_string(win));
			bet.append("}");
		} // for win_mask_it	
	}	// if bet Array()

	// generate bonus
 	if (val["bonus"].isArray())
	{
		int bet_len = val["bonus"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}


		//win_mask.resize(bet_len);
		auto bonus_mask_it = begin(bonus_mask);
		int mask_count = 0;	
		// append bet
		total_tile_count = 0;
		for (auto a: val["bonus"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = *bonus_mask_it;
			if(box && !horseshoe)
				tile = 10;
			bonus_mask_it++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; b < count; b++,total_tile_count ++)
			{
				if (total_tile_count)	
					bonus.append(",");
				bonus.append("{\"tile\":");
				bonus.append(to_string(tile));
				bonus.append(",\"win\":");
				bonus.append(to_string(win));
				bonus.append("}");
				total_win += win;
			} // for tile
			
			

		}// for bet
		for(int b =total_tile_count; b<5;b++ )
		{
			int tile  = *bonus_mask_it;
			bonus_mask_it++;
			int win = 0;
			if (b)	
				bonus.append(",");
			bonus.append("{\"tile\":");
			bonus.append(to_string(tile));
			bonus.append(",\"win\":");
			bonus.append(to_string(win));
			bonus.append("}");
		} // for win_mask_it	



	}// if bonus
	
	for (int x = mask_tile_count; x<MAX_MASK_SIZE;x++)
	{
		int tile = *win_mask_it;
		mask.append(to_string(tile));
		if (x < MAX_MASK_SIZE - 1)
			mask.append(",");
	}
	bet_str.append("[");	
	bet_str.append(mask);	
	bet_str.append("],");	
	bet_str.append("\"bet\":[");	
	bet_str.append(bet);	
	bet_str.append("],\"bonus\":[");	
	bet_str.append(bonus);	
	bet_str.append("]}");	
	fprintf(stdout, "bet: %s\n", bet_str.c_str());
	return total_win;	
}

	Config & GamesConfigLoader::get_config_rng(int game_id, float rtp, int balance, int price, int bet_count)
{
	/*
		rtp selector
		
	*/	
	vector<int> lucky;
	for (int a=1;a<10;a++)
		lucky.push_back(a);
	time_t t = time(NULL);
  std::mt19937 random;
  random = std::mt19937(t);
	shuffle(lucky.begin(), lucky.end(),random);
	int config_index = 0;
	if (rtp >= 0.95f)
	{
		config_index = 2;		
	}
	//if (rtp >= 0.9f && rtp < 0.95f && lucky[0]==3)
	if (rtp >= 0.9f && rtp < 0.95f )
	{
		config_index = 0;		
	}
	if (rtp<0.9f && lucky[0]%2)
//	if (rtp < 0.9f)
	{
		config_index = 1;		
	}
	fprintf(stdout,"return config game_id %d index %d\n",
					game_id, config_index);	
	auto &cfg = games[game_id].config_rng[config_index]; 
	return cfg;
}


int  GamesConfigLoader::gen_bet_altyn(
							Json::Value &val, string &bet_str)
{
	string bet;
	string mask;
	int total_win = 0;
	int mask_tile_count = 0;
	// *****************
	// check elements
	// *****************
	
 	if (val["bet"].isArray())
	{
		int bet_len = val["bet"].size();
		if (!bet_len)
		{
			fprintf(stderr, "len of array %d elements\n", bet_len);
		}

		// append bet
		int total_tile_count = 0;
		for (auto a: val["bet"])
		{

/***************************************************
* 							values checking
***************************************************/
		  if (!a.isObject())
			{
				fprintf(stderr, "Empty object\n");
				break;
			}
			if (a["tile"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile\n");
				break;
			}
			if (a["count"] == Json::Value::null)
			{
				fprintf(stderr, "mistake in tile count\n");
				break;
			}
/***************************************************/
/***************************************************/
			int tile = 1;
			if (mask_tile_count)
			{
				mask.append(",");
			}
			mask.append(to_string(tile));
			mask_tile_count++;
			int win = a["tile"].asInt();
			int count = a["count"].asInt();
		  // random number
			for (int b = 0; (b < count) && win; b++)
			{
				if (total_tile_count)
					bet.append(",");

				bet.append("{\"tile\":");
				bet.append(to_string(tile));
				bet.append(",\"win\":");
				bet.append(to_string(win));
				bet.append("}");
				total_win += win;
				total_tile_count ++;
			} // for tile
			
			

		}// for bet
		for(int b =total_tile_count; b<4;b++ )
		{
			int tile  = 1;
			int win = 0;
			if (b)
				bet.append(",");
			bet.append("{\"tile\":");
			bet.append(to_string(tile));
			bet.append(",\"win\":");
			bet.append(to_string(win));
			bet.append("}");
		} // for win_mask_it	
	}	// if bet Array()

	
	for (int x = mask_tile_count; x<MAX_MASK_ALTYN_SIZE;x++)
	{
		int tile = 1;

		if (x)
			mask.append(",");
		mask.append(to_string(tile));
	}
	bet_str.append("[");	
	bet_str.append(mask);	
	bet_str.append("],");	
	bet_str.append("\"bet\":[");	
	bet_str.append(bet);	
	bet_str.append("]}");	
	//fprintf(stdout, "bet: %s\n", bet_str.c_str());
	return total_win;	
}
