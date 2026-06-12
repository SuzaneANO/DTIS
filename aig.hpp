#pragma once

#include <array>
#include <vector>
#include <unordered_map>

#include "truth_table.hpp"

namespace dtis
{
/*! \brief Simple And-Inverter Graph
 * 
 * This data structure implements a simple Boolean network
 * in which each node is a two-input AND, and each edge can
 * be complemented. Complemented edges indicate signal
 * inversion.
 * 
 */
class aig_network_t
{
public:

#pragma region Data Structures
  /*! \brief Network's edge, possibly inverted
  *
  * This data structure contains an index to a node, and
  * a complemented attribute to account for inversion of
  * the node's functionality.
  * 
  */
  struct signal_t
  {
    signal_t() = default;

    signal_t( uint32_t index, bool complemented )
      : index( index ),
        complemented( complemented )
    {}

    /*! \brief Invert a signal */
    signal_t operator!() const
    {
      return { index, !complemented };
    }

    /*! \brief Check if two signals are equivalent */
    bool operator==( signal_t const& other ) const
    {
      return index == other.index && complemented == other.complemented;
    }

    /*! \brief Check if two signals are different */
    bool operator!=( signal_t const& other ) const
    {
      return !operator==( other );
    }

    /*! \brief Check if a signal on the left is smaller than the signal on the right */
    bool operator<( signal_t const& other ) const
    {
      return index < other.index && complemented < other.complemented;
    }

    /*! \brief Index of the node the signal points to */
    uint32_t index;

    /*! \brief True if the edge has an inverter */
    bool complemented;
  };

  /*! \brief Network's node
  *
  * This data structure implements a network's node
  * as uniquely identified by an ordered pair of signals
  * that are its fanins. 
  * 
  */
  struct node_t
  {
    /*! \brief True if two nodes are equivalent */
    bool operator==( node_t const& other ) const
    {
      return children == other.children;
    }

    /*! \brief True if two nodes are different */
    bool operator!=( node_t const& other ) const
    {
      return !operator==( other );
    }

    /*! \brief Input signals of the AND gate */
    std::array<signal_t, 2u> children;
  };

  /*! \brief Hashing function for the AIG nodes */
  struct node_hash
  {
    uint64_t operator()( node_t n ) const noexcept
    {
      uint64_t seed = -2011;
      seed += n.children[0].index * 7937;
      seed += n.children[1].index * 2971;
      seed += uint32_t( n.children[0].complemented ) * 911;
      seed += uint32_t( n.children[1].complemented ) * 353;
      return seed;
    }
  };
#pragma endregion Data Structures

#pragma region Constructors

  explicit aig_network_t()
  {
    /* constant false */
    nodes.emplace_back();
    /* the inputs of a constant 0 are one the complment of the other */
    nodes[0].children[0] = { 1u << 31u, true };
    nodes[0].children[1] = { 1u << 31u, false };
  }

#pragma endregion Constructors

#pragma region Properties

  /*! \brief Returns the number of inputs ( PIs ) */
  uint64_t num_pis() const
  {
    return inputs.size();
  }

  /*! \brief Returns the number of outputs ( POs ) */
  uint64_t num_pos() const
  {
    return outputs.size();
  }

  /*! \brief Returns the number of nodes
   *
   * The nodes count includes PIs, constant 0, and AND gates
   *  
  */
  uint64_t num_nodes() const
  {
    return nodes.size();
  }

  /*! \brief Returns the number of AND gates */
  uint64_t num_gates() const
  {
    return nodes.size() - inputs.size() - 1u;
  }

  /*! \brief PIs are identified by equal inputs, with the same polarity */
  bool is_pi( node_t n ) const
  {
    return n.children[0] == n.children[1];
  }

  /*! \brief The constant 0 node hase inputs with the same index and opposite polarity */
  bool is_constant( node_t n ) const
  {
    return n.children[0] == !n.children[1];
  }

#pragma endregion

#pragma region Getter Methods

  /*! \brief Returns the constant signal */
  signal_t get_constant( bool value ) const
  {
    return signal_t{ 0, value };
  }

  /*! \brief Returns the signal of the PO at the given index */
  signal_t get_po( uint32_t index ) const
  {
    return outputs[index];
  }

  /*! \brief Returns the node stored at the given index */
  node_t get_node( uint32_t index ) const
  {
    return nodes[index];
  }

  /*! \brief Returns the node to which the signal points to */
  node_t get_node( signal_t f ) const
  {
    return get_node( f.index );
  }

