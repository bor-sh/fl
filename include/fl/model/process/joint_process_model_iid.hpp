/*
 * This is part of the FL library, a C++ Bayesian filtering library
 * (https://github.com/filtering-library)
 *
 * Copyright (c) 2014 Jan Issac (jan.issac@gmail.com)
 * Copyright (c) 2014 Manuel Wuthrich (manuel.wuthrich@gmail.com)
 *
 * Max-Planck Institute for Intelligent Systems, AMD Lab
 * University of Southern California, CLMC Lab
 *
 * This Source Code Form is subject to the terms of the MIT License (MIT).
 * A copy of the license can be found in the LICENSE file distributed with this
 * source code.
 */

/**
 * \file joint_process_model_iid.hpp
 * \date January 2015
 * \author Jan Issac (jan.issac@gmail.com)
 */

#ifndef FL__MODEL__PROCESS__JOINT_PROCESS_MODEL_IID_HPP
#define FL__MODEL__PROCESS__JOINT_PROCESS_MODEL_IID_HPP

#include <Eigen/Dense>

#include <memory>

#include <fl/util/traits.hpp>
#include <fl/util/meta.hpp>

#include <fl/model/process/process_model_interface.hpp>

namespace fl
{

// Forward declarations
template <typename ... Models> class JointProcessModel;

/**
 * Traits of JointProcessModel<MultipleOf<ProcessModel, Count>>
 */
template <
    typename ProcessModel,
    int Count
>
struct Traits<
           JointProcessModel<MultipleOf<ProcessModel, Count>>
        >
{
    enum : signed int { ModelCount = Count };

    typedef ProcessModel LocalProcessModel;
    typedef typename Traits<ProcessModel>::Scalar Scalar;
    typedef typename Traits<ProcessModel>::State LocalState;
    typedef typename Traits<ProcessModel>::Input LocalInput;
    typedef typename Traits<ProcessModel>::Noise LocalNoise;

    enum : signed int
    {
        StateDim = ExpandSizes<LocalState::SizeAtCompileTime, Count>::Size,
        NoiseDim = ExpandSizes<LocalNoise::SizeAtCompileTime, Count>::Size,
        InputDim = ExpandSizes<LocalInput::SizeAtCompileTime, Count>::Size
    };

    typedef Eigen::Matrix<Scalar, StateDim, 1> State;
    typedef Eigen::Matrix<Scalar, NoiseDim, 1> Noise;
    typedef Eigen::Matrix<Scalar, InputDim, 1> Input;

    typedef ProcessModelInterface<
                State,
                Noise,
                Input
            > ProcessModelBase;
};



/**
 * \ingroup process_models
 */
template <
    typename LocalProcessModel,
    int Count
>
class JointProcessModel<MultipleOf<LocalProcessModel, Count>>
    : public Traits<
                 JointProcessModel<MultipleOf<LocalProcessModel, Count>>
             >::ProcessModelBase
{
public:
    typedef JointProcessModel<MultipleOf<LocalProcessModel, Count>> This;

    typedef typename Traits<This>::State State;
    typedef typename Traits<This>::Noise Noise;
    typedef typename Traits<This>::Input Input;

public:
    explicit JointProcessModel(
            const std::shared_ptr<LocalProcessModel>& local_process_model,
            int count = ToDimension<Count>::Value)
        : local_process_model_(local_process_model),
          count_(count)
    {
        assert(count > 0);
    }

    ~JointProcessModel() { }

    virtual State predict_state(double delta_time,
                                const State& state,
                                const Noise& noise,
                                const Input& input)
    {
        State x = State::Zero(state_dimension(), 1);

        int state_dim = local_process_model_->state_dimension();
        int noise_dim = local_process_model_->noise_dimension();
        int input_dim = local_process_model_->input_dimension();

        for (int i = 0; i < count_; ++i)
        {
            x.middleRows(i * state_dim, state_dim) =
                local_process_model_->predict_state(
                    delta_time,
                    state.middleRows(i * state_dim, state_dim),
                    noise.middleRows(i * noise_dim, noise_dim),
                    input.middleRows(i * input_dim, input_dim));
        }

        return x;
    }

    virtual size_t state_dimension() const
    {
        return local_process_model_->state_dimension() * count_;
    }

    virtual size_t noise_dimension() const
    {
        return local_process_model_->noise_dimension() * count_;
    }

    virtual size_t input_dimension() const
    {
        return local_process_model_->input_dimension() * count_;
    }

    const std::shared_ptr<LocalProcessModel>& local_process_model()
    {
        return local_process_model_;
    }

protected:
    std::shared_ptr<LocalProcessModel> local_process_model_;
    int count_;
};

}

#endif

