#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

#include <iostream>
#include <ctime>
#include <json/json.h>

Json::Value get_user_by_id(redisContext *c, uint32_t user_id)
{
    std::string query("HGETALL player:" + std::to_string(user_id));

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements >= 2) {
        jvalue["user"]["id"] = user_id;

        for (int i = 0; i < reply->elements; i += 2) {
            jvalue["user"][reply->element[i]->str] = reply->element[i + 1]->str;
        }
    }

    return jvalue;
}

Json::Value get_user_list(redisContext *c, uint32_t parent_id)
{
    std::string query("SCAN 0 MATCH player:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("parent_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)      == parent_id) {
                        jvalue["users"].append(reply2->element[i]->str);
                        break;
                    }
                }
            }
        }
    }

    return jvalue;
}

Json::Value get_user_list_by_time(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    std::string query("SCAN 0 MATCH player:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        time_t ts_end = ts_start + seconds;

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("parent_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)      != parent_id) {
                        goto L;
                    } else if (strcmp("date_from", reply3->element[j]->str) == 0) {
                        time_t date_from = std::stoul(reply3->element[j + 1]->str);

                        if (date_from < ts_start || date_from > ts_end) {
                            goto L;
                        }
                    }
                }

                jvalue["users"].append(reply2->element[i]->str);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_user_list_by_geo(redisContext *c, uint32_t parent_id, uint32_t geo_id)
{
    std::string query("SCAN 0 MATCH player:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("parent_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)      != parent_id) {
                        goto L;
                    } else if (strcmp("geo_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)   != geo_id) {
                        goto L;
                    }
                }

                jvalue["users"].append(reply2->element[i]->str);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_user_list_by_channel(redisContext *c, uint32_t parent_id, uint32_t channel_id)
{
    std::string query("SCAN 0 MATCH player:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("parent_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)      != parent_id) {
                        goto L;
                    } else if (strcmp("channel_id", reply3->element[j]->str) == 0 &&
                        std::stoul(reply3->element[j + 1]->str)       != channel_id) {
                        goto L;
                    }
                }

                jvalue["users"].append(reply2->element[i]->str);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_user_role(redisContext *c, uint32_t user_id)
{
    std::string query("HGET player:" + std::to_string(user_id) + " role");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_STRING) {
        jvalue["role"] = reply->str;
    }

    return jvalue;
}

void set_role(redisContext *c, uint32_t user_id, uint32_t role)
{
    std::string query("HSET player:" + std::to_string(user_id) + " role " + std::to_string(role));

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());
}

void update_user_channel(redisContext *c, uint32_t user_id, uint32_t channel_id)
{
    std::string query("HSET player:" + std::to_string(user_id) + " channel_id  " + std::to_string(channel_id ));

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());
}

void update_user_parent(redisContext *c, uint32_t user_id, uint32_t parent_id)
{
    std::string query("HSET player:" + std::to_string(user_id) + " parent_id " + std::to_string(parent_id));

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());
}

Json::Value get_user_status(redisContext *c, uint32_t user_id)
{
    std::string query("HGET player:" + std::to_string(user_id) + " status");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_STRING) {
        jvalue["status"] = reply->str;
    }

    return jvalue;
}

void set_user_status(redisContext *c, uint32_t user_id, uint32_t status)
{
    std::string query("HSET player:" + std::to_string(user_id) + " status " + std::to_string(status));

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());
}

Json::Value get_user_sales(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds)
{
    std::string query("SCAN 0 MATCH sales_log:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        time_t ts_end = ts_start + seconds;

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                Json::Value jvalue2;

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("player_id", reply3->element[j]->str) == 0) {

                        if (std::stoul(reply3->element[j + 1]->str) != user_id) {
                            goto L;
                        }
                    } else if (strcmp("terminal_timestamp", reply3->element[j]->str) == 0) {
                        time_t date_from = std::stoul(reply3->element[j + 1]->str);

                        if (date_from < ts_start) {
                            goto L;
                        }
                    } else if (strcmp("game_id", reply3->element[j]->str) == 0 ||
                               strcmp("amount",  reply3->element[j]->str) == 0 ||
                               strcmp("prize",   reply3->element[j]->str) == 0) {
                        jvalue2[reply3->element[j]->str] = reply3->element[j + 1]->str;
                    }
                }

                jvalue["sales"].append(jvalue2);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_user_payin(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds)
{
    std::string query("SCAN 0 MATCH player_bank_gw_in_log:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        time_t ts_end = ts_start + seconds;

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                Json::Value jvalue2;

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("player_id", reply3->element[j]->str) == 0) {

                        if (std::stoul(reply3->element[j + 1]->str) != user_id) {
                            goto L;
                        }
                    } else if (strcmp("ts", reply3->element[j]->str) == 0) {
                        time_t date_from = std::stoul(reply3->element[j + 1]->str);

                        if (date_from < ts_start) {
                            goto L;
                        }
                    } else {
                        jvalue2[reply3->element[j]->str] = reply3->element[j + 1]->str;
                    }
                }

                jvalue["payins"].append(jvalue2);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_user_payout(redisContext *c, uint32_t user_id, time_t ts_start, uint32_t seconds)
{
    std::string query("SCAN 0 MATCH player_bank_gw_out_log:* COUNT 1000000 TYPE hash");

    redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

    Json::Value jvalue;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
        redisReply *reply2 = reply->element[1];

        time_t ts_end = ts_start + seconds;

        for (int i = 0; i < reply2->elements; i++) {
            query.assign("HGETALL ");
            query.append(reply2->element[i]->str);

            redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

            if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                Json::Value jvalue2;

                for (int j = 0; j < reply3->elements; j += 2) {

                    if (strcmp("player_id", reply3->element[j]->str) == 0) { 

                        if (std::stoul(reply3->element[j + 1]->str) != user_id) {
                            goto L;
                        }
                    } else if (strcmp("ts", reply3->element[j]->str) == 0) {
                        time_t date_from = std::stoul(reply3->element[j + 1]->str);

                        if (date_from < ts_start) {
                            goto L;
                        }
                    } else {
                        jvalue2[reply3->element[j]->str] = reply3->element[j + 1]->str;
                    }
                }

                jvalue["payouts"].append(jvalue2);
            }
        L:;
        }
    }

    return jvalue;
}

