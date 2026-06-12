#pragma once

#include "aig.hpp"
#include "truth_table.hpp"

namespace dtis
{

/*! \brief Simulator to assign a simulation pattern to each node
 *
 * If the network has at most 6 inputs, the simulaiton is exhaustive.
 * Otherwise we assign a random input pattern to each PI and simulate
 * the network based on these 64-bits input signatures, labeling each
 * node with a 64 bits signature.
 * 
*/
class simulator_t
{
public:
  using node_t = aig_network_t::node_t;

  explicit simulator_t( aig_network_t const & aig )
   : aig( aig )
  {
    /* Reserve enough space to store all the signatures */
    sims.reserve( aig.num_nodes() );

    /* Initialize based on the number of inputs */
    if ( aig.num_pis() <= 6u )
    {
      default_init();
    }
    else
    {
      random_init();
    }

    /* Simulate network */
    simulate();
  }

private:
  /*! \brief Assign to each PI a complete projection function */
  void default_init()
  {
    uint32_t const num_vars = aig.num_pis();

    /* Add the constant signature */
    sims.emplace_back( num_vars );

    /* Add the complete projection functions for the inputs */
    for( auto i = 0; i < num_vars; ++i )
    {
      sims.emplace_back( create_tt_nth_var( num_vars, i ) );
    }
  }

  /*! \brief Assign to each PI a random truth table */
  void random_init()
  {
    uint32_t const num_vars = std::min( ( uint64_t )6u, aig.num_pis() );

    /* Add the constant signature */
    sims.emplace_back( num_vars );

    /* Sample a random pattern for each input */
    for( auto i = 0; i < aig.num_pis(); ++i )
    {
      sims.emplace_back( create_random( num_vars, i+1 ) );
    }
  }

  /*! \brief Explore the network in topological order and simulates each node */
  void simulate()
  {
    aig.foreach_gate( [&]( node_t n, uint32_t i ){
      auto const& tt1 = sims[n.children[0].index]; 
      auto const& tt2 = sims[n.children[1].index]; 
      sims.emplace_back( aig.compute( n, tt1, tt2 ) );
    } );
  }

public:
  truth_table_t const& get_tt( node_t n ) const
  {
    uint32_t const index = aig.get_index( n );
    return sims[index];
  }

  truth_table_t const& operator[]( node_t n ) const
  {
    return get_tt( n );
  }

private:
  /*! \brief The simulated AIG */
  aig_network_t const& aig;

  /*! \brief The simulation patterns, one per each node */
  std::vector<truth_table_t> sims;
};

} // namespace dtis