#ifndef _REDISHELPERH
#define _REDISHELPERH
#include <hiredis.h>
int check_redis(redisContext **_r_ctx);
#define REDDIS_HOSTNAME "127.0.0.1"
#define REDDIS_PORT     6379
#define REDIS_SOCKET "/var/run/redis/redis-server.sock"
int get_user_game_id_session(redisContext *r_ctx, 
		const char *session, 
		int &user_id, int &game_id);

#endif
