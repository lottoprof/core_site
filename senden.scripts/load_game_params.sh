#!/bin/bash
host=$1
    echo "HMSET games 2110  'instant/builds/lucky-queen'" | redis-cli --pipe -h $host
    echo "HMSET 2110:params prices  '20,40,100,200,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 2110:params bonus_games_count 1" | redis-cli --pipe -h $host
    echo "HMSET 2110:params bonus_1_game_id 222200" | redis-cli --pipe -h $host
    echo "HMSET 2110:params bonus_1_prices '10,20,25,30,40,50,60,75,80,100,120,125,150,160,180,200,240,250,300,320,350,360,375,400,450,500,600,625,640,700,720,750,800,875,900,1000,1125,1200,1250,1280,1400,1440,1500,1600,1750,1800,1875,2000,2250,2400,2500,3000,3125,3200,3500,3600,3750,4000,4500,5000,6000,6250,6400,7000,7200,7500,8000,8750,9000,10000,11250,12000,12500,12800,14000,14400,15000,16000,17500,18000,18750,20000,22500,24000,25000,30000,31250,32000,35000,36000,37500,40000,45000,50000,60000,62500,64000,70000,72000,75000,80000,90000,100000,120000,125000,128000,140000,144000,150000,160000,180000,187500,200000,225000,240000,250000,300000,320000,360000,375000,400000,450000,500000,600000,625000,750000,800000,900000,1000000'" | redis-cli --pipe -h $host


    echo "HMSET games 2111  'instant/builds/crazy-lemon'" | redis-cli --pipe -h $host
    echo "HMSET 2111:params prices  '20,40,100,200,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 211101:params prices  '20,40,100,200,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 2111:params bonus_games_count 1" | redis-cli --pipe -h $host
    echo "HMSET 2111:params bonus_1_game_id 211101" | redis-cli --pipe -h $host
    echo "HMSET 2111:params bonus_1_prices '20,40,100,200,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET games 2112  'instant/builds/fruto-boom'" | redis-cli --pipe -h $host
    echo "HMSET 2112:params prices  '4,10,20,50,100,200,500,1000,2000'" | redis-cli --pipe -h $host
    echo "HMSET 211200:params prices  '4,10,20,50,100,200,500,1000,2000'" | redis-cli --pipe -h $host
    echo "HMSET 2112:params bonus_games_count 1" | redis-cli --pipe -h $host
    echo "HMSET 2112:params bonus_1_game_id 211200" | redis-cli --pipe -h $host
    echo "HMSET 2112:params bonus_1_prices '50,100,200,400,500,1000,2000,5000,15000,60000,120000'" | redis-cli --pipe -h $host

    echo "HMSET games 2113  'instant/builds/chukcha'" | redis-cli --pipe -h $host
    echo "HMSET 2113:params prices  '4,10,20,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 211301:params prices  '4,10,20,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 2113:params bonus_games_count 2" | redis-cli --pipe -h $host
    echo "HMSET 2113:params bonus_1_game_id 222200" | redis-cli --pipe -h $host
    echo "HMSET 2113:params bonus_1_prices '4,10,20,50,100,220,500,1000,33300'"
    echo "HMSET 2113:params bonus_1_prices '10,20,25,30,40,50,60,75,80,100,120,125,150,160,180,200,240,250,300,320,350,360,375,400,450,500,600,625,640,700,720,750,800,875,900,1000,1125,1200,1250,1280,1400,1440,1500,1600,1750,1800,1875,2000,2250,2400,2500,3000,3125,3200,3500,3600,3750,4000,4500,5000,6000,6250,6400,7000,7200,7500,8000,8750,9000,10000,11250,12000,12500,12800,14000,14400,15000,16000,17500,18000,18750,20000,22500,24000,25000,30000,31250,32000,35000,36000,37500,40000,45000,50000,60000,62500,64000,70000,72000,75000,80000,90000,100000,120000,125000,128000,140000,144000,150000,160000,180000,187500,200000,225000,240000,250000,300000,320000,360000,375000,400000,450000,500000,600000,625000,750000,800000,900000,1000000'" | redis-cli --pipe -h $host
    echo "HMSET 2113:params bonus_2_game_id 211301" | redis-cli --pipe -h $host
    echo "HMSET 2113:params bonus_2_prices '4,10,20,50,100,200,300,500,1000'" | redis-cli --pipe -h $host


    echo "HMSET games 2114  'instant/builds/fruit-n-ice'" | redis-cli --pipe -h $host
    echo "HMSET 2114:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 211400:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 211401:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 2114:params bonus_games_count 2" | redis-cli --pipe -h $host
    echo "HMSET 2114:params bonus_1_game_id 211400" | redis-cli --pipe -h $host
    echo "HMSET 2114:params bonus_1_prices '10,20,30,50,100,200,300,500,1000'" | redis-cli --pipe -h $host
    echo "HMSET 2114:params bonus_2_game_id 211401" | redis-cli --pipe -h $host
    echo "HMSET 2114:params bonus_2_prices '10,20,30,50,100,200,300,500,1000'" | redis-cli --pipe -h $host

    echo "HMSET games 2115  'instant/builds/aksha-bar'" | redis-cli --pipe -h $host
    echo "HMSET 2115:params prices  '5,10,15,20,50,100,250,500'" | redis-cli --pipe -h $host
    echo "HMSET games 211500  'instant/builds/aksha-bar'" | redis-cli --pipe -h $host
    echo "HMSET 211500:params prices  '5,10,15,20,50,100,250,500'" | redis-cli --pipe -h $host 
    echo "HMSET 2115:params bonus_games_count 1" | redis-cli --pipe -h $host
    echo "HMSET 2115:params bonus_1_game_id 211500" | redis-cli --pipe -h $host
    echo "HMSET 2115:params bonus_1_prices '5,10,15,20,50,100,250,500'" | redis-cli --pipe -h $host
    echo "HMSET games 222200  'instant/builds/lucky-queen'" | redis-cli  --pipe -h $host
    echo "HMSET 222200:params prices '10,20,25,30,40,50,60,75,80,100,120,125,150,160,180,200,240,250,300,320,350,360,375,400,450,500,600,625,640,700,720,750,800,875,900,1000,1125,1200,1250,1280,1400,1440,1500,1600,1750,1800,1875,2000,2250,2400,2500,3000,3125,3200,3500,3600,3750,4000,4500,5000,6000,6250,6400,7000,7200,7500,8000,8750,9000,10000,11250,12000,12500,12800,14000,14400,15000,16000,17500,18000,18750,20000,22500,24000,25000,30000,31250,32000,35000,36000,37500,40000,45000,50000,60000,62500,64000,70000,72000,75000,80000,90000,100000,120000,125000,128000,140000,144000,150000,160000,180000,187500,200000,225000,240000,250000,300000,320000,360000,375000,400000,450000,500000,600000,625000,750000,800000,900000,1000000'" | redis-cli  --pipe -h $host
    echo "HMSET games 211101  'instant/builds/crazy-lemon'" | redis-cli  --pipe -h $host
    echo "HMSET 211101:params prices  '20,40,100,200,500,1000'" | redis-cli  --pipe -h $host
    echo "HMSET games 211200  'instant/builds/fruto-boom'" | redis-cli  --pipe -h $host
    echo "HMSET 211200:params prices  '4,10,20,50,100,200,500,1000,2000'" | redis-cli  --pipe -h $host
    echo "HMSET games 211301  'instant/builds/chukcha'" | redis-cli  --pipe -h $host
    echo "HMSET 211301:params prices  '4,10,20,50,100,200,300,500,1000'" | redis-cli  --pipe -h $host
    echo "HMSET games 211400  'instant/builds/fruit-n-ice'" | redis-cli  --pipe -h $host
    echo "HMSET 211400:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli  --pipe -h $host
    echo "HMSET games 211401  'instant/builds/fruit-n-ice'" | redis-cli  --pipe -h $host
    echo "HMSET 211401:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli  --pipe -h $host
    echo "HMSET 211401:params prices '10,20,30,50,100,200,300,500,1000'" | redis-cli  --pipe -h $host
