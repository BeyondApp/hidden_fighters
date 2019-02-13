/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>
#include <vector>
using std::string;
using std::vector;
//typedef uint64_t color;
typedef uint64_t race_type;
typedef uint64_t id_type;
typedef string uri_type;
typedef uint64_t uuid;

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   class [[eosio::contract("hidden_fighters")]] token : public contract {
      public:
         using contract::contract;

         [[eosio::action]]
         void create( name   issuer, asset  maximum_supply);

         [[eosio::action]]
         void issue( name to, asset quantity, string memo );

         [[eosio::action]]
         void transfer( name from, name to, asset quantity, string memo );

         [[eosio::action]]
         void createnft(name issuer, string sym);

         [[eosio::action]]
         void issuenft(name to, asset quantity, std::vector<string> uris, string name, string memo);


         struct [[eosio::table]] nftaccount {
            asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };

          struct [[eosio::table]] nftstat {
            asset supply;
            name issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
            uint64_t get_issuer() const { return issuer.value; }
        };



         using account_index = eosio::multi_index<"nftaccounts"_n, nftaccount>;
         using nftstat_index = eosio::multi_index<"nftstats"_n, nftstat, 
                     indexed_by< "byissuer"_n, const_mem_fun< nftstat, uint64_t, &nftstat::get_issuer> > >;

        

      private:
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return issuer.value;}
            uint64_t get_supply()const { return supply.symbol.code().raw(); }
         };
         

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;


        void mint(name owner, name ram_payer, asset value, string uri, string name);
        void sub_balance(name owner, asset value);
        void add_balance(name owner, asset value, name ram_payer);
        void sub_supply(asset quantity);
        void add_supply(asset quantity);


   };

} /// namespace eosio
