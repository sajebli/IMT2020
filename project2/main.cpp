#include <ql/qldefines.hpp>
#ifdef BOOST_MSVC
#include <ql/auto_link.hpp>
#endif
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/batesengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/fdbermudanengine.hpp>
#include <ql/pricingengines/vanilla/fdeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/integralengine.hpp>
#include <ql/pricingengines/vanilla/mcamericanengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <boost/timer.hpp>
#include <iomanip>
#include <iostream>

#include "extendedbinomialtree.hpp"

#include <ql/experimental/lattices/extendedbinomialtree.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

Integer sessionId() { return 0; }

}  // namespace QuantLib
#endif

int main(int, char*[]) {
  try {
        boost::timer timer;
        std::cout << std::endl;

        // set up dates
        Calendar calendar = TARGET();
        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().evaluationDate() = todaysDate;

        // our options
        Option::Type type(Option::Put);
        Real underlying = 36;
        Real strike = 40;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;
        Date maturity(17, May, 1999);
        DayCounter dayCounter = Actual365Fixed();

        std::cout << "Option type = " << type << std::endl;
        std::cout << "Maturity = " << maturity << std::endl;
        std::cout << "Underlying price = " << underlying << std::endl;
        std::cout << "Strike = " << strike << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield) << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility) << std::endl;
        std::cout << std::endl;
        std::string method;
        QuantLib::Real price;
        std::cout << std::endl;

        // write column headings
        Size widths[] = {45, 14, 14, 14};
        std::cout << std::setw(widths[0]) << std::left << "Method"
                << std::setw(widths[1]) << std::left << "European"
                << std::setw(widths[2]) << std::left << "Bermudan"
                << std::setw(widths[3]) << std::left << "American" << std::endl;

        std::vector<Date> exerciseDates;
        for (Integer i = 1; i <= 4; i++)
        exerciseDates.push_back(settlementDate + 3 * i * Months);

        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));

        ext::shared_ptr<Exercise> bermudanExercise(
            new BermudanExercise(exerciseDates));

        ext::shared_ptr<Exercise> americanExercise(
            new AmericanExercise(settlementDate, maturity));

        Handle<Quote> underlyingH(
            ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

        // bootstrap the yield/dividend/vol curves
        Handle<YieldTermStructure> flatTermStructure(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));
        Handle<YieldTermStructure> flatDividendTS(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            ext::shared_ptr<BlackVolTermStructure>(new BlackConstantVol(
                settlementDate, calendar, volatility, dayCounter)));
        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(type, strike));
        ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
            new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                        flatTermStructure, flatVolTS));

        // options
        VanillaOption europeanOption(payoff, europeanExercise);
        VanillaOption bermudanOption(payoff, bermudanExercise);
        VanillaOption americanOption(payoff, americanExercise);

        // Analytic formulas:

        // Black-Scholes for European
    //     method = "Black-Scholes";
    //     europeanOption.setPricingEngine(
    //         ext::shared_ptr<PricingEngine>(new AnalyticEuropeanEngine(bsmProcess)));
    // //    americanOption.setPricingEngine(
    // //    ext::shared_ptr<PricingEngine>(new AnalyticEuropeanEngine(bsmProcess)));
    // //    bermudanOption.setPricingEngine(
    // //      ext::shared_ptr<PricingEngine>(new AnalyticEuropeanEngine(bsmProcess)));
    //     std::cout << std::setw(widths[0]) << std::left << method << std::fixed
    //             << std::setw(widths[1]) << std::left << europeanOption.NPV()
    //             << std::setw(widths[2]) << std::left << "N/A"  //bermudanOption.NPV()
    //             << std::setw(widths[3]) << std::left << "N/A" << std::endl;//american

    // Finite differences
        Size timeSteps = 500;
        timer.restart();
        double seconds = timer.elapsed();
        std::cout << " \nStart in ";
        std::cout << seconds << " s\n" << std::endl;
        // method = "Finite differences";
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDEuropeanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDBermudanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDAmericanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;

        // Binomial method: Jarrow-Rudd
        method = "Extended Binomial Jarrow-Rudd";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcess, timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcess, timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcess, timeSteps)));
       
        /*std::cout << std::setw(widths[0]) << std::left << method << std::fixed
                << std::setw(widths[1]) << std::left <<"  "
                << std::setw(widths[2]) << std::left << " "
                << std::setw(widths[3]) << std::left << " "
                << std::endl;*/
        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        
        
        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;

        timer.restart();
        





        method = "Extended Binomial Cox-Ross-Rubinstein";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcess,
                                                                timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcess,
                                                                timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcess,
                                                                timeSteps)));
        // std::cout << std::setw(widths[0]) << std::left << method << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;
        
        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;



        

        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();

        

        // Binomial method: Additive equiprobabilities
        method = "Extended Additive equiprobabilities";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcess,
                                                                    timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcess,
                                                                    timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcess,
                                                                    timeSteps)));
       
        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();

        // Binomial method: Binomial Trigeorgis
        method = "Extended Binomial Trigeorgis";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcess, timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcess, timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcess, timeSteps)));


        // std::cout << std::setw(widths[0]) << std::left << method << std::fixed
        //         << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //         << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //         << std::setw(widths[3]) << std::left << americanOption.NPV()
        //         << std::endl;



        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();

        // Binomial method: Binomial Tian
        method = "Extended Binomial Tian";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTian_2>(bsmProcess, timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTian_2>(bsmProcess, timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedTian_2>(bsmProcess, timeSteps)));


        // std::cout << std::setw(widths[0]) << std::left << method << std::fixed
        //         << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //         << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //         << std::setw(widths[3]) << std::left << americanOption.NPV()
        //         << std::endl;




        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();

        // Binomial method: Binomial Leisen-Reimer
        method = "Extended Binomial Leisen-Reimer";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcess,
                                                            timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcess,
                                                            timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcess,
                                                            timeSteps)));

        // std::cout << std::setw(widths[0]) << std::left << method << std::fixed
        //         << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //         << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //         << std::setw(widths[3]) << std::left << americanOption.NPV()
        //         << std::endl;


        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();






        // Binomial method: Binomial Joshi
        method = "Extended Binomial Joshi";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcess, timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcess, timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcess, timeSteps)));
        // std::cout << std::setw(widths[0]) << std::left << method << std::fixed
        //         << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //         << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //         << std::setw(widths[3]) << std::left << americanOption.NPV()
        //         << std::endl;

        

        std::cout<<method<< std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
        price=europeanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;

        std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
        price=bermudanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;


        std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
        price=americanOption.NPV();
        std::cout <<"option price:    "<<price<< std::endl;
        std::cout << std::endl;



        seconds = timer.elapsed();
        std::cout << " \nRun completed in ";
        std::cout << seconds << " s\n" << std::endl;
        std::cout <<  " ***********************************************************************" << std::endl;
        timer.restart();
        return 0;
        

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

