#include "vrcgenworldlink.h"

#include "fillrand.h"

#include "fmt/core.h"

std::string VRChad::Worlds::GenWorldLink(std::string_view worldId, uint32_t lobbyId, VRChad::Worlds::Privacy privacy, VRChad::Worlds::Region region, std::string_view userId)
{
    using namespace std::literals;

    std::string_view regionStr;
    switch (region) {
    case Region::USW:
        regionStr = "usw"sv;
        break;
    case Region::EU:
        regionStr = "eu"sv;
        break;
    case Region::JP:
        regionStr = "jp"sv;
        break;
    }

    if (privacy == Privacy::Public) {
        return fmt::format("{}:{}~region({})", worldId, lobbyId, regionStr);
    }

    char nonce[49];
    VRChad::Utils::FillRandHex(nonce, 48);
    nonce[48] = 0;
    std::string_view nonceView(nonce, 48);

    switch (privacy) {
    case Privacy::Friends:
        return fmt::format("{}:{}~friends({})~region({})~nonce({})", worldId, lobbyId, userId, regionStr, nonceView);
    case Privacy::FriendsPlus:
        return fmt::format("{}:{}~hidden({})~region({})~nonce({})", worldId, lobbyId, userId, regionStr, nonceView);
    case Privacy::Invite:
        return fmt::format("{}:{}~private({})~region({})~nonce({})", worldId, lobbyId, userId, regionStr, nonceView);
    case Privacy::InvitePlus:
        return fmt::format("{}:{}~private({})~canRequestInvite~region({})~nonce({})", worldId, lobbyId, userId, regionStr, nonceView);
    default:
        return std::string();
    }
}
