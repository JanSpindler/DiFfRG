#pragma once

#include <DiFfRG/common/interpolation.hh>
#include <DiFfRG/common/minimization.hh>
#include <DiFfRG/physics/thermodynamics.hh>
#include <DiFfRG/common/quadrature/matsubara.hh>
#include <DiFfRG/model/model.hh>
#include <cmath>
#include <complex>
#include <spdlog/spdlog.h>

using namespace DiFfRG;

template <typename NT>
NT matsubaraIntegrand(const double omega, const double k, const double muq, const NT M2Diquark_, const NT M2Sigma_, const NT xi2_, const NT b2_)
    {
      const std::complex<double> I(0, 1);
      const auto sxi = sqrt(xi2_);
      const auto pr2 = powr<2>(k);
      const auto kd_p = pr2 + powr<2>(omega + 2. * I * muq) + M2Diquark_;
      const auto kd_m = pr2 + powr<2>(omega - 2. * I * muq) + M2Diquark_;
      const auto ks   = pr2 + powr<2>(omega) + M2Sigma_;
      return real(sxi * (kd_p * ks + kd_m * (kd_p + ks) - xi2_) / (ks * (kd_p * kd_m - xi2_) * (kd_m * kd_p * ks + sxi * (b2_ - ks * sxi))));
      //return sxi / (ks * (kd_p * kd_m - xi2_) * (kd_m * kd_p * ks + sxi * (b2_ - ks * sxi)));
    } 

template <class X>
static inline X safe_sqrt(const X& x)
    {

      // choose an epsilon in the same scalar type as x
      const X eps = X(1e-14);

      // if you need branchless behavior for AD, use max(x,eps) if available
      return x > eps ? sqrt(x) : sqrt(eps);
    }

struct Parameters {
  Parameters(const JSONValue &value)
  {
    try {
      Lambda = value.get_double("/physical/Lambda");
      Nc = value.get_double("/physical/Nc");
      Nf = value.get_double("/physical/Nf");
      T = value.get_double("/physical/T");
      muq = value.get_double("/physical/muq");

      m2Phi = value.get_double("/physical/m2Phi");
      m2D = value.get_double("/physical/m2D");

      lambdaPhi = value.get_double("/physical/lambdaPhi");
      lambdaD = value.get_double("/physical/lambdaD");
      lambdaDPhi = value.get_double("/physical/lambdaDPhi");
      hPhi = value.get_double("/physical/hPhi");
      hD = value.get_double("/physical/hD");

      cSigma = value.get_double("/physical/cSigma");

    } catch (std::exception &e) {
      std::cout << "Error while reading parameters: " << e.what() << std::endl;
      throw;
    }
  }
  double Lambda, Nc, Nf, T, muq;
  double m2Phi, m2D, lambdaPhi, lambdaD, lambdaDPhi, hPhi, hD, cSigma;
};
using FEFunctionDesc = FEFunctionDescriptor<Scalar<"uPhi">, Scalar<"uD">, Scalar<"uPhiImag">, Scalar<"uDImag">>;
using Components = ComponentDescriptor<FEFunctionDesc>;
constexpr auto idxf = FEFunctionDesc{};

