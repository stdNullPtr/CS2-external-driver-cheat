#include "CCSPlayerController.hpp"

namespace sdk
{
    void CCSPlayerController::FreeMem() const
    {
        delete[] m_sSanitizedPlayerName;
    }
}
