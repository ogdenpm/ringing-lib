// -*- C++ -*- expression.h - Code to execute different types of expression
// Copyright (C) 2003, 2004, 2005, 2008, 2011 
// Richard Smith <richard@ex-parrot.com>

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

#ifndef GSIRIL_EXPRESSION_INCLUDED
#define GSIRIL_EXPRESSION_INCLUDED

#include <ringing/common.h>

#if RINGING_HAS_PRAGMA_INTERFACE
#pragma interface
#endif

#include "expr_base.h"
#if RINGING_OLD_INCLUDES
#include <utility.h>
#else
#include <utility>
#endif
#include <string>
#include <ringing/row.h>
#include <ringing/music.h>
#include <ringing/method.h>

// Forward declare ringing::method and ringing::change
RINGING_START_NAMESPACE
class method;
class change;
RINGING_END_NAMESPACE

RINGING_USING_NAMESPACE


class list_node : public expression::node
{
public:
  list_node( const expression& car, const expression& cdr )
    : car(car), cdr(cdr) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );
  virtual bool evaluate( proof_context &ctx );
  virtual expression::type_t type() const;

private:  
  expression car, cdr;
};

class nop_node : public expression::node
{
protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &, int dir );
  virtual bool isnop() const;
};

class repeated_node : public expression::node
{
public:
  repeated_node( int count,
		 const expression &child )
    : count(count), child(child) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:  
  int count;
  expression child;
};

class reverse_node : public expression::node
{
public:
  reverse_node( const expression &child )
    : child(child) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:  
  expression child;
};

class string_node : public expression::node
{
public:
  string_node( const string &str ) 
    : str(str) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  string str;
};

class pn_node : public expression::node
{
public:
  pn_node( int bells, const string &pn );

  pn_node( method const& m );
  pn_node( change const& ch );

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  vector< change > changes;
};

class transp_node : public expression::node
{
public:
  transp_node( int bells, const string &r );

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  row transp;
};

class symbol_node : public expression::node
{
public:
  symbol_node( const string &sym )
    : sym(sym) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  string sym;
};

class assign_node : public expression::node
{
public:
  assign_node( const string& sym, const expression& val )
    : defn( make_pair(sym, val) ) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  pair< const string, expression > defn;
};

class isrounds_node : public expression::bnode
{
protected:
  virtual void debug_print( ostream &os ) const;
  virtual bool evaluate( proof_context &ctx );
};

class pattern_node : public expression::bnode
{
public:
  pattern_node( int bells, const string& regex );

protected:
  virtual void debug_print( ostream &os ) const;
  virtual bool evaluate( proof_context &ctx );

private:
  music mus;
};

class and_node : public expression::bnode
{
public:
  and_node( expression const& left, expression const& right )
    : left(left), right(right) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual bool evaluate( proof_context &ctx );

private:
  expression left, right;
};

class or_node : public expression::bnode
{
public:
  or_node( expression const& left, expression const& right )
    : left(left), right(right) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual bool evaluate( proof_context &ctx );

private:
  expression left, right;
};

class if_match_node : public expression::node
{
public:
  if_match_node( const expression& test,
		 const expression& iftrue, const expression& iffalse )
    : test(test), iftrue(iftrue), iffalse(iffalse) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  expression test, iftrue, iffalse;
};

class exception_node : public expression::node
{
public:
  exception_node( script_exception::type t ) : t(t) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  script_exception::type t;
};

class load_method_node : public expression::node
{
public:
  load_method_node( string const& name )
    : name(name), read(false) {}

protected:
  virtual void debug_print( ostream &os ) const;
  virtual void execute( proof_context &ctx, int dir );

private:
  string name;

  // Reading the method is expensive; we don't want to re-read it every
  // time the node is evaluated.  Set read=true when it has been read.
  // If meth is sill empty, we'll throw an exception.
  bool read;
  method meth;
};

#endif // GSIRIL_EXPRESSION_INCLUDED
