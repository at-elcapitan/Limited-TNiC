#ifndef __TNIC_EMBEDS_H
#define __TNIC_EMBEDS_H

#include <concord/discord.h>

struct discord_embed tnic_errorEmbed(char *errstring, char *message);

#endif