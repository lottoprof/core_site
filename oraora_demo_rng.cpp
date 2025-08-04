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
#include <algorithm>
#include <string.h>
#include <codecvt>
#include <locale>
#include <chrono>
#include <random>
#include <codecvt>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <random>
#include <algorithm>
#include <sstream>
#include  <iomanip>
#include <json/json.h>
#include "utils.hpp"
#include "orakel.h"
#include "oraora_demo_rng.h"
#include "poke_test/Poke/PokeHelper.h"
using namespace std::chrono;
int get_numbers_cold(const char* session,int game_id, string &out)
{
	vector<int> mask;	
		for (int a=1;a<36;a++)
			mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(mask), end(mask), random);
		mask.resize(3);
		for (auto a:mask)
		{
			if (out.length())
				out.append(",");
			out.append(to_string(a));
		}
	return 0;
}
int get_winners_top( string &out)
{
  static std::mt19937 random = std::mt19937(time(NULL));
	vector<string> phones;
// +639155723782
//{country:3} {region:3}{x}{mask:2}{xx}{mask:2}	
	vector<int> region_mask;
	vector<int> phone_mask;
 	for (int a=909; a<=999;a++)
	{
		region_mask.push_back(a);
	}
		shuffle(begin(region_mask), end(region_mask), random);
	region_mask.resize(5);
	for (int a=10;a<99;a++)
		phone_mask.push_back(a);
	shuffle(begin(phone_mask), end(phone_mask), random);
	phone_mask.resize(10);
	vector<int> win_mask;	
		for (int a=1;a<20;a++)
			win_mask.push_back(a*100);
		shuffle(begin(win_mask), end(win_mask), random);
		win_mask.resize(5);
		auto it_region = region_mask.begin();
		auto it_phone = phone_mask.begin();
		auto it_win = win_mask.begin();
		int x=5;
		for (auto a:win_mask)
		{
			if (out.length())
				out.append(",");
			out.append("{\"currency\":\"PHP\",\"phone\":\"+63-");
			out.append(to_string(*it_region));
			out.append("x");
			out.append(to_string(*it_phone));
			it_phone++;
			out.append("xx");
			out.append(to_string(*it_phone));
			out.append("\",\"win\":\"");
			out.append(to_string(*it_win));
			out.append("\",\"ts\":");
			time_t t = time(NULL)-++x;
			out.append(to_string(t));
			out.append("}");
			it_region++;
			it_phone++;
			it_win++;
		}
	return 0;
}

/*! \brief RNG generator Aksha Bar
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_aksha_bar(int game_id, int price, _bet &b, int &error)
{
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
 	//time_t k = time(NULL);
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=11;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(5);

	if (k%6==0)
	{
	//	error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%5==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.2;
				break;
			case 2:
				mult = 0.4;
				break;
			case 3:
				mult = 0.6;
				break;
			case 4:
				mult = 0.8;
				break;
			case 5:
				mult = 1;
				break;
			case 6:
				mult = 1.2;
				break;
			case 7:
				mult = 1.6;
				break;
			case 8:
				mult = 2;
				break;
			case 9:
				mult = 2.4;
				break;
			case 10:
				mult = 4;
				break;
			case 11:
				mult = 6;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=11;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}

/*! \brief RNG generator 
*			for hot numbers
*	Function return comma
* separated string 
*
* @param game_id 
* @param session
* @return out
*
*/

int get_numbers_hot(const char* session,int game_id, string &out)
{
	vector<int> mask;	
		for (int a=1;a<36;a++)
			mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(mask), end(mask), random);
		mask.resize(3);
		for (auto a:mask)
		{
			if (out.length())
				out.append(",");
			out.append(to_string(a));
		}
	return 0;
}

