#include "embeds.h"
#define MUSIC_DESCRIPTION_STRLEN      690
/**
 * @brief This function creates a Discord embed with a specified error message.
 *
 * @param errstring A string representing the error type or title.
 * @param message A detailed error message.
 *
 * @return A discord_embed struct containing the error information.
 *
 * @note Embed must NOT be cleaned by discord_embed_cleanup()
 */
void tnic_sendErrorEmbed(tnic_application app, const struct discord_interaction *event, 
                         char *errstring, char *message) {
    struct discord_embed_footer footer = {
        .text = errstring,
        .icon_url = NULL,
        .proxy_icon_url = NULL
    };

    struct discord_embed embed = {
        .color = 0xFF0000,
        .title = message,
        .footer = &footer
    };

    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data) {
            .embeds = &(struct discord_embeds){
                .size = 1,
                .array = &embed,
            },
            .flags = DISCORD_MESSAGE_EPHEMERAL
        }
    };

    discord_create_interaction_response(app.bot, event->id, event->token, &params, NULL);
}

void tnic_sendInfoEmbed(tnic_application app, const struct discord_interaction *event, 
                        char *message) {
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data) {
            .content = message,
            .flags = DISCORD_MESSAGE_EPHEMERAL
        }
    };

    discord_create_interaction_response(app.bot, event->id, event->token, &params, NULL);
}