#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <random>
#include <time.h>
#include <chrono>

#include <json/json.h>
#include <json/reader.h>

using namespace std;

#include "bibingo.h"
Line::~Line()
{

}
Line::Line():
	win_size(0U),
	mult(0U),
	win(0U),
	win_draw_step(0U),
	card_index(0U)
{

}
Card::Card()
{
	return;
}
void Bibingo::clear()
{
	v_mask.clear();
	for (auto &card: cards)
	{
		for (auto &line: card.lines)
		{
			for (auto &a: line.mask)
			{
				a = 0;
			} // mask
		}	// lines
	} //cards
	return;		
}

int Bibingo::huingo()
{
	// Generate cards
	for (int i=1;i<=90;i++)
	{
		v_mask.push_back(i);
	}	
  	std::mt19937 random;
    using namespace std::chrono;

    auto now = steady_clock().now().time_since_epoch();
		number = 10000000 + now.count();
    //random = std::mt19937(duration_cast<milliseconds>(now).count());
    random = std::mt19937(number);

	shuffle(begin(v_mask), end(v_mask), random);	

	//cards.resize(6);
	for (auto &card: cards)
	{
		for (auto &line: card.lines)
		{
			vector<int> tmp_mask;
			for (auto &a: line.mask)
			{
				// generate random number, set in mask
				// 
				bool generated = false;
				for  (auto rng: v_mask)
				{
					a = rng;
					// examine line 
					// should not include digit
					// similar order
					for (auto b: tmp_mask)
					{
						int dA = a/10;
						int dB = b/10;
						
						if (dA == dB || (dA == 8 && dB == 9) || (dA == 9 && dB == 8))
						{
					//		fprintf(stderr,"b:%d a:%d\n",b,a);
							a=0;
							break;
						}
						else
						{
							//fprintf(stderr,"b:%d a:%d\n",b,a);
						}
					}
					if (a )
					{
						// add element to mask
						auto v_it = find(begin(v_mask), end(v_mask),a);
						if (v_it != end(v_mask))
							v_mask.erase(v_it);
						tmp_mask.push_back(a);
						generated = true;
						break;
					} // if find
				}//for generator
			} // mask
			sort(begin(line.mask),end(line.mask));
		}	// lines
	} //cards

#if 0
	fprintf(stdout,"v_mask: |");
	
	for (auto x: v_mask)
	{
		fprintf(stdout,"%d |",x);
	}
	fprintf(stdout, "\n");
#endif	
	int res = examine();
	return res;
}
int Bibingo::examine()
{
	for (auto &card: cards)
	{
		for (auto &line: card.lines)
		{
			for (auto &a: line.mask)
			{
				if (!a)
				{
					return 1;
				}
			} // mask
		}	// lines
	} //cards
	return 0;	
}

Bibingo::Bibingo(const char* buf_in, const int len,
									const uint64_t number, const unsigned int price,
								  const uint32_t _draw):draw(_draw),step(1)
{
		Json::Value root;
		Json::Reader reader;
		// load bet from redis to memory
		int res_parsing = reader.parse(buf_in,root);
		if (!res_parsing)
		{
			fprintf(stderr, "unable to parse request %s\n", buf_in);
			vector<Json::Reader::StructuredError> vec_err = reader.getStructuredErrors();
			for (auto a: vec_err)
			{
				fprintf(stderr, "%s\n",a.message);
			}// for err
		}// if error parsing

		//fprintf(stdout, "load %s\n", buf_in);
				
		int c = 0;
		for (auto card: root)
		{
			int l = 0;
			for (auto line :card)
			{
				int m = 0;
				for (auto symbol: line)
				{
					cards[c].lines[l].mask[m] = symbol.asInt();	
					m++;
				} // for mask
				cards[c].lines[l].set_line_index(l+1);
				cards[c].lines[l].set_card_index(c+1);
				l++;
			} // for line
			c++;
		} // for card
	
		
	return;
}

