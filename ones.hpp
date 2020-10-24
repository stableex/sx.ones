#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

namespace ones {

    using eosio::asset;
    using eosio::symbol;
    using eosio::name;
    using eosio::multi_index;

    using std::pair;


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
        return 20;
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
        auto pool = _pools.get(pair_id, "OnesLibrary: INVALID_PAIR_ID");
        eosio::check( pool.token1.symbol == sort || pool.token2.symbol == sort, "Pair symbols don't match" );

        return sort == pool.token1.symbol ?
            pair<asset, asset>{ pool.quantity1, pool.quantity2 } :
            pair<asset, asset>{ pool.quantity2, pool.quantity1 };
    }
}