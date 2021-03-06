// *****************************************************************************
/*!
  \file      src/Control/MeshConv/Types.h
  \copyright 2012-2015, J. Bakosi, 2016-2018, Los Alamos National Security, LLC.
  \brief     Types for MeshConv's parsers
  \details   Types for UnitTest's parsers. This file defines the components of
    the tagged tuple that stores heteroegeneous objects in a hierarchical way.
    These components are therefore part of the grammar stack that is filled
    during command-line argument parsing.
*/
// *****************************************************************************
#ifndef MeshConvTypes_h
#define MeshConvTypes_h

#include "TaggedTuple.h"
#include "Tags.h"
#include "Keyword.h"

namespace meshconv {
namespace ctr {

using namespace tao;

//! IO parameters storage
using ios = tk::tuple::tagged_tuple<
  tag::input,           std::string,    //!< Input filename
  tag::output,          std::string     //!< Output filename
>;

//! PEGTL location/position type to use throughout all of MeshConv's parsers
using Location = pegtl::position;

} // ctr::
} // meshconv::

#endif // MeshConvTypes_h
