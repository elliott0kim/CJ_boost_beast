#!/bin/bash

#URL="http://127.0.0.1:8082/setHeartRate"
URL="http://127.0.0.1:8082/getHeartRate?userId=elliott&heartRateDate=2024-08-04"
AUTH_HEADER="Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJlbGxpb3R0Iiwic3ViIjoi6rmA64-Z7ZiEIiwiaWF0IjoiMTU5NjYyMjIwMCIsImV4cCI6IjE1OTY2MjU4MDAiLCJyb2xlIjoidXNlciJ9.JWYAVcIR_iLcDix3AKesA9cPaJeBtTqATBiUNb24ZSo"

NUM_REQUESTS=1  # 보내고자 하는 요청의 수

# 요청 보내기
for i in $(seq 1 $NUM_REQUESTS)
do
    curl -s --location --request POST "$URL" --header "$AUTH_HEADER"  --data '{
    "data":18
}' &
    if (( $i % 100 == 0 )); then
        echo "$i requests sent"
    fi
done

wait
echo "All requests sent"
