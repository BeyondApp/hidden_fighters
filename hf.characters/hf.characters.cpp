#include "hf.characters.hpp"
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
                string character_name,
                string race,
                string _class,
                uint64_t level,
                string memo) {

	eosio_assert( is_account( to ), "to account does not exist");

        // e,g, Get EOS from 3 EOS
        auto symbol = quantity.symbol;
        eosio_assert( symbol.is_valid(), "invalid symbol name" );
        eosio_assert( symbol.precision() == 0, "quantity must be a whole number" );
        eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

	eosio_assert( character_name.size() <= 32, "name has more than 32 bytes" );

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
        string char_id = "CHARACTER" +tbllength;
         // Increase supply
	    add_supply( quantity );

        // Mint nfts
           mint( to, st.issuer, char_id, character_name, race, _class, level, asset{1, symbol});
         
        // Add balance to account
        add_balance( to, quantity, st.issuer );

        
}

void nft::mint( name 	owner,
                name 	ram_payer,
                string character_id,
                string character_name,
                string race,
                string _class,
                uint64_t level,
                asset 	value) {

        // Add token with creator paying for RAM
        tokens.emplace( ram_payer, [&]( auto& token ) {
            token.index = tokens.available_primary_key();
            token.owner = owner;
            token.character_id = character_id;
            token.character_name = character_name;
            token.race = race;
            token._class = _class;
            token.level = level;
            token.value = value;
            
        });
}

void nft::setrampayer(name payer, string character_id,
                string character_name,
                string race,
                string _class,
                uint64_t level, 
                id_type index) {

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
                token.character_id = character_id;
                token.character_name = character_name;
                token.race = race;
                token._class = _class;
                token.level = level;
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

EOSIO_DISPATCH( nft, (create)(issue)(setrampayer))
