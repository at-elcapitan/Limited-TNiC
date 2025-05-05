#ifndef __TNIC_EMBEDS_H
#define __TNIC_EMBEDS_H

#include "application.h"

#include <concord/discord.h>

void tnic_sendErrorEmbed(tnic_application app, const struct discord_interaction *event,
	char *errstring, char *message);
void tnic_sendInfoEmbed(
	tnic_application app, const struct discord_interaction *event, char *message);
#endif
