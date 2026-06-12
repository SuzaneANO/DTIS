#pragma once

#include "bdd.hpp"
#include "aig.hpp"
#include "simulator.hpp"

namespace dtis
{

/*! \brief Data structure storing the result of equivalence checking
 *
 * There are two attributes:
 * - perm      : is a vector storing the output permutations.
 *               perm[i] = j implies that the output i of the
 *               first network is equivalent to the output j
 *               of the second network. If PO[i] of the first
 *               network is not equivalent to any PO of the
 *               second network, perm[i] = -1
 * - equivalent: true if each output of the first network is
 *               is equivalent to at least one unique output
 *               of the second network.
 * 
 */
struct check_result
{
  check_result( uint32_t num_pos, bool equivalent = false )
  : perm( num_pos, -1 ),
    equivalent( equivalent )
  {}

  /*! \brief Reverence to a permutation entry for storing permutations */
  int & operator[]( uint32_t index )
  {
    return perm[index];
  }

  /*! \brief True if the networks are equivalent */
  bool equivalent;

  /*! \brief perm[i] = j if PO1[i]=PO2[j], if j = -1 there is no PO2 equivalent to PO1[i] */
  std::vector<int> perm;
};

/*! \brief Implements a simulation-guided, BDD based equivalence checker.
 *
 * This data structures verifies if two networks are equivalent.
 * For each network, a network manager ( `ntk_manager` ) is constructed.
 * This object has the following attributes:
 * - `aig`         : a constant reference to the AIG
 * - `sim`         : a simulator assigning a 64-bits simulation signature to each node
 * - `node_to_lit` : an hash table to store the mapping of an AIG node to a BDD index
 * 
 */
class checker
{
  using node_t = aig_network_t::node_t;
  using signal_t = aig_network_t::signal_t;
  using index_t = bdd_t::index_t;
  using node_hash = aig_network_t::node_hash;
  using node_map_t = std::unordered_map<node_t, index_t, node_hash>;

  struct ntk_manager
  {
    ntk_manager( aig_network_t const& aig )
    : aig( aig ),
      sim( aig ) 
    {}
  
    node_map_t node_to_lit;
    simulator_t sim;
    aig_network_t const& aig;
  };

public:
  explicit checker( aig_network_t const& aig1, aig_network_t const& aig2 ) :
    num_pis( aig1.num_pis() ),
    num_pos( aig1.num_pos() ),
    bdd( num_pis ),
    ntk1( aig1 ),
    ntk2( aig2 )
  {}

  check_result run()
  {
    auto const& aig1 = ntk1.aig;
    auto const& aig2 = ntk2.aig;

    /* trivial cases */
    if( ( aig1.num_pis() != aig2.num_pis() ) || ( aig1.num_pos() != aig2.num_pos() ) )
    {
      check_result const res( num_pos );
      return res;
    }

    /* define the inputs */
    initialize_map( ntk1 );
    initialize_map( ntk2 );

    /* add the AIG nodes */
    construct_gates( ntk1 );
    construct_gates( ntk2 );

    /* check equivalence */
    return check();
  }
private:
  /*! \brief map the PIs of the networks to the bdd's literals */
  void initialize_map( ntk_manager& ntk )
  {
    aig_network_t const& aig = ntk.aig;
    node_map_t & map = ntk.node_to_lit;

    aig.foreach_pi( [&]( uint32_t const& index, uint32_t i ){
      node_t const& n = aig.get_node( index );
      map[n] = bdd.literal( i );
    } );
  }

  /*! \brief construct the BDD from the AIG nodes */
  void construct_gates( ntk_manager & ntk )
  {
    aig_network_t const& aig = ntk.aig;
    node_map_t & map = ntk.node_to_lit;

    /* TODO: implement the 'construct_gates' function */
  }

  check_result check()
  {
    /* TODO: implement the 'check' function */
    return { num_pos, false };
  }

  uint32_t num_pis;
  uint32_t num_pos;
  bdd_t bdd;
  ntk_manager ntk1;
  ntk_manager ntk2;
  
};

/*! \brief Check equivalence combining signatures and BDDs */
check_result check_equivalence( aig_network_t const& aig1, aig_network_t const& aig2 )
{
  checker cec( aig1, aig2 );
  return cec.run();
}

}; // namespace dtis