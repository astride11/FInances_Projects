// black_scholes.cpp
// Pricing Black-Scholes + Greeks + Implied Volatility (Newton + fallback bisection)
// Compile: g++ -O2 -std=c++17 black_scholes.cpp -o black_scholes

#include <cmath>
#include <iostream>
#include <limits>   
#include <algorithm>
using namespace std;

const double PI = 3.14159265358979323846;
const double EPS = 1e-12;

// --- N(0,1) pdf and cdf ---
inline double norm_pdf(double x) {
    return exp(-0.5 * x * x) / sqrt(2.0 * PI);
}

inline double norm_cdf(double x) {
    return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}

// --- Black-Scholes price european (Call / Put) ---
double bs_price(char option_type, double S, double K, double r, double sigma, double T) {
    if (T <= 0.0) { // option at maturity
        if (option_type == 'c') return max(0.0, S - K);
        else return max(0.0, K - S);
    }
    if (sigma <= 0.0) { // zero volatility -> intrinsic discounted
        double forward = S * exp(r * T);
        if (option_type == 'c') return max(0.0, S - K * exp(-r * T));
        else return max(0.0, K * exp(-r * T) - S);
    }

    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    double d2 = d1 - sigma * sqrtT;

    if (option_type == 'c') {
        return S * norm_cdf(d1) - K * exp(-r * T) * norm_cdf(d2);
    } else {
        return K * exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
    }
}

// --- Greeks ---
double bs_delta(char option_type, double S, double K, double r, double sigma, double T) {
    if (T <= 0.0) {
        if (option_type == 'c') return S > K ? 1.0 : 0.0;
        else return S < K ? -1.0 : 0.0;
    }
    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    if (option_type == 'c') return norm_cdf(d1);
    else return norm_cdf(d1) - 1.0;
}

double bs_gamma(double S, double K, double r, double sigma, double T) {
    if (T <= 0.0 || sigma <= 0.0) return 0.0;
    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    return norm_pdf(d1) / (S * sigma * sqrtT);
}

double bs_vega(double S, double K, double r, double sigma, double T) {
    if (T <= 0.0) return 0.0;
    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    return S * norm_pdf(d1) * sqrtT; // note: vega per 1.0 = percentage points (0.01 -> 1%)
}


double bs_theta(char option_type, double S, double K, double r, double sigma, double T) {
    if (T <= 0.0) return 0.0;
    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    double d2 = d1 - sigma * sqrtT;
    double first = - (S * norm_pdf(d1) * sigma) / (2.0 * sqrtT);
    if (option_type == 'c') {
        double second = r * K * exp(-r * T) * norm_cdf(d2);
        return first - second;
    } else {
        double second = r * K * exp(-r * T) * norm_cdf(-d2);
        return first + second;
    }
}

double bs_rho(char option_type, double S, double K, double r, double sigma, double T) {
    if (T <= 0.0) return 0.0;
    double sqrtT = sqrt(T);
    double d1 = (log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * sqrtT);
    double d2 = d1 - sigma * sqrtT;
    if (option_type == 'c') {
        return K * T * exp(-r * T) * norm_cdf(d2);
    } else {
        return -K * T * exp(-r * T) * norm_cdf(-d2);
    }
}

// --- Implied volatility: Newton-Raphson with fallback bisection ---
double implied_vol(char option_type, double market_price, double S, double K, double r, double T,
                   double initial_sigma = 0.2, double tol = 1e-8, int max_iter = 100) {
    // Market price bounds
    double intrinsic = (option_type == 'c') ? max(0.0, S - K * exp(-r * T)) : max(0.0, K * exp(-r * T) - S);
    double upper_price = S; // trivial upper bound for call < S (for put could be K*e^{-rT})
    if (market_price <= intrinsic + 1e-14) return 0.0; // market price equals intrinsic -> sigma ~ 0
    // set sigma bounds for bisection
    double low = 1e-12;
    double high = 5.0; // large vol upper bound
    // sanity: if market price > price at high vol, increase high (rare)
    double price_high = bs_price(option_type, S, K, r, high, T);
    while (price_high < market_price && high < 1e2) {
        high *= 2.0;
        price_high = bs_price(option_type, S, K, r, high, T);
    }

    // Newton iterations
    double sigma = max(1e-8, initial_sigma);
    for (int i = 0; i < max_iter; ++i) {
        double price = bs_price(option_type, S, K, r, sigma, T);
        double diff = price - market_price;
        if (fabs(diff) < tol) return sigma;
        double vega = bs_vega(S, K, r, sigma, T);
        if (vega > 1e-8) {
            double step = diff / vega;
            sigma -= step;
            // keep sigma in bounds
            if (sigma <= 0.0 || sigma != sigma || sigma > high) {
                // Newton produced invalid sigma, break to fallback
                break;
            }
        } else {
            // Vega too small, fallback
            break;
        }
    }

    // Fallback: bisection to ensure root in [low, high]
    double a = low, b = high;
    double fa = bs_price(option_type, S, K, r, a, T) - market_price;
    double fb = bs_price(option_type, S, K, r, b, T) - market_price;
    if (fa * fb > 0) {
        // No sign change: market price outside achievable range (maybe numerical)
        // Return best effort: clamp
        if (fabs(fa) < fabs(fb)) return a;
        else return b;
    }

    double mid = 0.0;
    for (int i = 0; i < 200; ++i) {
        mid = 0.5 * (a + b);
        double fm = bs_price(option_type, S, K, r, mid, T) - market_price;
        if (fabs(fm) < tol) return mid;
        if (fa * fm <= 0) {
            b = mid;
            fb = fm;
        } else {
            a = mid;
            fa = fm;
        }
        if (b - a < tol) break;
    }
    return mid;
}