Bibingo::Bibingo()
{
	int it = 0;
	step = 1;
	while(huingo())
	{
		it++;
	}
//		fprintf(stdout, "it %d\n", it);
	return;
}
int Bibingo::copy_win_mask(char* buf, int size)
{
	const int origin_size = size;	
	int size_buf=0;
	int result = 0;
	for (auto &card:cards)
	{
		if (!card.win)
		{
			continue;
		}
		for (auto &win_line:card.win_lines)
		{
			if(win_line)
			{
				// dump to buf	
			}	
		}
	}

}
int Bibingo::copy_mask(char* buf, int size)
{
	const int origin_size = size;	
	int size_buf=0;
	int result = 0;
	for (int i = 0; i < MAX_CARDS; i++)
	{
		result = snprintf(buf+size_buf, size,"[");
		size-=result;
		size_buf+=result;
		for (int b = 0; b < MAX_LINES; b++)
		{
			result = snprintf(buf+size_buf,size,"  [");
			size_buf+=result;
			size -= result;
			for (int a = 0; a < MAX_MASK; a++)
			{
				if (a+1 == MAX_MASK)
				{
					result = snprintf(buf+size_buf, size,"%d",	
						cards[i].lines[b].mask[a]);
					size -= result;
					size_buf+=result;
				}
				else
				{
					result = snprintf(buf+size_buf, size,"%d,",
						cards[i].lines[b].mask[a]);
					size_buf+=result;
					size -= result;
				}
			} // mask
			if (b+1 == MAX_LINES)
			{
				result = snprintf(buf+size_buf, size,"]\n");
				size_buf+=result;
				size -= result;
			}
			else
			{
				result = snprintf(buf+size_buf, size,"],\n");
				size_buf+=result;
				size -= result;
			}
		}	// lines
		if (i+1== MAX_CARDS)
		{
			result = snprintf(buf+size_buf, size,"]\n");
			size_buf+=result;
			size -= result;
		}
		else
		{
			result = snprintf(buf+size_buf, size,"],\n");
			size_buf+=result;
			size -= result;
		}
	} //cards
	//result = snprintf(buf+size_buf, size,"]\n");
	//size_buf+=result;
	//size -= result;
	return size_buf;
}
int Bibingo::copy_json(char* buf, int size)
{
	const int origin_size = size;	
	int size_buf=0;
	int result = snprintf(buf, size,"{\n\"number\":\"%llu\",\"price\":\"%u\",\"draw\":\"%u\",\n \"mask\":[\n",
	number,price,draw);
	size_buf+=result;
	size-=result;
	for (int i = 0; i < MAX_CARDS; i++)
	{
		result = snprintf(buf+size_buf, size,"[");
		size-=result;
		size_buf+=result;
		for (int b = 0; b < MAX_LINES; b++)
		{
			result = snprintf(buf+size_buf,size,"  [");
			size_buf+=result;
			size -= result;
			for (int a = 0; a < MAX_MASK; a++)
			{
				if (a+1 == MAX_MASK)
				{
					result = snprintf(buf+size_buf, size,"%d",	
						cards[i].lines[b].mask[a]);
					size -= result;
					size_buf+=result;
				}
				else
				{
					result = snprintf(buf+size_buf, size,"%d,",
						cards[i].lines[b].mask[a]);
					size_buf+=result;
					size -= result;
				}
			} // mask
			if (b+1 == MAX_LINES)
			{
				result = snprintf(buf+size_buf, size,"]\n");
				size_buf+=result;
				size -= result;
			}
			else
			{
				result = snprintf(buf+size_buf, size,"],\n");
				size_buf+=result;
				size -= result;
			}
		}	// lines
		if (i+1== MAX_CARDS)
		{
			result = snprintf(buf+size_buf, size,"]\n");
			size_buf+=result;
			size -= result;
		}
		else
		{
			result = snprintf(buf+size_buf, size,"],\n");
			size_buf+=result;
			size -= result;
		}
	} //cards
	result = snprintf(buf+size_buf, size,"]}\n");
	size_buf+=result;
	size -= result;
	return size_buf;
}
void Bibingo::dump_json()
{
	fprintf(stdout,"{\n\"number\":%llu,\n \"mask\":[\n",
	number);
	for (int i = 0; i < MAX_CARDS; i++)
	{
		fprintf(stdout,"[");
		for (int b = 0; b < MAX_LINES; b++)
		{
			fprintf(stdout,"  [");
			for (int a = 0; a < MAX_MASK; a++)
			{
				if (a+1 == MAX_MASK)
					fprintf(stdout,"%d",	
						cards[i].lines[b].mask[a]);
				else
					fprintf(stdout,"%d,",
						cards[i].lines[b].mask[a]);
			} // mask
			if (b+1 == MAX_LINES)
				fprintf(stdout,"]\n");
			else
				fprintf(stdout,"],\n");
		}	// lines
		if (i+1== MAX_CARDS)
			fprintf(stdout,"]\n");
		else
			fprintf(stdout,"],\n");
	} //cards
	fprintf(stdout,"]}\n");
}

