#pragma once

#include <optional>

#include "Snapshot.h"
#include "mapping_common.h"

namespace LinAlgPointMapping {
    std::optional<PointF> map_snapshot_to_cursor(const Snapshot &src, const ScreenCorners &dst_corners);
};
