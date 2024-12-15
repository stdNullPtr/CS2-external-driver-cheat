#pragma once

#pragma once
#include <iomanip>
#include <ostream>

namespace util
{
    struct BoneMatrix
    {
        float m[4][2];

        const float* operator[](const int index) const
        {
            return m[index];
        }

        float* operator[](const int index)
        {
            return m[index];
        }

        friend std::wostream& operator<<(std::wostream& os, const BoneMatrix& matrix)
        {
            os << L"BoneMatrix:\n";
            for (int i{0}; i < 4; ++i)
            {
                os << L"[ ";
                for (int j{0}; j < 2; ++j)
                {
                    os << std::setw(10) << std::fixed << std::setprecision(3) << matrix[i][j] << L" ";
                }
                os << L"]\n";
            }
            return os;
        }
    };
}