void Bibingo::dump_cards()
{

	for (auto &card: cards)
	{
		for (auto &line: card.lines)
		{
			fprintf(stdout,"|");
			for (auto &a: line.mask)
			{
						if (a<10)
							fprintf(stdout," %d|",a);
						else
							fprintf(stdout,"%d|",a);
			} // mask
			fprintf(stdout,"\n");
		}	// lines
		fprintf(stdout,"\n");
	} //cards
}
Bibingo::~Bibingo()
{
	return;
}
int Bibingo::examine_line()
{
	return 0;
}
int Bibingo::import_json(char *buf, const int size)
{
	
	return 0;
}
int Bibingo::draw_step(const int ball)
{
	fprintf(stdout, "\n===============\n\tstep %d\n",
					step);
	for (auto &card:cards)
	{
		process_card(card, ball);
	}
	step++;
}
int Bibingo::process_card(Card &card, const int ball)
{

	int line_index = 0;
	int mult = 0;
	int win = 0;
	int max_win_lines = 0;	
	
	// Do not need to recalculate win card
	if (card.total_win_lines == 3)
	{
		fprintf(stdout, "step %d escape processing card\n",
		step
		);
		return 0;
	}

	for (auto &line:card.lines)
	{
		//return 1 if line win 
		int process_res = process_line(line, ball);
	
		//*******************	
		// if line is not win
		if(process_res)	
		{
			line_index++;
			continue;
		}
		// ******************

		if(line.win_draw_step == step)
		{
		
			card.win_lines[line_index] = 1;
			card.total_win_lines++;
			fprintf(stdout, "total win lines:%d \n",
			card.total_win_lines);
		}
		
		line_index++;
  } // for card.lines

// no win lines - must return 
if (!card.total_win_lines)
	return 0;

// calculate bingo card
// ********************
if (card.total_win_lines != 3)
	return 0;

	for (auto &line:card.lines)
	{
		// must return if card
		// having win lines
		if (line.win != 0 )
		{
			return 0;
		}
	}

 if (step > 30 && step <=50)
 {
  	mult = 300;
 }

 if (step>50 && step <=55)
 {
  	mult = 50;
 }

 if (step > 55 && step <= 60)
 {
  	mult = 10;
 }

 if (step > 60 && step <= 65)
 {
  	mult = 4;
 }

 if (step > 65 && step <= 70)
 {
  	mult = 2;
 }

 card.win_bingo = mult * price;
 fprintf(stdout, "BINGO %d !", card.win_bingo);
// ********************

}
int Bibingo::process_line(Line &line, const int ball)
{
	int mult = 0;
	if(line.win_draw_step)
	{
		return 1; //do not need process the line
	}
	for (auto &m:line.mask)
	{
		if (m == ball)
		{
			line.win_size ++;
			fprintf(stdout, "card:%d line %d: ball %d matched. total %d \n",
			line.get_card_index(),
			line.get_line_index(),
			 m, line.win_size);
			if (line.win_size == MAX_MASK)
			{
				// set the current step of draw
				line.win_draw_step = step;

				 if(step <=10)
				 {
					mult=750;
				 }
				 if (step>10 && step <=15)
				 {
					if (price>500)
						mult=65;
					else
						mult=75;
				 }
				 if (step>15 && step <=20)
				 {
						mult=15;
				 }
				 if (step>20 && step <=25)
				 {
						mult=5;
				 }
				 if (step>25 && step <=30)
				 {
						mult=3;
				 }
				 line.win = mult * price;
				 fprintf(stdout, "step:%d bingo: %d\n",
						step, line.win);
				 return 0;
				 break; // only one ball is equal
			} //if line.win_size
		}	// if m==ball
	} //for mask
	return 1;
}
