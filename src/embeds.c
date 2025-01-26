#include "embeds.h"

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
struct discord_embed tnic_errorEmbed(char *errstring, char *message) {
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
    
    return embed;
}