/*! \brief RNG generator Fruit and Ice
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_fruit_and_ice(int game_id, int price, _bet &b, int &error)
{
 	//time_t k = time(NULL);
		int k = 0;
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=7;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		k = b.mask[6];
		b.mask.resize(5);

	if (k%6==0)
	{
	//	error=1;
	}
	if (k%2==0)
	{
		return 0;
	}
	if (k%5==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.5;
				break;
			case 2:
				mult = 5;
				break;
			case 3:
				mult = 10;
				break;
			case 4:
				mult = 50;
				break;
			case 5:
				mult = 100;
				break;
			case 6:
				mult = 500;
				break;
			case 7:
				mult = 1000;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=7;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}
/*! \brief RNG generator Chukcha
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_chukcha(int game_id, int price, _bet &b, int &error)
{
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
 	//time_t k = time(NULL);
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=7;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(5);

	if (k%6==0)
	{
		//error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%5==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.5;
				break;
			case 2:
				mult = 1.5;
				break;
			case 3:
				mult = 15;
				break;
			case 4:
				mult = 20;
				break;
			case 5:
				mult = 60;
				break;
			case 6:
				mult = 75;
				break;
			case 7:
				mult = 300;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=7;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}

/*! \brief RNG generator Fruto Boom
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_frutoboom(int game_id, int price, _bet &b, int &error)
{
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=11;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(5);

	if (k%6==0)
	{
		//error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%5==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.5;
				break;
			case 2:
				mult = 2;
				break;
			case 3:
				mult = 5;
				break;
			case 4:
				mult = 10;
				break;
			case 5:
				mult = 25;
				break;
			case 6:
				mult = 50;
				break;
			case 7:
				mult = 100;
				break;
			case 8:
				mult = 150;
				break;
			case 9:
				mult = 250;
				break;
			case 10:
				mult = 500;
				break;
			case 11:
				mult = 100;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=11;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}

/*! \brief RNG generator Crazy Lemon
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_crazy_lemon(int game_id, int price, _bet &b, int &error)
{
 	//time_t k = time(NULL);
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=9;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(5);

	if (k%6==0)
	{
		//error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%5==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.75;
				break;
			case 2:
				mult = 1;
				break;
			case 3:
				mult = 2;
				break;
			case 4:
				mult = 5;
				break;
			case 5:
				mult = 7.5;
				break;
			case 6:
				mult = 10;
				break;
			case 7:
				mult = 25;
				break;
			case 8:
				mult = 50;
				break;
			case 9:
				mult = 100;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=9;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}

/*! \brief RNG generator 
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_lucky_queen_bonus(int game_id, int price, _bet &b, int &error)
{
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
 //	time_t k = time(NULL);
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=5;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(1);

		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 40;
				break;
			case 2:
				mult = 100;
				break;
			case 3:
				mult = 140;
				break;
			case 4:
				mult = 200;
				break;
			case 5:
				mult = 560;
				break;
			default:
				break;
		}
	if (k%6==0)
	{
	//	error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%7==0)
	{
		b.win = b.price * mult;	
	}
}

/*! \brief RNG generator 
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/

int get_bet_lucky_queen(int game_id, int price, _bet &b, int &error)
{
 	uint64_t k =  std::chrono::duration_cast<std::chrono::nanoseconds>
											(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
 	//time_t k = time(NULL);
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		for(int a=1;a<=11;a++)
			b.mask.push_back(a);
  	static std::mt19937 random = std::mt19937(time(NULL));
		shuffle(begin(b.mask), end(b.mask), random);
		b.mask.resize(5);

	if (k%6==0)
	{
	//	error=1;
	}
	if (k%3==0)
	{
		return 0;
	}
	if (k%7==0)
	{
		error = 0;
		b.mask.resize(3);
		b.mask[1] = b.mask[0];
		b.mask[2] = b.mask[0];
		float mult = 0;
		switch (b.mask[0])
		{
			case 1:
				mult = 0.5;
				break;
			case 2:
				mult = 1;
				break;
			case 3:
				mult = 2;
				break;
			case 4:
				mult = 3;
				break;
			case 5:
				mult = 5;
				break;
			case 6:
				mult = 5;
				break;
			case 7:
				mult = 10;
				break;
			case 8:
				mult = 20;
				break;
			case 9:
				mult = 25;
				break;
			case 10:
				mult = 30;
				break;
			case 11:
				mult = 50;
				break;
			case 12:
				mult = 250;
				break;
			default:
				break;
		}
		b.win = price*mult;
		b.count_digits = 3;
		b.win_category = b.mask[0];
		vector<int> nowin_mask;
		for (int a = 1; a<=12;a++)
		{
			if (a!=b.mask[0])
			{
				nowin_mask.push_back(a);
			}
		}
		shuffle(begin(nowin_mask), end(nowin_mask), random);
		nowin_mask.resize(2);
		for (auto &a:nowin_mask)
			b.mask.push_back(a);
	/*	if (nowin_mask[0]%2)
		{
			b.mask[0] = 13;
			b.mask[1] = 13;
			b.mask[2] = 13;
		}*/
		shuffle(begin(b.mask), end(b.mask), random);
		
	}
	
}
/*! \brief RNG generator 
*
*	Function return _bet structure 
*
* @param game_id 
* @param price 
* @return _bet 
*
*/
int get_bet_poker(vector<string> &cards_mask,vector<string> &cards, string &result, int &win)
{
	
	vector<wstring> generator;
	generator.push_back(L"Joker");
	vector<wstring> suits;
	vector<wstring> faces;
	faces.push_back(L"J");
	faces.push_back(L"Q");
	faces.push_back(L"K");
	faces.push_back(L"A");
	suits.push_back(L"♥");
	suits.push_back(L"♠");
	suits.push_back(L"♦");
	suits.push_back(L"♣");
	for (auto suit: suits)
	{
  	for (int i = 2; i<=10;i++)
		{
			wstring card = suit;
			card.append(to_wstring(i));
			generator.push_back(card);
		}
		for (auto face:faces)
		{
			wstring card = suit;
			card.append(face);
			generator.push_back(card);
		}
	}
	for (auto &card: cards_mask)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		wstring value = converter.from_bytes(card);
		generator.erase(std::remove(generator.begin(), generator.end(),value ), generator.end());
	}	
    time_t current_time = time(NULL);
    std::mt19937 random;
    random = std::mt19937(current_time);
    shuffle(begin(generator), end(generator), random);
		generator.resize(5);
		std::wstring comb;
		int iterator = 0;


	for (int a = 0; a<5; a++)
	{
		if (cards_mask[a] != "0")
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			wstring value = converter.from_bytes(cards_mask[a]);
			generator[a] = value;
		}
	}	
		for (wstring &card: generator)
		{
			//
			// convert utf16 to utf8
			//
#if 1
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::string str = converter.to_bytes(card);
			cards.push_back(str);
#endif

			if (iterator != 0)
			{
				comb.append(L",");
			}
			comb.append(card);
			iterator++;
		}
		comb.append(L";");
    const auto ph = PokeHelper(comb);
		win = 0;
    if(ph.hasPoker())
    {
				win = 3000;
        result+="Poker";
    }

    if(ph.hasRoyalFlush())
    {
				win = 1500;
        result+="RoyalFlush";
    }

    if(ph.hasStraightFlush())
    {
				win = 400;
        result+="StraightFlush";
    }

    if(ph.hasFourOfAKind())
    {
				win = 200;
        result+="FourOfAKind";
    }

    if(ph.hasFullHouse())
    {
				win = 60;
        result+="FullHouse";
    }

    if(ph.hasFlush())
    {
				win = 45;
        result+="Flush";
    }

    if(ph.hasStraight())
    {
				win = 35;
        result+="Straight";
    }

    if(ph.hasThreeOfAKind())
    {
				win = 25;
        result+="ThreeOfAKind";
    }

    if(ph.hasTwoPair())
    {
				win = 15;
        result+="TwoPair";
    }

    if(result.empty())
    {
        result = "nothing";
    }


	/*
		get win of combination
	*/

	return 1;
}
int get_bet_rng(Config& config, int game_id, int price, 
								vector<_bet> &bb,int bet_count, int &error)
{
	int res = slot_rng(config, bet_count, game_id, price,bb);

}
int get_bet(int game_id, int price, _bet &b, int &error)
{
#if 0
	if (price == 1000)
	{
		b.number = 0;
		b.win = 0;
		b.price = 0;
		//b.error = 1;
		error = 4;
		return 0;
	}
#endif
	b.number= 77777;
	if (game_id == 2116 || game_id == 2121||
			 game_id == 2117|| game_id == 2118
		|| game_id == 2122|| game_id == 2119	)
	{
		// card games without symbols
		// without win

		b.win = 0;
		b.game_id = game_id;
		b.price = price;
		b.draw	= 666;
		b.count_digits = 0;
		b.win_category = 0;
		b.mask.push_back(0);
		b.mask.push_back(0);
		b.mask.push_back(0);
		b.mask.push_back(0);
		b.mask.push_back(0);
	
		return 0;
	}
	switch (game_id)
	{
		//case 211000:
		//	get_bet_lucky_queen_bonus(game_id, price, b, error);
		//	break;	
		case 2110:
			get_bet_lucky_queen(game_id, price, b, error);
			break;	
		case 2124:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2125:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2126:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2127:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2128:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2129:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2130:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2131:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2132:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2133:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2134:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2111:
			get_bet_crazy_lemon(game_id, price, b, error);
			break;	
		case 2112:
			get_bet_frutoboom(game_id, price, b, error);
			break;	
		case 2113:
			get_bet_chukcha(game_id, price, b, error);
			break;	
		case 2114:
			get_bet_fruit_and_ice(game_id, price, b, error);
			break;	
		case 2115:
			get_bet_aksha_bar(game_id, price, b, error);
			break;	
		default:
			break;
	}

  return 0;
}



