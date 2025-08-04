#include <stdlib.h>
#include <hiredis.h>
#include <syslog.h>
#include <unistd.h>
#include "redis_helper.h"
#include "oraora_errors.h"

/*! \brief check redis state
*
*	Function check redis context
* if context are die - hiredis trying
* to open new connection to server
* @param r_ctx - pointer to pointer of redis context
* @return 0 on success else error code. See error codes in oraora_errors.h
*
*/
int check_redis(redisContext **_r_ctx)
{
		if(!_r_ctx)
			return 1;

		char *endptr = NULL;
		int redis_port = 6379;
		char *redis_host = secure_getenv("REDIS_HOST");
		char *str_redis_port = secure_getenv("REDIS_PORT");

		redisContext *r_ctx	=*_r_ctx;
		int redis_error = 0;
		redisReply *r_reply =(redisReply*) redisCommand(r_ctx, "PING");
		if (r_reply == NULL)
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "ERR NULL reply PING %s\n",
			r_ctx->errstr); 
#endif
			syslog(LOG_ERR, "ERR NULL reply PING %s",
			r_ctx->errstr); 
			redis_error++;
		}

		if (r_reply->type == REDIS_REPLY_ERROR)
		{
			redis_error++;
		}

		if (r_reply != NULL)
		{
			freeReplyObject(r_reply);
		}

		if (redis_error) 
		{
		
			if (str_redis_port != NULL)
			{
				redis_port = strtol(str_redis_port,&endptr,10);
			}
			if (r_ctx)
			{
				redisFree(r_ctx);
			}
			if (redis_host == NULL)
			{
				r_ctx = redisConnectUnix(REDIS_SOCKET);
			}
			else
			{
				r_ctx = redisConnect(redis_host, redis_port);
			}
		}
		if (r_ctx == NULL )
		{
#ifdef _CONSOLE_DEBUG
			fprintf(stderr, "err allocate redis ctx host [%s] port [%d]\n", redis_host, redis_port);
#endif
    	syslog(LOG_ERR, "err allocate redis ctx host [%s] port [%d]", redis_host, redis_port);
			return 1;
		}
		*_r_ctx = r_ctx;
	return 0;
}
/*! \brief Return from session SZ user_id 
*
*	Function return user_id (SZ) and game_id
* from current user session
*
* @param session - string of user session
* @param user_id - out user_id
* @param game_id - out game_id
* @return 0 on success else error code
*
*/

int get_user_game_id_session(redisContext *r_ctx, 
		const char *session, int &user_id, int &game_id)
{
					if (check_redis(&r_ctx))
					{
						return ERR_REDIS;
					}
					redisReply* r_reply = NULL;
					r_reply =(redisReply*) redisCommand(r_ctx,"HMGET %s player game_id", session);
					if (r_reply == NULL)
					{
#ifdef _CONSOLE_DEBUG
						fprintf(stderr, "get session error: %s\n", r_ctx->errstr );
#endif
						syslog(LOG_ERR, "get session error: %s", r_ctx->errstr );
						return ERR_SESSION;
					}	
					if (r_reply->type == REDIS_REPLY_ERROR)
					{
#ifdef _CONSOLE_DEBUG
						fprintf(stderr, "no player %s\n", session);
#endif
						syslog(LOG_ERR, "no player %s", session);
						if (r_reply != NULL)
						{
							freeReplyObject(r_reply);
						}
						return ERR_REDIS;
					}
					else
					{
						
							char *endptr  = NULL;
						if (r_reply->element[0]->len>0)
						{
							user_id =strtol(r_reply->element[0]->str, &endptr,10); 
						}
						if (r_reply->element[1]->len>0)
						{
							game_id =strtol(r_reply->element[1]->str, &endptr,10); 
						}
					}
			

					if (r_reply != NULL)
					{
						freeReplyObject(r_reply);
					}

	return 0;	
}
