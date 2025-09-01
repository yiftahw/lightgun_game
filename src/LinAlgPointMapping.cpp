#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>

#include "LinAlgPointMapping.h"

namespace LinAlgPointMapping {
    template <size_t N> using float_mat = std::array<std::array<float, N>, N>;
    template <size_t N> using float_arr = std::array<float, N>;

    using float3_arr = float_arr<3>;
    using float3_mat = float_mat<3>;
    using float8_arr = float_arr<8>;
    using float8_mat = float_mat<8>;

    // destructible call, doesn't copy `lhs` or `rhs`
    template<size_t N>
    static std::optional<float_arr<N>> gaussianEliminationInPlace(float_mat<N> &lhs, float_arr<N> &rhs) {
        float_arr<N> result{};

        for (size_t it = 0; it < N; it++) {
            // select row with largest first element for numerical stability
            auto pivot_row = it;
            for (size_t row_it = it + 1; row_it < N; row_it++) {
                if (std::fabs(lhs[row_it][it]) > std::fabs(lhs[pivot_row][it])) {
                    pivot_row = row_it;
                }
            }
            if (std::fabs(lhs[pivot_row][it]) < std::numeric_limits<float>::epsilon()) {
                return std::nullopt;
            }

            // swap pivot row to the top
            if (it != pivot_row) {
                std::swap(lhs[it], lhs[pivot_row]);
                std::swap(rhs[it], rhs[pivot_row]);
            }

            // normalize the pivot row (which is now at the top)
            float norm_factor = lhs[it][it];
            std::ranges::for_each(lhs[it], [norm_factor](float &f) { f /= norm_factor; });
            rhs[it] /= norm_factor;

            // eliminate the current column in all rows below
            for (size_t row_it = it + 1; row_it < N; row_it++) {
                float elim_factor = lhs[row_it][it];
                for (size_t col_it = it; col_it < N; col_it++) {
                    lhs[row_it][col_it] -= elim_factor * lhs[it][col_it];
                }
                rhs[row_it] -= elim_factor * rhs[it];
            }
        }

        // if we got here, we now have an upper triangular matrix
        // we can use back substitution to find the solution
        for (size_t it = 0; it < N; it++) {
            auto row_it = N - 1 - it;
            result[row_it] = rhs[row_it];
            for (size_t col_it = row_it + 1; col_it < N; col_it++) {
                result[row_it] -= lhs[row_it][col_it] * result[col_it];
            }
        }

        return result;
    }

    template<size_t N>
    // basic CPU based matrix-vector multiplication
    float_arr<N> operator*(const float_mat<N> &lhs, const float_arr<N> &rhs) {
        float_arr<N> result{};
        for (size_t row = 0; row < N; row++) {
            result[row] = 0;
            for (size_t col = 0; col < N; col++) {
                result[row] += lhs[row][col] * rhs[col];
            }
        }
        return result;
    }

    std::optional<float3_mat> getPerspectiveTransform(const ScreenCorners &src, const ScreenCorners &dst) {
        float8_mat lhs{};
        float8_arr rhs{};

        auto fill = [&lhs, &rhs](size_t i, const PointF &src, const PointF &dst) {
            lhs[2 * i] = {src.x, src.y, 1, 0, 0, 0, -1.0f * src.x * dst.x, -1.0f * src.y * dst.x};
            rhs[2 * i] = dst.x;
            lhs[2 * i + 1] = {0, 0, 0, src.x, src.y, 1, -1.0f * src.x * dst.y, -1.0f * src.y * dst.y};
            rhs[2 * i + 1] = dst.y;
        };

        fill(0, src.top_left, dst.top_left);
        fill(1, src.top_right, dst.top_right);
        fill(2, src.bot_left, dst.bot_left);
        fill(3, src.bot_right, dst.bot_right);

        auto opt_result = gaussianEliminationInPlace(lhs, rhs);
        if (!opt_result.has_value())
        {
            return std::nullopt;
        }

        // convert to a 3x3 matrix and return the result
        const auto &r = opt_result.value();
        return float3_mat{
            float3_arr{r[0], r[1], r[2]}, 
            float3_arr{r[3], r[4], r[5]}, 
            float3_arr{r[6], r[7], 1.0f}};
    }

    std::optional<PointF> map_snapshot_to_cursor(const Snapshot &src, const ScreenCorners &dst_corners)
    {
        /*
        - common steps as euclidean geometry based mapping:
            calculate the screen corners based on the wii IR sensor width and the screen size
        
            - mapping as a homogeneous transformation steps:
            create an 8x8 linear algebra system of equations based on 4 2d points:
            - top left      maps to (0,0)
            - top right     maps to (0, screen pixel width)
            - bottom left   maps to (screen pixel height, 0)
            - bottom right  maps to (screen pixel height, screen pixel width)
            - 9th dimension is fixed at 1
            
            - this could be simplified by calling `OpenCV::getPerspectiveTransform()` which will internally
            do the homogenous transformation at the cost of adding an additional dependency on `OpenCV`.
        */

        if (!src.is_valid())
        {
            return std::nullopt;
        }

        auto opt_corners = calculate_screen_corners(src);
        if (!opt_corners.has_value())
        {
            return std::nullopt;
        }
        [[maybe_unused]] auto &src_corners = opt_corners.value();

        auto opt_transform = getPerspectiveTransform(src_corners, dst_corners);
        if (!opt_transform.has_value())
        {
            return std::nullopt;
        }
        const auto &transform = opt_transform.value();
        float3_arr ir_centers_homogenous{ir_camera_centers[0], ir_camera_centers[1], 1.0f};

        const auto mapped = transform * ir_centers_homogenous;
        if (std::fabs(mapped[2]) < std::numeric_limits<float>::epsilon())
        {
            return std::nullopt;
        }

        return PointF{mapped[0]/mapped[2], mapped[1]/mapped[2]};
    }
}
