#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

struct st_log
{
    uint64_t key;
    name submitter;
    string period;
    double price0_avg_price;
    double price1_avg_price;
    time_point_sec last_update;
};

struct price
{
    uint64_t key;

    name contract0;
    name contract1;

    symbol sym0;
    symbol sym1;

    uint64_t price0_cumulative_last;
    uint64_t price1_cumulative_last;

    time_point_sec last_update;

    uint64_t primary_key() const { return key; }
};

typedef multi_index<"prices"_n, price> prices;
