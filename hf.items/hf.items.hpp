#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>
#include <vector>

using namespace eosio;
using std::string;
using std::vector;
typedef uint128_t uuid;
typedef uint64_t id_type;

class [[eosio::contract("hf.items")]] nft : public eosio::contract {

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
                    string object_name, 
                    string category,
                    string type,
                    string _class,
                    uint64_t weight,
                    signed defense,
                    signed attack,
                    bool magic,
                    string character_id,  
                    string memo);


         [[eosio::action]]
        void burn(name owner, id_type token_index);

         [[eosio::action]]
	      void setrampayer(name payer, id_type id);

         [[eosio::action]] 
         void close( name owner, const symbol& symbol );

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
           string object_id;
           string object_name;  
           string category;
           string type;
           string _class;
           uint64_t weight;
           signed defense;
           signed attack;
           bool magic; 
           string character_id;                 
           asset value;         	 
       
            id_type primary_key() const { return index; }
            uint64_t get_owner() const { return owner.value; }
            string get_object_id() const { return object_id; }
            string get_object_name() const { return object_name; }
            string get_category() const { return category; }
            string get_type() const { return type; }
            string get_class () const { return _class; }
            uint64_t get_weight () const { return weight; }
            signed get_defense () const { return defense; }
            signed get_attack () const { return attack; }
            bool get_magic_val () const { return magic; }
            string get_character_id() const { return character_id; }
	        uint64_t get_symbol() const { return value.symbol.code().raw(); }
            asset get_value() const { return value; }
	     
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
                    string object_id,
                    string object_name, 
                    string category,
                    string type,
                    string _class,
                    uint64_t weight,
                    signed defense,
                    signed attack,
                    bool magic,  
                    string character_id, 
                    asset value);

        void sub_balance(name owner, asset value);
        void add_balance(name owner, asset value, name ram_payer);
        void sub_supply(asset quantity);
        void add_supply(asset quantity);
};
