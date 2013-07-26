//******************************************************************************
/*!
  \file      src/Geometry/AnalyticGeometry.h
  \author    J. Bakosi
  \date      Fri 12 Jul 2013 08:24:31 PM MDT
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Analytic geometry definition
  \details   Analytic geometry definition
*/
//******************************************************************************
#ifndef AnalyticGeometry_h
#define AnalyticGeometry_h

#include <Exception.h>
#include <Geometry.h>

namespace Quinoa {

class Primitive;

//! Analytic geometry definition
class AnalyticGeometry : public Geometry {

  public:
    //! Constructor
    explicit AnalyticGeometry( Memory* const memory,
                               Paradigm* const paradigm,
                               Control* const control,
                               Timer* const timer) noexcept :
      Geometry(memory, paradigm, control, timer) {}

    //! Destructor
    ~AnalyticGeometry() noexcept = default;

    //! Initialize analytic geometry
    virtual void init() {}

    //! Space-fill analytic geometry
    virtual void fill() {}

  private:
    //! Don't permit copy constructor
    AnalyticGeometry(const AnalyticGeometry&) = delete;
    //! Don't permit copy assigment
    AnalyticGeometry& operator=(const AnalyticGeometry&) = delete;
    //! Don't permit move constructor
    AnalyticGeometry(AnalyticGeometry&&) = delete;
    //! Don't permit move assigment
    AnalyticGeometry& operator=(AnalyticGeometry&&) = delete;

    std::vector<Primitive*> m_primitive;     //!< Vector of geometric primitives
};

} // namespace Quinoa

#endif // AnalyticGeometry_h