#!/bin/bash

pkill -f python 2> /dev/null
rm -f test_server/*
cd src
python server.py ../test_server