// -*- C++ -*-

// Learner.cc
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
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
   * $Id: Learner.cc,v 1.1 2002/09/05 00:02:22 plearner Exp $
   ******************************************************* */

#include "Learner.h"
#include "TmpFilenames.h"
#include "fileutils.h"
#include "stringutils.h"
#if USING_MPI
#include "PLMPI.h"
#include "MPIStream.h"
#endif

namespace PLearn <%
using namespace std;

oassignstream& Learner::default_vlog()
{
  static oassignstream default_vlog = cout;
  return default_vlog;
}
int Learner::use_file_if_bigger = 64000000L;
bool Learner::force_saving_on_all_processes = false;

Learner::Learner(int the_inputsize, int the_targetsize, int the_outputsize)
  :train_objective_stream(0), epoch_(0), 
  inputsize_(the_inputsize), targetsize_(the_targetsize), outputsize_(the_outputsize), 
  weightsize_(0), save_at_every_epoch(false), best_step(0)
{
  test_every = 1;
  minibatch_size = 1; // by default call use, not apply
  setEarlyStopping(-1, 0, 0); // No early stopping by default
  vlog = default_vlog();
  report_test_progress_every = 10000;
  measure_cpu_time_first=false;
  setTestStatistics(mean_stats() & stderr_stats());
}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(Learner);
void Learner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  Object::makeDeepCopyFromShallowCopy(copies);
  //Measurer::makeDeepCopyFromShallowCopy(copies);
  //deepCopyField(test_sets, copies);
  //deepCopyField(measurers, copies);
  deepCopyField(avg_objective, copies);
  deepCopyField(avgsq_objective, copies);
  deepCopyField(test_costfuncs, copies);
  deepCopyField(test_statistics, copies);
}

void Learner::outputResultLineToFile(const string & fname, const Vec& results,bool append,const string& names)
{
  ofstream teststream(fname.c_str(),ios::out|(append?ios::app:0));
  if(teststream.tellp()==0)
    teststream << "#: epoch " << names << endl;
  teststream << setw(5) << epoch_ << "  " << results << endl;
}

string Learner::basename() const
{       
  if(!experiment_name.empty())
    {
        PLWARNING("** Warning: the experiment_name system is DEPRECATED, please use the expdir system from now on, through setExperimentDirectory, and don't set an experiment_name. For now I'll be using the specified experiment_name=%s as the default basename for your results, but this won't be supported in the future",experiment_name.c_str());
      return experiment_name;
    }
  else if(expdir.empty())
    {
      PLERROR("Problem in Learner: Please call setExperimentDirectory for your learner prior to calling a train/test");
    }
  else if(!train_set)
    {
        PLWARNING("You should call setTrainingSet at the beginning of the train method in class %s ... Using 'unknown' as alias for now...", classname().c_str());
      return expdir + "unknown";
    }
  else if(train_set->getAlias().empty())
    {
        PLWARNING("The training set has no alias defined for it (you could call setAlias(...)) Using 'unknown' as alias");
      return expdir + "unknown";
    }
  return expdir+train_set->getAlias();
}

/*
string Learner::help() const
{
  return string(
                "Options for Learner: \n"
                "  - inputsize:  <integer> (0) \n"
                "    dimensionality of input vector \n"
                "  - outputsize: <integer> (0) \n"
                "    dimensionality of output \n"
                "  - targetsize: <integer> (0) \n"
                "    dimensionality of target \n"
                "  - earlystop_testsetnum: <int> (-1) \n"
                "    index of test set (in test_sets) to use for early \n"
                "    stopping (-1 means no early-stopping) \n"
                "  - earlystop_testresultindex: <int> (0) \n"
                "    index of statistic (as returned by test) to use\n"
                "  - earlystop_max_degradation: <real> (0) \n"
                "    maximum degradation in error from last best value\n"
                "  - earlystop_min_value: <real> (-FLT_MAX)\n"
                "    minimum error beyond which we stop\n"
                "  - earlystop_min_improvement: <real> (0)\n"
                "    minimum improvement in error otherwise we stop\n"
                "  - earlystop_relative_changes: <bool> (true) \n"
                "    are max_degradation and min_improvement relative?\n"
                "  - earlystop_save_best: <bool> (true) \n"
                "    if yes, then return with saved 'best' model\n"
                "  - earlystop_max_degraded_steps: <int> (-1) \n"
                "    ax. nb of steps beyond best found (-1 means ignore) \n"
                "  - save_at_every_epoch: <bool> (false)\n"
                "    save learner at each epoch?\n")
    + Object::help();
}
*/