Json::Value get_parent_sales(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH sales_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {

                            if (std::stoul(reply3->element[k + 1]->str)      != user_id) {
                                goto L;
                            }
                        } else if (strcmp("terminal_timestamp", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else if (strcmp("game_id", reply3->element[k]->str) == 0 ||
                                   strcmp("amount",  reply3->element[k]->str) == 0 ||
                                   strcmp("prize",   reply3->element[k]->str) == 0) {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["sales"].append(jvalue3);
                }
            L:;
            }
        }

    }

    return jvalue2;
}

Json::Value get_parent_payin(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH player_bank_gw_in_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {
                            
                            if (std::stoul(reply3->element[k + 1]->str) != user_id) {
                                goto L;
                            }
                        } else if (strcmp("ts", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["payins"].append(jvalue3);
                }
            L:;
            }
        }

    }

    return jvalue2;
}

Json::Value get_parent_payout(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH player_bank_gw_out_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {

                            if (std::stoul(reply3->element[k + 1]->str) != user_id) {
                                goto L;
                            }
                        } else if (strcmp("ts", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["payouts"].append(jvalue3);
                }
            L:;
            }
        }
    }

    return jvalue2;
}

Json::Value get_parent_sales_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH sales_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {

                            if (std::stoul(reply3->element[k + 1]->str)!= user_id) {
                                goto L;
                            } 

                            jvalue3["player_id"] = user_id;
                        } else if (strcmp("terminal_timestamp", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else if (strcmp("game_id", reply3->element[k]->str) == 0 ||
                            strcmp("amount",  reply3->element[k]->str) == 0 ||
                            strcmp("prize",   reply3->element[k]->str) == 0) {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["sales"].append(jvalue3);
                }
            L:;
            }
        }
    }

    return jvalue2;
}

Json::Value get_parent_payin_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH player_bank_gw_in_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {

                            if (std::stoul(reply3->element[k + 1]->str)!= user_id) {
                                goto L;
                            } 

                            jvalue3["player_id"] = user_id;
                        } else if (strcmp("ts", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["payins"].append(jvalue3);
                }
            L:;
            }
        }
    }

    return jvalue2;
}

Json::Value get_parent_payout_per_user(redisContext *c, uint32_t parent_id, time_t ts_start, uint32_t seconds)
{
    Json::Value jvalue = get_user_list(c, parent_id);

    Json::Value jvalue2;

    for (int i = 0; i < jvalue["users"].size(); i++) {
        std::string value(jvalue["users"][i].asCString());
        uint32_t    user_id = std::stoul(value.substr(7, value.length()));

        std::string query("SCAN 0 MATCH player_bank_gw_out_log:* COUNT 1000000 TYPE hash");

        redisReply *reply = (redisReply*)redisCommand(c, query.c_str());

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            redisReply *reply2 = reply->element[1];

            time_t ts_end = ts_start + seconds;

            for (int j = 0; j < reply2->elements; j++) {
                query.assign("HGETALL ");
                query.append(reply2->element[j]->str);

                redisReply *reply3 = (redisReply*)redisCommand(c, query.c_str());

                if (reply3->type == REDIS_REPLY_ARRAY && reply3->elements >= 2) {
                    Json::Value jvalue3;

                    for (int k = 0; k < reply3->elements; k += 2) {

                        if (strcmp("player_id", reply3->element[k]->str) == 0) {

                            if (std::stoul(reply3->element[k + 1]->str)!= user_id) {
                                goto L;
                            } 

                            jvalue3["player_id"] = user_id;
                        } else if (strcmp("ts", reply3->element[k]->str) == 0) {
                            time_t date_from = std::stoul(reply3->element[k + 1]->str);

                            if (date_from < ts_start) {
                                goto L;
                            }
                        } else {
                            jvalue3[reply3->element[k]->str] = reply3->element[k + 1]->str;
                        }
                    }

                    jvalue2["payouts"].append(jvalue3);
                }
            L:;
            }
        }
    }

    return jvalue2;
}
