#ifndef __TNIC_MUSIC_H
#define __TNIC_MUSIC_H

#include <string.h>
#include <stdlib.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

#include "application.h"
#include "rustvibes.h"
#include "playlist.h"
#include "embeds.h"

void tnic_proccessApplicationCommand(tnic_application app, const struct discord_interaction *event);
void tnic_registerMusicCommands(struct discord *bot, const struct discord_ready *event);
void tnic_cmusicProcessEvent(tnic_application app, struct coglink_client *c_client, struct coglink_node *node, struct coglink_track_end *trackEnd);

#endif