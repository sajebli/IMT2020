/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2008 John Maiden

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "extendedbinomialtree.hpp"
#include <ql/math/distributions/binomialdistribution.hpp>
#include <iostream>
namespace QuantLib {

    ExtendedJarrowRudd_2::ExtendedJarrowRudd_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : ExtendedEqualProbabilitiesBinomialTree_2<ExtendedJarrowRudd_2>(
                                                        process, end, steps) {
        // drift removed
        up_ = process->stdDeviation(0.0, x0_, dt_);
        //std::cout<<"here jarrow rudd"<<std::endl;
    }

    Real ExtendedJarrowRudd_2::upStep(Time stepTime) const {
        (this->count2)++;
        return treeProcess_->stdDeviation(stepTime, x0_, dt_);
    }



    ExtendedCoxRossRubinstein_2::ExtendedCoxRossRubinstein_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : ExtendedEqualJumpsBinomialTree_2<ExtendedCoxRossRubinstein_2>(
                                                        process, end, steps) {

        dx_ = process->stdDeviation(0.0, x0_, dt_);
        pu_ = 0.5 + 0.5*this->driftStep(0.0)/dx_;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }

    Real ExtendedCoxRossRubinstein_2::dxStep(Time stepTime) const {
        (this->count3)++;
        return this->treeProcess_->stdDeviation(stepTime, x0_, dt_);
    }

    Real ExtendedCoxRossRubinstein_2::probUp(Time stepTime) const {
        (this->count4)++;
        return 0.5 + 0.5*this->driftStep(stepTime)/dxStep(stepTime);
    }


    ExtendedAdditiveEQPBinomialTree_2::ExtendedAdditiveEQPBinomialTree_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : ExtendedEqualProbabilitiesBinomialTree_2<ExtendedAdditiveEQPBinomialTree_2>(
                                                        process, end, steps) {

          up_ = - 0.5 * this->driftStep(0.0) + 0.5 *
            std::sqrt(4.0*process->variance(0.0, x0_, dt_)-
                      3.0*this->driftStep(0.0)*this->driftStep(0.0));
    }

    Real ExtendedAdditiveEQPBinomialTree_2::upStep(Time stepTime) const {
        (this->count2)++;
        return (- 0.5 * this->driftStep(stepTime) + 0.5 *
            std::sqrt(4.0*this->treeProcess_->variance(stepTime, x0_, dt_)-
            3.0*this->driftStep(stepTime)*this->driftStep(stepTime)));
    }




    ExtendedTrigeorgis_2::ExtendedTrigeorgis_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : ExtendedEqualJumpsBinomialTree_2<ExtendedTrigeorgis_2>(process, end, steps) {

        dx_ = std::sqrt(process->variance(0.0, x0_, dt_)+
            this->driftStep(0.0)*this->driftStep(0.0));
        pu_ = 0.5 + 0.5*this->driftStep(0.0)/this->dxStep(0.0);
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }

    Real ExtendedTrigeorgis_2::dxStep(Time stepTime) const {
        (this->count3)++;
        return std::sqrt(this->treeProcess_->variance(stepTime, x0_, dt_)+
            this->driftStep(stepTime)*this->driftStep(stepTime));
    }

    Real ExtendedTrigeorgis_2::probUp(Time stepTime) const {
        (this->count4)++;
        return 0.5 + 0.5*this->driftStep(stepTime)/dxStep(stepTime);
    }


