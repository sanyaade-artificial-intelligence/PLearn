// -*- C++ -*-

// SVMClassificationTorch.h
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id: SVMClassificationTorch.h,v 1.2 2005/01/25 14:34:35 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SVMClassificationTorch.h */


#ifndef SVMClassificationTorch_INC
#define SVMClassificationTorch_INC

#include <plearn/ker/Kernel.h>
#include <plearn_learners/generic/PLearner.h>
#include <torch/MatDataSet.h>
#include <torch/QCTrainer.h>
#include <torch/SVMClassification.h>

namespace PLearn {

class SVMClassificationTorch: public PLearner {

private:

  typedef PLearner inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  // Fields below are not options.

  //! Allocator for Torch memory management.
  Torch::Allocator* allocator;

  //! The Torch training set.
  Torch::DataSet* dataset;

  //! A sequence to provide inputs to the Torch SVM.
  Torch::Sequence* inputs;

  //! The underlying Torch SVM.
  Torch::SVMClassification* svm;

  //! The Torch trainer.
  Torch::QCTrainer* trainer;

public:

  // ************************
  // * public build options *
  // ************************

  real C_cst;
  real cache_size_in_megs;
  Ker kernel;
  bool output_the_class;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  SVMClassificationTorch();

  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  //! To free memory allocated in Torch.
  virtual ~SVMClassificationTorch();

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(SVMClassificationTorch);

  // **************************
  // **** PLearner methods ****
  // **************************

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  virtual void forget();
    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();

  //! Computes the output from the input.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  virtual TVec<std::string> getTrainCostNames() const;

  //! Overridden to store the training_set in 'dataset'.
  virtual void setTrainingSet(VMat training_set, bool call_forget = true);

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
  // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
  // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
  // virtual int nTestCosts() const;
  // virtual int nTrainCosts() const;
  // virtual void resetInternalState();
  // virtual bool isStatefulLearner() const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(SVMClassificationTorch);
  
} // end of namespace PLearn

#endif
