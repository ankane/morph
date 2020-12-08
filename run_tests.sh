#!/bin/sh

set -e

echo "flushall"
morph-cli flushall

echo "set"
morph-cli set hello world

echo "get"
morph-cli get hello

echo "get missing"
morph-cli get missing

echo "mset"
morph-cli mset key1 hello key2 world

echo "mget"
morph-cli mget key1 key2 missing

echo "keys"
morph-cli keys "*"

echo "dbsize"
morph-cli dbsize

echo "info"
morph-cli info

echo "localhost"
morph-cli -h localhost info
