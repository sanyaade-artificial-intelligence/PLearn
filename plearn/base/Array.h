// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


 

/* *******************************************************      
   * $Id: Array.h,v 1.2 2002/08/07 16:54:21 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Array.h */

#ifndef ARRAY_INC
#define ARRAY_INC

#include <map>
#include <set>
#include <string>
#include <vector>
#include "TypeTraits.h"
#include "general.h"
#include "fileutils.h"

//#include "/home/morinf/PLearn/UserExp/morinf/pl_streams.h"


namespace PLearn <%
using namespace std;


  template <class T> 
    class Array
    {
protected:
  T* array;
  int array_size;
  int array_capacity;

public:
  void increaseCapacity(int increase = 10)
  {
    T* newarray = new T[array_capacity+increase];
    for(int i=0; i<array_size; i++)
      newarray[i] = array[i];
    delete[] array;
    array = newarray;
    array_capacity += increase;
  }

  explicit Array<T>(int the_size=0, int extra_space = 10)
    :array_size(the_size), array_capacity(the_size+extra_space)
    {
      array = new T[array_capacity];
    }

  Array<T>(const T& elem1)
    :array_size(1), array_capacity(1)
    {
      array = new T[array_capacity];
      array[0] = elem1;
    }

  Array<T>(const T& elem1, const T& elem2)
    :array_size(2), array_capacity(2)
    {
      array = new T[array_capacity];
      array[0] = elem1;
      array[1] = elem2;
    }

  Array<T>(const Array& other)
    :array_size(other.size()), array_capacity(other.size())
    {
      array = new T[array_capacity];
      for(int i=0; i<array_size; i++)
        array[i] = other[i];
    }

  Array<T>(const vector<T> &other)
    : array_size(other.size()), array_capacity(other.size())
    {
        array = new T[array_capacity];
        for (int i = 0; i < array_size; ++i)
            array[i] = other[i];
    }

  Array<T>* operator->()
    { return this; }
  
  Array<T>& operator&=(const T& elem)
    { append(elem); return *this; }

  Array<T>& operator&=(const Array<T>& ar)
    { append(ar); return *this; }

  Array<T>& operator&=(const vector<T> &ar)
    { append(ar); return *this; };

  Array<T> operator&(const T& elem) const
    {
      Array<T> newarray(array_size, array_size+1);
      newarray = *this;
      newarray.append(elem);
      return newarray;
    }

  Array<T> operator&(const Array<T>& ar) const
    {
      Array<T> newarray(array_size, array_size+ar.size());
      newarray = *this;
      newarray.append(ar);
      return newarray;
    }

  Array<T> operator&(const vector<T> &ar) const
    {
        Array<T> newarray(array_size, array_size + ar.size());
        newarray = *this;
        newarray.append(ar);
        return newarray;
    }

  Array<T> subArray(int start, int len)
    {
      if (start+len>array_size)
        PLERROR("Array::subArray start(%d)+len(%d)>size(%d)",
              start,len,array_size);
      Array<T> newarray(len);
      for (int i=0;i<len;i++)
        newarray[i]=array[start+i];
      return newarray;
    }

  int size() const
    { return array_size; }

  int length() const
    { return array_size; }

  int capacity() const
    { return array_capacity; } 

  void resize(int new_size, int extra_space=0)
    {
      if(new_size>array_capacity)
        {
          array_capacity = new_size+extra_space;
          T* new_array = new T[array_capacity];
          if(array)
          {
            for(int i=0; i<array_size; i++)
              new_array[i] = array[i];
            delete[] array;
          }
          array = new_array;
        }
      array_size = new_size;
    }

  void clear()
    { array_size = 0; }

  void operator=(const Array& other)
    {
      resize(other.size());
      for(int i=0; i<array_size; i++)
        array[i] = other[i];
    }

  void operator=(const vector<T> &other)
    {
        resize(other.size());
        for(int i = 0; i < array_size; ++i)
            array[i] = other[i];
    }

  void fill(const T& elem)
    {
       for(int i=0; i<array_size; i++)
         array[i] = elem;
    }

  bool operator==(const Array& other)
    {
      if (other.size() != array_size) 
         return false;
      for(int i=0; i<array_size; i++)
	  if (array[i] != other[i]) return false;
      return true;
    }

  bool operator!=(const Array& other)
    { return !((*this)==other); }

  void append(const T& element)
  {
    resize(array_size+1, array_size);
    array[array_size-1] = element;
  }

  inline void push_back(const T& element)
      { append(element); }

  void appendIfNotThereAlready(const T& element)
  {
    for (int i=0;i<array_size;i++)
      if (element==array[i]) return;
    resize(array_size+1, array_size);
    array[array_size-1] = element;
  }

  void append(const Array<T>& ar)
    {
      int currentsize = size();
      resize(currentsize+ar.size());
      for(int i=0; i<ar.size(); i++)
        array[currentsize+i] = ar[i];
    }

  void append(const vector<T> &ar)
    {
        int current_size = size();
        resize(currentsize + ar.size());
        for (int i = 0; i < ar.size(); ++i)
            array[currentsize + 1] = ar[i];
    }

  //!  These functions call operator[] to ensure they are bounds-checked
  T& last() { return (*this)[array_size-1]; }
  const T& last() const { return (*this)[array_size-1]; }

  T& first() { return (*this)[0]; }
  const T& first() const { return (*this)[0]; }

  bool contains(const T& element) const
    {
      bool is_contained = false;
      for (int i=0;i<array_size && !is_contained;i++)
        is_contained = ( element == array[i] );
      return is_contained;
    }

  T& operator[](int i) const
    { 
      #ifdef BOUNDCHECK
      if(i<0 || i>=array_size)
        PLERROR("In Array<T>::operator[] OUT OF BOUND ACCESS : %d",i);
      #endif
      return array[i]; 
    }

    void print(ostream& out) const
    {
      for(int i=0; i<array_size; i++)
        out << array[i] << endl;
    }

    void write(ostream &out_) const
    {
        pl_ostream out = out_;
        newwrite(out);
    }

    void newwrite(pl_ostream& out) const
    {
        out << raw << size() << " [ ";
        for(int i=0; i<array_size; i++)
            out << array[i];
        out << raw << " ]\n";
    }

    void read(istream &in_)
    {
        pl_istream in = in_;
        newread(in);
    }

    void newread(pl_istream &in)
    {  
      T val;
      skipBlanksAndComments(in);
      int c = in.peek();
      if(c=='[') // format is "[val1; val2; va3; ...]"
        {
          in.get(); // skip '['
          resize(0);
          skipBlanksAndComments(in);
          c = in.peek();
          if(c!=']')
            {
              while(in)
                {
                  in >> val;
                  if(!in)
                    PLERROR("In Array::read with format [ v1; v2; v3 ] problem while reading value");
                  push_back(val);
                  skipBlanksAndComments(in);
                  c = in.get();
                  if(c==']')
                    break;
                  else if(c!=';')
                    PLERROR("In Array::read with format [ v1; v2; v3 ] bad character read where semicolumn expected: %c",c);
                }
            }
          if(!in)
            PLERROR("Problem in Array:read stream in bad state");
        }
      else if(isdigit(c)) // format is "size val1 val2 val3 ..." or "size[ val1 val2 val3 ... ]"
      {
          int size;
          in >> size;
          resize(size);
          skipBlanksAndComments(in);
          c = in.get();
          if(c!='[')
            in.unget();
          for(int i=0; i<size; i++)
              in >> array[i];
          if(c=='[')
            {
              skipBlanksAndComments(in);
              if((c=in.get())!=']')
                PLERROR("In Array::read(istream&) wrong array format, read %c when expecting a ']' ",c);                 
            }
      }
      else
        PLERROR("In Array::read(istream&) wrong array format, read %c when expecting a '[' or a digit",c); 
      
      // skip next blank
      if(!isspace(in.get()))
         in.unget();
    }

    void deepWrite(ostream& out, DeepWriteSet& already_saved) const
    {
      writeHeader(out, "Array");
      PLearn::deepWrite(out, already_saved, array_size);
      PLearn::deepWrite(out, already_saved, array_capacity);
      out << "\n";
      for(int i=0; i<array_size; i++)
        PLearn::deepWrite(out, already_saved, array[i]);
      writeFooter(out, "Array");
    }

    void deepRead(istream& in, DeepReadMap& old2new)
    {
      readHeader(in, "Array");
      PLearn::deepRead(in, old2new, array_size);
      PLearn::deepRead(in, old2new, array_capacity);
      for(int i=0; i<array_size; i++)
        PLearn::deepRead(in, old2new, array[i]);
      readFooter(in, "Array");
    }

    void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
    {
      for (int i=0; i<array_size; i++)
        deepCopyField(array[i], copies);
    }

    ~Array()
    {
      delete[] array;
    }

};