void Learner::declareOptions(OptionList& ol)
{
  declareOption(ol, "inputsize", &Learner::inputsize_, OptionBase::buildoption, 
                "    dimensionality of input vector \n");

  declareOption(ol, "outputsize", &Learner::outputsize_, OptionBase::buildoption, 
                "    dimensionality of output \n");

  declareOption(ol, "targetsize", &Learner::targetsize_, OptionBase::buildoption, 
                "    dimensionality of target \n");

  declareOption(ol, "weightsize", &Learner::weightsize_, OptionBase::buildoption, 
                "    NOTE: THIS WORKS ONLY WITH NeuralNet !!! (for now -- just don't set this value for other learners.)  Number of weights within target.  The last 'weightsize' fields of the target vector will be used as cost weights.  (0 <= weightsize < targetsize) \n");

  declareOption(ol, "earlystop_testsetnum", &Learner::earlystop_testsetnum, OptionBase::buildoption, 
                "    index of test set (in test_sets) to use for early \n"
                "    stopping (-1 means no early-stopping) \n");

  declareOption(ol, "earlystop_testresultindex", &Learner::earlystop_testresultindex, OptionBase::buildoption, 
                "    index of statistic (as returned by test) to use\n");

  declareOption(ol, "earlystop_max_degradation", &Learner::earlystop_max_degradation, OptionBase::buildoption, 
                "    maximum degradation in error from last best value\n");

  declareOption(ol, "earlystop_min_value", &Learner::earlystop_min_value, OptionBase::buildoption, 
                "    minimum error beyond which we stop\n");

  declareOption(ol, "earlystop_min_improvement", &Learner::earlystop_min_improvement, OptionBase::buildoption, 
                "    minimum improvement in error otherwise we stop\n");

  declareOption(ol, "earlystop_relative_changes", &Learner::earlystop_relative_changes, OptionBase::buildoption, 
                "    are max_degradation and min_improvement relative?\n");

  declareOption(ol, "earlystop_save_best", &Learner::earlystop_save_best, OptionBase::buildoption, 
                "    if yes, then return with saved 'best' model\n");

  declareOption(ol, "earlystop_max_degraded_steps", &Learner::earlystop_max_degraded_steps, OptionBase::buildoption, 
                "    ax. nb of steps beyond best found (-1 means ignore) \n");

  declareOption(ol, "save_at_every_epoch", &Learner::save_at_every_epoch, OptionBase::buildoption, 
                "    save learner at each epoch?\n");

  declareOption(ol, "expdir", &Learner::expdir, OptionBase::buildoption,
                "   The directory in which to save results \n");

  declareOption(ol, "test_costfuncs", &Learner::test_costfuncs, OptionBase::buildoption,
                "   The cost functions used by the default useAndCost method \n");

  declareOption(ol, "test_statistics", &Learner::test_statistics, OptionBase::buildoption,
                "   The test statistics used by the default test method \n",
                "mean_stats() & stderr_stats()");

  declareOption(ol, "test_every", &Learner::test_every, OptionBase::buildoption, 
                "????");

  declareOption(ol, "minibatch_size", &Learner::minibatch_size, 
                OptionBase::buildoption, 
                "size of blocks over which to perform tests, calling 'apply' if >1, otherwise caling 'use'\n");

  inherited::declareOptions(ol);
}

