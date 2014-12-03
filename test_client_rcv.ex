#!/bin/bash

cd src/
python client.py localhost 1 6 ../test_clients/client_6 &
python client.py localhost 1 7 ../test_clients/client_7 &
python client.py localhost 1 8 ../test_clients/client_8 &
python client.py localhost 1 9 ../test_clients/client_9 &
python client.py localhost 1 10 ../test_clients/client_10 &
wait