/* I think these are not needed anymore...
template <class T>
  void write(ostream& out, const Array<T>& a) { a.write(out); }

template <class T>
  void read(istream& in, Array<T>& a) { a.read(in); }
*/
template <class T> inline pl_istream &
operator>>(pl_istream &in, Array<T> &a)
{ a.newread(in); return in; }

template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const Array<T> &a)
{ a.newwrite(out); return out; };

template <class T> inline pl_istream &
operator>>(pl_istream &in, vector<T> &v)
{
    // Easy way
    Array<T> a;
    a.newread(in);
    v.clear();
    v.reserve(a.size());
    for (int i= 0; i < a.size(); ++i) {
        v.push_back(a[i]);
    }
    return in;
}

template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const vector<T> &v)
{
    // Easy way
    Array<T> a = v;
    a.newwrite(out);
    return out;
}

template <class T>
  void deepWrite(ostream& out, DeepWriteSet& already_saved, const Array<T>& a)
  { a.deepWrite(out, already_saved); }

template <class T>
  void deepRead(istream& in, DeepReadMap& old2new, Array<T>& a)
  { a.deepRead(in, old2new); }

template <class T>
inline void deepCopyField(Array<T>& field, CopiesMap& copies)
{
  field.makeDeepCopyFromShallowCopy(copies);
}

template<class T>
Array<T> operator&(const T& elem, const Array<T>& a)
{ return Array<T>(elem) & a; }

template<class T>
ostream& operator<<(ostream& out, const Array<T>& a)
{ 
  a.print(out);
  return out;
}

inline string join(const Array<string>& s, const string& separator)
{
  string result;
  for(int i=0; i<s.size(); i++)
  {
    result += s[i];
    if(i<s.size()-1)
      result += separator;
  }
  return result;
}

template<class T>
class TypeTraits< Array<T> >
{
public:
  static inline string name()
  { return string("Array< ") + TypeTraits<T>::name()+" >"; }
};
/*
template<class T>
inline void write(ostream& out, const vector<T>& v) {
    // Easy way
    Array<T> array = v;
    array.write(out);
}

template<class T>
inline void read(istream& in, vector<T>& v) {
    // Easy way
    Array<T> array;
    array.read(in);
    v.clear();
    v.reserve(array.size());
    for (int i = 0; i < array.size(); ++i)
        v.push_back(array[i]);
}
*/

%> // end of namespace PLearn

#endif
