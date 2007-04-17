// -*- C++ -*-

// RBMBinomialLayer.cc
//
// Copyright (C) 2006 Pascal Lamblin & Dan Popovici
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

// Authors: Pascal Lamblin & Dan Popovici

/*! \file RBMBinomialLayer.cc */



#include "RBMBinomialLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMBinomialLayer,
    "Layer in an RBM formed with binomial units.",
    "");

RBMBinomialLayer::RBMBinomialLayer( real the_learning_rate ) :
    inherited( the_learning_rate )
{
}

RBMBinomialLayer::RBMBinomialLayer( int the_size, real the_learning_rate ) :
    inherited( the_learning_rate )
{
    size = the_size;
    activation.resize( the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    bias.resize( the_size );
    bias_pos_stats.resize( the_size );
    bias_neg_stats.resize( the_size );
}

/*
//! Uses "rbmp" to obtain the activations of unit "i" of this layer.
//! This activation vector is computed by the "i+offset"-th unit of "rbmp"
void RBMBinomialLayer::getUnitActivations( int i, PP<RBMParameters> rbmp,
                                           int offset )
{
    Vec activation = activations.subVec( i, 1 );
    rbmp->computeUnitActivations( i+offset, 1, activation );
    expectation_is_up_to_date = false;
}

//! Uses "rbmp" to obtain the activations of all units in this layer.
//! Unit 0 of this layer corresponds to unit "offset" of "rbmp".
void RBMBinomialLayer::getAllActivations( PP<RBMParameters> rbmp, int offset )
{
    rbmp->computeUnitActivations( offset, size, activations );
    expectation_is_up_to_date = false;
}
*/

void RBMBinomialLayer::generateSample()
{
    computeExpectation();

    for( int i=0 ; i<size ; i++ )
        sample[i] = random_gen->binomial_sample( expectation[i] );
}

void RBMBinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    for( int i=0 ; i<size ; i++ )
        expectation[i] = sigmoid( -activation[i] );

    expectation_is_up_to_date = true;
}

void RBMBinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<size ; i++ )
        output[i] = sigmoid( -input[i] - bias[i] );
}

void RBMBinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                              Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    for( int i=0 ; i<size ; i++ )
        output[i] = sigmoid( -input[i] - rbm_bias[i]);
}

void RBMBinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
                                   Vec& input_gradient,
                                   const Vec& output_gradient,
                                   bool accumulate)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( size );
        input_gradient.clear();
    }

    if( momentum != 0. )
        bias_inc.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        real in_grad_i = - output_i * (1-output_i) * output_gradient[i];
        input_gradient[i] += in_grad_i;

        if( momentum == 0. )
        {
            // update the bias: bias -= learning_rate * input_gradient
            bias[i] -= learning_rate * in_grad_i;
        }
        else
        {
            // The update rule becomes:
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
            bias[i] += bias_inc[i];
        }
    }
}

//! TODO: add "accumulate" here
void RBMBinomialLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                                   const Vec& output,
                                   Vec& input_gradient, Vec& rbm_bias_gradient,
                                   const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        input_gradient[i] = - output_i * (1-output_i) * output_gradient[i];
    }

    rbm_bias_gradient << input_gradient;
}

real RBMBinomialLayer::fpropNLL(const Vec& target)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );

    real ret = 0;
    real target_i, expectation_i;
    for( int i=0 ; i<size ; i++ )
    {
        target_i = target[i];
        expectation_i = expectation[i];
        if(!fast_exact_is_equal(target_i,0.0))
            ret -= target_i * pl_log(expectation_i);
        if(!fast_exact_is_equal(target_i,1.0))
            ret -= (1-target_i) * pl_log(1-expectation_i);
    }
    return ret;
}

void RBMBinomialLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        bias_gradient[i] = target[i]-expectation[i];
    }
}

void RBMBinomialLayer::declareOptions(OptionList& ol)
{
/*
    declareOption(ol, "size", &RBMBinomialLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMBinomialLayer::build_()
{
}

void RBMBinomialLayer::build()
{
    inherited::build();
    build_();
}


void RBMBinomialLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
