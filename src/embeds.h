#ifndef __TNIC_EMBEDS_H
#define __TNIC_EMBEDS_H

#include <concord/discord.h>

#include "application.h"

void tnic_sendErrorEmbed(tnic_application app, const struct discord_interaction *event, 
                         char *errstring, char *message);

#endif