  uint32_t get_index( node_t const& n ) const
  {
    auto const it = hash.find( n );
    if ( it == hash.end() )
    {
      std::cerr << "[e] node not found" << std::endl;
      return -1;
    }
    return it->second;
  }

  /*! \brief Returns the PI node at a given index */
  node_t pi_at( uint32_t index ) const
  {
    return nodes[inputs[index]];
  }

#pragma endregion Getter Methods

#pragma region Nodes Creation

  /*! \brief Create a PI, which is a node with equal children */
  signal_t create_pi()
  {
    uint32_t const index = nodes.size();
    nodes.emplace_back();
    auto& node = nodes[index];
    node.children[0].index = node.children[1].index = inputs.size();
    hash.emplace( node, index );
    inputs.emplace_back( index );
    
    return { index, false };
  }

  /*! \brief A PO is a signal that is stred as an output */
  void create_po( signal_t f )
  {
    outputs.emplace_back( f );
  }

  /*! \brief Create an inverter */
  signal_t create_not( signal_t f )
  {
    return !f;
  }

  /*! \brief Create an AND gate
   *
   * The input signals are sorted for achieving node-level canonicity.
   * Next, the method checks for trivial cases, like constant inputs,
   * that would replace node creation with constant or signal propagation.
   * Finally, the method checks if the node is already present in the
   * network, and adds it otherwise.
   * 
  */
  signal_t create_and( signal_t a, signal_t b )
  {
    /* sort the inputs to ensure node-level canonicity */
    if( b < a )
    {
      std::swap( a, b );
    }

    /* trivial cases */
    if( a.index == b.index ) /* AND( a, a ) = a */
    {
      return ( a.complemented == b.complemented ) ? a : get_constant( false );
    }
    else if( a.index == 0 ) /* AND( 0, b ) = 0  AND( 1, b ) = b */
    {
      return a.complemented ? b : get_constant( false );
    }

    node_t n;
    n.children[0] = a;
    n.children[1] = b;

    /* check if the node is already in the network */
    auto const it = hash.find( n );
    if ( it != hash.end() )
    {
      return { it->second, false };
    }

    /* add the node to the network */
    uint32_t const index = nodes.size();
    nodes.emplace_back( n );
    n.children = { a, b };
    hash.emplace( n, index );
    return { index, false };
  }

#pragma endregion Nodes Creation

#pragma region Iterators

  /*! \brief iterator to apply a lambda to all the nodes */
  template<typename Fn>
  void foreach_po( Fn&& fn ) const
  {
    for ( int i = 0; i < outputs.size(); i++ )
    {
      fn( outputs[i], i );
    }
  }

  /*! \brief Iterator to apply a lambda function to all the AND gates */
  template<typename Fn>
  void foreach_gate( Fn&& fn ) const
  {
    for ( int i = 1; i < nodes.size(); i++ )
    {
      if( is_pi( nodes[i] )  )
        continue;

      fn( nodes[i], i );
    }
  }

  /*! \brief Iterator to apply a lambda function to all the nodes */
  template<typename Fn>
  void foreach_node( Fn&& fn ) const
  {
    for ( int i = 0; i < nodes.size(); i++ )
    {
      fn( nodes[i], i );
    }
  }

  /*! \brief Iterator to apply a lambda function to all the PIs */
  template<typename Fn>
  void foreach_pi( Fn&& fn ) const
  {
    for ( int i = 0; i < inputs.size(); i++ )
    {
      fn( inputs[i], i );
    }
  }

#pragma endregion Iterators

#pragma region Simulation

  /*! \brief Evaluate a node given simulation patterns at its inputs. */
  truth_table_t compute( node_t const& n, truth_table_t const& tt1, truth_table_t const& tt2  ) const
  {
    assert( ( !is_constant( n ) && !is_pi( n ) ) );
    
    /* get the input signals */
    auto const& c1 = n.children[0];
    auto const& c2 = n.children[1];

    assert( ( tt1.n_var() == tt2.n_var() ) );

    return ( c1.complemented ? ~( tt1 ) : tt1 ) & ( c2.complemented ? ~( tt2 ) : tt2 );
  }

#pragma endregion Simulation

protected:
  /*! \brief Vector containing the constant node, the PIs, and the AND gates */
  std::vector<node_t> nodes;

  /*! \brief inputs[i] contains the index to access the i-th PI in nodes */
  std::vector<uint32_t> inputs;

  /*! \brief Each signal identifies a PO */
  std::vector<signal_t> outputs;

  /*! \brief Hash map for structural hashing */
  std::unordered_map<node_t, uint32_t, node_hash> hash;
};

} // namespace dtis
