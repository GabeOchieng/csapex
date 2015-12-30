//						DelegateBind.h
//  Helper file for Delegates. Provides bind() function, enabling
//  Delegates to be rapidly compared to programs using std::function and std::bind.
//
//  Documentation is found at http://www.codeproject.com/cpp/Delegate.asp
//
//		Original author: Jody Hagins.
//		 Minor changes by Don Clugston.
//
// Warning: The arguments to 'bind' are ignored! No actual binding is performed.
// The behaviour is equivalent to std::bind only when the basic placeholder 
// arguments _1, _2, _3, etc are used in order.
//
// HISTORY:
//	1.4 Dec 2004. Initial release as part of Delegate 1.4.


#ifndef DelegateBIND_H
#define DelegateBIND_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////
//						Delegate bind()
//
//				bind() helper function for boost compatibility.
//				(Original author: Jody Hagins).
//
// Add another helper, so Delegate can be a dropin replacement
// for std::bind (in a fair number of cases).
// Note the elipses, because std::bind() takes place holders
// but Delegate does not care about them.  Getting the place holder
// mechanism to work, and play well with boost is a bit tricky, so
// we do the "easy" thing...
// Assume we have the following code...
//      using std::bind;
//      bind(&Foo:func, &foo, _1, _2);
// we should be able to replace the "using" with...
//      using Delegate::bind;
// and everything should work fine...
////////////////////////////////////////////////////////////////////////////////

#ifdef Delegate_ALLOW_FUNCTION_TYPE_SYNTAX

namespace delegate {

//N=0
template <class X, class Y, class RetType>
Delegate< RetType (  ) >
bind(
    RetType (X::*func)(  ),
    Y * y,
    ...)
{ 
  return Delegate< RetType (  ) >(y, func);
}

template <class X, class Y, class RetType>
Delegate< RetType (  ) >
bind(
    RetType (X::*func)(  ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType (  ) >(y, func);
}

//N=1
template <class X, class Y, class RetType, class Param1>
Delegate< RetType ( Param1 p1 ) >
bind(
    RetType (X::*func)( Param1 p1 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1>
Delegate< RetType ( Param1 p1 ) >
bind(
    RetType (X::*func)( Param1 p1 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1 ) >(y, func);
}

//N=2
template <class X, class Y, class RetType, class Param1, class Param2>
Delegate< RetType ( Param1 p1, Param2 p2 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2>
Delegate< RetType ( Param1 p1, Param2 p2 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2 ) >(y, func);
}

//N=3
template <class X, class Y, class RetType, class Param1, class Param2, class Param3>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3 ) >(y, func);
}

//N=4
template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) >(y, func);
}

//N=5
template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) >(y, func);
}

//N=6
template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) >(y, func);
}

//N=7
template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) >(y, func);
}

//N=8
template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ),
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) >(y, func);
}

template <class X, class Y, class RetType, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) >
bind(
    RetType (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const,
    Y * y,
    ...)
{ 
  return Delegate< RetType ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) >(y, func);
}


#endif //Delegate_ALLOW_FUNCTION_TYPE_SYNTAX

} // namespace Delegate

#endif // !defined(DelegateBIND_H)

