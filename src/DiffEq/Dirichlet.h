// *****************************************************************************
/*!
  \file      src/DiffEq/Dirichlet.h
  \copyright 2012-2015, J. Bakosi, 2016-2018, Los Alamos National Security, LLC.
  \brief     Dirichlet SDE
  \details   This file implements the time integration of a system of stochastic
    differential equations (SDEs), whose invariant is the [Dirichlet
    distribution](http://en.wikipedia.org/wiki/Dirichlet_distribution).

    In a nutshell, the equation integrated governs a set of scalars,
    \f$0\!\le\!Y_\alpha\f$, \f$\alpha\!=\!1,\dots,N-1\f$,
    \f$\sum_{\alpha=1}^{N-1}Y_\alpha\!\le\!1\f$, as
    \f[
       \mathrm{d}Y_\alpha(t) = \frac{b_\alpha}{2}\big[S_\alpha Y_N -
       (1-S_\alpha)Y_\alpha\big]\mathrm{d}t + \sqrt{\kappa_\alpha Y_\alpha
       Y_N}\mathrm{d}W_\alpha(t), \qquad \alpha=1,\dots,N-1
    \f]
    with parameter vectors \f$b_\alpha > 0\f$, \f$\kappa_\alpha > 0\f$, and \f$0
    < S_\alpha < 1\f$, and \f$Y_N = 1-\sum_{\alpha=1}^{N-1}Y_\alpha\f$. Here
    \f$\mathrm{d}W_\alpha(t)\f$ is an isotropic vector-valued [Wiener
    process](http://en.wikipedia.org/wiki/Wiener_process) with independent
    increments. The invariant distribution is the Dirichlet distribution,
    provided the parameters of the drift and diffusion terms satisfy
    \f[
      (1-S_1) b_1 / \kappa_1 = \dots = (1-S_{N-1}) b_{N-1} / \kappa_{N-1}.
    \f]
    To keep the invariant distribution Dirichlet, the above constraint on the
    coefficients must be satisfied. For more details on the Dirichlet SDE, see
    https://doi.org/10.1155/2013/842981.
*/
// *****************************************************************************
#ifndef Dirichlet_h
#define Dirichlet_h

#include <vector>
#include <cmath>

#include "InitPolicy.h"
#include "DirichletCoeffPolicy.h"
#include "RNG.h"
#include "Particles.h"
#include "SystemComponents.h"

namespace walker {

extern ctr::InputDeck g_inputdeck;
extern std::map< tk::ctr::RawRNGType, tk::RNG > g_rng;

//! \brief Dirichlet SDE used polymorphically with DiffEq
//! \details The template arguments specify policies and are used to configure
//!   the behavior of the class. The policies are:
//!   - Init - initialization policy, see DiffEq/InitPolicy.h
//!   - Coefficients - coefficients policy, see DiffEq/DirCoeffPolicy.h
template< class Init, class Coefficients >
class Dirichlet {

  private:
    using ncomp_t = tk::ctr::ncomp_type;

  public:
    //! \brief Constructor
    //! \param[in] c Index specifying which Dirichlet SDE to construct. There
    //!   can be multiple dirichlet ... end blocks in a control file. This index
    //!   specifies which Dirichlet SDE to instantiate. The index corresponds to
    //!   the order in which the dirichlet ... end blocks are given the control
    //!   file.
    explicit Dirichlet( ncomp_t c ) :
      m_c( c ),
      m_depvar( g_inputdeck.get< tag::param, tag::dirichlet, tag::depvar >().at(c) ),
      m_ncomp( g_inputdeck.get< tag::component >().get< tag::dirichlet >().at(c) ),
      m_offset( g_inputdeck.get< tag::component >().offset< tag::dirichlet >(c) ),
      m_rng( g_rng.at( tk::ctr::raw(
        g_inputdeck.get< tag::param, tag::dirichlet, tag::rng >().at(c) ) ) ),
      m_b(),
      m_S(),
      m_k(),
      coeff( m_ncomp,
             g_inputdeck.get< tag::param, tag::dirichlet, tag::b >().at(c),
             g_inputdeck.get< tag::param, tag::dirichlet, tag::S >().at(c),
             g_inputdeck.get< tag::param, tag::dirichlet, tag::kappa >().at(c),
             m_b, m_S, m_k ) {}

    //! Initalize SDE, prepare for time integration
    //! \param[in] stream Thread (or more precisely stream) ID 
    //! \param[in,out] particles Array of particle properties 
    void initialize( int stream, tk::Particles& particles ) {
      //! Set initial conditions using initialization policy
      Init::template
        init< tag::dirichlet >
            ( g_inputdeck, m_rng, stream, particles, m_c, m_ncomp, m_offset );
    }

    //! \brief Advance particles according to the Dirichlet SDE
    //! \param[in,out] particles Array of particle properties
    //! \param[in] stream Thread (or more precisely stream) ID
    //! \param[in] dt Time step size
    void advance( tk::Particles& particles,
                  int stream,
                  tk::real dt,
                  tk::real,
                  const std::map< tk::ctr::Product, tk::real >& )
    {
      const auto npar = particles.nunk();
      for (auto p=decltype(npar){0}; p<npar; ++p) {
        // Compute Nth scalar
        tk::real yn = 1.0 - particles(p, 0, m_offset);
        for (ncomp_t i=1; i<m_ncomp; ++i)
          yn -= particles( p, i, m_offset );

        // Generate Gaussian random numbers with zero mean and unit variance
        std::vector< tk::real > dW( m_ncomp );
        m_rng.gaussian( stream, m_ncomp, dW.data() );

        // Advance first m_ncomp (K=N-1) scalars
        for (ncomp_t i=0; i<m_ncomp; ++i) {
          tk::real& par = particles( p, i, m_offset );
          tk::real d = m_k[i] * par * yn * dt;
          d = (d > 0.0 ? std::sqrt(d) : 0.0);
          par += 0.5*m_b[i]*( m_S[i]*yn - (1.0-m_S[i]) * par )*dt + d*dW[i];
        }
      }
    }

  private:
    const ncomp_t m_c;                  //!< Equation system index
    const char m_depvar;                //!< Dependent variable
    const ncomp_t m_ncomp;              //!< Number of components
    const ncomp_t m_offset;             //!< Offset SDE operates from
    const tk::RNG& m_rng;               //!< Random number generator

    //! Coefficients
    std::vector< kw::sde_b::info::expect::type > m_b;
    std::vector< kw::sde_S::info::expect::type > m_S;
    std::vector< kw::sde_kappa::info::expect::type > m_k;

    //! Coefficients policy
    Coefficients coeff;
};

} // walker::

#endif // Dirichlet_h
