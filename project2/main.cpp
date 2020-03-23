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

// #include <ql/experimental/lattices/extendedbinomialtree.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
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
        Size widths[] = {45, 14, 14, 14 , 14};

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


        // Non constant yield curve
        std::vector<Date> dates(5);
        dates[0] = settlementDate;
        dates[1] = settlementDate + Period(1,Years);
        dates[2] = settlementDate + Period(2,Years);
        dates[3] = settlementDate + Period(3,Years);
        dates[4] = settlementDate + Period(4,Years);

        std::vector<Rate> rates(5);
        rates[0] = 0.03;
        rates[1] = 0.05; 
        rates[2] = 0.06;
        rates[3] =0.075; 
        rates[4] = 0.05;
        
        Handle<YieldTermStructure> termStructure(
            ext::shared_ptr<YieldTermStructure>(
                new ZeroCurve(dates, rates, dayCounter)));

        // Non constant volatility curve
        std::vector<Date> volDates(3);
        volDates[0] = settlementDate + Period(4,Months);
        volDates[1] = settlementDate + Period(8,Months);
        volDates[2] = settlementDate + Period(1,Years);

        std::vector<Volatility> volatilities(3);
        volatilities[0] = 0.018;
        volatilities[1] = 0.022; 
        volatilities[2] = 0.034;
        Handle<BlackVolTermStructure> volTS(
            ext::shared_ptr<BlackVolTermStructure>(new BlackVarianceCurve(
                settlementDate , volDates ,  volatilities , dayCounter)));

        // Definition of non constant BSM
        ext::shared_ptr<BlackScholesMertonProcess> nonConstBSMProcess(
            new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                        termStructure, volTS));

        std::vector<ext::shared_ptr<BlackScholesMertonProcess>> bsmProcesses(2);
        bsmProcesses[0] = bsmProcess;
        bsmProcesses[1] = nonConstBSMProcess;
        std::string bsmProcessType[] = {"Constatnt BSM Process" , "Non Constant BSM Process"};
        
        // options
        VanillaOption europeanOption(payoff, europeanExercise);
        VanillaOption bermudanOption(payoff, bermudanExercise);
        VanillaOption americanOption(payoff, americanExercise);

        timer.restart();
        double seconds = timer.elapsed();
        std::cout << " \nStart in ";
        std::cout << seconds << " s\n" << std::endl;

        std::vector<Size> timeStepsVect(3);
        timeStepsVect[1] = 500;
        timeStepsVect[2] = 1000;
        timeStepsVect[3] = 1500;
        

        method = "Extended Binomial Jarrow-Rudd";
        std::cout <<  method << std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeStep = timeStepsVect[i];
            std::cout << "Time steps : " << timeStep << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcesses[j], timeStep)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcesses[j], timeStep)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJarrowRudd_2>(bsmProcesses[j], timeStep)));
                

                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;
            
                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;

        method = "Extended Binomial Cox-Ross-Rubinstein";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcesses[j],
                                                                        timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcesses[j],
                                                                        timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedCoxRossRubinstein_2>(bsmProcesses[j],
                                                                        timeSteps)));

                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<   "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;
        

        // Binomial method: Additive equiprobabilities
        method = "Extended Additive equiprobabilities";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcesses[j],
                                                                            timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcesses[j],
                                                                            timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree_2>(bsmProcesses[j],
                                                                            timeSteps)));

                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;

        // Binomial method: Binomial Trigeorgis
        method = "Extended Binomial Trigeorgis";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcesses[j], timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcesses[j], timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTrigeorgis_2>(bsmProcesses[j], timeSteps)));


                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;


                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;

        // Binomial method: Binomial Tian
        method = "Extended Binomial Tian";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTian_2>(bsmProcesses[j], timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTian_2>(bsmProcesses[j], timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedTian_2>(bsmProcesses[j], timeSteps)));

                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;


                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;

        // Binomial method: Binomial Leisen-Reimer
        method = "Extended Binomial Leisen-Reimer";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcesses[j],
                                                                    timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcesses[j],
                                                                    timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedLeisenReimer_2>(bsmProcesses[j],
                                                                    timeSteps)));


                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;


                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        std::cout << std::endl;

        // Binomial method: Binomial Joshi
        method = "Extended Binomial Joshi";
        std::cout<<method<< std::endl;
        for(int i=1; i<=3; i++){
            std::cout << std::setw(widths[0]) << std::left << "    " << std::fixed
                << std::setw(widths[1]) << std::left <<"driftStep"
                << std::setw(widths[2]) << std::left << "upStep"
                << std::setw(widths[3]) << std::left << "dxStep"
                << std::setw(widths[4]) << std::left << "probUp"
                << std::setw(widths[4]) << std::left << "Option Price"
                << std::endl;
            Size timeSteps = timeStepsVect[i];
            std::cout << "Time steps : " << timeSteps << std::endl;
            std::cout << std::endl;
            for(int j=0; j<=1; j++){
                std::cout << bsmProcessType[j]<< std::endl;
                europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcesses[j], timeSteps)));
                bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcesses[j], timeSteps)));
                americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4_2>(bsmProcesses[j], timeSteps)));

                std::cout<<std::setw(widths[0]) << std::left << "European" << std::fixed;
                price=europeanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "Bermudan" << std::fixed;
                price=bermudanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                std::cout<<std::setw(widths[0]) << std::left << "American" << std::fixed;
                price=americanOption.NPV();
                std::cout << std::setw(widths[4]) << std::left << price << std::endl;

                seconds = timer.elapsed();
                std::cout << " \nRun completed in ";
                std::cout << seconds << " s\n" << std::endl;
                std::cout <<  "                              --------------------------------------------------------------------" << std::endl;
                timer.restart();
            }
        }
        std::cout <<  "*******************************************************************************************************************************" << std::endl;
        
        return 0;
        

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

