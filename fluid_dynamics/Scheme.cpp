//
// Created by petrs on 02.12.2023.
//

#include <cmath>
#include "Scheme.h"
#include "../structures/Primitive.h"
#include "Def.h"
#include "Bound.h"

void Scheme::updateCellDT(
        std::unordered_map<int, Cell> &cells, double CFL,
        bool useGlobalTimeStep) {

    double globalDT = 1e9;

    // iterate over all cells in mesh
    for (auto &cell: cells) {
        // compute primitive variables for each cell (p, rho, u, v, c, ...)
        Primitive pv = Primitive::computePV(cell.second.w);

        double u_xi = fabs(pv.u * cell.second.xi.ux + pv.v *
                cell.second.xi.uy);
        double u_eta = fabs(pv.u * cell.second.eta.ux + pv.v *
                cell.second.eta.uy);

        double d_xi = (u_xi + pv.c) / cell.second.xi.length;
        double d_eta = (u_eta + pv.c) / cell.second.eta.length;

        double res = CFL / (d_xi + d_eta);

        if(useGlobalTimeStep) {
            // take the minimum of all Delta t values
            globalDT = fmin(globalDT, res);
        } else {
            // local time step is assigned to each cell
            cell.second.dt = res;
        }
    }

    if(useGlobalTimeStep) {
        // global time step is assigned to every cell
        for (auto &cell: cells) {
            cell.second.dt = globalDT;
        }
    }
}

// HLL scheme
Conservative Scheme::HLL(const std::unordered_map<int, Cell> &cells,
                         const Interface &face) {
    Conservative res{};

    // conservative values vectors W at the right and left of the interface
    Conservative wl = cells.at(face.left).w;
    Conservative wr = cells.at(face.right).w;

    // primitive variables of vectors W_L and W_R
    Primitive pvl = Primitive::computePV(wl);
    Primitive pvr = Primitive::computePV(wr);

    // normal velocity
    double ql = pvl.u * face.nx + pvl.v * face.ny;
    double qr = pvr.u * face.nx + pvr.v * face.ny;

    double SL = fmin(ql - pvl.c, qr - pvr.c);
    double SR = fmax(ql + pvl.c, qr + pvr.c);

    Conservative FL = Scheme::flux(face, wl, ql, pvl.p);
    Conservative FR = Scheme::flux(face, wr, qr, pvr.p);

    if (SL > 0) {
        res = FL;
    } else if (SL <= 0 && 0 <= SR) {
        res = (SR * FL - SL * FR + SR * SL * (wr - wl)) / (SR - SL);
    } else if (SR < 0) {
        res = FR;
    } else {
        // case when FL || FR == not a number
        std::cout << "error in HLL scheme" << std::endl;
        Def::error = true;
    }

    return res;
}

void Scheme::computeScheme(std::unordered_map<int, Cell> &cells,
                           const std::unordered_map<std::pair<int, int>,
                                   Interface, pair_hash> &faces) {
    for (const auto &face: faces) {
        // chooses between using HLL and HLLC based on a bool isHLLC
        Conservative flux = Def::isHLLC ?
                HLLC(cells, face.second) : HLL(cells, face.second);

        cells.at(face.second.left).rezi -= cells.at(face.second.left).dt /
                cells.at(face.second.left).area * flux*face.second.length;
        cells.at(face.second.right).rezi += cells.at(face.second.right).dt/
                cells.at(face.second.right).area * flux*face.second.length;
    }
}

double Scheme::computeRezi(const std::unordered_map<int, Cell> &cells) {
    double res = 0;

    // for loop over all inner cells (excludes ghost cells
    for (int i = 0; i < Def::inner; ++i) {
        int k = Def::innerIndex(i);
        res += pow(cells.at(k).rezi.r1 /
                cells.at(k).dt, 2) * cells.at(k).area;
    }

    res = log(sqrt(res));
    return res;
}

void Scheme::updateCells(std::unordered_map<int, Cell> &cells) {
    for (int i = 0; i < Def::inner; ++i) {
        int k = Def::innerIndex(i);
        cells.at(k).w += cells.at(k).rezi;
        cells.at(k).rezi = 0;
    }
}

