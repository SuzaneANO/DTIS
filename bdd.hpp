#pragma once

#include "truth_table.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>

/*! \brief Methods to enable Hashing for std::pair. */
namespace std
{

template<class T>
inline void hash_combine( size_t& seed, T const& v )
{
  seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<>
struct hash<pair<uint32_t, uint32_t>>
{
  using argument_type = pair<uint32_t, uint32_t>;
  using result_type = size_t;
  result_type operator() ( argument_type const& in ) const
  {
    result_type seed = 0;
    hash_combine( seed, in.first );
    hash_combine( seed, in.second );
    return seed;
  }
};

} // namespace std

namespace dtis
{

/*! \brief This data structure implements a simple BDD
 *
 * The BDD is characterized by a number of variables `num_vars`.
 * The integer 0, 1, ..., `num_vars` - 1 are used to identify the
 * variables, stored under the alias var_t to specify when an
 * The leaves are identified with the vart_t `num_vars`.
 * A BDD can contain many functions, and each node in the BDD
 * is identified by an unique integer index. This class has some
 * methods to recursively compute functions of BDD indices
 * ( other functions ), while maintaining the BDD of each functionality
 * reduced and ordered by construction.
 * 
 */
class bdd_t
{
public:
  /*! \brief BDD nodes are indexed using unsigned integers */
  using index_t = uint32_t;

  /*! \brief BDD variables are labeled using unsigned integers */
  using var_t = uint32_t;

private:

  /*! \brief Each BDD node ITE( v, T, E ) is specified by ( v, T, E ) */
  struct node_t
  {
    var_t v; /* corresponding variable */
    index_t T; /* index of THEN child */
    index_t E; /* index of ELSE child */
  };

public:
  explicit bdd_t( uint32_t num_vars )
    : unique_table( num_vars ) /* `unique_table` is initialized with `num_vars` empty maps. */
  {
    /* `nodes` is initialized with two `node_t`s representing the terminal (constant) nodes.
     * Their `v` is `num_vars` and their indices are 0 and 1.
     * (Note that the real variables range from 0 to `num_vars - 1`.)
     * Both of their children point to themselves, just for convenient representation.
     *  */
    nodes.emplace_back( node_t( { num_vars, 0, 0 } ) ); /* constant 0 */
    nodes.emplace_back( node_t( { num_vars, 1, 1 } ) ); /* constant 1 */
  }

#pragma region Basic Building Blocks
  uint32_t num_vars() const
  {
    return unique_table.size();
  }

  /*! \brief Get the (index of) constant node. */
  index_t constant( bool value ) const
  {
    return value ? 1 : 0;
  }

  /*! \brief Get the (index of) constant node. */
  bool is_const0( index_t index ) const
  {
    return index == 0;
  }

  /*! \brief Look up (if exist) or build (if not) the node with variable `var`,
   * THEN child `T`, and ELSE child `E`. */
  index_t unique( var_t var, index_t T, index_t E )
  {
    assert( var < num_vars() && "Variables range from 0 to `num_vars - 1`." );
    assert( T < nodes.size() && "Make sure the children exist." );
    assert( E < nodes.size() && "Make sure the children exist." );
    assert( nodes[T].v > var && "With static variable order, children can only be below the node." );
    assert( nodes[E].v > var && "With static variable order, children can only be below the node." );

    /* Reduction rule: Identical children */
    if ( T == E )
    {
      return T;
    }

    /* Look up in the unique table. */
    const auto it = unique_table[var].find( {T, E} );
    if ( it != unique_table[var].end() )
    {
      /* The required node already exists. Return it. */
      return it->second;
    }
    else
    {
      /* Create a new node and insert it to the unique table. */
      index_t const new_index = nodes.size();
      nodes.emplace_back( node_t({var, T, E}) );
      unique_table[var][{T, E}] = new_index;
      return new_index;
    }
  }