//#define init_arr(name, size) name[size]; int name##_size = size; (void) name##_size

//#define init_matrix(name, height, width) name[height][width];\
//    int name##_width = width; int name##_height = height;\
//    (void) name##_width; (void) name##_height


class Ticket {
public:
    int num = 0;
    int line_num = 0;
    std::vector<int> reels;
    int symbol = 0;
    int count = 0;
    double win_amount = 0;

    std::string to_string() const {
				char buf[1024] = {0x00};
				sprintf(buf, "%d;%d;%s;%d;%d;%.2f",num,line_num,array_to_string(reels, true).c_str(),
						symbol,count,win_amount);
				std::string ret = (const char*)buf;
        return ret;
    }
};

class Generator {
public:
    Config& config;
    int current_num = 0;

    Generator(Config& config) : config(config) {};

    std::string** generate_game_matrix_with_probabilities(std::mt19937& random) {
        std::uniform_int_distribution<> size_interval(0, 11);

        auto** game_matrix = new std::string*[config.rows];

        for (int i = 0; i < config.reels; i++) {
            int* probabilities = config.probability_matrix_int[i];

            auto distrib = std::discrete_distribution<int>(probabilities, probabilities + config.num_symbols);

            for (int j = 0; j < config.rows; j++) {
                if (i == 0) {
                    game_matrix[j] = new std::string[config.reels];
                }

                game_matrix[j][i] = std::to_string(distrib(random) + 1);
            }
        }

        return game_matrix;
    }

