﻿// Copyright (c) 2019 - Jorma Rebane 3DCrane UE4
#pragma once
#include <string> // std::string

namespace crane3d
{
    /**
     * Allows switching between different crane model dynamics
     */
    enum class ModelType
    {
        // The most basic and foolproof crane model
        Linear,

        // Non-linear model with constant pendulum length with 2 control forces.
        // LiftLine (Fwind) is ignored
        NonLinearConstLine,


        // Non-linear fully dynamic model with all 3 forces
        NonLinearComplete,

        // Original non-linear fully dynamic model with all 3 forces and refined friction formulae
        NonLinearOriginal,
    };

    //////////////////////////////////////////////////////////////////////
    // Basic physics relations

    template<class T> struct Unit
    {
        double Value = 0.0;
        static const Unit Zero;
        Unit operator+(Unit b) const { return { Value + b.Value }; }
        Unit operator-(Unit b) const { return { Value - b.Value }; }
        bool operator>(Unit b) const { return Value > b.Value; }
        Unit operator+(double x) const { return { Value + x }; }
        Unit operator-(double x) const { return { Value - x }; }
        Unit operator*(double x) const { return { Value * x }; }
        Unit operator/(double x) const { return { Value / x }; }
        bool operator>(double b) const { return Value > b; }
        bool operator<(double b) const { return Value < b; }
        double operator/(Unit b) const { return Value / b.Value; }
        Unit operator-() const { return { -Value }; }
    };
    struct _Force {};
    struct _Mass  {};
    struct _Accel {};
    using Force = Unit<_Force>;
    using Mass  = Unit<_Mass>;
    using Accel = Unit<_Accel>;

    inline double sign(double x) { return x > 0 ? 1.0 : (x < 0 ? -1.0 : 0.0); }
    template<class T> inline double sign(Unit<T> x) { return sign(x.Value); }
    template<class T> inline Unit<T> abs(Unit<T> x) { return { std::abs(x.Value) }; }

    template<class T> inline Unit<T> operator*(double x, Unit<T> u) { return { x * u.Value }; }
    template<class T> inline bool operator>(double x, Unit<T> u) { return x > u.Value; }

    // F = ma
    inline Force operator*(Mass m, Accel a) { return { m.Value * a.Value }; }
    inline Force operator*(Accel a, Mass m) { return { m.Value * a.Value }; }
    // a = F/m
    inline Accel operator/(Force F, Mass m) { return { F.Value / m.Value }; }

    constexpr Force operator""_N (long double newtons) { return { double(newtons) }; }
    constexpr Mass  operator""_kg(long double kilos)   { return { double(kilos)   }; }
    constexpr Accel operator""_ms2(long double accel)  { return { double(accel)   }; }

    constexpr Force operator""_N (unsigned long long newtons) { return { double(newtons) }; }
    constexpr Mass  operator""_kg(unsigned long long kilos)   { return { double(kilos)   }; }
    constexpr Accel operator""_ms2(unsigned long long accel)  { return { double(accel)   }; }

    //////////////////////////////////////////////////////////////////////

    // compute new velocity:
    // v = v0 + a*Δt
    inline double integrate_velocity(double v0, Accel a, double Δt)
    {
        return v0 + a.Value*Δt;
    }

    // compute new position from v and a:
    // Velocity Verlet integration
    // x = x0 + (oldV + newV)*dt*0.5
    inline double integrate_pos(double x0, double v, Accel a, double Δt)
    {
        double oldV = v;
        double newV = v + a.Value*Δt;
        return x0 + (oldV + newV)*Δt*0.5;
    }

    // avg velocity = (x2 - x1) / (t2 - t1)
    inline double average_velocity(double x1, double x2, double Δt)
    {
        return (x2 - x1) / Δt;
    }

    //////////////////////////////////////////////////////////////////////

    struct Vec3d
    {
        double X = 0.0, Y = 0.0, Z = 0.0;

        Vec3d operator+(const Vec3d& v) const { return { X + v.X, Y + v.Y, Z + v.Z }; }
        Vec3d operator-(const Vec3d& v) const { return { X - v.X, Y - v.Y, Z - v.Z }; }
        Vec3d operator*(const Vec3d& v) const { return { X * v.X, Y * v.Y, Z * v.Z }; }
        Vec3d operator/(const Vec3d& v) const { return { X / v.X, Y / v.Y, Z / v.Z }; }
    };

    /**
     * Output state of the model
     */
    struct ModelState
    {
        double Alfa = 0.0; // α pendulum measured alfa angle
        double Beta = 0.0; // β pendulum measured beta angle

        double RailOffset = 0.0; // Xw distance of the rail with the cart from the center of the construction frame
        double CartOffset = 0.0; // Yw distance of the cart from the center of the rail
        double LiftLine   = 0.0; // R lift-line length

        // Payload 3D coordinates
        double PayloadX = 0.0;
        double PayloadY = 0.0;
        double PayloadZ = 0.0;

        void Print() const;
    };

