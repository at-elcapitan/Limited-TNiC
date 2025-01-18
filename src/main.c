#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

struct coglink_client *c_client;

void handle_sigint(int signum) {
    ccord_shutdown_async();
}

void on_coglink_ready(struct coglink_client *client, struct coglink_node *node, struct coglink_ready *ready) {
    log_info("[COGLINK] Node connected [%s]", ready->session_id);
}

int main(void) {
    // signal(SIGINT, handle_sigint);
    puts("AT PROJECT Limited, 2021 - 2025; ATNiC-v0.0.1a");
    puts("Product licensed by GPLv3, file `LICENSE`");
    puts("This is a prototype version and should not be used in production environments");
    puts("by Vladislav 'ElCapitan' Nazarov");

    struct discord *bot = discord_config_init("config.json");
    c_client = malloc(sizeof(struct coglink_client));

    if (bot == NULL) {
        fprintf(stderr, "Client initialization failed");
        return -1;
    }

    discord_add_intents(bot, DISCORD_GATEWAY_GUILD_VOICE_STATES);
    discord_add_intents(bot, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_add_intents(bot, DISCORD_GATEWAY_GUILDS);

    struct coglink_nodes nodes = {
        .array = (struct coglink_node[]){
        {
            .name = "Node 1",
            .hostname = "127.0.0.1",
            .port = 2333,
            .password = "youshallnotpass",
            .ssl = false,
        }
        },
        .size = 1
    };

    c_client->events = &(struct coglink_events){
        .on_ready = &on_coglink_ready
    };

    c_client->bot_id = 1265446831660466297;
    c_client->num_shards = "1";

    coglink_connect_nodes(c_client, bot, &nodes);
    discord_run(bot);

    discord_cleanup(bot);
    coglink_cleanup(c_client);
    ccord_global_cleanup();
    return 0;
}
