#ifndef __TNIC_STRUCT_H
#define __TNIC_STRUCT_H

#include <concord/discord.h>
#include <coglink/lavalink.h>
#include <stdbool.h>

typedef struct {
    bool botReady;
    struct discord *bot;
    struct coglink_client *client;
} tnic_application;

#endif