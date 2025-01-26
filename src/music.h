#ifndef __TNIC_MUSIC_H
#define __TNIC_MUSIC_H

#include <string.h>
#include <stdlib.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

#include "application.h"
#include "rustvibes.h"
#include "embeds.h"

void tnic_proccessApplicationCommand(tnic_application app, const struct discord_interaction *event);
void tnic_registerMusicCommands(struct discord *bot, const struct discord_ready *event);

#endif