#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

#include "error.h"
#include "music.h"
#include "embeds.h"
#include "application.h"
#include "rustvibes.h"

tnic_application app = {
    .bot = NULL,
    .client = NULL,
    .config = NULL,
    .botReady = false,
    .coglinkReady = false,
};

// Clean command
void applicationClean(tnic_application app) {
    discord_cleanup(app.bot);
    ccord_shutdown_async();

    if (!app.coglinkReady) {
        free(app.client);
    } else {
        coglink_cleanup(app.client);
    }

    ccord_global_cleanup();

    if (app.config->botStatus != NULL) {
        free(app.config->botStatus);
    }

    if (app.config->botGameName != NULL) {
        free(app.config->botGameName);
    }

    if (app.playlistController->playlist != NULL) {
        playlist_clearPlaylist(app.playlistController->playlist);
        free(app.playlistController->playlist);
    }

    free(app.playlistController);
    free(app.config);
}

// Events
void on_coglink_ready(struct coglink_client *client, struct coglink_node *node, 
                      struct coglink_ready *ready) {
    log_info("[COGLINK] Node connected [%s]", ready->session_id);
    app.botReady = true;
}

void on_ready(struct discord *bot, const struct discord_ready *event) {
    if (!app.botReady) {
        log_fatal("[TNiC] CogLink node wasn't connected");
        applicationClean(app);
        exit(-1);
    }

    log_info("[TNiC] Bot ready");

    // Setting up bot's status
    if (app.config->botGameName != NULL) {
        struct discord_activity activities[] = {{
            .name = app.config->botGameName,
            .type = DISCORD_ACTIVITY_GAME,
            }
        };

        struct discord_presence_update status = {
            .activities =
                &(struct discord_activities){
                    .size = sizeof(activities) / sizeof *activities,
                    .array = activities,
                },
            .status = app.config->botStatus,
            .afk = false,
            .since = discord_timestamp(bot)
        };

        discord_update_presence(bot, &status);
    }

    tnic_registerMusicCommands(bot, event);
}

void on_interaction(struct discord *bot, const struct discord_interaction *event) {
#ifdef DEBUG
    if (app.config->allowedBotAdmin != 0 && 
        event->member->user->id != app.config->allowedBotAdmin) {
        tnic_sendErrorEmbed(app, event, "#eDEB", 
                            "You are not allowed to use bot");
        return;
    }
#endif

    if (event->type == DISCORD_INTERACTION_APPLICATION_COMMAND) {
        tnic_proccessApplicationCommand(app, event);
        return;
    }
}

void on_coglink_track_end(struct coglink_client *c_client, struct coglink_node *node, 
                          struct coglink_track_end *trackEnd) {
    tnic_cmusicProcessEvent(app, c_client, node, trackEnd);
}

void on_sigint_sigtrm(int signal) {
    log_info("[TNiC] Catched signal %d, cleaning application", signal);
    applicationClean(app);
    exit(0);
}

