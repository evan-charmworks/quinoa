//******************************************************************************
/*!
  \file      src/RNGTest/TestU01Props.h
  \author    J. Bakosi
  \date      Sat 28 Jun 2014 10:10:05 PM MDT
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     TestU01 statistical test properties
  \details   TestU01 statistical test properties
*/
//******************************************************************************
#ifndef TestU01Props_h
#define TestU01Props_h

#include <vector>
#include <functional>

#include <RNG.h>
#include <Options/RNG.h>
#include <TestU01Util.h>
#include <TestStack.h>

namespace rngtest {

extern TestStack g_testStack;

template< tk::ctr::RawRNGType id >
extern unif01_Gen* createTestU01Gen( const std::string& name );

//! TestU01 properties used to initialize TestU01 tests. Used to abstract away
//! the initialization for TestU01 statistical tests. Needed because of the move
//! semantics and variadic templates since Charm++ chares do not support these
//! yet. TestU01Props is therefore not a chare, but TestU01, initialized with a
//! TestU01Props object, is. Note that TestU01Props still needs to be
//! migratable, i.e., defines the pack/unpack operator, as it is an argument to
//! chare TestU01's constructor.
template< class Test,                   //!< Statistical test tag, struct{}
          class Proxy,                  //!< Host proxy
          class Result,                 //!< Results type
          Result* (*Creator)(void),     //!< Results creator
          void (*Deleter)(Result *),    //!< Results deleter
          typename... Ts >              //!< Extra runner args types
class TestU01Props {

  public:
    //! Test extra arguments type
    using Xargs = std::tuple< Ts... >;
    //! Test runner function pointer type
    using RunFn = std::vector<double> (*)( unif01_Gen*, Result*, const Xargs& );
    //! TestU01 results type with a custom deleter by TestU01
    using ResultPtr = TestU01Ptr< Result, Deleter >;

    //! Constructor taking no args, required as migratable, init what we can
    explicit TestU01Props() :
      m_rng( tk::ctr::RNGType::NO_RNG ),
      m_runner( g_testStack.TestU01.runner.get<Test>() ),
      m_res( ResultPtr(Creator()) ) {}

    //! Constructor
    explicit TestU01Props( Proxy& proxy,
                           tk::ctr::RNGType rng,
                           std::vector< std::string >&& names,
                           unif01_Gen* gen,
                           Ts&&... xargs ) :
      m_proxy( proxy ),
      m_rng( rng ),
      m_names( std::move(names) ),
      m_xargs( std::forward<Ts>(xargs)... ),
      m_gen( gen ),
      m_runner( g_testStack.TestU01.runner.get<Test>() ),
      m_res( ResultPtr(Creator()) ) {}

    //! Copy assignment
    TestU01Props& operator=( const TestU01Props& x) {
      m_proxy = x.m_proxy;
      m_rng = x.m_rng;
      m_names = x.m_names;
      m_xargs = x.m_xargs;
      m_gen = x.m_gen;
      m_runner = x.m_runner;
      m_res = ResultPtr(Creator());
      return *this;
    }
    //! Copy constructor: in terms of copy assignment
    TestU01Props( const TestU01Props& x ) { operator=(x); }
    //! Move assignment
    TestU01Props& operator=( TestU01Props&& ) = default;
    //! Move constructor
    TestU01Props( TestU01Props&& ) = default;

    //! Pack/Unpack
    void pup( PUP::er& p ) {
      p | m_proxy;
      using tk::operator|;
      p | m_rng;
      p | m_names;
      p | m_xargs;
      if (p.isUnpacking()) {
        pup( p, m_gen );
        m_runner = g_testStack.TestU01.runner.get<Test>();
        m_res = ResultPtr(Creator());
      }
    }
    friend void operator|( PUP::er& p, TestU01Props& c ) { c.pup(p); }

    //! Host proxy accessor
    Proxy& proxy() noexcept { return m_proxy; }

