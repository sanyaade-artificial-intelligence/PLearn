// -*- C++ -*-

// SequentialModelSelector.h
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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



#ifndef SEQUENTIAL_MODEL_SELECTOR
#define SEQUENTIAL_MODEL_SELECTOR

#include "SequentialLearner.h"

namespace PLearn <%
using namespace std;

/*!
*/

class SequentialModelSelector: public SequentialLearner
{
  protected:
 
    //Mat mean_costs; // [time_index,model_index], contains mean of model.errors[.,cost_index] over interval [max(0,time_index-max_train_len),time_index) of non-missing values
    TVec<int> best_model; // best model selected at time t
    Vec sequence_costs;  // the costs of each model on the training set

  public:

    TVec< PP<SequentialLearner> > models;  // list of all the models
    int init_train_size; // size of first training set
    int cost_index; // which element of costs vector is used to select best model
    string cost_type; // the type of cost to be used to select the best model

  private:
    //! This does the actual building
    void build_();

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Constructor
    SequentialModelSelector();

    //! compute the cost of the given sequence of errors (based on the cost_type)
    real sequenceCost(const Vec& sequence_errors);

    //! simply calls parentclass::build() then build_()
    virtual void build();

  //! Redefines so that it ALSO calls the method on all the learners in the TVec models
  virtual void setExperimentDirectory(const string& _expdir);

    virtual void train();
 
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
        VMat testoutputs=0, VMat testcosts=0) const;

    virtual void computeOutput(const Vec& input, Vec& output) const;

   virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
        const Vec& target, Vec& costs) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
        Vec& output, Vec& costs) const;
 
    virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;

    //! This should return the names of the costs computed by computeCostsFromOutputs
    virtual TVec<string> getTestCostNames() const;

    //! This should return the names of the objective costs that the train method
    //! computes and for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const;

    virtual void forget();

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    PLEARN_DECLARE_OBJECT_METHODS(SequentialModelSelector, "SequentialModelSelector", SequentialLearner);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialModelSelector);

%> // end of namespace PLearn

#endif