/*
string Learner::getOptionsToSave() const
{ 
  return string("inputsize "
                "outputsize "
                "targetsize "
                "expdir "
                "save_at_every_epoch "
                "use_file_if_bigger "
                "test_every "
                "earlystop_testsetnum "
                "earlystop_testresultindex "
                "earlystop_max_degradation "
                "earlystop_min_value "
                "earlystop_min_improvement "
                "earlystop_relative_changes "
                "earlystop_save_best "
                "earlystop_max_degraded_steps "
                "save_at_every_epoch "
                "experiment_name "
                "test_costfuncs "
                "test_statistics ")
    + inherited::getOptionsToSave(); 
}

void Learner::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="inputsize")
    PLearn::read(in,inputsize_);
  else if (optionname=="outputsize")
    PLearn::read(in,outputsize_);
  else if (optionname=="targetsize")
    PLearn::read(in,targetsize_);
  else if (optionname=="experiment_name")
    PLearn::read(in,experiment_name);
  else if (optionname=="expdir")
    PLearn::read(in,expdir);
  else if (optionname=="use_file_if_bigger")
    PLearn::read(in,use_file_if_bigger);
  else if (optionname=="measure_cpu_time_first")
    PLearn::read(in,measure_cpu_time_first);
  else if (optionname=="test_every") // recently added by senecal
    PLearn::read(in,test_every);
  else if (optionname=="earlystop_testsetnum")
    PLearn::read(in,earlystop_testsetnum);
  else if (optionname=="earlystop_testresultindex")
    PLearn::read(in,earlystop_testresultindex);
  else if (optionname=="earlystop_max_degradation")
    PLearn::read(in,earlystop_max_degradation);
  else if (optionname=="earlystop_min_value")
    PLearn::read(in,earlystop_min_value);
  else if (optionname=="earlystop_min_improvement")
    PLearn::read(in,earlystop_min_improvement);
  else if (optionname=="earlystop_relative_changes")
    PLearn::read(in,earlystop_relative_changes);
  else if (optionname=="earlystop_save_best")
    PLearn::read(in,earlystop_save_best);
  else if (optionname=="earlystop_max_degraded_steps")
    PLearn::read(in,earlystop_max_degraded_steps);
  else if (optionname=="save_at_every_epoch")
    PLearn::read(in,save_at_every_epoch);
  else if (optionname=="experiment_name")
    PLearn::read(in,experiment_name);
  else if (optionname=="test_costfuncs")
    PLearn::read(in,test_costfuncs);
  else if (optionname=="test_statistics")
    PLearn::read(in,test_statistics);
  else
    inherited::readOptionVal(in, optionname);  

}



void Learner::writeOptionVal(ostream& out, const string& optionname) const
{
  if (optionname=="inputsize")
    PLearn::write(out,inputsize_);
  else if (optionname=="outputsize")
    PLearn::write(out,outputsize_);
  else if (optionname=="targetsize")
    PLearn::write(out,targetsize_);
  else if (optionname=="experiment_name")
    PLearn::write(out,experiment_name);
  else if (optionname=="expdir")
    PLearn::write(out,expdir);
  else if (optionname=="use_file_if_bigger")
    PLearn::write(out,use_file_if_bigger);
  else if (optionname=="measure_cpu_time_first")
    PLearn::write(out,measure_cpu_time_first);
  else if (optionname=="test_every") // recently added by senecal
    PLearn::write(out,test_every);
  else if (optionname=="earlystop_testsetnum")
    PLearn::write(out,earlystop_testsetnum);
  else if (optionname=="earlystop_testresultindex")
    PLearn::write(out,earlystop_testresultindex);
  else if (optionname=="earlystop_max_degradation")
    PLearn::write(out,earlystop_max_degradation);
  else if (optionname=="earlystop_min_value")
    PLearn::write(out,earlystop_min_value);
  else if (optionname=="earlystop_min_improvement")
    PLearn::write(out,earlystop_min_improvement);
  else if (optionname=="earlystop_relative_changes")
    PLearn::write(out,earlystop_relative_changes);
  else if (optionname=="earlystop_save_best")
    PLearn::write(out,earlystop_save_best);
  else if (optionname=="earlystop_max_degraded_steps")
    PLearn::write(out,earlystop_max_degraded_steps);
  else if (optionname=="save_at_every_epoch")
    PLearn::write(out,save_at_every_epoch);
  else if (optionname=="experiment_name")
    PLearn::write(out,experiment_name);
  else if (optionname=="test_costfuncs")
    PLearn::write(out,test_costfuncs);
  else if (optionname=="test_statistics")
    PLearn::write(out,test_statistics);
  else
    inherited::writeOptionVal(out, optionname);  
}
*/


void Learner::setExperimentDirectory(const string& the_expdir) 
{ 
#if USING_MPI
  if(PLMPI::rank==0) {
#endif
  if(!force_mkdir(the_expdir))
  {
    PLERROR("In Learner::setExperimentDirectory Could not create experiment directory %s",the_expdir.c_str());}
#if USING_MPI
  }
#endif
  expdir = abspath(the_expdir);
}

void Learner::build_()
{
  // Early stopping initialisation
  earlystop_previousval = FLT_MAX;
  earlystop_minval = FLT_MAX;
}

void Learner::build()
{
  inherited::build();
  build_();
}

void Learner::forget()
{
  // Early stopping parameters initialisation
  earlystop_previousval = FLT_MAX;
  earlystop_minval = FLT_MAX;
  epoch_ = 0;
}

void Learner::useAndCostOnTestVec(const VMat& test_set, int i, const Vec& output, const Vec& cost)
{
  tmpvec.resize(test_set.width());
  test_set->getRow(i,tmpvec);
  useAndCost(tmpvec.subVec(0,inputsize()), tmpvec.subVec(inputsize(),targetsize()), output, cost);
}

void Learner::useAndCost(const Vec& input, const Vec& target, Vec output, Vec cost)
{
  // cout << "target: " << target[0] << endl;
  use(input,output);
  computeCost(input, target, output, cost);
}

void Learner::computeCost(const Vec& input, const Vec& target, const Vec& output, const Vec& cost)
{
  for (int k=0; k<test_costfuncs.size(); k++)
    cost[k] = test_costfuncs[k](output, target);
  // cerr << "target=" << target << endl;
  // cerr << "output=" << output << endl;
  // cerr << "cost  =" << cost << endl;
  // if(argmax(output)!=int(target[0]))
  //   cerr << '!';
}

void Learner::setTestDuringTrain(ostream& out, int every, Array<VMat> testsets)
{
  testout = out;
  test_every = every;
  test_sets = testsets;
}

void Learner::openTrainObjectiveStream()
{
  string filename = expdir.empty() ? string("/dev/null") : expdir+"train.objective";
  if(train_objective_stream)
    delete train_objective_stream;
  train_objective_stream = new ofstream(filename.c_str(),ios::out|ios::app);
  ostream& out = *train_objective_stream;
  if(out.bad())
    PLERROR("could not open file %s for appending",filename.c_str());
  if(out.tellp()==0)
    out << "#  epoch | " << join(trainObjectiveNames()," | ") << endl;
}

