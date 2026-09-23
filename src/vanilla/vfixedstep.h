// Fixed-step timing and visual pose interpolation for Hi-Octane 202X.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef VFIXEDSTEP_H
#define VFIXEDSTEP_H

#include "vbase.h"
#include <algorithm>
#include <cmath>

namespace VFixedStep {

constexpr irr::f32 StepSeconds = 0.05f;
constexpr int MaxStepsPerFrame = 4;

class Clock {
public:
    void Advance(irr::f32 frameDeltaTime) {
        // A long stall must not make the next frame simulate unbounded time.
        mRemainder += std::clamp(frameDeltaTime, 0.0f,
                                 StepSeconds * MaxStepsPerFrame);
    }

    bool PopStep() {
        if (mRemainder + 0.000001f < StepSeconds) {
            return false;
        }
        mRemainder = std::max(0.0f, mRemainder - StepSeconds);
        return true;
    }

    irr::f32 Fraction() const {
        return std::clamp(mRemainder / StepSeconds, 0.0f, 1.0f);
    }

private:
    irr::f32 mRemainder = 0.0f;
};

inline irr::f32 InterpolateAngle(irr::f32 previous, irr::f32 current, irr::f32 alpha) {
    return std::remainder(previous + std::remainder(current - previous, 360.0f) * alpha,
                          360.0f);
}

inline VehicleViewStruct InterpolateView(const VehicleViewStruct& previous,
                                         const VehicleViewStruct& current,
                                         irr::f32 alpha) {
    VehicleViewStruct result;
    result.Position = previous.Position + (current.Position - previous.Position) * alpha;
    result.AngleXY = InterpolateAngle(previous.AngleXY, current.AngleXY, alpha);
    result.AngleZY = InterpolateAngle(previous.AngleZY, current.AngleZY, alpha);
    result.AngleXZ = InterpolateAngle(previous.AngleXZ, current.AngleXZ, alpha);
    return result;
}

} // namespace VFixedStep

#endif // VFIXEDSTEP_H
