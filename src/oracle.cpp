#include <oracle.hpp>

ACTION oracle::init(name source)
{
   require_auth(get_self());
   check(!_globals.exists(), "contract has already been initialized, can not change source after init.");
   globals glb = _globals.get_or_default(globals{.source = source});
   glb.source = source;
   _globals.set(glb, _self);
}

ACTION oracle::update(name submitter, uint64_t pairid)
{
   require_auth(submitter); // can be anyone

   _sender = submitter;
   _pairid = pairid;

   globals glb = _globals.get();

   markets _market(glb.source, glb.source.value); // fetch last price info from source

   const auto &pricedata = _market.get(pairid, "no pair found");

   update_all(pricedata);
}

void oracle::update_all(const market pricedata)
{
   update_price("10second", pricedata, 10);

   update_price("1minute", pricedata, 60);
   update_price("5minute", pricedata, 60 * 5);
   update_price("10minute", pricedata, 60 * 10);
   update_price("30minute", pricedata, 60 * 30);

   update_price("1hour", pricedata, 60 * 60);
   update_price("5hour", pricedata, 60 * 60 * 5);
   update_price("10hour", pricedata, 60 * 60 * 10);
   update_price("24hour", pricedata, 60 * 60 * 24);

   update_price("3day", pricedata, 60 * 60 * 24 * 3);
   update_price("7day", pricedata, 60 * 60 * 24 * 7);
   update_price("15day", pricedata, 60 * 60 * 24 * 15);
   update_price("30day", pricedata, 60 * 60 * 24 * 30);
}

void oracle::update_price(const string period, const market pricedata, const uint64_t key)
{
   avgprices _avgprices(_self, _pairid);

   auto itr = _avgprices.find(key);

   if (itr == _avgprices.end())
   {
      _avgprices.emplace(_sender, [&](auto &s) {
         s.key = key;
         s.period = period;
         s.submitter = _sender;
         s.price0_cumulative_last = pricedata.price0_cumulative_last;
         s.price1_cumulative_last = pricedata.price1_cumulative_last;
         s.last_update = pricedata.last_update;
      });
   }
   else
   {
      auto timeElapsed = pricedata.last_update.sec_since_epoch() - itr->last_update.sec_since_epoch();

      // ensure that at least one full period has passed since the last update
      if (timeElapsed >= key && 
         pricedata.price0_cumulative_last > itr->price0_cumulative_last && 
         pricedata.price1_cumulative_last > itr->price1_cumulative_last)
      {
         double price0_avg_price = (pricedata.price0_cumulative_last - itr->price0_cumulative_last) / timeElapsed;
         double price1_avg_price = (pricedata.price1_cumulative_last - itr->price1_cumulative_last) / timeElapsed;

         _avgprices.modify(itr, same_payer, [&](auto &s) {
            s.key = key;
            s.period = period;
            s.submitter = _sender;
            s.price0_cumulative_last = pricedata.price0_cumulative_last;
            s.price1_cumulative_last = pricedata.price1_cumulative_last;
            s.price0_avg_price = price0_avg_price;
            s.price1_avg_price = price1_avg_price;
            s.last_update = pricedata.last_update;
         });

         st_log data{.key = itr->key,
                     .submitter = itr->submitter,
                     .period = itr->period,
                     .price0_avg_price = itr->price0_avg_price,
                     .price1_avg_price = itr->price1_avg_price,
                     .last_update = itr->last_update};

         action(
             permission_level{get_self(), "active"_n},
             get_self(),
             name("log"),
             data)
             .send();
      }
   }
}

void oracle::log(st_log data)
{
   require_auth(get_self());
}