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
 * \file random.hpp
 * \date 2014
 * \author Jan Issac (jan.issac@gmail.com)
 * \author Manuel Wuthrich (manuel.wuthrich@gmail.com)
 */

#ifndef FL__UTIL__RANDOM_HPP
#define FL__UTIL__RANDOM_HPP

#include <ctime>
#include <random>

/**
 * \brief global seed
 *
 * \ingroup random
 */
#ifdef USE_RANDOM_SEED
    #define RANDOM_SEED (unsigned int) std::time(0)
#else
    #define RANDOM_SEED 1
#endif

namespace fl
{

/**
 * Mersenne Twister specialization mt11213b \cite matsumoto1998mersenne
 * \ingroup random
 *
 *
 * mt11213b is slightly faster than mt19937
 */
typedef std::mersenne_twister_engine<
                uint32_t,
                32, 351, 175, 19,
                0xccab8ee7, 11,
                0xffffffff, 7,
                0x31b6ab00, 15,
                0xffe50000, 17,
                1812433253 > mt11213b;

/**
 * \return A seed. If USE_RANDOM_SEED was set true the seed is set to the
 * current time, otherwise, the seed will be 1.
 *
 * \ingroup random
 */
inline unsigned int seed()
{
#ifdef USE_RANDOM_SEED
    return (unsigned int) std::time(0);
#else
    return 1;
#endif
}

}

#endif