class QuarkMesonDiquarkLPA
    : public def::AbstractModel<QuarkMesonDiquarkLPA, Components>,
      public def::fRG, // this handles the fRG time
      public def::FlowBoundaries<QuarkMesonDiquarkLPA>, // use Inflow/Outflow boundaries
      public def::AD<QuarkMesonDiquarkLPA>              // define all jacobians per AD
{
  private:
    const Parameters prm;
    MatsubaraQuadrature<double> matsubara_quadrature;

  public:
    static constexpr uint dim = 2; //0: rhoPhiReal, 1: rhoDReal
    QuarkMesonDiquarkLPA(const JSONValue &json): def::fRG(json.get_double("/physical/Lambda")), prm(json), matsubara_quadrature(prm.T) {
    } 
    template <typename Vector> 
    void initial_condition(const Point<dim> &pos, Vector &values) const
    {
      const auto rhoPhi = pos[0];
      const auto rhoD = pos[1];

      const auto uPhi = prm.m2Phi + 0.5 * prm.lambdaPhi * rhoPhi + prm.lambdaDPhi * rhoD;
      const auto uD = prm.m2D + 0.5 * prm.lambdaD * rhoD + prm.lambdaDPhi * rhoPhi;
      const auto uPhiImag = 0;
      const auto uDImag = 0;

      values[idxf("uPhi")] = uPhi;
      values[idxf("uD")] = uD;
      values[idxf("uPhiImag")] = uPhiImag;
      values[idxf("uDImag")] = uDImag;
    }

    template <typename NT, typename Solution>
    void flux(std::array<Tensor<1, dim, NT>, Components::count_fe_functions(0)> &flux, const Point<dim> &pos,
              const Solution &sol) const
    {
      const double hD = prm.hD;
      const double k2 = powr<2>(k);
      const double k5 = powr<5>(k);

      const auto rhoPhi = pos[0];
      const auto rhoD = pos[1];

      const auto &fe_functions = get<"fe_functions">(sol);     // uPhi, uD
      const auto &fe_derivatives = get<"fe_derivatives">(sol); // d2V pieces, mixing 

      // Frequently used curvatures
      const auto I = complex<double>(0,1);
      const auto m2Pion = fe_functions[idxf("uPhi")];      // = d1V_phi
      const auto m2PionImag = 0.;//fe_functions[idxf("uPhiImag")];      // = d1V_phiImag
      const auto m2PionComplex = m2Pion + I * m2PionImag;
      const auto m2Sigma = m2Pion + 2.0 * rhoPhi * fe_derivatives[idxf("uPhi")][0]; // = ∂2V_phi
      const auto m2SigmaImag = 0.;//m2PionImag; // = ∂2V_phiImag
      const auto m2SigmaComplex = m2Sigma + I * m2SigmaImag;
      const auto m2Diquark = fe_functions[idxf("uD")];
      const auto m2DiquarkImag = fe_functions[idxf("uDImag")];
      const auto m2DiquarkComplex = m2Diquark + I * m2DiquarkImag;
      const auto m2Quark = powr<2>(prm.hPhi) * rhoPhi / (prm.Nf);
      const auto m2QuarkImag = 0; 
      const auto m2QuarkComplex = m2Quark + I * m2QuarkImag;
      const auto M2Sigma = m2Pion + 2.0 * rhoPhi * fe_derivatives[idxf("uPhi")][0];
      const auto M2SigmaImag = fe_functions[idxf("uPhiImag")];
      const auto M2SigmaComplex = M2Sigma + I * M2SigmaImag;
      const auto M2Diquark = fe_functions[idxf("uD")] + rhoD * fe_derivatives[idxf("uD")][1];
      const auto M2DiquarkImag = fe_functions[idxf("uDImag")] + rhoD * fe_derivatives[idxf("uDImag")][1];
      const auto M2DiquarkComplex = M2Diquark + I * M2DiquarkImag;
      //std::cout << fe_derivatives[idxf("uD")][1] << std::endl;
      
      //const auto epsqReal = safe_sqrt(k2 + m2Quark);
      const auto epsqComplex = sqrt(k2 + m2QuarkComplex);
      //const auto Delta = safe_sqrt(rhoD_pos) * prm.hD;
      const auto DeltaComplex = sqrt(rhoD) * hD;

      //const NT xi2 = powr<2>(rhoD * fe_functions[idxf("uD")]);
      const auto xi2Complex = powr<2>(rhoD * (fe_functions[idxf("uD")] + I * fe_functions[idxf("uDImag")]));
      //const NT chiD = safe_sqrt(16 * powr<2>(prm.muq) * (k2 + M2Diquark) + xi2);
      const auto chiDComplex = sqrt(16. * powr<2>(prm.muq) * (k2 + M2DiquarkComplex) + xi2Complex);
      const auto b2Complex = 4. * rhoD * rhoPhi * powr<2>(fe_derivatives[idxf("uPhi")][1] + I * fe_derivatives[idxf("uPhiImag")][1]);
      //const auto b2 = 4. * rhoD * rhoPhi * std::pow(std::complex<NT>(fe_derivatives[idxf("uPhi")][1], fe_derivatives[idxf("uPhiImag")][1]), 2);

      auto finite = [](auto x) { return std::isfinite(static_cast<double>(x)); };

      /*auto fail = [&](const char *which, const NT value,
                      const NT t_qrg, const NT t_qb, const NT t_pi, const NT t_si,
                      const NT t_d, const NT t_du, const NT t_su) {
        spdlog::get("log")->error(
          "Non-finite {} at k={}, rhoPhi={}, rhoD={}, uPhi={}, uD={}",
          which, (double)this->k, (double)rhoPhi, (double)rhoD,
          (double)fe_functions[idxf("uPhi")], (double)fe_functions[idxf("uD")]
        );
        spdlog::get("log")->error(
          "m2Pion={}, m2Sigma={}, m2Diquark={}, M2Sigma={}, M2Diquark={}, m2Quark={}, epsq={}, Delta={}, chiD={}",
          (double)m2Pion, (double)m2Sigma, (double)m2Diquark,
          (double)M2Sigma, (double)M2Diquark, (double)m2Quark,
          (double)epsq, (double)Delta, (double)chiD
        );
        spdlog::get("log")->error(
          "terms: qrg={}, qb={}, pi={}, si={}, d={}, du={}, su={}",
          (double)t_qrg, (double)t_qb, (double)t_pi, (double)t_si,
          (double)t_d, (double)t_du, (double)t_su
        );
        spdlog::get("log")->error("value({})={}", which, (double)value);
        throw std::runtime_error("Non-finite rhs_common");
      };*/

      // Compute terms in order; after each one, validate.
      // For terms not computed yet, pass 0 placeholders to the fail() logger.
      const auto t_qrg = fluxQuarkRedGreen<NT>(prm.T, prm.muq, prm.Nf, epsqComplex, DeltaComplex);
      //if (!finite(t_qrg)) fail("t_qrg", t_qrg, t_qrg, NT(0), NT(0), NT(0), NT(0), NT(0), NT(0));

      const auto t_qb  = fluxQuarkBlue<NT>(prm.T, prm.muq, prm.Nf, epsqComplex);
      //if (!finite(t_qb))  fail("t_qb",  t_qb,  t_qrg, t_qb,  NT(0), NT(0), NT(0), NT(0), NT(0));

      const auto t_pi  = fluxPion<NT>(prm.T, m2PionComplex);
      //if (!finite(t_pi))  fail("t_pi",  t_pi,  t_qrg, t_qb,  t_pi,  NT(0), NT(0), NT(0), NT(0));

      const auto t_si  = fluxSigma<NT>(prm.T, m2SigmaComplex);
      //if (!finite(t_si))  fail("t_si",  t_si,  t_qrg, t_qb,  t_pi,  t_si,  NT(0), NT(0), NT(0));

      const auto t_d   = fluxDiquark<NT>(prm.T, prm.muq, m2DiquarkComplex);
      //if (!finite(t_d))   fail("t_d",   t_d,   t_qrg, t_qb,  t_pi,  t_si,  t_d,   NT(0), NT(0));

      const auto t_du  = fluxDiquarkUncoupled<NT>(prm.T, prm.muq, chiDComplex, M2DiquarkComplex);
      //if (!finite(t_du))  fail("t_du",  t_du,  t_qrg, t_qb,  t_pi,  t_si,  t_d,   t_du,  NT(0));

      const auto t_su  = fluxSigmaUncoupled<NT>(prm.T, M2SigmaComplex);
      //if (!finite(t_su))  fail("t_su",  t_su,  t_qrg, t_qb,  t_pi,  t_si,  t_d,   t_du,  t_su);

      // auto f_omega = make_F(k, M2Diquark, M2Sigma, xi2, b2);


      // quad uses double nodes; f_omega accepts "auto omega" so it's fine
      // matsubara_quadrature.reinit(prm.T, k);
      //auto nonAnalyticalFlowMatsubaraSum = matsubara_quadrature.sum_arguments(matsubaraIntegrand<NT>, k, prm.muq, M2Diquark, M2Sigma, xi2, b2);
      //const NT t_d_na = - 0.5 * b2 * k5 * nonAnalyticalFlowMatsubaraSum / (3. * M_PI * M_PI);
      // if (!finite(t_d_na))  fail("t_d_na",  t_d_na,  NT(0), NT(0), NT(0), NT(0), NT(0), NT(0));

      // std::cout << f_omega(0.) << std::endl; 

      // std::cout << "type: " << typeid(f_omega(0.)).name() << std::endl;
      // std::cout << "t_d_na type: " << typeid(t_d_na).name() << std::endl;
      // std::cout << "Non-analytical flow matsubara sum: " << nonAnalyticalFlowMatsubaraSum << std::endl;

      //const auto rhs_common = t_qrg + t_qb + t_pi + t_si + t_d + t_du + t_su - 0.5 * b2 * k5 * nonAnalyticalFlowMatsubaraSum / (3. * M_PI * M_PI);
      const auto rhs_real = real(t_qrg) + real(t_qb) + real(t_pi) + real(t_si) + real(t_d) + real(t_du);// + t_d_na;
      const auto rhs_imag = imag(t_qrg) + imag(t_qb) + imag(t_pi) + imag(t_si) + imag(t_d) + imag(t_du);// + t_d_na;
      //if (!finite(rhs_common)) fail("rhs_common", rhs_common, t_qrg, t_qb, t_pi, t_si, t_d, t_du, t_su);

      flux[idxf("uPhi")][0] = rhs_real;
      flux[idxf("uD")][1]   = rhs_real;
      flux[idxf("uPhiImag")][0] = rhs_imag;
      flux[idxf("uDImag")][1]   = rhs_imag;

      auto is_finite = [](auto x){ return std::isfinite(static_cast<double>(x)); };

      /*if (!is_finite(rhs_common)) {
        spdlog::get("log")->error(
          "Non-finite rhs_common at rhoPhi={}, rhoD={}, k={}, uPhi={}, uD={}",
          (double)rhoPhi, (double)rhoD, (double)this->k,
          (double)fe_functions[idxf("uPhi")], (double)fe_functions[idxf("uD")]
        );
        spdlog::get("log")->error(
          "m2Pion={}, m2Sigma={}, m2Diquark={}, M2Sigma={}, M2Diquark={}, m2Quark={}, epsq={}, Delta={}, chiD={}",
          (double)m2Pion, (double)m2Sigma, (double)m2Diquark,
          (double)M2Sigma, (double)M2Diquark, (double)m2Quark,
          (double)epsq, (double)Delta, (double)chiD
        );
        //spdlog::get("log")->error(
        //  "terms: qrg={}, qb={}, pi={}, si={}, d={}, du={}, su={}, d_na={}",
        //  (double)t_qrg, (double)t_qb, (double)t_pi, (double)t_si,
        //  (double)t_d, (double)t_du, (double)t_su, (double)t_d_na
        //);
        throw std::runtime_error("Non-finite rhs_common");
      }*/
    }
  private:
    template <typename T1, typename T2, typename T3, typename T4>
    auto make_F(const double k, T1 M2Diquark_, T2 M2Sigma_, T3 xi2_in, T4 b2_in) const
{
    const auto pr2 = powr<2>(k);

    // const auto M2Diquark_ = M2Diquark_in;
    // const auto M2Sigma_   = M2Sigma_in;
    const auto xi2_       = xi2_in;
    const auto b2_        = b2_in;

    const auto muq_ = prm.muq; 

    // Now define the Ks as functions of real omega (same scalar type as omega)
    auto Kdplus = [](const auto omega, const auto pr2, const auto muq_, const auto M2Diquark_)
    {
        return pr2 - powr<2>(omega + 2 * muq_) + M2Diquark_;
    };

    auto Kdminus = [](const auto omega, const auto pr2, const auto muq_, const auto M2Diquark_)
    {
        return pr2 - powr<2>(omega - 2 * muq_) + M2Diquark_;
    };

    auto Ksigma = [](const auto omega, const auto pr2, const auto M2Sigma_)
    {
        return pr2 - powr<2>(omega) + M2Sigma_;
    };

    // Return ONE function of omega suitable for MatsubaraQuadrature<double>::sum
    return [=](const auto omega)
    {
        const auto kd_p = Kdplus(omega, pr2, prm.muq, M2Diquark_);
        const auto kd_m = Kdminus(omega, pr2, prm.muq, M2Diquark_);
        const auto ks   = Ksigma(omega, pr2, M2Sigma_);

        // Your expression, rewritten in terms of kd_p/kd_m/ks and xi2_/b2_
        const auto sxi = safe_sqrt(xi2_);

        return sxi * (kd_p * ks + kd_m * (kd_p + ks) - xi2_) / (ks * (kd_p * kd_m - xi2_) * (kd_m * kd_p * ks + sxi * (b2_ - ks * sxi)));
    };
}
    template <typename NT>
    struct ReIm {
      NT re;
      NT im;
    };
    template <typename NT>
    inline ReIm<NT> make_reim(const NT& re, const NT& im) {
      return {re, im};
    }
    template <typename NT>
    inline ReIm<NT> make_real(const NT& re) {
      return {re, NT(0)};
    }
    ReIm<double> alpha_part() const
    {
      using std::log;
      using std::exp;


      const double sAmp   = prm.sAmp;
      const double sWidth = prm.sWidth;
      const double sig2   = sWidth * sWidth;

      const double ell  = log(k / prm.muq);
      const double a    = sAmp * exp(-(ell*ell) / (2.0*sig2));
      const double adot = - a * ell / (sig2 * k);

      const double Areal = 1. - 4.*a*a - a*adot;      // Re
      const double Aimag = 5.*a + (2./5.) * adot;     // Im

      return {Areal, Aimag};
    }   

    template <typename NT>
    auto fluxQuarkRedGreen(const double T, const double muq, const double Nf,
                              const auto epsq, const auto Delta) const
    {
      const double T0 = T;
      const double k5 = powr<5>(k); 

      const auto chiMinus = sqrt(powr<2>(epsq - prm.muq) + powr<2>(Delta));
      const auto chiPlus  = sqrt(powr<2>(epsq + prm.muq) + powr<2>(Delta));

      const auto A = alpha_part(); // ReIm<double>
      const auto Areal = A.re;
      const auto Aimag = A.im;
      const auto Acomplex = complex(A.re, A.im);

      const auto common = ((epsq - muq)/epsq) * TanhFiniteT(chiMinus, T0)/chiMinus + ((epsq + muq)/epsq) * TanhFiniteT(chiPlus,  T0)/chiPlus;

      const auto pref = -Nf * k5 / (3. * M_PI * M_PI);
      return pref * Acomplex * common;
    }
    template <typename NT> 
    auto fluxQuarkBlue(const double T, const double muq, const double Nf, const auto epsq) const{
      const double T0 = T;
      const double k5 = powr<5>(k);
      const auto complexReturn = -Nf * k5 / (3. * M_PI * M_PI) * (TanhFiniteT((epsq - muq), T0) + TanhFiniteT((epsq + muq), T0))/(2 * epsq);
      return complexReturn;
    }
    template <typename NT>
    auto fluxSigma(const double T, const auto m2Sigma) const{
      const double T0 = T;
      
      const double k2 = powr<2>(k);
      const double k5 = powr<5>(k);
      const auto result = k5/(12. * M_PI * M_PI) * CothFiniteT(sqrt(k2 + m2Sigma),T0)/sqrt(k2 + m2Sigma);
      return result;
    }
    //Hier kein Pi2????
    template <typename NT>
    auto fluxPion(const double T, const auto m2Pion) const{
      const double T0 = T;

      const double k2 = powr<2>(k);
      const double k5 = powr<5>(k);
      const auto pref = powr<2>(prm.Nf)-1;

      const auto result = pref * k5/(12 * M_PI * M_PI) * CothFiniteT(sqrt(k2 + m2Pion), T0)/sqrt(k2 + m2Pion);
      return result;
    }
    template <typename NT>
    auto fluxDiquark(const double T, const double muq, const auto m2Diquark) const{
      const double T0 = prm.T;
      
      const double k2 = powr<2>(k);
      const double k5 = powr<5>(k);

      const auto result = k5/(3 * M_PI * M_PI) * (CothFiniteT(sqrt(k2 + m2Diquark) + 2. * muq, T0)/(4. * sqrt(k2 + m2Diquark))+ CothFiniteT(sqrt(k2 + m2Diquark) - 2. * muq, T0)/(4. * sqrt(k2 + m2Diquark)));
      return result;
    }
    template <typename NT>
    auto fluxDiquarkUncoupled(const double T, const double muq, const auto chiD, const auto M2Diquark) const
      {
        const double T0 = T;
        const double k2 = powr<2>(k);
        const double k5 = powr<5>(k);

        const auto arg_minus = k2 + 4. * std::pow(muq, 2) + M2Diquark - chiD;
        const auto arg_plus  = k2 + 4. * std::pow(muq, 2) + M2Diquark + chiD;

        const auto Em = sqrt(arg_minus);
        const auto Ep = sqrt(arg_plus);

        const double pref = k5/(12. * M_PI * M_PI);
        const auto r = 8. * powr<2>(muq) / chiD;
        const auto result = pref * ( (1 - r) * CothFiniteT(Em, T0) / Em + (1 + r) * CothFiniteT(Ep, T0) / Ep );
        return result;
      }
    template <typename NT> 
    auto fluxSigmaUncoupled(const double T, const auto M2Sigma) const{
      //const std::complex<NT> T0(prm.T, 0);
      const double T0 = prm.T;
      
      const double k2 = powr<2>(k);
      const double k5 = powr<5>(k);

      const auto result = k5/(12 * M_PI * M_PI) * CothFiniteT(sqrt(k2 + M2Sigma),T0)/sqrt(k2 + M2Sigma);

      return result;
    }
  public:
    template <int dim, typename Vector>
    std::array<double, 2> EoM(const Point<dim> &x, const Vector &u_i) const
    {
      constexpr double eps = 1e-14;
      const double cSigma = prm.cSigma;

      const double rhoPhi = x[0];   // assuming ordering (rhoPhi, rhoD)
      const double rhoD   = x[1];

      const double sigma = std::sqrt(2.0 * std::max(rhoPhi, 0.0));
      (void)rhoD;

      const double d1V_phi = u_i[idxf("uPhi")]; // ∂_{ρφ} V
      const double d1V_d   = u_i[idxf("uD")];   // ∂_{ρd} V

      const double F_phi = d1V_phi - cSigma / (sigma + eps);
      const double F_d   = d1V_d;

      return {{F_phi, F_d}};
    }

    mutable double last_EoM_phi = 0.;
    mutable bool lock_EoM_phi = false;
    mutable double last_EoM_d = 0.;
    mutable bool lock_EoM_d = false;

    template <int dim, typename Vector>
    Point<dim> EoM_postprocess(const Point<dim> &EoM, const Vector &) const
    {
      Point<dim> out = EoM;

      // Apply locking ONLY to the mesonic EoM (index 0)
      if (lock_EoM_phi || last_EoM_phi - EoM[0] > 5e-4) {
        lock_EoM_phi = true;
        out[0] = last_EoM_phi;
      } else {
        last_EoM_phi = EoM[0];
      }
      if (lock_EoM_d || last_EoM_d - EoM[1] > 5e-4) {
        lock_EoM_d = true;
        out[1] = last_EoM_d;
      } else {
        last_EoM_d = EoM[1];
      }
      return out;
    }


    template <int dim, typename DataOut, typename Solutions>
    void readouts(DataOut &output, const Point<dim> &x, const Solutions &sol) const
    {
      // pick ONE default output file for your main run
      this->readouts(output, x, sol, "QMD_data.csv", 0);
    }



    template <int dim, typename DataOut, typename Solutions>
    void readouts(DataOut &output, const Point<dim> &x, const Solutions &sol, const std::string filename, const int) const
    {
      const auto &fe_functions = get<"fe_functions">(sol);
      const auto &fe_derivatives = get<"fe_derivatives">(sol);

      const double rhoPhi = x[0];
      const double rhoD = x[1];
      const double sigma = std::sqrt(2. * rhoPhi);

      //const auto hPhi = (variables[idxv("hPhi")]);
      //const auto hD = (variables[idxv("hD")]);
      const auto d2V = fe_derivatives[idxf("uPhi")][0];
      //const auto dd2V = fe_derivatives[idxf("uD")][1];
      const auto mixV1 = fe_derivatives[idxf("uPhi")][1];
      const auto mixV2 = fe_derivatives[idxf("uD")][0];

      const auto m2Pion = fe_functions[idxf("uPhi")];
      const auto m2PionImag = fe_functions[idxf("uPhiImag")];
      const auto m2Diquark = fe_functions[idxf("uD")];
      const auto m2DiquarkImag = fe_functions[idxf("uDImag")];
      const auto m2Sigma = m2Pion + 2. * rhoPhi * d2V;
      const auto hPhi = prm.hPhi;
      const auto hD = prm.hD;

      const double mQuark = (sigma * hPhi / std::sqrt(2. * 2.));
      const double mPion = m2Pion > 0. ? std::sqrt(m2Pion) : 0.;
      const double mSigma = m2Sigma > 0. ? std::sqrt(m2Sigma) : 0.;
      const double mDiquark = m2Diquark > 0. ? std::sqrt(m2Diquark) : 0.;

      auto &out_file = output.csv_file(filename);
      out_file.set_Lambda(Lambda);

      out_file.value("sigma [GeV]", sigma);

      out_file.value("hPhi", hPhi);
      out_file.value("hD", hD);

      out_file.value("m_{pi} [GeV]", mPion);
      out_file.value("m_{sigma} [GeV]", mSigma);
      out_file.value("m_q [GeV]", mQuark);
      out_file.value("m_{diquark} [GeV]", mDiquark);
      out_file.value("diquark_exp value", std::sqrt(rhoD));
      out_file.value("mixingV1", mixV1);
      out_file.value("mixingV2", mixV2);
      out_file.value("m2_{pi} imag [GeV]", m2PionImag);
      out_file.value("m2_{D} imag [GeV]", m2DiquarkImag);
    }

    template <typename FUN, typename DataOut>
    void readouts_multiple(FUN &helper, DataOut &dataOut) const
    {
      // 1) chiral EoM (no explicit breaking in mesonic EoM)
      helper(
          [&](const auto &x, const auto &u_i) {
            const double d1V_phi = u_i[idxf("uPhi")];
            const double d1V_d   = u_i[idxf("uD")];
            return std::array<double, 2>{{d1V_phi, d1V_d}};
          },
          [&](auto &output, const auto &x, const auto &sol) {
            this->readouts(output, x, sol, "data_chiral.csv", 0);
          });

      // 2) physical EoM (with explicit breaking in phi-direction only)
      helper(
          [&](const auto &x, const auto &u_i) {
            constexpr double eps = 1e-14;
            const double rhoPhi = x[0];
            const double sigma  = std::sqrt(2.0 * std::max(rhoPhi, 0.0));

            const double d1V_phi = u_i[idxf("uPhi")];
            const double d1V_d   = u_i[idxf("uD")];

            const double F_phi = d1V_phi - prm.cSigma / (sigma + eps);
            const double F_d   = d1V_d;

            return std::array<double, 2>{{F_phi, F_d}};
          },
          [&](auto &output, const auto &x, const auto &sol) {
            this->readouts(output, x, sol, "data_running_EoM.csv", 1);
          });
    }

    
};