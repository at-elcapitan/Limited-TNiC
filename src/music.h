#ifndef __TNIC_MUSIC_H
#define __TNIC_MUSIC_H

#include <string.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

void tnic_onInteraction(struct discord *bot, const struct discord_interaction *event);

#endif