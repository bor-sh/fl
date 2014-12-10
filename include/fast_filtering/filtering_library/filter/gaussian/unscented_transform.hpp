/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014 Max-Planck-Institute for Intelligent Systems,
 *                     University of Southern California
 *    Jan Issac (jan.issac@gmail.com)
 *    Manuel Wuthrich (manuel.wuthrich@gmail.com)
 *
 *
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
 * \date 11/5/2014
 * \author Jan Issac (jan.issac@gmail.com)
 * Max-Planck-Institute for Intelligent Systems,
 * University of Southern California
 */

#ifndef FL__FILTER__GAUSSIAN__UNSCENTED_TRANSFORM_HPP
#define FL__FILTER__GAUSSIAN__UNSCENTED_TRANSFORM_HPP

#include <fast_filtering/utils/traits.hpp>
#include <fast_filtering/distributions/gaussian.hpp>
#include <fast_filtering/filtering_library/filter/gaussian/point_set_transform.hpp>

namespace fl
{

/**
 * This is the Unscented Transform used in the Unscented Kalman Filter
 * \cite wan2000unscented. It implememnts the PointSetTransform interface.
 *
 * \copydetails PointSetTransform
 */
class UnscentedTransform
        : public PointSetTransform<UnscentedTransform>
{
public:
    /**
     * Creates a UnscentedTransform
     *
     * \param alpha     UT Scaling parameter alpha (distance to the mean)
     * \param beta      UT Scaling parameter beta  (2.0 is optimal for Gaussian)
     * \param kappa     UT Scaling parameter kappa (higher order parameter)
     */
    UnscentedTransform(double alpha = 1., double beta = 2., double kappa = 0.)
        : PointSetTransform<UnscentedTransform>(this),
          alpha_(alpha),
          beta_(beta),
          kappa_(kappa)
    { }


    /**
     * \copydoc PointSetTransform::forward(const Gaussian&,
     *                                     PointSet&) const
     *
     * \throws WrongSizeException
     * \throws ResizingFixedSizeEntityException
     */
    template <typename Gaussian_, typename PointSet_>
    void forward(const Gaussian_& gaussian,
                 PointSet_& point_set) const
    {
        forward(gaussian, gaussian.Dimension(), 0, point_set);
    }

    /**
     * \copydoc PointSetTransform::forward(const Gaussian&,
     *                                     size_t global_dimension,
     *                                     size_t dimension_offset,
     *                                     PointSet&) const
     *
     * \throws WrongSizeException
     * \throws ResizingFixedSizeEntityException
     */
    template <typename Gaussian_, typename PointSet_>
    void forward(const Gaussian_& gaussian,
                 size_t global_dimension,
                 size_t dimension_offset,
                 PointSet_& point_set) const
    {
        typedef typename Traits<PointSet_>::Point  Point;
        typedef typename Traits<PointSet_>::Weight Weight;

        const double dim = double(global_dimension);
        const size_t point_count = number_of_points(dim);

        /**
         * \internal
         *
         * \remark
         * A PointSet with a fixed number of points must have the
         * correct number of points which is required by this transform
         */
        if (IsFixed<Traits<PointSet_>::NumberOfPoints>() &&
            Traits<PointSet_>::NumberOfPoints != point_count)
        {
            BOOST_THROW_EXCEPTION(
                WrongSizeException("Incompatible number of points of the"
                                   " specified fixed-size PointSet"));
        }

        // will resize of transform size is different from point count.
        point_set.resize(point_count);

        auto&& covariance_sqrt = gaussian.SquareRoot() * gamma_factor(dim);

        Point point_shift;
        const Point& mean = gaussian.Mean();

        // set the first point
        point_set.point(0, mean, Weight{weight_mean_0(dim), weight_cov_0(dim)});

        // compute the remaining points
        Weight weight_i{weight_mean_i(dim), weight_cov_i(dim)};

        // use squential loops to enable loop unrolling
        const size_t start_1 = 1;
        const size_t limit_1 = start_1 + dimension_offset;
        const size_t limit_2 = limit_1 + gaussian.Dimension();
        const size_t limit_3 = global_dimension;

        for (size_t i = start_1; i < limit_1; ++i)
        {
            point_set.point(i, mean, weight_i);
            point_set.point(global_dimension + i, mean, weight_i);
        }

        for (size_t i = limit_1; i < limit_2; ++i)
        {
            point_shift = covariance_sqrt.col(i - dimension_offset - 1);
            point_set.point(i, mean + point_shift, weight_i);
            point_set.point(global_dimension + i, mean - point_shift, weight_i);
        }

        for (size_t i = limit_2; i <= limit_3; ++i)
        {
            point_set.point(i, mean, weight_i);
            point_set.point(global_dimension + i, mean, weight_i);
        }
    }

    /**
     * \return Number of points generated by this transform
     *
     * \param dimension Dimension of the Gaussian
     */
    static constexpr size_t number_of_points(int dimension)
    {
        return (dimension != Eigen::Dynamic) ? 2 * dimension + 1 : 0;
    }

public:
    /** \cond INTERNAL */

    /**
     * \return First mean weight
     *
     * \param dim Dimension of the Gaussian
     */
    double weight_mean_0(double dim) const
    {
        return lambda_scalar(dim) / (dim + lambda_scalar(dim));
    }

    /**
     * \return First covariance weight
     *
     * \param dim Dimension of the Gaussian
     */
    double weight_cov_0(double dim) const
    {
        return weight_mean_0(dim) + (1 - alpha_ * alpha_ + beta_);
    }

    /**
     * \return i-th mean weight
     *
     * \param dimension Dimension of the Gaussian
     */
    double weight_mean_i(double dim) const
    {
        return 1. / (2. * (dim + lambda_scalar(dim)));
    }

    /**
     * \return i-th covariance weight
     *
     * \param dimension Dimension of the Gaussian
     */
    double weight_cov_i(double dim) const
    {
        return weight_mean_i(dim);
    }

    /**
     * \param dim Dimension of the Gaussian
     */
    double lambda_scalar(double dim) const
    {
        return alpha_ * alpha_ * (dim + kappa_) - dim;
    }

    /**
     * \param dim  Dimension of the Gaussian
     */
    double gamma_factor(double dim) const
    {
        return std::sqrt(dim + lambda_scalar(dim));
    }
    /** \endcond */

protected:
    /** \cond INTERNAL */
    double alpha_;
    double beta_;
    double kappa_;
    /** \endcond */
};

}

#endif
