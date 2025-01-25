#include "music.h"

void commandPing(struct discord *bot, const struct discord_interaction *event) {
    struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data) {
                .content = "pong",
                .flags = DISCORD_MESSAGE_EPHEMERAL
            }
        };

    discord_create_interaction_response(bot, event->id, event->token, &params, NULL);
}

void proccessApplicationCommand(struct discord *bot, const struct discord_interaction *event) {
    if (strcmp(event->data->name, "ping") == 0) {
        commandPing(bot, event);
        return;
    }
}

void tnic_onInteraction(struct discord *bot, const struct discord_interaction *event) {
    if (event->type == DISCORD_INTERACTION_APPLICATION_COMMAND) {
        proccessApplicationCommand(bot, event);
        return;
    }
}