    //! Run test and return its status as a vector of vector of strings. The
    //! return type could potentially be a more specific type, e.g., a struct
    //! with fields RNGType, and a vector of strings for the p-values, and the
    //! names. Instead we return a more generic vector of vector of strings
    //! type. This helps keeping the corresponding argument to Battery::
    //! evaluate() generic, which may come in handy when other test libraries
    //! are added in the future. The return value is thus the following in a
    //! vector of vectors
    //! 0: Name(s) of statistics (note that a test may produce more than one
    //!    statistics and thus p-values)
    //! 1: p-value strings: "pass" or "fail, p-value = ..." for all tests run
    //!    (note that a test may produce more than one statistics and thus
    //!    p-values)
    //! 2: RNG name used to run the test: sub-vector length = 1
    std::vector< std::vector< std::string > > run() {
      const auto pvals = m_runner( m_gen, m_res.get(), m_xargs );
      std::vector< std::string > pvalstrs;
      for (std::size_t p=0; p<m_names.size(); ++p) {
        if ( fail(pvals[p]) )
          pvalstrs.emplace_back( "fail, p-value = " + pval( pvals[p] ) );
        else
          pvalstrs.emplace_back( "pass" );
      }
      return { m_names, pvalstrs, { tk::ctr::RNG().name(m_rng) } };
    }

    //! Number of results/test (i.e., p-values) accessor
    std::size_t npval() const { return m_names.size(); }

    //! Test name(s) accessor
    std::string names() const {
      // serialize vector of strings into a single semi-colon-separated string
      std::string names;
      for (const auto& n : m_names) names += n + ";";
      return names;
    }

//     //! Test name accessor
//     std::string name( std::size_t p ) const {
//       Assert( p < m_names.size(), tk::ExceptType::FATAL,
//               "Indexing outside of container bounds in TestU01Props::name()" );
//       return m_names[p];
//     }
// 
//     //! RNG name accessor
//     std::string rngname() const { return m_rngname; }

    //! Query whether test is failed
    bool fail( double val ) const {
      if ((val <= gofw_Suspectp) || (val >= 1.0-gofw_Suspectp))
        return true;
      else
        return false;
    }

