// *****************************************************************************
/*!
  \file      src/Control/UnitTest/Types.h
  \copyright 2012-2015, J. Bakosi, 2016-2018, Los Alamos National Security, LLC.
  \brief     Types for UnitTest's parsers
  \details   Types for UnitTest's parsers. This file defines the components of
    the tagged tuple that stores heteroegeneous objects in a hierarchical way.
    These components are therefore part of the grammar stack that is filled
    during command-line argument parsing.
*/
// *****************************************************************************
#ifndef UnitTestTypes_h
#define UnitTestTypes_h

#include "Tags.h"
#include "Keyword.h"

namespace unittest {
namespace ctr {

using namespace tao;

//! PEGTL location/position type to use throughout all of UnitTest's parsers
using Location = pegtl::position;

} // ctr::
} // unittest::

#endif // UnitTestTypes_h
