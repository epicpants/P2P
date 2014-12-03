#!/bin/bash

cd src/
python client.py localhost 0 1 ../test_clients/client_1
python client.py localhost 0 2 ../test_clients/client_2
python client.py localhost 0 3 ../test_clients/client_3
python client.py localhost 0 4 ../test_clients/client_4
python client.py localhost 0 5 ../test_clients/client_5