  private:
    //! Pack/Unpack TestU01 external generator pointer
    void pup( PUP::er& p, unif01_Gen*& g ) {
      // Admittedly, the code below is ugly and looks stupid at first sight.
      // However, this is a translation of runtime information (a user-selected
      // RNG that this statistical test exercises) to compile-time information:
      // associating an RNG id from an enum class, tk::ctr::RNGType::value, to a
      // compile-time constant, underlying_type value, facilitating a different
      // global-scope TestU01 external generator with code reuse. Note that
      // createTestU01Gen must be global-scope as it is used to create a
      // different external generator to TestU01, depending on the RNG.
      // Templating createTestU01Gen on the id enables the compiler generate a
      // different wrapper for a different RNG facilitating simultaneous calls
      // to any or all wrappers as they are unique functions.
      using tk::ExceptType;
      using tk::ctr::RNGType;
      using tk::ctr::raw;
      const auto& rngname = tk::ctr::RNG().name(m_rng);
      Assert(!rngname.empty(), ExceptType::FATAL, "Empty RNG name not allowed");
      Assert(m_rng != RNGType::NO_RNG, ExceptType::FATAL, "No RNG type allowed");
      #ifdef HAS_MKL
      if (m_rng == RNGType::MKL_MCG31)
        g = createTestU01Gen< raw(RNGType::MKL_MCG31) >( rngname );
      else if (m_rng == RNGType::MKL_R250)
        g = createTestU01Gen< raw(RNGType::MKL_R250) >( rngname );
      else if (m_rng == RNGType::MKL_MRG32K3A)
        g = createTestU01Gen< raw(RNGType::MKL_MRG32K3A) >( rngname );
      else if (m_rng == RNGType::MKL_MCG59)
        g = createTestU01Gen< raw(RNGType::MKL_MCG59) >( rngname );
      else if (m_rng == RNGType::MKL_WH) 
        g = createTestU01Gen< raw(RNGType::MKL_WH) >( rngname );
      else if (m_rng == RNGType::MKL_MT19937)
        g = createTestU01Gen< raw(RNGType::MKL_MT19937) >( rngname );
      else if (m_rng == RNGType::MKL_MT2203)
        g = createTestU01Gen< raw(RNGType::MKL_MT2203) >( rngname );
      else if (m_rng == RNGType::MKL_SFMT19937)
        g = createTestU01Gen< raw(RNGType::MKL_SFMT19937) >( rngname );
      else if (m_rng == RNGType::MKL_SOBOL)
        g = createTestU01Gen< raw(RNGType::MKL_SOBOL) >( rngname );
      else if (m_rng == RNGType::MKL_NIEDERR)
        g = createTestU01Gen< raw(RNGType::MKL_NIEDERR) >( rngname );
      //else if (m_rng == RNGType::MKL_IABSTRACT)
      //  g = createTestU01Gen< raw(RNGType::MKL_IABSTRACT) >( rngname );
      //else if (m_rng == RNGType::MKL_DABSTRACT)
      //  g = createTestU01Gen< raw(RNGType::MKL_DABSTRACT) >( rngname );
      //else if (m_rng == RNGType::MKL_SABSTRACT)
      //  g = createTestU01Gen< raw(RNGType::MKL_SABSTRACT) >( rngname );
      else if (m_rng == RNGType::MKL_NONDETERM)
        g = createTestU01Gen< raw(RNGType::MKL_NONDETERM) >( rngname );
      #endif
      else if (m_rng == RNGType::RNGSSE_GM19)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GM19) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GM29)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GM29) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GM31)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GM31) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GM55)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GM55) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GM61)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GM61) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GQ581)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GQ581) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GQ583)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GQ583) >( rngname );
      else if (m_rng == RNGType::RNGSSE_GQ584)
        g = createTestU01Gen< raw(RNGType::RNGSSE_GQ584) >( rngname );
      else if (m_rng == RNGType::RNGSSE_MT19937)
        g = createTestU01Gen< raw(RNGType::RNGSSE_MT19937) >( rngname );
      else if (m_rng == RNGType::RNGSSE_LFSR113)
        g = createTestU01Gen< raw(RNGType::RNGSSE_LFSR113) >( rngname );
      else if (m_rng == RNGType::RNGSSE_MRG32K3A)
        g = createTestU01Gen< raw(RNGType::RNGSSE_MRG32K3A) >( rngname );
    }

    //! Return humand-readable p-value (ala TestU01::bbattery.c::WritePval)
    std::string pval( double val ) const {
      std::stringstream ss;
      if (val < gofw_Suspectp) {

        if ((val >= 0.01) && (val <= 0.99))
          ss << val;
        else if (val < gofw_Epsilonp)
          ss << "eps";
        else if (val < 0.01)
          ss << val;
        else if (val >= 1.0 - gofw_Epsilonp1)
          ss << "1 - eps1";
        else if (val < 1.0 - 1.0e-4)
          ss << val;
        else
          ss << 1.0 - val;

      } else if (val > 1.0 - gofw_Suspectp) {

        if (val >= 1.0 - gofw_Epsilonp1)
          ss << "1 - eps1";
        else if (val >= 1.0 - 1.0e-4)
          ss << "1 - " << 1.0 - val;
        else
          ss << val;

      }
      return ss.str();
    }

    Proxy m_proxy;                      //!< Host proxy
    tk::ctr::RNGType m_rng;             //!< RNG id
    std::vector< std::string > m_names; //!< name(s) of tests
    Xargs m_xargs;                      //!< extra args for run()
    unif01_Gen* m_gen;                  //!< pointer to generator
    RunFn m_runner;                     //!< test runner function
    ResultPtr m_res;                    //!< testU01 results
};

} // rngtest::

#endif // TestU01Props_h