ostream& Learner::getTrainObjectiveStream()
{
  if(!train_objective_stream)
    openTrainObjectiveStream();
  return *train_objective_stream;
}

//! opens the files in append mode for writing the test results
void Learner::openTestResultsStreams()
{
  freeTestResultsStreams();
  int n = test_sets.size();
  test_results_streams.resize(n);
  for(int k=0; k<n; k++)
    {
      string alias = test_sets[k]->getAlias();
      // if(alias.empty())
      //   PLERROR("In Learner::openTestResultsStreams testset #%d has no defined alias",k);
      string filename = alias.empty() ? string("/dev/null") : expdir+alias+".results";
      test_results_streams[k] = new ofstream(filename.c_str(), ios::out|ios::app);
      ostream& out = *test_results_streams[k];
      if(out.bad())
        PLERROR("In Learner::openTestResultsStreams could not open file %s for appending",filename.c_str());
      if(out.tellp()==0)
        out << "#: epoch " << join(testResultsNames()," ") << endl;
    }
}

void Learner::freeTestResultsStreams()
{
  int n = test_results_streams.size();
  for(int k=0; k<n; k++)
    delete test_results_streams[k];
  test_results_streams.resize(0);
}

// There are as many test results streams as there are 
ostream& Learner::getTestResultsStream(int k) 
{ 
  if(test_results_streams.size()==0)
    openTestResultsStreams();
  return *test_results_streams[k]; 
}


void Learner::setTestDuringTrain(Array<VMat> testsets)
{  test_sets = testsets; }

Learner::~Learner()
{
  if(train_objective_stream)
    delete train_objective_stream;
  freeTestResultsStreams();
}

// which_testset and which_testresult select the appropriate testset and
// costfunction to base early-stopping on from those that were specified
// in setTestDuringTrain 
// * degradation is the difference between the current value and the
// smallest value ever attained, training will be stopped if it grows
// beyond max_degradation 
// * training will be stopped if current value goes below min_value
// * training will be stopped if difference between previous value and
// current value is below min_improvement
void Learner::setEarlyStopping(int which_testset, int which_testresult, 
                               real max_degradation, real min_value, 
                               real min_improvement, bool relative_changes,
                               bool save_best, int max_degraded_steps)
{
  earlystop_testsetnum = which_testset;
  earlystop_testresultindex = which_testresult;
  earlystop_max_degradation = max_degradation;
  earlystop_min_value = min_value;
  earlystop_previousval = FLT_MAX;
  earlystop_minval = FLT_MAX;
  earlystop_relative_changes = relative_changes;
  earlystop_min_improvement = min_improvement;
  earlystop_save_best = save_best;
  earlystop_max_degraded_steps = max_degraded_steps;
}