    Ticket* check_winning_lines(std::string** game_matrix) {
        auto win_info = new Ticket[config.num_win_lines];

        for (int i = 0; i < config.num_win_lines; i++) {
            const auto& line = config.win_lines[i];

            std::string symbols_in_line[config.win_line_length];

            for (int j = 0; j < config.win_line_length; j++) {
                symbols_in_line[j] = game_matrix[line[j].x][line[j].y];
            }

            int symbol_count[config.num_symbols] = {0x00};

            //for (int j = 0; j < config.num_symbols; j++) {
            //    symbol_count[j] = 0;
           // }
						int wild_count = 0;

            for (int j = 0; j < config.win_line_length; j++)
					  {
              symbol_count[std::stoi(symbols_in_line[j]) - 1]++;
							if (std::stoi(symbols_in_line[j]) == config.wild_number)	
							{
								wild_count++;
							}
            }

            Ticket ticket;

            ticket.num = current_num;
            ticket.line_num = i + 1;

            for (int k = 0; k < config.win_line_length; k++) {
                ticket.reels.push_back(std::stoi(symbols_in_line[k]));
            }
/******************
*	check wild count
******************/
						
						if (wild_count >= 3 || wild_count == 0)
						{							
	
            	for (int j = 0; j < config.num_symbols; j++) 
							{
                int count = symbol_count[j];
                std::string symbol = std::to_string(j + 1);

                auto is_win = std::find(
                    config.winning_matches.begin(),
                    config.winning_matches.end(),
                count);

                if (is_win != config.winning_matches.end()) {
                    auto win_amount = config.paytable.at(symbol)[is_win - config.winning_matches.begin()];

                    ticket.symbol = std::stoi(symbol);
                    ticket.count = count;
                    ticket.win_amount = win_amount;

                    break;
                	}
            		}	 // calculate win
							} //check wild count >=3 or wild_count==0, normal calculate
						
						
						if (wild_count == 1)
						{							
							// check count of pairs	
							int pairs_count = 0;
							int pairs_symbol = 0;
            	for (int j = 0; j < config.num_symbols; j++) 
							{
                int count = symbol_count[j];
                std::string symbol = std::to_string(j + 1);
								if (count < 2)
								{
									continue;
								}
								int symbol_number = std::stoi(symbol);

								if (pairs_symbol == 0)
								{
									pairs_symbol = symbol_number; 
									pairs_count = count+1;
								}
								else if (pairs_symbol != symbol_number)
								{
									// check pair and set
								
									if (config.wild_order == 1)
									{
										if (symbol_number> pairs_symbol)
										{
											pairs_symbol = symbol_number;
										}
									}	
									else
									{
										if (symbol_number< pairs_symbol)
										{
											pairs_symbol = symbol_number;
										}
										
									}	
									pairs_count = count+1;
								}
            		}	 // calculate win

                auto is_win = std::find(
                    config.winning_matches.begin(),
                    config.winning_matches.end(),
                pairs_count);
								if (is_win != config.winning_matches.end())
								{
										auto symbol = std::to_string(pairs_symbol);
                    auto win_amount = config.paytable.at(symbol)[is_win - config.winning_matches.begin()];
                    ticket.symbol = std::stoi(symbol);
                    ticket.count = pairs_count;
                    ticket.win_amount = win_amount;

                  //  break;

								}	
							} //check wild count == 1 normal calculate

						if (wild_count == 2)
						{							
							// get hight symbol	
							int tile_symbol = 0;
							int tile_count = 0;
            	for (int j = 0; j < config.num_symbols; j++) 
							{
                int count = symbol_count[j];
                std::string symbol = std::to_string(j + 1);
								int symbol_number = std::stoi(symbol);

								if (tile_symbol == 0)
								{
									tile_symbol = symbol_number; 
									tile_count = count+2;
								}
								else if (tile_symbol != symbol_number)
								{
									// check pair and set
								
									if (config.wild_order == 1)
									{
										if (symbol_number> tile_symbol)
										{
											tile_symbol = symbol_number;
											tile_count = count+2;
										}
									}	
									else
									{
										if (symbol_number< tile_symbol)
										{
											tile_symbol = symbol_number;
											tile_count = count+2;
										}
										
									}	
								}
            		}	 // calculate win
                auto is_win = std::find(
                    config.winning_matches.begin(),
                    config.winning_matches.end(),
                tile_count);
								if (is_win != config.winning_matches.end())
								{
										auto symbol = std::to_string(tile_symbol);
                    auto win_amount = config.paytable.at(symbol)[is_win - config.winning_matches.begin()];
                    ticket.symbol = std::stoi(symbol);
                    ticket.count = tile_count;
                    ticket.win_amount = win_amount;
								}
                  //  break;

							} //check wild count == 1 normal calculate


            win_info[i] = ticket;

            current_num++;
        } // lines calc

        return win_info;
    }
};