    // Coordinate system of the Crane model
    // X: outermost movement of the rail, considered as forward
    // Y: left-right movement of the cart
    // Z: up-down movement of the payload
    class Model
    {
    public:
        /**
         * NOTE: These are the customization parameters of the model
         */
        // Which model to use? Linear is simple and foolproof		
        ModelType Type = ModelType::Linear;
        Mass Mpayload = 1.000_kg; // Mc mass of the payload
        Mass Mcart    = 1.155_kg; // Mw mass of the cart
        Mass Mrail    = 2.200_kg; // Ms mass of the moving rail
        double G { 9.81 };  // gravity constant, 9.81m/s^2
        Accel g = 9.81_ms2; // gravity constant, 9.81m/s^2

        double RailFriction = 100.0; // Tx rail friction
        double CartFriction = 82.0;  // Ty cart friction
        double WindingFriction = 75.0;  // Tr liftline winding friction 

        // cart, rail, line limits
        double RailLimitMin = -0.3;
        double RailLimitMax = +0.3;

        double CartLimitMin = -0.35;
        double CartLimitMax = +0.35;

        double LineLimitMin = 0.05;
        double LineLimitMax = 0.90;

    private:
        double X = 0.0; // distance of the rail with the cart from the center of the construction frame
        double Y = 0.0; // distance of the cart from the center of the rail;
        double R = 0.5; // length of the lift-line
        double Alfa = 0.0; // α angle between y axis (cart moving left-right) and the lift-line
        double Beta = 0.0; // β angle between negative direction on the z axis and the projection
                           // of the lift-line onto the xz plane
        
        // only used for basic linear model
        double Δα = 0.0, Δα_vel = 0.0;
        double Δβ = 0.0, Δβ_vel = 0.0;

        // velocity time derivatives
        double X_vel = 0.0; // rail X velocity
        double Y_vel = 0.0; // cart Y velocity
        double R_vel = 0.0; // payload Z velocity
        double Alfa_vel = 0.0;
        double Beta_vel = 0.0;

        // x1..x10 as per 3DCrane mathematical model description
        double u1, u2, u3; // driving acceleration of cart, rail, wind
        double T1, T2, T3; // friction accel of cart, rail, wind
        double N1, N2, N3; // net acceleration of cart, rail, wind

        double ADrcart, ADrrail, ADrwind; // driving accel of cart, rail, wind
        double AFrcart, AFrrail, AFrwind; // friction accel of cart, rail, wind
        Accel ANetcart, ANetrail, ANetwind; // net accel of cart, rail, wind
        double μ1, μ2; // coefficient of friction: payload/cart ratio;  payload/railcart ratio

        
        Force FappRail, FappCart, FappWind;
        Force FnetRail, FnetCart, FnetWind;
        Force FfriRail, FfriCart, FfriWind;

        // friction coefficient for Steel-Steel (depends highly on type of steel)
        // https://hypertextbook.com/facts/2005/steel.shtml
        double μStaticDrySteel  = 0.8; // static coeff, dry surface
        double μKineticDrySteel = 0.7; // kinetic coeff, dry surface

        // simulation time sink for running correct number of iterations every update
        double SimulationTime = 0.0;
        int SimulationCounter = 0; // for debugging

    public:

        Model();

        /**
         * Updates the model using a fixed time step
         * @param fixedTime Size of the fixed time step. For example 0.01
         * @param deltaTime Time since last update
         * @param Frail force driving the rail with cart (Fx)
         * @param Fcart force driving the cart along the rail (Fy)
         * @param Fwind force winding the lift-line (Fr)
         * @return New state of the crane model
         */
        ModelState UpdateFixed(double fixedTime, double deltaTime, Force Frail, Force Fcart, Force Fwind);

        /**
         * Updates the model using deltaTime as the time step. This can be unstable if deltaTime varies.
         * @param deltaTime Time since last update
         * @param Frail force driving the rail with cart (Fx)
         * @param Fcart force driving the cart along the rail (Fy)
         * @param Fwind force winding the lift-line (Fr)
         * @return New state of the crane model
         */
        ModelState Update(double deltaTime, Force Frail, Force Fcart, Force Fwind);

        /**
         * @return Current state of the crane:
         *  distance of the rail, cart, length of lift-line and swing angles of the payload
         */
        ModelState GetState() const;

        std::wstring GetStateDebugText() const;

        Force NetForce(Force Fapplied, double velocity,
            Mass m, double μStatic, double μKinetic, Force* outFriction) const;

    private:


        void PrepareBasicRelations(Force Frail, Force Fcart, Force Fwind);
        
        // ------------------
        
        void BasicLinearModel(double dt, Force Frail, Force Fcart, Force Fwind);

        // ------------------

        void NonLinearConstLine(double dt, Force Frail, Force Fcart, Force Fwind);
        void NonLinearCompleteModel(double dt, Force Frail, Force Fcart, Force Fwind);
        void NonLinearOriginalModel(double dt, Force Frail, Force Fcart, Force Fwind);

        // ------------------

        void ApplyLimits();
        void DampenAllValues();
    };

}
