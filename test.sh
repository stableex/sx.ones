#!/bin/bash

# build
eosio-cpp __tests__/basic.cpp -I ../

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract basic . basic.wasm basic.abi

# getreserve
# "eos2usde"_n.value => 6138456187445706752
cleos -v push action basic getreserves '[1, "4,EOS"]' -p basic
# //=>

# getfee
cleos -v push action basic getfee '[]' -p basic
# //=> 30