/***********************************
slot generator main function

************************************/





int slot_rng(Config &config, int bet_count,
int game_id, int price,vector<_bet> &bb) {
    int num_iter = 1;

    std::random_device rd;
// ********** preload all configs **********
// *****************************************

    Generator generator(config);
    std::mt19937 rand_gen(rd());


        std::string** game_matrix =
            generator.generate_game_matrix_with_probabilities(rand_gen);

/*        if (!quiet) {
            for (int i = 0; i < config.rows; i++) {
                std::cout << array_to_string(game_matrix[i], config.reels) << std::endl;
            }
        }
*/
        Ticket* tickets = generator.check_winning_lines(game_matrix);
       for (int i = 0; i < bet_count; i++) {
           const auto& ticket = tickets[i];
					_bet b;
					b.mask = ticket.reels;	
					b.game_id = game_id;
					b.price = price;
					b.win_category = ticket.symbol;
					b.count_digits = ticket.count;
					b.win = ticket.win_amount * price; // ATTENTION. may be wrong convert to float
					bb.push_back(b);
					for (auto x:ticket.reels)
					{
						fprintf(stdout,"%d ",x);
					}
						fprintf(stdout,"\n");
					
          // print_ticket(ticket);
       }

  /*      for (int i = 0; i < config.num_win_lines; i++) {
            const auto& ticket = tickets[i];

            stream << ticket.to_string() << std::endl;
        }
        if (!quiet) {
            for (int i = 0; i < config.num_win_lines; i++) {
                const auto& ticket = tickets[i];

               // print_ticket(ticket);
            }

            std::cout << "--------------------" << std::endl;
        }
*/


    return 0;
}