    ExtendedTian_2::ExtendedTian_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : ExtendedBinomialTree_2<ExtendedTian_2>(process, end, steps) {

        Real q = std::exp(process->variance(0.0, x0_, dt_));

        Real r = std::exp(this->driftStep(0.0))*std::sqrt(q);

        up_ = 0.5 * r * q * (q + 1 + std::sqrt(q * q + 2 * q - 3));
        down_ = 0.5 * r * q * (q + 1 - std::sqrt(q * q + 2 * q - 3));

        pu_ = (r - down_) / (up_ - down_);
        pd_ = 1.0 - pu_;

        // doesn't work
        //     treeCentering_ = (up_+down_)/2.0;
        //     up_ = up_-treeCentering_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }

    Real ExtendedTian_2::underlying(Size i, Size index) const {
        Time stepTime = i*this->dt_;
        Real q = std::exp(this->treeProcess_->variance(stepTime, x0_, dt_));
        Real r = std::exp(this->driftStep(stepTime))*std::sqrt(q);

        Real up = 0.5 * r * q * (q + 1 + std::sqrt(q * q + 2 * q - 3));
        Real down = 0.5 * r * q * (q + 1 - std::sqrt(q * q + 2 * q - 3));

        return x0_ * std::pow(down, Real(BigInteger(i)-BigInteger(index)))
            * std::pow(up, Real(index));
    }

    Real ExtendedTian_2::probability(Size i, Size, Size branch) const {
        Time stepTime = i*this->dt_;
        Real q = std::exp(this->treeProcess_->variance(stepTime, x0_, dt_));
        Real r = std::exp(this->driftStep(stepTime))*std::sqrt(q);

        Real up = 0.5 * r * q * (q + 1 + std::sqrt(q * q + 2 * q - 3));
        Real down = 0.5 * r * q * (q + 1 - std::sqrt(q * q + 2 * q - 3));

        Real pu = (r - down) / (up - down);
        Real pd = 1.0 - pu;

        return (branch == 1 ? pu : pd);
    }


    ExtendedLeisenReimer_2::ExtendedLeisenReimer_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real strike)
    : ExtendedBinomialTree_2<ExtendedLeisenReimer_2>(process, end,
                                                     (steps%2 ? steps : steps+1)),
      end_(end), oddSteps_(steps%2 ? steps : steps+1), strike_(strike) {

        QL_REQUIRE(strike>0.0, "strike " << strike << "must be positive");
        Real variance = process->variance(0.0, x0_, end);

        Real ermqdt = std::exp(this->driftStep(0.0) + 0.5*variance/oddSteps_);
        Real d2 = (std::log(x0_/strike) + this->driftStep(0.0)*oddSteps_ ) /
            std::sqrt(variance);

        pu_ = PeizerPrattMethod2Inversion(d2, oddSteps_);
        pd_ = 1.0 - pu_;
        Real pdash = PeizerPrattMethod2Inversion(d2+std::sqrt(variance),
                                                 oddSteps_);
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);

    }

    Real ExtendedLeisenReimer_2::underlying(Size i, Size index) const {
        Time stepTime = i*this->dt_;
        Real variance = this->treeProcess_->variance(stepTime, x0_, end_);
        Real ermqdt = std::exp(this->driftStep(stepTime) + 0.5*variance/oddSteps_);
        Real d2 = (std::log(x0_/strike_) + this->driftStep(stepTime)*oddSteps_ ) /
            std::sqrt(variance);

        Real pu = PeizerPrattMethod2Inversion(d2, oddSteps_);
        Real pdash = PeizerPrattMethod2Inversion(d2+std::sqrt(variance),
            oddSteps_);
        Real up = ermqdt * pdash / pu;
        Real down = (ermqdt - pu * up) / (1.0 - pu);

        return x0_ * std::pow(down, Real(BigInteger(i)-BigInteger(index)))
            * std::pow(up, Real(index));
    }

    Real ExtendedLeisenReimer_2::probability(Size i, Size, Size branch) const {
        Time stepTime = i*this->dt_;
        Real variance = this->treeProcess_->variance(stepTime, x0_, end_);
        Real d2 = (std::log(x0_/strike_) + this->driftStep(stepTime)*oddSteps_ ) /
            std::sqrt(variance);

        Real pu = PeizerPrattMethod2Inversion(d2, oddSteps_);
        Real pd = 1.0 - pu;

        return (branch == 1 ? pu : pd);
    }



    Real ExtendedJoshi4_2::computeUpProb(Real k, Real dj) const {
        Real alpha = dj/(std::sqrt(8.0));
        Real alpha2 = alpha*alpha;
        Real alpha3 = alpha*alpha2;
        Real alpha5 = alpha3*alpha2;
        Real alpha7 = alpha5*alpha2;
        Real beta = -0.375*alpha-alpha3;
        Real gamma = (5.0/6.0)*alpha5 + (13.0/12.0)*alpha3
            +(25.0/128.0)*alpha;
        Real delta = -0.1025 *alpha- 0.9285 *alpha3
            -1.43 *alpha5 -0.5 *alpha7;
        Real p =0.5;
        Real rootk= std::sqrt(k);
        p+= alpha/rootk;
        p+= beta /(k*rootk);
        p+= gamma/(k*k*rootk);
        // delete next line to get results for j three tree
        p+= delta/(k*k*k*rootk);
        return p;
    }

    ExtendedJoshi4_2::ExtendedJoshi4_2(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real strike)
    : ExtendedBinomialTree_2<ExtendedJoshi4_2>(process, end,
                                               (steps%2 ? steps : steps+1)),
      end_(end), oddSteps_(steps%2 ? steps : steps+1), strike_(strike) {

        QL_REQUIRE(strike>0.0, "strike " << strike << "must be positive");
        Real variance = process->variance(0.0, x0_, end);

        Real ermqdt = std::exp(this->driftStep(0.0) + 0.5*variance/oddSteps_);
        Real d2 = (std::log(x0_/strike) + this->driftStep(0.0)*oddSteps_ ) /
            std::sqrt(variance);

        pu_ = computeUpProb((oddSteps_-1.0)/2.0,d2 );
        pd_ = 1.0 - pu_;
        Real pdash = computeUpProb((oddSteps_-1.0)/2.0,d2+std::sqrt(variance));
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);
    }

    Real ExtendedJoshi4_2::underlying(Size i, Size index) const {
        Time stepTime = i*this->dt_;
        Real variance = this->treeProcess_->variance(stepTime, x0_, end_);
        Real ermqdt = std::exp(this->driftStep(stepTime) + 0.5*variance/oddSteps_);
        Real d2 = (std::log(x0_/strike_) + this->driftStep(stepTime)*oddSteps_ ) /
            std::sqrt(variance);

        Real pu = computeUpProb((oddSteps_-1.0)/2.0,d2 );
        Real pdash = computeUpProb((oddSteps_-1.0)/2.0,d2+std::sqrt(variance));
        Real up = ermqdt * pdash / pu;
        Real down = (ermqdt - pu * up) / (1.0 - pu);

        return x0_ * std::pow(down, Real(BigInteger(i)-BigInteger(index)))
            * std::pow(up, Real(index));
    }

    Real ExtendedJoshi4_2::probability(Size i, Size, Size branch) const {
        Time stepTime = i*this->dt_;
        Real variance = this->treeProcess_->variance(stepTime, x0_, end_);
        Real d2 = (std::log(x0_/strike_) + this->driftStep(stepTime)*oddSteps_ ) /
            std::sqrt(variance);

        Real pu = computeUpProb((oddSteps_-1.0)/2.0,d2 );
        Real pd = 1.0 - pu;

        return (branch == 1 ? pu : pd);
    }

}
