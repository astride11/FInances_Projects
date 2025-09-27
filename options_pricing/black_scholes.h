#ifndef BLACK_SCHOLES_H
#define BLACK_SCHOLES_H

#include <string>

// Fonctions de base
double bs_price(char option_type, double S, double K, double r, double sigma, double T);

// Greeks
double bs_delta(char option_type, double S, double K, double r, double sigma, double T);
double bs_gamma(double S, double K, double r, double sigma, double T);
double bs_vega(double S, double K, double r, double sigma, double T);
double bs_theta(char option_type, double S, double K, double r, double sigma, double T);
double bs_rho(char option_type, double S, double K, double r, double sigma, double T);

// Volatilité implicite
double implied_vol(char option_type, double market_price, double S, double K, double r, double T,
                   double initial_sigma, double tol, int max_iter);

// Fonction qui ajoute la colonne implied_vol_compute dans un CSV
void add_implied_vol_column(const std::string& input_file,
                            const std::string& output_file,
                            char option_type, double S, double r, double T, double initial_sigma);

#endif // BLACK_SCHOLES_H