bool Learner::measure(int step, const Vec& costs)
{
  if (costs.length()<1)
    PLERROR("Learner::measure: costs.length_=%d should be >0", costs.length());
  
//  vlog << ">>> Now measuring for step " << step << " (costs = " << costs << " )" << endl; 

  //  if (objectiveout)
  //  objectiveout << setw(5) << step << "  " << costs << "\n";


  if ((!PLMPI::synchronized && each_cpu_saves_its_errors) || PLMPI::rank==0)
    outputResultLineToFile(basename()+".objective",costs,true,join(trainObjectiveNames()," "));

  bool muststop = false;

  if (((!PLMPI::synchronized && each_cpu_saves_its_errors) || PLMPI::rank==0) && save_at_every_epoch)
    {
      string fname  = basename()+".epoch"+tostring(epoch())+".psave";
      vlog << " >> Saving model in " << fname << endl;
      PLearn::save(fname, *this);
    }
  if (step%test_every==0)
  {
    int ntestsets = test_sets.size();
    Array<Vec> test_results(ntestsets);
    for (int n=0; n<ntestsets; n++) // looping over test sets
    {
      //vlog << " >> testing on set #" << n << " ..." << endl; 
      test_results[n] = test(test_sets[n]);
      // testout << test_results[n] << "    ";
      if ((!PLMPI::synchronized && each_cpu_saves_its_errors) || PLMPI::rank==0)
        outputResultLineToFile(basename()+"."+test_sets[n]->getAlias()+".hist.results",test_results[n],true,
                                   join(testResultsNames()," "));
      // getTestResultsStream(n) << setw(5) << epoch_ << "  " << test_results[n] << endl;
    }
    //    if (!measure_cpu_time_first)
    //  testout<<getStopRunningTime(true,true);
    // testout<<endl;
    if (ntestsets>0 && earlystop_testsetnum>=0) // are we doing early stopping?
    {
      real earlystop_currentval = 
        test_results[earlystop_testsetnum][earlystop_testresultindex];
      // Check if early-stopping condition was met
      if ((earlystop_relative_changes &&
           ((earlystop_currentval-earlystop_minval > 
             earlystop_max_degradation * abs(earlystop_minval))
            || (earlystop_currentval < earlystop_min_value)
            || (earlystop_previousval-earlystop_currentval < 
                earlystop_min_improvement * abs(earlystop_previousval)))) ||
          (!earlystop_relative_changes &&
           ((earlystop_currentval-earlystop_minval > earlystop_max_degradation)
            || (earlystop_currentval < earlystop_min_value)
            || (earlystop_previousval-earlystop_currentval < 
                earlystop_min_improvement))) ||
          (earlystop_max_degraded_steps>=0 &&
           (step-best_step>=earlystop_max_degraded_steps) && 
           (earlystop_minval < FLT_MAX)))
      { // earlystopping met
        if (earlystop_save_best)
        {
          string fname  = basename()+".psave";
          vlog << "Met early-stopping condition!" << endl;
          vlog << "earlystop_currentval = " << earlystop_currentval << endl;
          vlog << "earlystop_minval = " << earlystop_minval << endl;
          vlog << "threshold = " << earlystop_max_degradation*earlystop_minval << endl;
          vlog << "STOPPING (reloading best model)" << endl;
          if(expdir.empty()) // old deprecated mode
            load();
          else
            PLearn::load(fname,*this, OptionBase::learntoption);
          
        }
        muststop = true;
      }
      else // earlystopping not met
      {
        earlystop_previousval = earlystop_currentval;
        if (PLMPI::rank==0 && earlystop_save_best
            && (earlystop_currentval < earlystop_minval))
        {
          string fname  = basename()+".psave";
          vlog << "saving model in " << fname <<  " because of earlystopping / improvement: " << endl;
          vlog << "earlystop_currentval = " << earlystop_currentval << endl;
          vlog << "earlystop_minval = " << earlystop_minval << endl;
          PLearn::save(fname,*this);
          // update .results file
          if ((!PLMPI::synchronized && each_cpu_saves_its_errors) || PLMPI::rank==0)
            for (int n=0; n<ntestsets; n++) // looping over test sets
              outputResultLineToFile(basename()+"."+test_sets[n]->getAlias()+".results",test_results[n],false,
                                     join(testResultsNames()," "));
        }
      }
      if (earlystop_currentval < earlystop_minval)
      {
        earlystop_minval = earlystop_currentval;
        best_step = step;
        if(PLMPI::rank==0)
          vlog << "currently best step at " << best_step << " with " << earlystop_currentval << endl;
        
      }
    } 
    else
      // save tests in .results
      if ((!PLMPI::synchronized && each_cpu_saves_its_errors) || PLMPI::rank==0)
        for (int n=0; n<ntestsets; n++) // looping over test sets
          outputResultLineToFile(basename()+"."+test_sets[n]->getAlias()+".results",test_results[n],false,
                                 join(testResultsNames()," "));
  }

  for (int i=0; i<measurers.size(); i++)
    muststop = muststop || measurers[i]->measure(step,costs);

  ++epoch_;

// BUG: This doesn't work as intented in certain cases (ie. me!)
//#if USING_MPI
//MPI_Barrier(MPI_COMM_WORLD);
//#endif

  return muststop;
}

// Call the 'use' method many times on the first inputsize() elements of
// each row of a 'data' VMat, and put the
// machine's 'outputs' in a writable VMat (e.g. maybe a file, or a matrix).
void Learner::apply(const VMat& data, VMat outputs)
{
  int n=data.length();
  Vec data_row(data.width());
  Vec input = data_row.subVec(0,inputsize());
  Vec output(outputsize());
  for (int i=0;i<n;i++)
  {
    data->getRow(i,data_row); // also gets input_row and target
    use(input,output);
    outputs->putRow(i,output);
  }
}

