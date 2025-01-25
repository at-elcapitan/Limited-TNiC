#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

#include "rustvibes.h"
#include "struct.h"
#include "error.h"
#include "music.h"

tnic_application app = {
    .bot = NULL,
    .client = NULL,
    .botReady = false
};

void on_coglink_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready) {
    log_info("[COGLINK] Node connected [%s]", ready->session_id);
    app.botReady = true;
}

void on_ready(struct discord *bot, const struct discord_ready *event) {
    log_info("Bot ready");

    struct discord_create_global_application_command params = {
        .name = "ping",
        .description = "Ping command!"
    };

    discord_create_global_application_command(bot, event->application->id, &params, NULL);
}

void applicationClean(tnic_application app) {
    discord_cleanup(app.bot);
    coglink_cleanup(app.client);
    ccord_global_cleanup();
    free(app.config);
}

void botPrepear(struct discord *bot) {
    // Setting up intents
    discord_add_intents(bot, DISCORD_GATEWAY_GUILD_VOICE_STATES);
    discord_add_intents(bot, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_add_intents(bot, DISCORD_GATEWAY_GUILDS);

    // Adding application commands
    discord_set_on_ready(bot, &on_ready);
    discord_set_on_interaction_create(bot, &tnic_onInteraction);
}

/**
 * @brief Load application configuration from a file
 * @return Function returns tnic_errnoReturn struct.
 * @retval If errno is not tnic_OK, data field won't contain information (NULL).
 * @retval If errno is tnic_OK, data will contain void* to tnic_applicationConfig type.
 */
tnic_errnoReturn loadApplicationConfig() {
    tnic_errnoReturn err;
    
    macro_todo();

    err.data = NULL;
    err.errno = tnic_OK;
    return err;
}

int main(void) {
    struct coglink_client *client;
    struct discord *bot;
    u64snowflake botId;

    puts("AT PROJECT Limited, 2021 - 2025; ATNiC-v0.0.1a");
    puts("Product licensed by GPLv3, file `LICENSE`");
    puts("This is a prototype version and should not be used in production environments");
    puts("by Vladislav 'ElCapitan' Nazarov");

    // Creating bot and coglink client
    bot = discord_config_init("config.json");
    client = malloc(sizeof(struct coglink_client));

    // Checking wether variables was initialized
    if (bot == NULL) {
        log_fatal("Concord initialization failed");
        return -1;
    }

    if (client == NULL) {
        log_fatal("Coglink initialization failed");
        return -1;
    }

    // Adding bot and coglink client to the application struct
    app.bot = bot;
    app.client = client;

    // Prepearing application
    botPrepear(app.bot);

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

    app.client->events = &(struct coglink_events){
        .on_ready = &on_coglink_ready
    };

    tnic_errnoReturn loadErrno = loadApplicationConfig();
    if (loadErrno.errno != tnic_OK) {
        log_fatal("Unable to load app config");
        applicationClean(app);
        return -1;
    }

    app.config = (tnic_applicationConfig*)loadErrno.data;
    app.client->num_shards = "1";

    int nodesState = coglink_connect_nodes(app.client, app.bot, &nodes);

    if (nodesState == COGLINK_FAILED) {
        log_fatal("[COGLINK] Can't connect nodes");
        applicationClean(app);
        return -1;
    }

    // Running the application
    discord_run(app.bot);

    applicationClean(app);
    return 0;
}
