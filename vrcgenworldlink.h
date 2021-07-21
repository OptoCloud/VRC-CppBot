#ifndef VRCGENWORLDLINK_H
#define VRCGENWORLDLINK_H

#include <string>
#include <string_view>
#include <cstdint>

namespace VRChad::Worlds {

enum class Privacy
{
    Public,
    FriendsPlus,
    Friends,
    InvitePlus,
    Invite
};

enum class Region
{
    USW,
    EU,
    JP
};

std::string GenWorldLink(std::string_view worldId, std::uint32_t lobbyId, Privacy privacy, Region region, std::string_view userId);

}

#endif // VRCGENWORLDLINK_H