// This method calls useAndCost repetitively on all the rows of data,
// throwing away the resulting output vectors but putting all the cost vectors
// in the costs VMat.
  void Learner::computeCosts(const VMat& data, VMat costs)
  {
    int n=data.length();
    int ncostfuncs = costsize();
    // Vec data_row(data.width());
    // Vec input_row = data_row.subVec(0,inputsize());
    // Vec target = data_row.subVec(inputsize(), targetsize());
    Vec output_row(outputsize());
    Vec cost(ncostfuncs);
    for (int i=0;i<n;i++)
    {
      // data->getRow(i,data_row); // also gets input_row and target
      // useAndCost(input_row,target,output_row,cost); // does the work
      useAndCostOnTestVec(data, i, output_row, cost);
      costs->putRow(i,cost); // save the costs
    }
  }

  void Learner::computeLeaveOneOutCosts(const VMat& data, VMat costsmat)
  {
    //    Vec testsample(inputsize()+targetsize());
    // Vec testinput = testsample.subVec(0,inputsize());
    // Vec testtarget = testsample.subVec(inputsize(),targetsize());
    Vec output(outputsize());
    Vec cost(costsize());
    // VMat subset;
    for(int i=0; i<data.length(); i++)
    {
      // data->getRow(i,testsample);
      train(removeRow(data,i));
      useAndCostOnTestVec(data, i, output, cost);
      // useAndCost(testinput,testtarget,output,cost);
      costsmat->putRow(i,cost);
      vlog << '.' << flush;
      if(i%100==0)
        vlog << '\n' << i << flush;
    }
  }

  void Learner::computeLeaveOneOutCosts(const VMat& data, VMat costsmat, CostFunc costf)
  {
    if(costsmat.length() != data.length() | costsmat.width()!=1)
      PLERROR("In Learner::computeLeaveOneOutCosts bad dimensions for costsmat VMat");
    Vec testsample(inputsize()+targetsize());
    Vec testinput = testsample.subVec(0,inputsize());
    Vec testtarget = testsample.subVec(inputsize(),targetsize());
    Vec output(outputsize());
    VMat subset;
    for(int i=0; i<data.length(); i++)
    {
      data->getRow(i,testsample);
      train(removeRow(data,i));
      use(testinput,output);
      costsmat->put(i,0,costf(output,testtarget));
      vlog << '.' << flush;
      if(i%100==0)
        vlog << '\n' << i << flush;
    }
  }

// This method calls useAndCost repetitively on all the rows of data,
// putting all the resulting output and cost vectors in the outputs and
// costs VMat's.
  void Learner::applyAndComputeCosts(const VMat& data, VMat outputs, VMat costs)
  {
    int n=data.length();
    int ncostfuncs = costsize();
    // Vec data_row(data.width());
    // Vec input_row = data_row.subVec(0,inputsize());
    // Vec target = data_row.subVec(inputsize(), targetsize());
    Vec output_row(outputsize());
    Vec costs_row(ncostfuncs);
    for (int i=0;i<n;i++)
    {
      // data->getRow(i,data_row); // also gets input_row and target
      useAndCostOnTestVec(data, i, output_row, costs_row);
      // useAndCost(input_row,target,output_row,costs_row); // does the work
      outputs->putRow(i,output_row); // save the outputs
      costs->putRow(i,costs_row); // save the costs
    }
  }

  Vec Learner::computeTestStatistics(const VMat& costs)
  {
    return concat(test_statistics.computeStats(costs));
  }


// [PASCAL TODO:] 
// 1) Handle weights properly
// 2) Fix parallel code to use MPIStream for more efficient buffering (and check Yoshua's problem)
// 4) let save parameters be VMatrix (on which to call append)

//! This function should work with and without MPI
//! Only MPI process 0 will save to file and gather and compute statistics
//! All others MPI processes will call useAndCost on different sections of the test_set

