// -*- C++ -*-

// PLearnerOutputVMatrix.h
//
// Copyright (C) 2003 Yoshua Bengio 
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
   * $Id: PLearnerOutputVMatrix.h,v 1.1 2003/09/23 01:04:02 yoshua Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file PLearnerOutputVMatrix.h */


#ifndef PLearnerOutputVMatrix_INC
#define PLearnerOutputVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"
#include "PLearner.h"

namespace PLearn <%
using namespace std;

class PLearnerOutputVMatrix: public RowBufferedVMatrix
{
protected:
  // *********************
  // * protected options *
  // *********************

  // NON-OPTIONs

  mutable Vec row;
  mutable Vec learner_input;
  mutable Vec learner_output;
  mutable Vec learner_target;
  mutable Vec learner_costs;
  mutable Vec non_input_part_of_data_row;

public:

  typedef RowBufferedVMatrix inherited;

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here

  VMat data; // whose input field will be applied to learner in order to obtain new input part of this VMatrix
  PP<PLearner> learner;
  bool put_raw_input; // if true not only the learner output but also the raw data input are in the input part of the VMatrix
  bool put_costs; // if true then the test costs are also put in the input part of the VMatrix

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  PLearnerOutputVMatrix();

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(PLearnerOutputVMatrix);

};

%> // end of namespace PLearn
#endif
