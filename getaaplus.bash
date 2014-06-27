#!/bin/bash
rm -rf aaplus
mkdir aaplus
cd aaplus
wget http://www.naughter.com/download/aaplus.zip
unzip aaplus.zip
cmake .
make -j4
