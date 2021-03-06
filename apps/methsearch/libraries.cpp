// -*- C++ -*- libraries.cpp - singleton containing the method libraries
// Copyright (C) 2002, 2009, 2010 Richard Smith <richard@ex-parrot.com>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// $Id$

#include <ringing/common.h>

#if RINGING_HAS_PRAGMA_INTERFACE
#pragma implementation
#endif

#include "libraries.h"
#include "format.h"
#include <string>
#if RINGING_OLD_INCLUDES
#include <vector.h>
#else
#include <vector>
#endif
#if RINGING_HAVE_OLD_IOSTREAMS
#include <iostream.h>
#else
#include <iostream>
#endif
#if RINGING_OLD_C_INCLUDES
#include <cstdlib.h>
#else
#include <cstdlib>
#endif
#include <ringing/method.h>
#include <ringing/library.h>
#include <ringing/cclib.h>
#include <ringing/mslib.h>
#include <ringing/xmllib.h>
#include <ringing/methodset.h>


RINGING_USING_NAMESPACE
RINGING_USING_STD

method_libraries &method_libraries::instance()
{
  static method_libraries tmp;
  return tmp;
}

method_libraries::method_libraries()
  : done_init(false)
{
}

bool method_libraries::has_libraries()
{
  return !instance().library_names.empty();
}

void method_libraries::add_new_library( const string &filename )
{
  instance().library_names.push_back( filename );
  instance().done_init = false;
}

void method_libraries::read_library( const string &filename )
{
  try
    {
      library l( filename );

      if (!l.good())
	{
	  cerr << "Unable to read library " << filename << "\n";
	  exit(1);
	}

      if ( l.empty() )
	{
	  cerr << "The library " << filename << " appears to be empty\n";
	  exit(1);
	}
 
      copy( l.begin(), l.end(), back_inserter( instance() ) );
    }
  catch ( const exception &e )
    {
      cerr << "Unable to read library: " << filename << ": " 
	   << e.what() << '\n';
      exit(1);
    }

}

void method_libraries::init()
{
  if ( !instance().done_init && has_libraries() )
    {
      cclib::registerlib();
      mslib::registerlib();
      xmllib::registerlib();

      if ( char const* const methlibpath = getenv("METHOD_LIBRARY_PATH") )
        library::setpath( methlibpath );
      else if ( char const* const methlibpath = getenv("METHLIBPATH") )
        library::setpath( methlibpath );
      
      instance().clear();

      if ( formats_have_cc_ids() ) {
        // Older GCCs (e.g. 3.1) can't cope with template members on bases
        methodset &base = instance();
        base.store_facet< cc_collection_id >();
      }

      for ( vector< string >::const_iterator 
	      i( instance().library_names.begin() ), 
	      e( instance().library_names.end() );
	    i != e; ++i )
	{
	  instance().read_library( *i );
	}

      instance().done_init = true;
    }
}

method method_libraries::lookup_method( const method &m )
{
  library_entry i( instance().find( m ) );
  if ( ! i.null() )
    return i.meth();
  else 
    return m;
}


