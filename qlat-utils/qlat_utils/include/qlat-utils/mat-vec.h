// vim: set ts=2 sw=2 expandtab:

// Copyright (c) 2022 Luchang Jin
// All rights reserved.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <qlat-utils/config.h>
#include <qlat-utils/qacc.h>
#include <qlat-utils/assert.h>
#include <qlat-utils/complex.h>
#include <qlat-utils/qcd-setting.h>
#include <qlat-utils/handle.h>

namespace qlat
{

template <class T>
struct CommaLoader {
  Vector<T> m;
  int i;
  //
  CommaLoader(Vector<T> m_, int i_)
  {
    m = m_;
    i = i_;
  }
  //
  CommaLoader operator,(const T& x)
  {
    m[i] = x;
    return CommaLoader(m, i + 1);
  }
};

// --------------------

template <int DIMN, class T>
struct API ALIGN(sizeof(T) * DIMN * DIMN) MatrixT {
  T p[DIMN * DIMN];
  //
  qacc T* data() { return p; }
  qacc const T* data() const { return p; }
  //
  // convert to double array
  qacc double* d() { return (double*)p; }
  qacc const double* d() const { return (const double*)p; }
  //
  qacc T& operator()(int i, int j)
  {
    qassert(0 <= i && i < DIMN);
    qassert(0 <= j && j < DIMN);
    return p[i * DIMN + j];
  }
  qacc const T& operator()(int i, int j) const
  {
    qassert(0 <= i && i < DIMN);
    qassert(0 <= j && j < DIMN);
    return p[i * DIMN + j];
  }
  //
  qacc const MatrixT& operator+=(const MatrixT& x)
  {
    *this = *this + x;
    return *this;
  }
  //
  qacc const MatrixT& operator-=(const MatrixT& x)
  {
    *this = *this - x;
    return *this;
  }
  //
  qacc const MatrixT& operator*=(const MatrixT& x)
  {
    *this = *this * x;
    return *this;
  }
  //
  qacc const MatrixT& operator*=(const T& x)
  {
    *this = *this * x;
    return *this;
  }
  //
  qacc const MatrixT& operator/=(const T& x)
  {
    *this = *this / x;
    return *this;
  }
  //
  CommaLoader<T> operator<<(const T& x)
  {
    Vector<T> m(p, DIMN * DIMN);
    m[0] = x;
    return CommaLoader<T>(m, 1);
  }
};

template <class T = Real>
struct API ColorMatrixT : MatrixT<NUM_COLOR, ComplexT<T> > {
  qacc ColorMatrixT() {}
  qacc ColorMatrixT(const MatrixT<NUM_COLOR, ComplexT<T> >& m) { *this = m; }
  //
  qacc const ColorMatrixT& operator=(const MatrixT<NUM_COLOR, ComplexT<T> >& m)
  {
    *this = (const ColorMatrixT&)m;
    return *this;
  }
};

template <class T = Real>
struct API WilsonMatrixT : MatrixT<4 * NUM_COLOR, ComplexT<T> > {
  qacc WilsonMatrixT() {}
  qacc WilsonMatrixT(const MatrixT<4 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = m;
  }
  //
  qacc const WilsonMatrixT& operator=(
      const MatrixT<4 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = (const WilsonMatrixT&)m;
    return *this;
  }
};

template <class T = Real>
struct API SpinMatrixT : MatrixT<4, ComplexT<T> > {
  qacc SpinMatrixT() {}
  qacc SpinMatrixT(const MatrixT<4, ComplexT<T> >& m) { *this = m; }
  //
  qacc const SpinMatrixT& operator=(const MatrixT<4, ComplexT<T> >& m)
  {
    *this = (const SpinMatrixT&)m;
    return *this;
  }
};

template <class T = Real>
struct API NonRelWilsonMatrixT : MatrixT<2 * NUM_COLOR, ComplexT<T> > {
  qacc NonRelWilsonMatrixT() {}
  qacc NonRelWilsonMatrixT(const MatrixT<2 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = m;
  }
  //
  qacc const NonRelWilsonMatrixT& operator=(
      const MatrixT<2 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = (const NonRelWilsonMatrixT&)m;
    return *this;
  }
};

template <class T = Real>
struct API IsospinMatrixT : MatrixT<2, ComplexT<T> > {
  qacc IsospinMatrixT() {}
  qacc IsospinMatrixT(const MatrixT<2, ComplexT<T> >& m) { *this = m; }
  //
  qacc const IsospinMatrixT& operator=(const MatrixT<2, ComplexT<T> >& m)
  {
    *this = (const IsospinMatrixT&)m;
    return *this;
  }
};

using ColorMatrix = ColorMatrixT<>;

using WilsonMatrix = WilsonMatrixT<>;

using SpinMatrix = SpinMatrixT<>;

using NonRelWilsonMatrix = NonRelWilsonMatrixT<>;

using IsospinMatrix = IsospinMatrixT<>;

// --------------------

template <int DIMN, class T>
struct API ALIGN(sizeof(T) * DIMN) MvectorT {
  T p[DIMN];
  //
  qacc T* data() { return p; }
  qacc const T* data() const { return p; }
  //
  // convert to double array
  qacc double* d() { return (double*)p; }
  qacc const double* d() const { return (const double*)p; }
  //
  qacc T& operator()(int i)
  {
    qassert(0 <= i && i < DIMN);
    return p[i];
  }
  qacc const T& operator()(int i) const
  {
    qassert(0 <= i && i < DIMN);
    return p[i];
  }
  //
  qacc const MvectorT& operator+=(const MvectorT& x)
  {
    *this = *this + x;
    return *this;
  }
  //
  qacc const MvectorT& operator-=(const MvectorT& x)
  {
    *this = *this - x;
    return *this;
  }
  //
  qacc const MvectorT& operator*=(const T& x)
  {
    *this = *this * x;
    return *this;
  }
  //
  qacc const MvectorT& operator/=(const T& x)
  {
    *this = *this / x;
    return *this;
  }
  //
  CommaLoader<T> operator<<(const T& x)
  {
    Vector<T> m(p, DIMN);
    m[0] = x;
    return CommaLoader<T>(m, 1);
  }
};

template <class T = Real>
struct API WilsonVectorT : MvectorT<4 * NUM_COLOR, ComplexT<T> > {
  qacc WilsonVectorT() {}
  qacc WilsonVectorT(const MvectorT<4 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = m;
  }
  //
  qacc const WilsonVectorT& operator=(
      const MvectorT<4 * NUM_COLOR, ComplexT<T> >& m)
  {
    *this = (const WilsonVectorT&)m;
    return *this;
  }
};

template <class T = Real>
struct API SpinVectorT : MvectorT<4, ComplexT<T> > {
  qacc SpinVectorT() {}
  qacc SpinVectorT(const MvectorT<4, ComplexT<T> >& m) { *this = m; }
  //
  qacc const SpinVectorT& operator=(const MvectorT<4, ComplexT<T> >& m)
  {
    *this = (const SpinVectorT&)m;
    return *this;
  }
};

using WilsonVector = WilsonVectorT<>;

using SpinVector = SpinVectorT<>;

// --------------------

}  // namespace qlat