#ifndef ORAKELH
#define ORAKELH
#include <map>
#include <list>
#include <vector>
#include <set>
#include <unordered_map>
#include <cstdint>
#include <chrono> 
#include <random>



#define MAX_TICKET_BUF (1024U*10U)
#define KILL_TIME_UP 1687526767 

struct modified_hash {
    static uint64_t splitmix64(uint64_t x)
    {
 
        // 0x9e3779b97f4a7c15,
        // 0xbf58476d1ce4e5b9,
        // 0x94d049bb133111eb are numbers
        // that are obtained by dividing
        // high powers of two with Phi
        // (1.6180..) In this way the
        // value of x is modified
        // to evenly distribute
        // keys in hash table
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    int operator()(uint64_t x) const
    {
        static const uint64_t random
            = std::chrono::steady_clock::now()
                  .time_since_epoch()
                  .count();
 
        // The above line generates a
        // random number using
        // high precision clock
        return splitmix64(
 
            // It returns final hash value
            x + random);
    }
};

typedef std::vector<int> Mask;

/*
	class for loading tickets into memory
*/
#define MAX_SIGN_LEN 55

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
	unsigned char sign [MAX_SIGN_LEN];
  ~_bet()
  {
    mask.clear();
  }
};
#endif
