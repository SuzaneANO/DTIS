#include "truth_table.hpp"
#include "bdd.hpp"
#include "aig.hpp"
#include "checker.hpp"
#include "reader.hpp"

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;
using namespace dtis;

void check( truth_table_t const& tt, string const& ans )
{
  cout << "  checking function correctness";
  if ( tt == truth_table_t( ans ) )
  {
    cout << "...passed." << endl;
  }
  else
  {
    cout << "...failed. (expect " << ans << ", but get " << tt << ")" << endl;
  }
}

void check( uint64_t dd_size, uint64_t expected )
{
  cout << "  checking bdd size";
  if ( dd_size == expected )
  {
    cout << "...passed." << endl;
  }
  else
  {
    cout << "...failed. (expect " << expected << ", but get " << dd_size << " nodes)" << endl;
  }
}

template<class T>
void check( T value, T expected )
{
  cout << "  checking equivalence";
  if ( value == expected )
  {
    cout << "...passed." << endl;
  }
  else
  {
    cout << "...failed. (expect " << expected << ", but get " << value << " )" << endl;
  }
}

int main()
{
  {
    cout << "test 00: x0 XOR x1" << endl;
    bdd_t bdd( 2 );
    auto const x0 = bdd.literal( 0 );
    auto const x1 = bdd.literal( 1 );
    auto const f = bdd.create_xor( x0, x1 );
    auto const tt = bdd.get_tt( f );
    bdd.print( f );
    cout << tt << endl;
    check( tt, "0110" );
    check( bdd.num_nodes( f ), 3 );
  }

  {
    cout << "test 01: x0 AND x1" << endl;
    bdd_t bdd( 2 );
    auto const x0 = bdd.literal( 0 );
    auto const x1 = bdd.literal( 1 );
    auto const f = bdd.create_and( x0, x1 );
    auto const tt = bdd.get_tt( f );
    bdd.print( f );
    cout << tt << endl;
    check( tt, "1000" );
    check( bdd.num_nodes( f ), 2 );
  }

  {
    cout << "test 02: ITE(x0, x1, x2)" << endl;
    bdd_t bdd( 3 );
    auto const x0 = bdd.literal( 0 );
    auto const x1 = bdd.literal( 1 );
    auto const x2 = bdd.literal( 2 );
    auto const f = bdd.create_ite( x0, x1, x2 );
    auto const tt = bdd.get_tt( f );
    bdd.print( f );
    cout << tt << endl;
    check( tt, "11011000" );
    check( bdd.num_nodes( f ), 3 );
  }

  {
    cout << "test 03: Equivalent, single output" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test00A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test00B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, true );
    check( cec.perm[0], 0 );
  }

  {
    cout << "test 04: Not equivalent, single output" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test01A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test01B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, false );
    check( cec.perm[0], -1 );
  }

  {
    cout << "test 05: Equivalent, two-outputs" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test02A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test02B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, true );
    check( cec.perm[0], 0 );
    check( cec.perm[1], 1 );
  }

  {
    cout << "test 06: Equivalent, two-outputs permuted" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test03A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test03B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, true );
    check( cec.perm[0], 1 );
    check( cec.perm[1], 0 );
  }

  {
    cout << "test 07: Not equivalent, two-outputs - case 1" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test04A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test04B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, false );
    check( cec.perm[0], 0 );
    check( cec.perm[1], -1 );
  }

  {
    cout << "test 08: Not equivalent, two-outputs - case 2" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test05A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test05B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, false );
    check( cec.perm[0], -1 );
    check( cec.perm[1], 0 );
  }

  {
    cout << "test 09: Equivalent, 4-outputs" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test06A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test06B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, true );
    check( cec.perm[0], 2 );
    check( cec.perm[1], 0 );
    check( cec.perm[2], 1 );
    check( cec.perm[3], 3 );
  }

  {
    cout << "test 10: Not equivalent, 6-outputs" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    if( !read_aiger( aig1, "benchmarks/test07A.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }
    if( !read_aiger( aig2, "benchmarks/test07B.aig" ) )
    {
      std::cout << "[e] Parsing error" << std::endl;
      return 1;
    }

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, false );
    check( cec.perm[0], 5 );
    check( cec.perm[1], 4 );
    check( cec.perm[2], 3 );
    check( cec.perm[3], 2 );
    check( cec.perm[4], 1 );
    check( cec.perm[5], -1 );
  }

  /* Some tests building simple AIGs */
  {
    cout << "test 11: Equivalent AIGs" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    auto const a1 = aig1.create_pi();
    auto const b1 = aig1.create_pi();
    auto const a2 = aig2.create_pi();
    auto const b2 = aig2.create_pi();
    aig1.create_po( a1 );
    aig1.create_po( b1 );
    aig2.create_po( b2 );
    aig2.create_po( a2 );

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, true );
    check( cec.perm[0], 1 );
    check( cec.perm[1], 0 );
  }

  {
    cout << "test 12: Not equivalent AIGs" << endl;
    aig_network_t aig1;
    aig_network_t aig2;
    auto const a1 = aig1.create_pi();
    auto const b1 = aig1.create_pi();
    auto const c1 = aig1.create_pi();
    auto const a2 = aig2.create_pi();
    auto const b2 = aig2.create_pi();
    auto const c2 = aig2.create_pi();
    aig1.create_po( a1 );
    aig1.create_po( c1 );
    aig2.create_po( b2 );
    aig2.create_po( a2 );

    auto const cec = check_equivalence( aig1, aig2 );
    check( cec.equivalent, false );
    check( cec.perm[0], 1 );
    check( cec.perm[1], -1 );
  }


  return 0;
}