  /* Return a node (represented with its index) of function F = x_var or F = ~x_var. */
  index_t literal( var_t var, bool complement = false )
  {
    return unique( var, constant( !complement ), constant( complement ) );
  }
#pragma endregion

#pragma region bdd_t Operations

/* Compute ITE(f, g, h), i.e., f ? g : h,  */
inline index_t create_ite( index_t f, index_t g, index_t h )
{
  assert( f < nodes.size() && "Make sure f exists." );
  assert( g < nodes.size() && "Make sure g exists." );
  assert( h < nodes.size() && "Make sure h exists." );

  if ( f == constant( true ) )
  {
    return g;
  }
  if ( f == constant( false ) )
  {
    return h;
  }
  if ( h == g )
  {
    return h;
  }

  node_t const& F = nodes[f];
  node_t const& G = nodes[g];
  node_t const& H = nodes[h];
  var_t x;
  index_t f0, f1, g0, g1, h0, h1;

  x = std::min( H.v, std::min( F.v, G.v ) );
  if( F.v == x )
  {
    f0 = F.E;
    f1 = F.T;
  }
  else
  {
    f0 = f1 = f;
  }

  if( G.v == x )
  {
    g0 = G.E;
    g1 = G.T;
  }
  else
  {
    g0 = g1 = g;
  }

  if( H.v == x )
  {
    h0 = H.E;
    h1 = H.T;
  }
  else
  {
    h0 = h1 = h;
  }

  index_t const r0 = create_ite( f0, g0, h0 );
  index_t const r1 = create_ite( f1, g1, h1 );
  return unique( x, r1, r0 );
}

/* Compute ~f */
inline index_t create_not( index_t f )
{
  /* TODO: implement the 'create_not' function for 'bdd_t' */
  return constant( false );
}

/* Compute f ^ g */
inline index_t create_xor( index_t f, index_t g )
{
  /* TODO: implement the 'create_xor' function for 'bdd_t' */
  return constant( false );
}

/* Compute f & g */
inline index_t create_and( index_t f, index_t g )
{
  /* TODO: implement the 'create_and' function for 'bdd_t' */
  return constant( false );
}

#pragma endregion

#pragma region Printing and Evaluating
  /* Print the bdd_t rooted at node `f`. */
  void print( index_t f, std::ostream& os = std::cout ) const
  {
    for ( auto i = 0u; i < nodes[f].v; ++i )
    {
      os << "  ";
    }
    if ( f <= 1 )
    {
      os << "node " << f << ": constant " << f << std::endl;
    }
    else
    {
      os << "node " << f << ": var = " << nodes[f].v << ", T = " << nodes[f].T 
         << ", E = " << nodes[f].E << std::endl;
      for ( auto i = 0u; i < nodes[f].v; ++i )
      {
        os << "  ";
      }
      os << "> THEN branch" << std::endl;
      print( nodes[f].T, os );
      for ( auto i = 0u; i < nodes[f].v; ++i )
      {
        os << "  ";
      }
      os << "> ELSE branch" << std::endl;
      print( nodes[f].E, os );
    }
  }

  /* Get the truth table of the bdd_t rooted at node f. */
  truth_table_t get_tt( index_t f ) const
  {
    assert( f < nodes.size() && "Make sure f exists." );
    assert( num_vars() <= 6 && "truth_table_t only supports functions of no greater than 6 variables." );

    if ( f == constant( false ) )
    {
      return truth_table_t( num_vars() );
    }
    else if ( f == constant( true ) )
    {
      return ~truth_table_t( num_vars() );
    }
    
    /* Shannon expansion: f = x f_x + x' f_x' */
    var_t const x = nodes[f].v;
    index_t const fx = nodes[f].T;
    index_t const fnx = nodes[f].E;
    truth_table_t const tt_x = create_tt_nth_var( num_vars(), x );
    truth_table_t const tt_nx = create_tt_nth_var( num_vars(), x, false );
    return ( tt_x & get_tt( fx ) ) | ( tt_nx & get_tt( fnx ) );
  }

  /* Get the number of nodes in the whole package (whether used or not), excluding constants. */
  uint64_t num_nodes() const
  {
    return nodes.size() - 2;
  }

  /* Get the number of nodes in the sub-graph rooted at node f, excluding constants. */
  uint64_t num_nodes( index_t f ) const
  {
    assert( f < nodes.size() && "Make sure f exists." );

    if ( f == constant( false ) || f == constant( true ) )
    {
      return 0u;
    }

    std::vector<bool> visited( nodes.size(), false );
    visited[0] = true;
    visited[1] = true;

    return num_nodes_rec( f, visited );
  }
#pragma endregion

private:
#pragma region Helper Functions
  uint64_t num_nodes_rec( index_t f, std::vector<bool>& visited ) const
  {
    assert( f < nodes.size() && "Make sure f exists." );
    

    uint64_t n = 0u;
    node_t const& F = nodes[f];
    assert( F.T < nodes.size() && "Make sure the children exist." );
    assert( F.E < nodes.size() && "Make sure the children exist." );
    if ( !visited[F.T] )
    {
      n += num_nodes_rec( F.T, visited );
      visited[F.T] = true;
    }
    if ( !visited[F.E] )
    {
      n += num_nodes_rec( F.E, visited );
      visited[F.E] = true;
    }
    return n + 1u;
  }
#pragma endregion

private:
  std::vector<node_t> nodes;

  /*! \brief `unique_table` is a vector of `num_vars` maps storing the built nodes of each variable.
   * 
   * Each map maps from a pair of node indices (T, E) to a node index, if it exists.
   * See the implementation of `unique` for example usage.
   * 
   */
  std::vector<std::unordered_map<std::pair<index_t, index_t>, index_t>> unique_table;
};

} // namespace dtis
