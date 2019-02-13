/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "hidden_fighters.hpp"

namespace eosio {

void token::create( name   issuer,
                    asset  maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

   void token::createnft( name issuer, std::string sym ) {

	require_auth( _self );

	// Check if issuer account exists
	eosio_assert( is_account( issuer ), "issuer account does not exist");

        // Valid symbol
        asset supply(0, symbol( symbol_code( sym.c_str() ), 0) );

        auto symbol = supply.symbol;
        eosio_assert( symbol.is_valid(), "invalid symbol name" );

        // Check if currency with symbol already exists
	auto symbol_name = symbol.code().raw();
        nftstat_index nftstat_table( _self, symbol_name );
        auto existing_nftstat = nftstat_table.find( symbol_name );
        eosio_assert( existing_nftstat == nftstat_table.end(), "token with symbol already exists" );

        // Create new currency
        nftstat_table.emplace( _self, [&]( auto& currency ) {
           currency.supply = supply;
           currency.issuer = issuer;
        });
}


void token::issue( name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    if( to != st.issuer ) {
      SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
                          { st.issuer, to, quantity, memo }
      );
    }
}

void token::transfer( name    from,
                      name    to,
                      asset   quantity,
                      string  memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;    
}


//-----------------------------------------------------------------
//Functions only


void token::sub_balance( name owner, asset value ) {

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

void token::add_balance( name owner, asset value, name ram_payer ) {

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

//nftstats table
void token::sub_supply( asset quantity ) {

	auto symbol_name = quantity.symbol.code().raw();
        nftstat_index nftstat_table( _self, symbol_name );
        auto current_nftstat = nftstat_table.find( symbol_name );

        nftstat_table.modify( current_nftstat, _self, [&]( auto& nftstat ) {
            nftstat.supply -= quantity;
        });
}

void token::add_supply( asset quantity ) {

        auto symbol_name = quantity.symbol.code().raw();
        nftstat_index nftstat_table( _self, symbol_name );
        auto current_nftstat = nftstat_table.find( symbol_name );

        nftstat_table.modify( current_nftstat, name(0), [&]( auto& nftstat ) {
            nftstat.supply += quantity;
        });
}

} /// namespace eosio

EOSIO_DISPATCH( eosio::token, (create)(issue)(transfer)(createnft) )
