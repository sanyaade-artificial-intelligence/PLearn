// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001,2002 Pascal Vincent
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
   * $Id: StatsCollector.cc,v 1.2 2002/08/09 16:14:32 jkeable Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "StatsCollector.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

  IMPLEMENT_NAME_AND_DEEPCOPY(StatsCollector);

  StatsCollector::StatsCollector(int the_maxnvalues)
    : nmissing_(0), nnonmissing_(0), 
    sum_(0.), sumsquare_(0.), 
    min_(MISSING_VALUE), max_(MISSING_VALUE),
    maxnvalues(the_maxnvalues)
  {
    counts[FLT_MAX] = StatsCollectorCounts();
  }
  
  void StatsCollector::update(real val)
  {
    if(is_missing(val))
      nmissing_++;
    else
      {
        double sqval = val*val;
        sum_ += val;
        sumsquare_ += sqval;
        if(nnonmissing_==0) // first value encountered
          min_ = max_ = val;
        else if(val<min_)
          min_ = val;
        else if(val>max_)
          max_ = val;
        nnonmissing_++;

        map<real,StatsCollectorCounts>::iterator it;
        if(int(counts.size())<=maxnvalues) // Still remembering new unseen values
        {
          it = counts.find(val);
          if(it==counts.end())
            counts[val].id=counts.size()-1;
          counts[val].n++;
        }
        else // We've filled up counts already
          {
            it = counts.lower_bound(val);
            if(it->first==val) // found the exact value
              it->second.n++;
            else // found the value just above val (possibly FLT_MAX)
              {
                it->second.nbelow++;
                it->second.sum += val;
                it->second.sumsquare += sqval;
              }
          }
      }
  }                           

  RealMapping StatsCollector::getBinMapping(int discrete_mincount, int continuous_mincount) const
  {
    real mapto=0.;
    RealMapping mapping;
    mapping.setMappingForOther(-1);
    map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
    int nleft = counts.size()-1; // loop on all but last

    int count = 0;
    real low = min_;
    real high = min_;
    bool low_has_been_appended = false;
    while(nleft--)
      {
        high = it->first;
        count += it->second.nbelow;
        cout<<count<<endl;
        if(count>=continuous_mincount)
          {
            // append continuous range
            mapping.addMapping(RealRange(low_has_been_appended?']':'[',low, high, '['), mapto++);
            low = high;
            low_has_been_appended = false;
            count = 0;
          }

        if(it->second.n >= discrete_mincount)
          {
            if(count>0) // then append the previous continuous range
              {
                mapping.addMapping(RealRange(low_has_been_appended?']':'[',low, high, '['), mapto++);
                count = 0;
              }
            // append discrete point
            mapping.addMapping(RealRange('[',high,high,']'), mapto++);
            low = high;
            low_has_been_appended = true;
          }
        else
          count += it->second.n;

        ++it;
      }

    if(it->first<=max_)
      PLERROR("Bug in StatsCollector::getBinMapping expected last element of mapping to be FLT_MAX...");

    // make sure we include max_
    pair<RealRange, real>& m = mapping.lastMapping();
    if(m.first.low == m.first.high)
      {
        // we appended a single point, let's append a range until max
        mapping.addMapping(RealRange(']',m.first.high,max_,']'), mapto++);
      }
    else // we appended a real range, let's extend it until max_
      {
        m.first.high = max_;
        m.first.rightbracket = ']';
      }      

    return mapping;
  }

  Mat StatsCollector::cdf(bool normalized) const
  {
    int l = 2*counts.size();

    Mat xy(l+1,2);
    int i=0;
    int currentcount = 0;
    xy(i,0) = min_;
    xy(i++,1) = 0;    
    map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
    map<real,StatsCollectorCounts>::const_iterator itend = counts.end();    
    for(; it!=itend; ++it)
      {
        real val = it->first;
        if(val>max_)
          val = max_;

        currentcount += it->second.nbelow;
        xy(i,0) = val;
        xy(i++,1) = currentcount;

        currentcount += it->second.n;
        xy(i,0) = val;
        xy(i++,1) = currentcount;        
      }
    if(normalized)
      xy.column(1) /= real(nnonmissing_);

    return xy;
  }

  void StatsCollector::print(ostream& out) const
  {
    out << "# samples: " << n() << "\n";
    out << "# missing: " << nmissing() << "\n";
    out << "mean: " << mean() << "\n";
    out << "stddev: " << stddev() << "\n";
    out << "stderr: " << stderror() << "\n";
    out << "min: " << min() << "\n";
    out << "max: " << max() << "\n\n";
    out << "counts size: " << counts.size() << "\n";
    /*
    map<real,Counts>::const_iterator it = counts.begin();
    map<real,Counts>::const_iterator itend = counts.end();
    for(; it!=itend; ++it)
    {
      out << "value: " << it->first 
          << "  #equal:" << it->second.n
          << "  #less:" << it->second.nbelow
          << "  avg_of_less:" << it->second.sum/it->second.nbelow << endl;
    }
    */
  }

  void StatsCollector::write(ostream& out) const
  {
    writeHeader(out,"StatsCollector",0);
    writeField(out, "nmissing_", nmissing_);    
    writeField(out, "nnonmissing_", nnonmissing_);    
    writeField(out, "sum_", sum_);
    writeField(out, "sumsquare_", sumsquare_);
    writeField(out, "min_", min_);
    writeField(out, "max_", max_);
    writeField(out, "maxnvalues", maxnvalues);

    writeFieldName(out, "counts");
    PLearn::write(out, (int)counts.size());
    writeNewline(out);
    map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
    map<real,StatsCollectorCounts>::const_iterator itend = counts.end();
    for(; it!=itend; ++it)
    {
      PLearn::write(out, it->first);
      PLearn::write(out, it->second.n);
      PLearn::write(out, it->second.nbelow);
      PLearn::write(out, it->second.sum);
      PLearn::write(out, it->second.sumsquare);
      writeNewline(out);
    }
    writeFooter(out,"StatsCollector");
  }

  void StatsCollector::read(istream& in)
  {
    int version = readHeader(in,"StatsCollector");
    if(version!=0)
      PLERROR("In StatsCollector::deepRead don't know how to read this version");
    readField(in, "nmissing_", nmissing_);    
    readField(in, "nnonmissing_", nnonmissing_);    
    readField(in, "sum_", sum_);
    readField(in, "sumsquare_", sumsquare_);
    readField(in, "min_", min_);
    readField(in, "max_", max_);
    readField(in, "maxnvalues", maxnvalues);

    readFieldName(in, "counts", true);
    counts.clear();
    int ncounts;
    PLearn::read(in, ncounts);
    readNewline(in);
    for(int i=0; i<ncounts; i++)
    {
      real value;
      StatsCollectorCounts c;
      PLearn::read(in, value);
      PLearn::read(in, c.n);
      PLearn::read(in, c.nbelow);
      PLearn::read(in, c.sum);
      PLearn::read(in, c.sumsquare);
      readNewline(in);
      counts[value] = c;
    }
    readFooter(in,"StatsCollector");
  }

  // *********************************
  // *** ConditionalStatsCollector ***
  // *********************************

  IMPLEMENT_NAME_AND_DEEPCOPY(ConditionalStatsCollector);

  ConditionalStatsCollector::ConditionalStatsCollector()
    :condvar(0) {}

  void ConditionalStatsCollector::setBinMappingsAndCondvar(const TVec<RealMapping>& the_ranges, int the_condvar) 
  { 
    ranges = the_ranges;
    condvar = the_condvar;
    int nvars = ranges.length();
    counts.resize(nvars);
    sums.resize(nvars);
    sumsquares.resize(nvars);
    int nranges_condvar = ranges[condvar].length();
    for(int k=0; k<nvars; k++)
      {        
        int nranges_k = ranges[k].length();
        counts[k].resize(nranges_k+1, nranges_condvar+1);
        sums[k].resize(nranges_k, nranges_condvar);
        sumsquares[k].resize(nranges_k, nranges_condvar);
      }
  }

  int ConditionalStatsCollector::findrange(int varindex, real val) const
  {
    RealMapping& r = ranges[varindex];
    if(is_missing(val))
      return r.length();
    else
      return (int) r.map(val);
  }
  
  void ConditionalStatsCollector::update(const Vec& v)
  {
    int nvars = ranges.length();
    if(v.length()!=nvars)
      PLERROR("IN ConditionalStatsCollectos::update length of update vector and nvars differ!");
    int j = findrange(condvar, v[condvar]);
    if(j==-1)
      PLWARNING("In ConditionalStatsCollector::update value of conditioning var in none of the ranges");
    for(int k=0; k<nvars; k++)
      {
        real val = v[k];
        int i = findrange(k, val);
        if(i==-1)
          PLWARNING("In ConditionalStatsCollector::update value of variable #%d in none of the ranges",k);
        counts[k](i,j)++;
        if(!is_missing(val))
          {
            sums[k](i,j) += val;
            sumsquares[k](i,j) += val;
          }
      }
  }

  void ConditionalStatsCollector::write(ostream& out) const
  {
    writeHeader(out,"ConditionalStatsCollector",0);
    writeField(out, "condvar", condvar);    
    writeField(out, "ranges", ranges);    
    writeField(out, "counts", counts);
    writeField(out, "sums", sums);
    writeField(out, "sumsquares", sumsquares);
    writeFooter(out,"ConditionalStatsCollector");
  }

  void ConditionalStatsCollector::oldread(istream& in)
  {
    int version = readHeader(in,"ConditionalStatsCollector");
    if(version!=0)
      PLERROR("In ConditionalStatsCollector::deepRead don't know how to read this version");
    readField(in, "condvar", condvar);    
    readField(in, "ranges", ranges);    
    readField(in, "counts", counts);
    readField(in, "sums", sums);
    readField(in, "sumsquares", sumsquares);
    readFooter(in,"ConditionalStatsCollector");
  }

TVec<RealMapping> computeRanges(TVec<StatsCollector> stats, int discrete_mincount, int continuous_mincount)
{
  TVec<RealMapping> ranges;
  int n = stats.length();
  ranges.resize(n);
  for(int k=0; k<n; k++)
    ranges[k] = stats[k].getBinMapping(discrete_mincount, continuous_mincount);
  return ranges;
}


%> // end of namespace PLearn


