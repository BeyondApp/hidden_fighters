#include "hf.items.hpp"
using namespace eosio;

void nft::create( name issuer, std::string sym ) {

	require_auth( _self );

	// Check if issuer account exists
	eosio_assert( is_account( issuer ), "issuer account does not exist");

        // Valid symbol
        asset supply(0, symbol( symbol_code( sym.c_str() ), 0) );

        auto symbol = supply.symbol;
        eosio_assert( symbol.is_valid(), "invalid symbol name" );

        // Check if currency with symbol already exists
	auto symbol_name = symbol.code().raw();
        currency_index currency_table( _self, symbol_name );
        auto existing_currency = currency_table.find( symbol_name );
        eosio_assert( existing_currency == currency_table.end(), "token with symbol already exists" );

        // Create new currency
        currency_table.emplace( _self, [&]( auto& currency ) {
           currency.supply = supply;
           currency.issuer = issuer;
        });
}

void nft::issue( name to, 
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
                    string memo) {

	eosio_assert( is_account( to ), "to account does not exist");

        // e,g, Get EOS from 3 EOS
        auto symbol = quantity.symbol;
        eosio_assert( symbol.is_valid(), "invalid symbol name" );
        eosio_assert( symbol.precision() == 0, "quantity must be a whole number" );
        eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

	    eosio_assert( object_name.size() <= 32, "name has more than 32 bytes" );

        // Ensure currency has been created
        auto symbol_name = symbol.code().raw();
        currency_index currency_table( _self, symbol_name );
        auto existing_currency = currency_table.find( symbol_name );
        eosio_assert( existing_currency != currency_table.end(), "token with symbol does not exist. create token before issue" );
        const auto& st = *existing_currency;

        // Ensure have issuer authorization and valid quantity
        require_auth( st.issuer );
        eosio_assert( quantity.is_valid(), "invalid quantity" );
        eosio_assert( quantity.amount > 0, "must issue positive quantity of NFT" );
        eosio_assert( symbol == st.supply.symbol, "symbol precision mismatch" );

        auto tablelength =0;
        for (auto itr = tokens.begin(); itr != tokens.end(); ++itr) {
        ++tablelength;
        }
        string tbllength = std::to_string(tablelength); 
        string obj_ID = "SWORD" +tbllength;

         // Increase supply
	    add_supply( quantity );

        // Mint nfts
           mint( to, st.issuer, obj_ID, object_name, category, type, _class, weight, defense, attack, magic, character_id, asset{1, symbol});

        // Add balance to account
        add_balance( to, quantity, st.issuer );

        
}
 void nft::transferid( name	from,
                        name 	to,
                        id_type	index,
                        string	memo ) {
        // Ensure authorized to send from account
        eosio_assert( from != to, "cannot transfer to self" );
        require_auth( from );

        // Ensure 'to' account exists
        eosio_assert( is_account( to ), "to account does not exist");

	// Check memo size and print
        eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

        // Ensure token ID exists
        auto send_token = tokens.find( index );
        eosio_assert( send_token != tokens.end(), "token with specified ID does not exist" );

	// Ensure owner owns token
        eosio_assert( send_token->owner == from, "sender does not own token with specified ID");

	const auto& st = *send_token;

	// Notify both recipients
        require_recipient( from );
        require_recipient( to );

        // Transfer NFT from sender to receiver
        tokens.modify( send_token, from, [&]( auto& token ) {
	        token.owner = to;
        });

        // Change balance of both accounts
        sub_balance( from, st.value );
        add_balance( to, st.value, from );
}

void nft::mint( name owner,
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
                asset value) {
        // Add token with creator paying for RAM
        
        tokens.emplace( ram_payer, [&]( auto& token ) {
            token.index = tokens.available_primary_key();
            token.owner = owner;
            token.object_id = object_id;
            token.object_name = object_name;
            token.category = category;
            token.type = type;
            token._class = _class;
            token.weight = weight;
            token.defense = defense;
            token.attack = attack;
            token.magic = magic;
            token.character_id = character_id;
            token.value = value;        
        });

}
void nft::setrampayer(name payer, id_type index) {

	require_auth(payer);

	// Ensure token ID exists
	auto payer_token = tokens.find( index );
	eosio_assert( payer_token != tokens.end(), "token with specified ID does not exist" );

	// Ensure payer owns token
	eosio_assert( payer_token->owner == payer, "payer does not own token with specified ID");

	const auto& st = *payer_token;

	// Notify payer
	require_recipient( payer );


	// Set owner as a RAM payer
	tokens.modify(payer_token, payer, [&](auto& token){
		token.index = st.index;
		token.owner = st.owner;
		token.value = st.value;
		
	});

	sub_balance( payer, st.value );
	add_balance( payer, st.value, payer );
}


void nft::sub_balance( name owner, asset value ) {

	account_index from_acnts( _self, owner.value );
        const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
        eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );


        if( from.balance.amount == value.amount ) {
            from_acnts.erase( from );
        } else {
            from_acnts.modify( from, owner, [&]( auto& a ) {
                a.balance -= value;
            });
        }
}

void nft::add_balance( name owner, asset value, name ram_payer ) {

	account_index to_accounts( _self, owner.value );
        auto to = to_accounts.find( value.symbol.code().raw() );
        if( to == to_accounts.end() ) {
            to_accounts.emplace( ram_payer, [&]( auto& a ){
                a.balance = value;
            });
        } else {
            to_accounts.modify( to, _self, [&]( auto& a ) {
                a.balance += value;
            });
        }
}

void nft::sub_supply( asset quantity ) {

	auto symbol_name = quantity.symbol.code().raw();
        currency_index currency_table( _self, symbol_name );
        auto current_currency = currency_table.find( symbol_name );

        currency_table.modify( current_currency, _self, [&]( auto& currency ) {
            currency.supply -= quantity;
        });
}

void nft::add_supply( asset quantity ) {

        auto symbol_name = quantity.symbol.code().raw();
        currency_index currency_table( _self, symbol_name );
        auto current_currency = currency_table.find( symbol_name );

        currency_table.modify( current_currency, name(0), [&]( auto& currency ) {
            currency.supply += quantity;
        });
}

EOSIO_DISPATCH( nft, (create)(issue)(transferid)(setrampayer) )