Vec Learner::test(VMat test_set, const string& save_test_outputs, const string& save_test_costs)
{
  int ncostfuncs = costsize();

  Vec output(outputsize());
  Vec cost(ncostfuncs);
  Mat output_block(minibatch_size,outputsize());
  Mat cost_block(minibatch_size,outputsize());

  Vec result;

  VMat outputs; // possibly where to save outputs (and target)
  VMat costs; // possibly where to save costs
  if(PLMPI::rank==0 && !save_test_outputs.empty())
    outputs = new FileVMatrix(save_test_outputs, test_set.length(), outputsize());

  if(PLMPI::rank==0 && !save_test_costs.empty())
    costs = new FileVMatrix(save_test_costs, test_set.length(), ncostfuncs);

  int l = test_set.length();
  ProgressBar progbar(vlog, "Testing " + test_set->getAlias(), l);
  // ProgressBar progbar(cerr, "Testing " + test_set->getAlias(), l);
  // ProgressBar progbar(nullout(), "Testing " + test_set->getAlias(), l);

  // Do the test statistics require multiple passes?
  bool multipass = test_statistics.requiresMultiplePasses(); 

  // If multiple passes are required, make sure we save the individual costs in an appropriate 'costs' VMat
  if (PLMPI::rank==0 && save_test_costs.empty() && multipass)
  {
    TmpFilenames tmpfile(1);
    bool save_on_file = ncostfuncs*test_set.length() > use_file_if_bigger;
    if (save_on_file)
      costs = new FileVMatrix(tmpfile.newFile(),test_set.length(),ncostfuncs);
    else
      costs = Mat(test_set.length(),ncostfuncs);
  }

  if(!multipass) // stats can be computed in a single pass?
    test_statistics.init(ncostfuncs);

  if(USING_MPI && PLMPI::synchronized && PLMPI::size>1)
    { // parallel implementation
      // cout << "PARALLEL-DATA TEST" << endl;
#if USING_MPI
      PLMPI::synchronized = false;
      if(PLMPI::rank==0) // process 0 gathers costs, computes statistics and writes stuff to output files if required
        {
          MPIStreams mpistreams(200,200);
//          MPI_Status status;
          for(int i=0; i<l; i++)
            {
              int pnum = 1 + i%(PLMPI::size-1);
              if(!save_test_outputs.empty()) // receive and save output
                {
//                  MPI_Recv(cost.data(), cost.length(), PLMPI_REAL, pnum, 0, MPI_COMM_WORLD, &status);
                  //cerr << "/ MPI #" << PLMPI::rank << " received " << cost.length() << " values from MPI #" << pnum << endl;
                  PLearn::binread(mpistreams[pnum], output);
                  outputs->putRow(i, output);
                }
/*              else // receive output and cost only
                {
                  MPI_Recv(output.data(), output.length()+cost.length(), PLMPI_REAL, pnum, 0, MPI_COMM_WORLD, &status);
									//cerr << "/ MPI #" << PLMPI::rank << " received " << cost.length() << " values from MPI #" << pnum << endl;
                  outputs->putRow(i,output);
									}*/
              // receive cost
              PLearn::binread(mpistreams[pnum], cost);
              if(costs) // save costs?
                costs->putRow(i,cost);
              if(!multipass) // stats can be computed in a single pass?
                test_statistics.update(cost);
              progbar(i);
            }
        }
      else // other processes compute output and cost on different rows of the test_set and send them to process 0
        {
          MPIStream mpistream(0,200,200); // stream to node 0
          int step = PLMPI::size-1;
          for(int i=PLMPI::rank-1; i<l; i+=step)
            {
              useAndCostOnTestVec(test_set, i, output, cost);
              // test_set->getRow(i, sample);
              // useAndCost(input,target,output,cost);
/*              if(save_test_outputs.empty()) // send only cost
                {
                  //cerr << "/ MPI #" << PLMPI::rank << " sending " << cost.length() << " values to MPI #0" << endl;
                  MPI_Send(cost.data(), cost.length(), PLMPI_REAL, 0, 0, MPI_COMM_WORLD);
                }
              else // send output and cost only
							{
								//cerr << "/ MPI #" << PLMPI::rank << " sending " << cost.length() << " values to MPI #0" << endl;
                MPI_Send(output.data(), output.length()+cost.length(), PLMPI_REAL, 0, 0, MPI_COMM_WORLD);
							}
						}
						}*/
              if(!save_test_outputs.empty()) // send output
                PLearn::binwrite(mpistream, output);
              // send cost
              PLearn::binwrite(mpistream, cost);
            }
        }

      // Finalize statistics computation
      int result_len;
      if(PLMPI::rank==0) // process 0 finalizes stats computation and broadcasts them
        {
          if(!multipass)
            {
              test_statistics.finish();
              result = concat(test_statistics.getResults());
            }
          else    
            result = concat(test_statistics.computeStats(costs));
          result_len = result.length();
        }
      MPI_Bcast(&result_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
      result.resize(result_len);
      MPI_Bcast(result.data(), result.length(), PLMPI_REAL, 0, MPI_COMM_WORLD);
      PLMPI::synchronized = true;
#endif
    }
  else // default sequential implementation
    {
      for (int i=0; i<l; i++)
        {
          if (i%10000<minibatch_size) stop_if_wanted();
          if (minibatch_size>1 && i+minibatch_size<l)
          {
            applyAndComputeCostsOnTestMat(test_set, i, output_block, cost_block);
            i+=minibatch_size;
            if(outputs) // save outputs?
              outputs->putMat(i,0,output_block);
            if(costs) // save costs?
              costs->putMat(i,0,cost_block);
            if(!multipass) // stats can be computed in a single pass?
              test_statistics.update(cost_block);
          }
          else
          {
            useAndCostOnTestVec(test_set, i, output, cost);
            if(outputs) // save outputs?
              outputs->putRow(i,output);
            if(costs) // save costs?
              costs->putRow(i,cost);
            if(!multipass) // stats can be computed in a single pass?
              test_statistics.update(cost);
          }
          // test_set->getRow(i, sample);
          // useAndCost(input, target, output, cost);

          progbar(i);

        }

      // Finalize statistics computation
      if(!multipass)
        {
          test_statistics.finish();
          result = concat(test_statistics.getResults());
        }
      else    
        result = concat(test_statistics.computeStats(costs));

    }

  return result;
}

void Learner::applyAndComputeCostsOnTestMat(const VMat& test_set, int i, const Mat& output_block, 
                                            const Mat& cost_block)
{
  applyAndComputeCosts(test_set.subMatRows(i,output_block.length()),output_block,cost_block);
}

void Learner::setModel(const Vec& options) { 
  PLERROR("setModel: method not implemented for this Learner (and DEPRECATED!!! DON'T IMPLEMENT IT, DON'T CALL IT. SEE setOption INSTEAD)"); 
}

int Learner::costsize() const 
{ return test_costfuncs.size(); }

Array<string> Learner::costNames() const
  {
    Array<string> cost_names(test_costfuncs.size());
    for (int i=0; i<cost_names.size(); i++)
      cost_names[i] = space_to_underscore(test_costfuncs[i]->info());
    return cost_names;
  }

  Array<string> Learner::testResultsNames() const
  {
    Array<string> cost_names = costNames();
    Array<string> names(test_statistics.size()*cost_names.size());
    int k=0;
    for (int i=0;i<test_statistics.size();i++)
    {
      string stati = test_statistics[i]->info();
      for (int j=0;j<cost_names.size();j++)
        names[k++] = space_to_underscore(cost_names[j] + "." + stati);
    }
    return names;
  }

Array<string> Learner::trainObjectiveNames() const
{ return testResultsNames(); }

  void Learner::oldwrite(ostream& out) const
  {
    writeHeader(out,"Learner",1);
    writeField(out,"inputsize",inputsize_);
    writeField(out,"outputsize",outputsize_);
    writeField(out,"targetsize",targetsize_);
    writeField(out,"test_every",test_every); // recently added by senecal
    writeField(out,"earlystop_testsetnum",earlystop_testsetnum);
    writeField(out,"earlystop_testresultindex",earlystop_testresultindex);
    writeField(out,"earlystop_max_degradation",earlystop_max_degradation);
    writeField(out,"earlystop_min_value",earlystop_min_value);
    writeField(out,"earlystop_min_improvement",earlystop_min_improvement);
    writeField(out,"earlystop_relative_changes",earlystop_relative_changes);
    writeField(out,"earlystop_save_best",earlystop_save_best);
    writeField(out,"earlystop_max_degraded_steps",earlystop_max_degraded_steps);
    writeField(out,"save_at_every_epoch",save_at_every_epoch);
    writeField(out,"experiment_name",experiment_name);
    writeField(out,"test_costfuncs",test_costfuncs);
    writeField(out,"test_statistics",test_statistics);
    writeFooter(out,"Learner");
  }

  void Learner::oldread(istream& in)
  {
    int version=readHeader(in,"Learner");
    if(version>=2)
      {
        readField(in,"expdir",expdir);
        readField(in,"epoch",epoch_);
      }
    readField(in,"inputsize",inputsize_);
    readField(in,"outputsize",outputsize_);
    readField(in,"targetsize",targetsize_);
    readField(in,"test_every",test_every);
    readField(in,"earlystop_testsetnum",earlystop_testsetnum);
    readField(in,"earlystop_testresultindex",earlystop_testresultindex);
    readField(in,"earlystop_max_degradation",earlystop_max_degradation);
    readField(in,"earlystop_min_value",earlystop_min_value);
    readField(in,"earlystop_min_improvement",earlystop_min_improvement);
    readField(in,"earlystop_relative_changes",earlystop_relative_changes);
    readField(in,"earlystop_save_best",earlystop_save_best);
    if (version>=1)
      readField(in,"earlystop_max_degraded_steps",earlystop_max_degraded_steps);
    else
      earlystop_max_degraded_steps=-1;
    readField(in,"save_at_every_epoch",save_at_every_epoch);
    readField(in,"experiment_name",experiment_name);
    readField(in,"test_costfuncs",test_costfuncs);
    readField(in,"test_statistics",test_statistics);
    readFooter(in,"Learner");
  }

  void Learner::save(const string& filename) const
  {
#if USING_MPI
    if (PLMPI::rank!=0 && !force_saving_on_all_processes)
      return;
#endif
    if(!filename.empty())
      Object::save(filename);
    else if(!experiment_name.empty())
      Object::save(experiment_name);
    else
      PLERROR("Called Learner::save with an empty filename, while experiment_name is also empty. What file name am I supposed to use???? Anyway this method is DEPRECATED, you should call directly function PLearn::save(whatever_filename_you_want, the_object) ");
  }

  void Learner::load(const string& filename)
  {
    if (!filename.empty())
      Object::load(filename);
    else if (!experiment_name.empty())
      Object::load(experiment_name);
    else
      PLERROR("Called Learner::load with an empty filename, while experiment_name is also empty. What file name am I supposed to use???? Anyway this method is DEPRECATED, you should call directly function PLearn::load(whatever_filename_you_want, the_object) ");
  }

  void Learner::stop_if_wanted()
  {
    string stopping_filename = basename()+".stop";
    if (file_exists(stopping_filename.c_str()))
    {
#ifdef PROFILE
      string profile_report_name = basename();
#if USING_MPI
      profile_report_name += "_r" + tostring(PLMPI::rank);;
#endif
      profile_report_name += ".profile";
      ofstream profile_report(profile_report_name.c_str());
      Profiler::report(profile_report);
#endif
#if USING_MPI
      MPI_Barrier(MPI_COMM_WORLD);
      if (PLMPI::rank==0)
      {
        string fname = basename()+".stopped.psave";
        PLearn::save(fname,*this);
        vlog << "saving and quitting because of stop signal" << endl;
        unlink(stopping_filename.c_str()); // remove file if possible
      }
      exit(0);
#else
      unlink(stopping_filename.c_str()); // remove file if possible
      exit(0);
#endif
    }
  }



%> // end of namespace PLearn

