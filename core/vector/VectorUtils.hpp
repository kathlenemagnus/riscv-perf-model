// <VectorUtils.hpp> -*- C++ -*-

#pragma once

#include <cstdint>
#include <set>
#include <algorithm>

namespace sparta
{
    class TreeNode;
}

namespace olympia::vector_utils
{
    static const std::set<uint32_t> valid_vlen_values{128, 256, 512, 1024};
    static const std::set<uint32_t> valid_sew_values{8, 16, 32, 64};
    static const std::set<float> valid_lmul_values{1.0 / 8, 1.0 / 4, 1.0 / 2, 1, 2, 4, 8};

    inline bool isVlenValid(const uint32_t vlen)
    {
        return std::find(valid_vlen_values.begin(), valid_vlen_values.end(), vlen)
               != valid_vlen_values.end();
    }

    inline bool isDlenValid(const uint32_t dlen)
    {
        return std::find(valid_vlen_values.begin(), valid_vlen_values.end(), dlen)
               != valid_vlen_values.end();
    }

    inline bool isSewValid(const uint32_t sew)
    {
        return std::find(valid_sew_values.begin(), valid_sew_values.end(), sew)
               != valid_sew_values.end();
    }

    inline bool isLmulValid(const float lmul)
    {
        return std::find(valid_lmul_values.begin(), valid_lmul_values.end(), lmul)
               != valid_lmul_values.end();
    }

    inline uint32_t getVlmax(const uint32_t vlen, const uint32_t sew, const float lmul)
    {
        return (vlen / sew) * lmul;
    }
} // namespace olympia::vector_utils
