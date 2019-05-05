// Copyright (c) 2019 - Jorma Rebane Crane3D
// Distributed under MIT License
#pragma once
#include "Kinematics.h"

namespace crane3d
{
    //////////////////////////////////////////////////////////////////////

    /**
     * Single force component with its own Position, Velocity, Acceleration and Net Force
     */
    struct Component
    {
        Mass Mass = 1_kg;
        double Pos = 0.0;
        double LimitMin = 0.0;
        double LimitMax = 0.0;
        double VelMax = 0.0; // velocity limit, 0 = disabled
        double AccMax = 0.0; // acceleration limit, 0 = disabled
        double Vel = 0.0;
        Accel Acc = 0_ms2; // actual acceleration

        Force Applied; // applied force
        Force SFriction; // static friction
        Force KFriction; // kinematic friction
        Force Fnet; // net force
        Accel NetAcc; // net driving acceleration


        double FrictionDir = 1.0;

        // If true, this component's `Pos` will not be changed during Update.
        bool Const = false;

        // friction coefficient for Steel-Steel (depends highly on type of steel)
        // https://hypertextbook.com/facts/2005/steel.shtml
        double CoeffStatic = 0.8; // static coeff, dry surface
        double CoeffKinetic = 0.7; // kinetic coeff, dry surface

        Component() = default;
        Component(double pos, double limitMin, double limitMax)
            : Pos{pos}, LimitMin{limitMin}, LimitMax{limitMax} {}

        void SetLimits(double min, double max) { LimitMin = min; LimitMax = max; }

        // Resets all dynamic variables: Pos, Vel, Acc, Fnet, NetAcc
        void Reset();

        // Update pos and vel using "Velocity Verlet" integration
        void Update(Accel new_acc, double dt);
        
        // Apply driving forces
        void ApplyForce(Force applied, Accel g);
        void ApplyForceNonLinear(Force applied, Accel g, double T, double Ts);

        // Prevent applying force when against frame
        Force ClampForceByPosLimits(Force force) const;
    };

    //////////////////////////////////////////////////////////////////////
}