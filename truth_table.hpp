#pragma once

#include <iostream>
#include <cassert>
#include <random>
#include <string>

namespace dtis
{

/* masks used to filter out unused bits */
static const uint64_t length_mask[] = {
  0x0000000000000001,
  0x0000000000000003,
  0x000000000000000f,
  0x00000000000000ff,
  0x000000000000ffff,
  0x00000000ffffffff,
  0xffffffffffffffff};

/* masks used to get the bits where a certain variable is 1 */
static const uint64_t var_mask_pos[] = {
  0xaaaaaaaaaaaaaaaa,
  0xcccccccccccccccc,
  0xf0f0f0f0f0f0f0f0,
  0xff00ff00ff00ff00,
  0xffff0000ffff0000,
  0xffffffff00000000};

/* masks used to get the bits where a certain variable is 0 */
static const uint64_t var_mask_neg[] = {
  0x5555555555555555,
  0x3333333333333333,
  0x0f0f0f0f0f0f0f0f,
  0x00ff00ff00ff00ff,
  0x0000ffff0000ffff,
  0x00000000ffffffff};

/* return i if n == 2^i and i <= 6, 0 otherwise */
inline uint8_t power_two( const uint32_t n )
{
  switch( n )
  {
    case 2u: return 1u;
    case 4u: return 2u;
    case 8u: return 3u;
    case 16u: return 4u;
    case 32u: return 5u;
    case 64u: return 6u;
    default: return 0u;
  }
}

class truth_table_t
{
public:
  // explicit truth_table_t() = default;

  truth_table_t( uint8_t num_var )
   : num_var( num_var ), bits( 0u )
  {
    assert( num_var <= 6u );
  }

  truth_table_t( truth_table_t const& other )
   : num_var( other.num_var ), bits( other.bits )
  {
  }

  truth_table_t( uint8_t num_var, uint64_t bits )
   : num_var( num_var ), bits( bits & length_mask[num_var] )
  {
    assert( num_var <= 6u );
  }

  truth_table_t( const std::string str )
   : num_var( power_two( str.size() ) ), bits( 0u )
  {
    if ( num_var == 0u )
    {
      return;
    }

    for ( auto i = 0u; i < str.size(); ++i )
    {
      if ( str[i] == '1' )
      {
        set_bit( str.size() - 1 - i );
      }
      else
      {
        assert( str[i] == '0' && "Error: Invalid truth table format." );
      }
    }
  }

  bool get_bit( uint8_t const position ) const
  {
    assert( position < ( 1 << num_var ) );
    return ( ( bits >> position ) & 0x1 );
  }

  void set_bit( uint8_t const position )
  {
    assert( position < ( 1 << num_var ) );
    bits |= ( uint64_t( 1 ) << position );
    bits &= length_mask[num_var];
  }

  uint8_t n_var() const
  {
    return num_var;
  }

public:
  uint8_t const num_var; /* number of variables involved in the function */
  uint64_t bits; /* the truth table */
};

/* overload std::ostream operator for convenient printing */
inline std::ostream& operator<<( std::ostream& os, truth_table_t const& tt )
{
  for ( int8_t i = ( 1 << tt.num_var ) - 1; i >= 0; --i )
  {
    os << ( tt.get_bit( i ) ? '1' : '0' );
  }
  return os;
}

/* bit-wise NOT operation */
inline truth_table_t operator~( truth_table_t const& tt )
{
  return truth_table_t( tt.num_var, ~tt.bits );
}

/* bit-wise OR operation */
inline truth_table_t operator|( truth_table_t const& tt1, truth_table_t const& tt2 )
{
  assert( tt1.num_var == tt2.num_var );
  return truth_table_t( tt1.num_var, tt1.bits | tt2.bits );
}

/* bit-wise AND operation */
inline truth_table_t operator&( truth_table_t const& tt1, truth_table_t const& tt2 )
{
  assert( tt1.num_var == tt2.num_var );
  return truth_table_t( tt1.num_var, tt1.bits & tt2.bits );
}

/* bit-wise XOR operation */
inline truth_table_t operator^( truth_table_t const& tt1, truth_table_t const& tt2 )
{
  assert( tt1.num_var == tt2.num_var );
  return truth_table_t( tt1.num_var, tt1.bits ^ tt2.bits );
}

/* check if two truth_table_ts are the same */
inline bool operator==( truth_table_t const& tt1, truth_table_t const& tt2 )
{
  if ( tt1.num_var != tt2.num_var )
  {
    return false;
  }
  return tt1.bits == tt2.bits;
}

inline bool operator!=( truth_table_t const& tt1, truth_table_t const& tt2 )
{
  return !( tt1 == tt2 );
}


/* Returns the truth table of f(x_0, ..., x_num_var) = x_var (or its complement). */
inline truth_table_t create_tt_nth_var( uint8_t const num_var, uint8_t const var, bool const polarity = true )
{
  assert ( num_var <= 6u && var < num_var );
  return truth_table_t( num_var, polarity ? var_mask_pos[var] : var_mask_neg[var] );
}

/* Returns a random truth table with num_var variables. */
inline truth_table_t create_random( uint8_t const num_var, std::default_random_engine::result_type seed = 1 )
{
  assert ( num_var <= 6u );
  std::default_random_engine gen( seed );
  std::uniform_int_distribution<uint64_t> dist( 0ul, std::numeric_limits<uint64_t>::max() );
  return truth_table_t( num_var, dist( gen ) );
}

} // namespace dtis