// compute pressure coefficient for data visualisation in paraView
double Scheme::computeCP(double p_inner) {
    return (p_inner - Bound::p_infty) /
    (0.5 * Bound::rho_infty * pow(Bound::u_infty, 2) +
    pow(Bound::v_infty, 2));
}

// HLLC scheme
Conservative Scheme::HLLC(const std::unordered_map<int, Cell> &cells,
                          const Interface &face) {
    Conservative wl = cells.at(face.left).w;
    Conservative wr = cells.at(face.right).w;
    Primitive pvl = Primitive::computePV(wl);
    Primitive pvr = Primitive::computePV(wr);

    double ql = pvl.u * face.nx + pvl.v * face.ny;
    double qr = pvr.u * face.nx + pvr.v * face.ny;

    double q_bar = Scheme::bar(pvl.rho, pvr.rho, ql, qr);
    double h_bar = Scheme::bar(pvl.rho, pvr.rho, pvl.h, pvr.h);
    double u_bar = Scheme::bar(pvl.rho, pvr.rho, pvl.u, pvr.u);
    double v_bar = Scheme::bar(pvl.rho, pvr.rho, pvl.v, pvr.v);
    double U_bar_sq = pow(u_bar, 2) + pow(v_bar, 2);
    double c_bar = sqrt((Def::KAPPA - 1) * (h_bar - 0.5 * U_bar_sq));

    double lambda_1 = ql - pvl.c;
    double lambda_m = qr + pvr.c;
    double lambda_1Roe = q_bar - c_bar;
    double lambda_mRoe = q_bar + c_bar;

    double SL = fmin(lambda_1, lambda_1Roe);
    double SR = fmax(lambda_m, lambda_mRoe);
    double SM = (pvr.rho * qr * (SR - qr) - pvl.rho * ql * (SL - ql) +
            pvl.p - pvr.p) / (pvr.rho * (SR - qr) - pvl.rho * (SL - ql));

    double p_star = pvl.rho * (ql - SL) * (ql - SM) + pvl.p;

    Conservative wlStar = 1 / (SL - SM) *
            Scheme::fluxStar(face, wl, ql, SL, SM, pvl.p, p_star);
    Conservative wrStar = 1 / (SR - SM) *
            Scheme::fluxStar(face, wr, qr, SR, SM, pvr.p, p_star);

    if (SL > 0) {
        return Scheme::flux(face, wl, ql, pvl.p);
    } else if (SL <= 0 && 0 < SM) {
        return Scheme::flux(face, wlStar, SM, p_star);
    } else if (SM <= 0 && 0 <= SR) {
        return Scheme::flux(face, wrStar, SM, p_star);
    } else if (SR < 0) {
        return Scheme::flux(face, wr, qr, pvr.p);
    } else {
        // error handling (I'm a beginner in programming, ok?)
        std::cout << "Scheme::HLLC: unreachable state \n";
    }
}

double Scheme::bar(double rho_l, double rho_r, double vl, double vr) {
    return (sqrt(rho_l) * vl + sqrt(rho_r) * vr) /
    (sqrt(rho_l) + sqrt(rho_r));
}

// evaluation of the \hat F flux
Conservative Scheme::flux(Interface face, Conservative w,
                          double q, double p) {
    Conservative res{};

    res.r1 = w.r1 * q;
    res.r2 = w.r2 * q + p * face.nx;
    res.r3 = w.r3 * q + p * face.ny;
    res.r4 = (w.r4 + p) * q;
    return res;
}

// evaluation of the \hat F star flux used in HLLC scheme
Conservative Scheme::fluxStar(Interface face, Conservative w,
                              double q, double S, double SM,
                              double p, double p_star) {
    Conservative res{};

    res.r1 = w.r1 * (S - q) + 0;
    res.r2 = w.r2 * (S - q) + (p_star - p) * face.nx;
    res.r3 = w.r3 * (S - q) + (p_star - p) * face.ny;
    res.r4 = w.r4 * (S - q) + p_star * SM - p * q;
    return res;
}
