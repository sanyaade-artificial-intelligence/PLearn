
// -*- C++ -*-

// Grapher.h
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: Grapher.h,v 1.4 2004/06/26 00:24:15 plearner Exp $ 
   ******************************************************* */

/*! \file Grapher.h */
#ifndef GenerateDecisionPlot_INC
#define GenerateDecisionPlot_INC

#include "Object.h"
#include "PLearner.h"

namespace PLearn {
using namespace std;

class Grapher: public Object
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  typedef Object inherited;

  // ************************
  // * public build options *
  // ************************

  string basename;
  string task;
  real class1_threshold;
  PP<PLearner> learner;
  VMat trainset;
  TVec<int> griddim;
  TVec< pair<real,real> > gridrange;
  real radius;
  bool bw;
  
  string save_learner_as;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  Grapher();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();
  
  void computeAutoGridrange();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  //! Overload this for runnable objects (default method issues a runtime error)
  virtual void run();

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(Grapher);

  static void plot_2D_classification(string epsfname, VMat trainset, 
                                     TVec<int> griddim, TVec< pair<real,real> > gridrange,
                                     VMat gridoutputs, real radius, bool bw=false);

  static void plot_1D_regression(string basename, VMat trainset, 
                   TVec<int> griddim, TVec< pair<real,real> > gridrange, 
                   VMat gridoutputs, VMat trainoutputs, bool bw=false);
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(Grapher);
  
} // end of namespace PLearn

#endif
