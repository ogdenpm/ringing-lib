// -*- C++ -*- methodutils.h - utility functions missing from the ringing-lib
// Copyright (C) 2002 Richard Smith <richard@ex-parrot.com>

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

#ifdef __GNUG__
#pragma implementation
#endif

#include <ringing/common.h>
#include "methodutils.h"
#include <string>
#if RINGING_OLD_INCLUDES
#include <stdexcept.h>
#else
#include <stdexcept>
#endif
#if RINGING_OLD_C_INCLUDES
#include <stddef.h>
#include <assert.h>
#else
#include <cstddef>
#include <cassert>
#endif
#include <ringing/row.h>
#include <ringing/method.h>
#include <ringing/streamutils.h>


RINGING_USING_NAMESPACE
RINGING_USING_STD

change merge_changes( const change &a, const change &b )
{
  if ( a.bells() != b.bells() ) 
    throw logic_error("Mismatched numbers of bells");

  if ( (row() * a * b).order() > 2 )
    throw range_error("Result of change::operator& not a change");
  
  change c( a.bells() );

  for ( unsigned int i=0; i<a.bells()-1; ++i )
    if ( a.findswap(i) || b.findswap(i) )
      c.swappair(i);

  return c;
}

bool have_same_places( const change &a, const change &b )
{
  if ( a.bells() != b.bells() ) 
    throw logic_error("Mismatched numbers of bells");

  for ( unsigned int i=0; i<a.bells(); ++i )
    if ( a.findplace(i) && b.findplace(i) )
      return true;
  
  return false;
}

bool has_pbles( const method &m, int hunts )
{
  return m.huntbells() == hunts && m.isregular();
}

bool has_cyclic_les( const method &m, int hunts )
{
  const row lh( m.lh() );
  const int n( m.bells() - hunts );

  // Rounds doesn't count...
  if ( lh[hunts] == hunts ) 
    return false;

  {
    for ( int i=0; i<hunts; ++i )
      if ( lh[i] != i )
	return false;
  }

  {
    for ( int i=hunts+1; i<lh.bells(); ++i )
      if ( (lh[i] - hunts) % n != (lh[i-1] + 1 - hunts) % n )
	return false;
  }

  return true;
}

bool is_division_false( const method &m, const change &c, unsigned int divlen )
{
  if ( m.length() % divlen < 3 || m.length() % divlen == divlen-1 )
    return false;

  row r( c.bells() );

  vector< row > rows( 1, r );
  rows.reserve( divlen );

  for ( unsigned int i = (m.length() / divlen) * divlen; i < m.length(); ++i )
    {
      r *= m[i];
      rows.push_back( r );
    }

  if ( find( rows.begin(), rows.end(), r * c ) != rows.end() )
    return true;

  return false;
}

bool is_too_many_places( const method &m, const change &c, size_t max )
{
  for ( size_t i=0; i<c.bells(); ++i )
    if ( c.findplace(i) )
      {
	size_t count(2u);

	for ( ; count <= m.length()+1; ++count )
	  if ( !m[m.length()-count+1].findplace(i) )
	    break;

	if ( count > max )
	  return true;
      }

  return false;
}

bool has_consec_places( const change &c, size_t max_count )
{
  size_t count(0u);
  for ( size_t i=0u; i<c.bells() && count <= max_count ; ++i )
    if ( c.findplace(i) )
      ++count;
    else
      count = 0u;

  return count > max_count;
}

bool division_bad_parity_hack( const method &m, const change &c, 
			       unsigned int divlen )
{
  // It should be possible to do this easily without calculating rows.

  row r( c.bells() );

  vector< row > rows( 1, r );
  rows.reserve( divlen );

  {
    for ( unsigned int i = (m.length() / divlen) * divlen; 
	  i < m.length(); ++i )
      {
	r *= m[i];
	rows.push_back( r );
      }
  }
  
  rows.push_back( r * c );

  if ( rows.size() != divlen )
    cerr << rows.size() << "!=" << divlen << endl;
  assert( rows.size() == divlen );

  size_t even[2] = { 0u, 0u }, odd[2] = { 0u, 0u };
  
  for ( unsigned int i = 0; i < rows.size(); ++i )
    {
      if ( rows[i].sign() == +1 )
	++even[i%2];
      else
	++odd[i%2];
    } 

  if ( even[0] != odd[0] || even[1] != odd[1] )
    return true; // Bad
    
  return false; // OK
}

void do_single_compressed_pn( make_string &os, const change &ch, 
			      bool &might_need_dot, bool is_lh = false )
{
  if ( ch == change( ch.bells(), "X" ) ) 
    {
      os << '-'; might_need_dot = false;
    } 
  else 
    {
      string p( ch.print() );
      
      if ( p[0] == bell(0).to_char() )
	p = p.substr(1);
      if ( p[ p.size()-1 ] == bell( ch.bells() - 1 ).to_char() )
	p = p.substr( 0, p.size() - 1 );
      if ( p.empty() )
	p = bell( is_lh ? ch.bells() - 1 : 0 ).to_char();
      
      if (might_need_dot) os << '.';
      os << p;
      might_need_dot = true;
    }
}

string get_compressed_pn( const method &m )
{
  make_string os;

  bool might_need_dot(false);

  if ( m.issym() )
    {
      os << '&';
      for ( int i=0; i < m.length() / 2; ++i)
	do_single_compressed_pn( os, m[i], might_need_dot );

      os << ','; might_need_dot = false;
    }
  else
    {
      for ( int i=0; i < m.length() - 1; ++i)
	do_single_compressed_pn( os, m[i], might_need_dot );
    }

  do_single_compressed_pn( os, m.back(), might_need_dot, true );

  return os;
}

unsigned int max_blows_per_place( const method &m )
{
  int maxn = 0;

  for ( method::const_iterator b(m.begin()), i(b), e(m.end()); i != e; ++i )
    for ( unsigned int p(0); p < m.bells(); ++p )
      if ( i->findplace(p) )
	{
	  // It's part of a longer run.
	  if ( i != b && (i-1)->findplace(p) )
	    continue;

	  // Find the size of this run of blows
	  int n=2;
	  {
	    for ( method::const_iterator j(i); j != e; ++j )
	      if ( j->findplace(p) )
		++n;
	      else
		goto end_run;
	  }

	  {
	    // And handle runs that continue across the lead-end
	    for ( method::const_iterator j(b); j != i; ++j )
	      if ( j->findplace(p) )
		++n;
	      else
		goto end_run;
	  }

	end_run:
	  if ( n > maxn )
	    maxn = n;
	}

  return maxn;
}


bool has_rotational_symmetry( const method &m )
{
  {
    // Rotational symmetry about a change
    for ( int i=0, n=m.size(); i<n/2+1; ++i )
      {
	// Try m[i] as the rotational symmetry point
	bool ok = true;

	for ( int j=0; j<n/2 && ok; ++j )
	  if ( m[ (i+j)%n ] != m[ (n+i-j)%n ].reverse() )
	    ok = false;

	if (ok)
	  return true;
      }
  }
  {
    // Rotational symmetry about a row
    for ( int i=0, n=m.size(); i<n/2+1; ++i )
      {
	// Try m[i] / m[(i+1)%n] as the rotational symmetry point
	bool ok = true;

	for ( int j=0; j<n/2 && ok; ++j )
	  if ( m[ (i+j+1)%n ] != m[ (n+i-j)%n ].reverse() )
	    ok = false;

	if (ok)
	  return true;
      }
  }
  return false;
}


