#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

namespace ones {

    using eosio::asset;
    using eosio::symbol;
    using eosio::name;
    using eosio::multi_index;

    using std::pair;

    const name id = "ones"_n;
    const name code = "onesgamedefi"_n;
    const string description = "ONES Game Converter";

    struct token_t {
        name    address;
        symbol  symbol;
    };
    /**
     * liquidity table
     */
    struct [[eosio::table]] liquidity_row {
        uint64_t        liquidity_id;
        token_t         token1;
        token_t         token2;
        asset           quantity1;
        asset           quantity2;
        uint64_t        liquidity_token;
        float_t         price1;
        float_t         price2;
        uint64_t        cumulative1;
        uint64_t        cumulative2;
        float_t         swap_weight;
        float_t         liquidity_weight;
        uint64_t        timestamp;

        uint64_t primary_key() const { return liquidity_id; }
    };
    typedef eosio::multi_index< "liquidity"_n, liquidity_row > liquidity;

    struct [[eosio::table("config")]] st_defi_config
    {
        uint64_t swap_time;
        uint64_t swap_quantity;
        uint64_t swap_suply;
        uint64_t swap_counter;
        uint64_t swap_issue;
        vector<uint64_t> market_suply;
        uint64_t market_time;
        uint64_t last_swap_suply;
        vector<uint64_t> market_quantity;
        uint64_t market_issue;
    };
    typedef singleton<"config"_n, st_defi_config> tb_defi_config;

    /**
     * ## STATIC `get_fee`
     *
     * Get total fee
     *
     * ### returns
     *
     * - `{uint8_t}` - total fee (trade + protocol)
     *
     * ### example
     *
     * ```c++
     * const uint8_t fee = ones::get_fee();
     * // => 20
     * ```
     */
    static uint8_t get_fee()
    {
        return 30;
    }

    /**
     * ## STATIC `get_reserves`
     *
     * Get reserves for a pair
     *
     * ### params
     *
     * - `{uint64_t} pair_id` - pair id
     * - `{symbol} sort` - sort by symbol (reserve0 will be first item in pair)
     *
     * ### returns
     *
     * - `{pair<asset, asset>}` - pair of reserve assets
     *
     * ### example
     *
     * ```c++
     * const uint64_t pair_id = 1;
     * const symbol sort = symbol{"EOS", 4};
     *
     * const auto [reserve0, reserve1] = ones::get_reserves( pair_id, sort );
     * // reserve0 => "4638.5353 EOS"
     * // reserve1 => "13614.8381 USDT"
     * ```
     */
    static pair<asset, asset> get_reserves( const uint64_t pair_id, const symbol sort )
    {
        // table
        ones::liquidity _pools( "onesgamedefi"_n, "onesgamedefi"_n.value );
        auto pool = _pools.get(pair_id, "OnesLibrary: INVALID_PAIR_ID ");
        //eosio::check( pool.token1.symbol == sort || pool.token2.symbol == sort, "DefiboxLibrary: sort symbol "+sort.code().to_string()+" does not match reserves: "+pool.token1.symbol.code().to_string()+","+pool.token2.symbol.code().to_string());
        eosio::check( pool.token1.symbol == sort || pool.token2.symbol == sort, "DefiboxLibrary: sort symbol doesn't match");

        return sort == pool.token1.symbol ?
            pair<asset, asset>{ pool.quantity1, pool.quantity2 } :
            pair<asset, asset>{ pool.quantity2, pool.quantity1 };
    }

    /**
     * ## STATIC `get_rewards`
     *
     * Get rewards for trading
     *
     * ### params
     *
     * - `{uint64_t} pair_id` - pair id
     * - `{asset} from` - tokens we are trading from
     * - `{asset} to` - tokens we are trading to
     *
     * ### returns
     *
     * - {asset} = rewards in ONES
     *
     * ### example
     *
     * ```c++
     * const uint64_t pair_id = 1;
     * const asset from = asset{10000, {"EOS", 4}};
     * const asset to = asset{12345, {"USDT", 4}};
     *
     * const auto rewards = ones::get_rewards( pair_id, from, to);
     * // rewards => "0.123456 ONES"
     * ```
     */
    static asset get_rewards( const uint64_t pair_id, asset in, asset out )
    {
        asset res {0, symbol{"ONES",4}};
        if(in.symbol != symbol{"EOS",4}) std::swap(in, out);
        if(in.symbol != symbol{"EOS",4})
            return res;     //return 0 if non-EOS pair

        ones::liquidity _pools( "onesgamedefi"_n, "onesgamedefi"_n.value );
        auto poolit = _pools.find(pair_id);
        if(poolit==_pools.end() || poolit->swap_weight==0) return res;

        //see: https://github.com/onesgame/defi/blob/master/onesgamemine/onesgamemine.cpp#L212
        ones::tb_defi_config _config( "onesgamemine"_n, "onesgamemine"_n.value );
        auto config = _config.get();

        float newsecs = current_time_point().sec_since_epoch() - config.swap_time;  //second since last update
        auto times = in.amount / 10000;
        auto total = config.swap_quantity + poolit->swap_weight * 0.02 * newsecs * 10000;
        while(times--){
            auto mined = total/10000;   //0.01% of the pool balance
            total -= mined;
            res.amount += mined;
        }

        return res;
    }
}