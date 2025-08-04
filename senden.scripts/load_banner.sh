#!/bin/bash
host=$1
echo "del banners:top"|redis-cli -h $host
echo "keys banners:top:*"|redis-cli|gawk -e '{print "del " $1}'|redis-cli -h $host
echo "lpush banners:top 1 2"|redis-cli -h $host
echo "hmset banners:top:1 img_url 'https://senden.swertefun.com/static/img/banner-img1.png'"|redis-cli -h $host
echo "hmset banners:top:1 site_url  '#'"|redis-cli -h $host
echo "hmset banners:top:1 game_id 2113"|redis-cli -h $host
echo "hmset banners:top:1 alt 'Press the play button'"|redis-cli -h $host
echo "hmset banners:top:1 text '200000 EUR win'"|redis-cli -h $host

echo "hmset banners:top:2 img_url 'https://senden.swertefun.com/static/img/banner-img2.png'"|redis-cli -h $host
echo "hmset banners:top:2 site_url  '#'"|redis-cli -h $host
echo "hmset banners:top:2 game_id 2113"|redis-cli -h $host
echo "hmset banners:top:2 alt 'Press the play button'"|redis-cli -h $host
echo "hmset banners:top:2 text '50000 EUR win'"|redis-cli -h $host
