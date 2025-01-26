#ifndef __TNIC_STRUCT_H
#define __TNIC_STRUCT_H

#include <concord/discord.h>
#include <coglink/lavalink.h>
#include <stdbool.h>

typedef struct {
    u64snowflake botId;
    char *botGameName;
    char *botStatus;
} tnic_applicationConfig;

typedef struct {
    bool botReady;
    bool coglinkReady;
    struct discord *bot;
    struct coglink_client *client;
    tnic_applicationConfig *config;
} tnic_application;

#endif