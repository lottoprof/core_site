#ifndef _ORAORAERRORSH
#define _ORAORAERRORSH
/*! @brief Define errors  
*
*/
#define ERR_TILE_TIMEOUT 9
#define ERR_SESSION 1
#define ERR_REDIS 2

#define ERR_REGISTRATION 1
#define ERR_SESS_DOUBLE 2
#define ERR_CLOSED_SERVER 3
#define ERR_SESS_MONEY 4
#define ERR_BETS 5
#define ERR_UNK 6

#define ERR_REGISTER_TIMEOUT -7
#define ERR_REGISTER_TIMEOUT_MESSAGE "token timeout"

#define ERR_REGISTER_USER 510
#define ERR_REGISTER_USER_MESSAGE "user unregistered"

#define ERR_REGISTER_RESEND 511
#define ERR_REGISTER_RESEND_MESSAGE "already registered"


#define ERR_TOKEN 513
#define ERR_TOKEN_MESSAGE "incorrect token"

#define ERR_TOKEN_TIMEOUT 514
#define ERR_TOKEN_TIMEOUT_MESSAGE "token timeout"

#define ERR_P12_GENERATE 515
#define ERR_P12_GENERATE_MESSAGE "p12 is not ready"

#define ERR_SESSION_GENERATE 516
#define ERR_SESSION_GENERATE_MESSAGE "session token is not ready"

#define ERR_REGISTER_RESEND_TG 517
#define ERR_REGISTER_RESEND_TG_MESSAGE "open tg bot tg://resolve?domain=lotodrive_bot&start=/start and share yours phone number"

#define ERR_SESSION_READY_TOKEN 518
#define ERR_SESSION_READY_TOKEN_MESSAGE "try to check lotodrive_bot token and send it for us"

#define CONTENT_TYPE_JSON "application/json"
#define CONTENT_TYPE_JSON_LEN sizeof(CONTENT_TYPE_JSON)

#define ERR_TRNID 222
#define ERR_BALANCE 4
#endif
