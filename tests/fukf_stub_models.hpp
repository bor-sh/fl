/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014 Max-Planck-Institute for Intelligent Systems,
 *                     University of Southern California
 *    Jan Issac (jan.issac@gmail.com)
 *    Manuel Wuthrich (manuel.wuthrich@gmail.com)
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @date 2014
 * @author Jan Issac (jan.issac@gmail.com)
 * @author Manuel Wuthrich (manuel.wuthrich@gmail.com)
 * Max-Planck-Institute for Intelligent Systems, University of Southern California
 */

#ifndef FAST_FILTERING_TESTS_STUB_MODELS_HPP
#define FAST_FILTERING_TESTS_STUB_MODELS_HPP

#include <Eigen/Dense>

#include <fast_filtering/models/process_models/interfaces/stationary_process_model.hpp>
#include <fast_filtering/filters/deterministic/factorized_unscented_kalman_filter.hpp>

template <typename State_>
class ProcessModelStub:
        ff::StationaryProcessModel<State_, Eigen::Matrix<double, 0, 0> >
{
public:
    typedef State_ State;
    typedef Eigen::Matrix<double, 0, 0> Input;

    virtual void Condition(const double& delta_time,
                           const State& state,
                           const Input& input)
    {
        state_ = state;
    }

    virtual State predict(const State& prior, const State& noise)
    {
        return prior + noise;
    }

    virtual Eigen::MatrixXd NoiseCovariance()
    {
        return Eigen::MatrixXd::Identity(state_.rows(), state_.cols()) * 0.08;
    }

    virtual size_t Dimension() { return State::SizeAtCompileTime; }
    virtual size_t NoiseDimension() { return State::SizeAtCompileTime; }
protected:
    State state_;
};

template <typename State_a, typename State_b_i>
class ObservationModelStub
{
public:
    typedef Eigen::Matrix<double, 1, 1> Measurement;


    virtual Measurement predict(const State_a& a,
                                const State_b_i& b_i,
                                const Measurement& noise)
    {
        if (a_ != a)
        {
            a_ = a;
        }

        Measurement y_i = noise;

        return y_i;
    }

    virtual Eigen::MatrixXd NoiseCovariance()
    {
        return Eigen::MatrixXd::Identity(1, 1) * 0.023;
    }

    virtual size_t Dimension() { return 1; }
    virtual size_t NoiseDimension() { return 1; }

protected:
    State_a a_;
};

#endif