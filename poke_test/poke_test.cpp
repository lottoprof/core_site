#include <cstdio>
#include <locale.h>
#include "Poke/PokeHelper.h"
#include <string.h>
#include <codecvt>
#include <locale>
#include <chrono>
#include <random>
#include <algorithm>
using namespace std;
/*
		{'♠', Suit0},
    {'♥', Suit1},
    {'♣', Suit2},
    {'♦', Suit3}
 */
void test_combo_make(const std::wstring& combo)
{
    const auto ph = PokeHelper(combo);

    std::wstring result;
    if(ph.canMakePoker())
    {
        result+=L" Poker";
    }

    if(ph.canMakeRoyalFlush())
    {
        result+=L" RoyalFlush";
    }

    if(ph.canMakeStraightFlush())
    {
        result+=L" StraightFlush";
    }

    if(ph.canMakeFourOfAKind())
    {
        result+=L" FourOfAKind";
    }

    if(ph.canMakeFullHouse())
    {
        result+=L" FullHouse";
    }

    if(ph.canMakeFlush())
    {
        result+=L" Flush";
    }

    if(ph.canMakeStraight())
    {
        result+=L" Straight";
    }

    if(ph.canMakeThreeOfAKind())
    {
        result+=L" ThreeOfAKind";
    }

    if(ph.canMakeTwoPair())
    {
        result+=L" TwoPair";
    }

    if(result.empty())
    {
        result = L" nothing";
    }

    fwprintf(stdout,L"%ls has %ls\n", combo.c_str(), result.c_str());
}

void test_combo(const std::wstring& combo)
{
    const auto ph = PokeHelper(combo);

    std::wstring result;
    if(ph.hasPoker())
    {
        result+=L" Poker";
    }

    if(ph.hasRoyalFlush())
    {
        result+=L" RoyalFlush";
    }

    if(ph.hasStraightFlush())
    {
        result+=L" StraightFlush";
    }

    if(ph.hasFourOfAKind())
    {
        result+=L" FourOfAKind";
    }

    if(ph.hasFullHouse())
    {
        result+=L" FullHouse";
    }

    if(ph.hasFlush())
    {
        result+=L" Flush";
    }

    if(ph.hasStraight())
    {
        result+=L" Straight";
    }

    if(ph.hasThreeOfAKind())
    {
        result+=L" ThreeOfAKind";
    }

    if(ph.hasTwoPair())
    {
        result+=L" TwoPair";
    }

    if(result.empty())
    {
        result = L" nothing";
    }

    fwprintf(stdout,L"%ls has %ls\n", combo.c_str(), result.c_str());
}

int main(int argc, char* argv[])
{
	char *locale = setlocale (LC_ALL, "ru_RU.UTF-8");
	#if 0
  char *param = argv[1];
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(param);

  int len = strlen(param);
  wchar_t buf[255] = {0x00};
  mbstowcs(buf,param, len);
 // fwprintf(stdout,L"P RF SF  4 FH  F  S  3 2x2\n");
  //fwprintf(stdout,L"%ls\n", buf);
	test_combo_make(buf);
    test_combo(L"♠2,♥2,♠4,♦4,Joker;");
    test_combo(L"♠2,♥2,♦3,♠4,♦4;");    
    test_combo(L"♥2,♥3,♥4,♥5,♥6;");    
    test_combo(L"♠2,♥2,♣2,♦2,Joker;");
    test_combo(L"♠2,♥8,♣9,♦A,♠5;");
#endif
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
	vector<wstring> cards_in;
	cards_in.push_back(L"♣K");
	cards_in.push_back(L"♠K");
	for (auto &card_in:cards_in)
	{
		generator.erase(remove(generator.begin(), generator.end(),card_in),generator.end());
	}
#if 1	
    fwprintf(stdout,L"気P RF SF  4 FH  F  S  3 2x2\n");
    time_t current_time = time(NULL);
    std::mt19937 random;
    random = std::mt19937(current_time);
    shuffle(begin(generator), end(generator), random);
		auto generator_it = generator.begin();
	for (auto &card_in:cards_in)
	{
		*generator_it = card_in;
		generator_it++;
	}
		generator.resize(5);
		std::wstring comb;
		int iterator = 0;
		for (auto card: generator)
		{
			if (iterator != 0)
			{
				comb.append(L",");
			}
			comb.append(card);
			iterator++;
		}
		comb.append(L";");
		test_combo(comb);
    //test_combo(L"♠Q,♠6,♠9,♣9,♦10;");
#endif    

    return 0;
}
