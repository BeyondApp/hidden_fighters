#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;
using std::string;
typedef uint64_t id_type;

class [[eosio::contract("hf.characters")]] nft : public eosio::contract {

     public:
	using contract::contract;
        nft( name receiver, name code, datastream<const char*> ds)
		: contract(receiver, code, ds), tokens(receiver, receiver.value) {
      }

         [[eosio::action]]
        void create(name issuer, std::string symbol);

         [[eosio::action]]
        void issue(name to, 
                    asset quantity,  
                    string character_name, 
                    string race,
                    string _class,
                    uint64_t level,
                    string memo);

         [[eosio::action]]
	      void setrampayer(name payer, string character_id,
                string character_name,
                string race,
                string _class,
                uint64_t level, 
                id_type id);

        struct [[eosio::table]] account {
            asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };


        struct [[eosio::table]] stats {
            asset supply;
            name issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
            uint64_t get_issuer() const { return issuer.value; }
        };

        struct [[eosio::table]] token {
            id_type index;          
            name owner;
            string character_id;
            string character_name;
            string race;
            string _class;
            uint64_t level;
            asset value;         
       
            id_type primary_key() const { return index; }
            uint64_t get_owner() const { return owner.value; }
            string get_character_id() const { return character_id; }
            string get_character_name() const { return character_name; }
            string get_race() const { return race; }
            string get_class() const { return _class; } 
            uint64_t get_level() const { return level; }
            asset get_value() const { return value; }
	        uint64_t get_symbol() const { return value.symbol.code().raw(); }
	        
      
        };
	   

	using account_index = eosio::multi_index<"accounts"_n, account>;

	using currency_index = eosio::multi_index<"stat"_n, stats,
	                       indexed_by< "byissuer"_n, const_mem_fun< stats, uint64_t, &stats::get_issuer> > >;

	using token_index = eosio::multi_index<"token"_n, token,
	                    indexed_by< "byowner"_n, const_mem_fun< token, uint64_t, &token::get_owner> >,
			    indexed_by< "bysymbol"_n, const_mem_fun< token, uint64_t, &token::get_symbol> > >;

    private:
	token_index tokens;

        void mint(name owner,
                    name ram_payer, 
                    string character_id,
                    string character_name,
                    string race,
                    string _class,
                    uint64_t level,
                    asset value);


        void sub_balance(name owner, asset value);
        void add_balance(name owner, asset value, name ram_payer);
        void sub_supply(asset quantity);
        void add_supply(asset quantity);
};
