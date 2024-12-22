#pragma once
#include <iomanip>
#include <ostream>

namespace util
{
    struct ViewMatrix
    {
        float m[4][4];

        const float* operator[](const int index) const
        {
            return m[index];
        }

        float* operator[](const int index)
        {
            return m[index];
        }

        friend std::wostream& operator<<(std::wostream& os, const ViewMatrix& matrix)
        {
            os << L"ViewMatrix:\n";
            for (int i{0}; i < 4; ++i)
            {
                os << L"[ ";
                for (int j{0}; j < 4; ++j)
                {
                    os << std::setw(10) << std::fixed << std::setprecision(3) << matrix[i][j] << L" ";
                }
                os << L"]\n";
            }
            return os;
        }
    };
}