void botPrepear(struct discord *bot) {
    // Setting up intents
    discord_add_intents(bot, DISCORD_GATEWAY_GUILD_VOICE_STATES);
    discord_add_intents(bot, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_add_intents(bot, DISCORD_GATEWAY_GUILDS);

    // Setting global
    discord_set_data(bot, &app);

    // Adding application commands
    discord_set_on_ready(bot, &on_ready);
    discord_set_on_interaction_create(bot, &on_interaction);
}

enum tnic_errorTypes loadApplicationConfig(tnic_application app) {
    struct ccord_szbuf_readonly value;

    app.config->botStatus = NULL;
    app.config->botGameName = NULL;

    value = discord_config_get_field(app.bot, (char *[1]) { "bot_id" }, 1);

    if (value.start == NULL) {
        return tnic_VALUE_NOT_FOUND;
    }

    app.config->botId = strtoull(value.start, NULL, 10);

    log_info("[TNiC] Loaded bot_id from config: %llu", app.config->botId);

    value = discord_config_get_field(app.bot, (char *[1]) { "game_name" }, 1);

    if (value.start != NULL) {
        app.config->botGameName = (char*)malloc(value.size + 1);
        snprintf(app.config->botGameName, value.size + 1, "%s", value.start);
        log_info("[TNiC] Loaded game name from config: \"%s\"", app.config->botGameName);

        value = discord_config_get_field(app.bot, (char *[1]) { "status" }, 1);

        if (value.start == NULL) {
            return tnic_VALUE_NOT_FOUND;
        }

        app.config->botStatus = (char*)malloc(value.size + 1);
        snprintf(app.config->botStatus, value.size + 1, "%s", value.start);
        log_info("[TNiC] Loaded status from config: %s", app.config->botStatus);
    }

#ifdef DEBUG
    value = discord_config_get_field(app.bot, (char *[1]) { "admin_debug_id" }, 1);

    if (value.start != NULL) {
        app.config->allowedBotAdmin = strtoull(value.start, NULL, 10);
        log_info("[TNiC] Loaded admin_debug_id from config: %llu", app.config->allowedBotAdmin);
    } else {
        app.config->allowedBotAdmin = 0;
        log_info("[TNiC] admin_debug_id not set, commands won't be locked", app.config->allowedBotAdmin);
    }
#endif

    return tnic_OK;
}

int main(void) {
    signal(SIGINT, &on_sigint_sigtrm);
    signal(SIGTERM, &on_sigint_sigtrm);
    struct coglink_client *client;
    struct discord *bot;

    puts("AT PROJECT Limited, 2021 - 2025; ATNiC-v0.0.7");
    puts("Product licensed by GPLv3, file `LICENSE`");
    puts("This is a prototype version and should not be used in production environments");
    puts("by Vladislav 'ElCapitan' Nazarov");

    // Creating bot and coglink client
    bot = discord_config_init("config.json");
    client = malloc(sizeof(struct coglink_client));
    
    // Setting up application
    app.bot = bot;
    app.client = client;
    app.config = (tnic_applicationConfig*)malloc(sizeof(tnic_applicationConfig));
    app.playlistController = (tnic_playlist_controller*)malloc(sizeof(tnic_playlist_controller));
    app.playlistController->playlist = NULL;

    // Checking wether variables was initialized
    if (bot == NULL) {
        log_fatal("[TNiC] Concord initialization failed");
        discord_cleanup(bot);
        applicationClean(app);
        return -1;
    }

    if (client == NULL) {
        log_fatal("[TNiC] Coglink initialization failed");
        applicationClean(app);
        return -1;
    }

    if (app.config == NULL) {
        log_fatal("[TNiC] Config initialization failed");
        applicationClean(app);
        return -1;
    }

    if (app.playlistController == NULL) {
        log_fatal("[TNiC] Playlist controller initialization failed");
        applicationClean(app);
        return -1;
    }

    // Prepearing application
    botPrepear(app.bot);
    if (loadApplicationConfig(app) == tnic_VALUE_NOT_FOUND) {
        log_fatal("[TNiC] Configuration file corrupt or wasn't properly filled up");
        applicationClean(app);
        return -1;
    }

    // Prepearing coglink
    struct coglink_nodes nodes = {
        .array = (struct coglink_node[]){{
                .name = "Node 1",
                .hostname = "127.0.0.1",
                .port = 2333,
                .password = "youshallnotpass",
                .ssl = false,
            }
        },
        .size = 1
    };

    app.client->bot_id = app.config->botId;
    app.client->events = &(struct coglink_events){
        .on_ready = &on_coglink_ready,
        .on_track_end = &on_coglink_track_end
    };
    app.client->num_shards = "1";

    // Connecting nodes
    if (coglink_connect_nodes(app.client, app.bot, &nodes) == COGLINK_FAILED) {
        log_fatal("[COGLINK] Can't connect nodes");
        applicationClean(app);
        return -1;
    }

    app.coglinkReady = true;

    // Running the application
    discord_run(app.bot);
    applicationClean(app);
    return 0;
}
