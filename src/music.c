#include "music.h"

// Private functions
macro_testCommand()
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

enum tnic_errorTypes checkNullInteractionEvent(void *item, tnic_application app, const struct discord_interaction *event, 
                               char *errstring, char *message) {
    if (item) {
        return tnic_OK;
    }

    struct discord_embed embed = tnic_errorEmbed(errstring, message);
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
    return tnic_IS_NULL;   
}

macro_testCommand()
void commandTestConnect(tnic_application app, const struct discord_interaction *event) {
    struct coglink_player *player = coglink_create_player(app.client, event->guild_id);
    if (checkNullInteractionEvent(player, app, event, "#e001x2", "Can't create coglink player") == tnic_IS_NULL) return;

    struct coglink_user *user = coglink_get_user(app.client, event->member->user->id);
    if (checkNullInteractionEvent(user, app, event, "#e002", "You are not connected to voice channel") == tnic_IS_NULL) return;

    coglink_join_voice_channel(app.client, app.bot, event->guild_id, user->channel_id);

    CURL *curl = curl_easy_init();
    if (checkNullInteractionEvent(curl, app, event, "#e001x3", "Failed to process request. Report this issue to administrator") == tnic_IS_NULL) return;

    char *search = curl_easy_escape(curl, "ado - nukegara", strlen("ado - nukegara"));
    if (checkNullInteractionEvent(user, app, event, "#e003x1", "Failed to process request. Report this issue to administrator") == tnic_IS_NULL) {
        curl_easy_cleanup(curl);
        return;
    }

    char *searchQuery = malloc(strlen(search) + sizeof("ytsearch:") + 1);
    if (checkNullInteractionEvent(searchQuery, app, event, "#eAx1", "Failed to process request. Report this issue to administrator") == tnic_IS_NULL) {
        curl_free(search);
        curl_easy_cleanup(curl);
        return;
    }

    snprintf(searchQuery, strlen(search) + sizeof("ytsearch:") + 1, "ytsearch:%s", search);

    struct coglink_load_tracks response = { 0 };

    int status = coglink_load_tracks(app.client, coglink_get_player_node(app.client, player), searchQuery, &response);

    curl_free(search);
    curl_easy_cleanup(curl);
    free(searchQuery);

    struct coglink_load_tracks_search *search_response = response.data;
    struct coglink_update_player_params params = {
        .track = &(struct coglink_update_player_track_params) {
            .encoded =search_response->array[0]->encoded,
        },
    };
    
    coglink_update_player(app.client, player, &params, NULL);
    coglink_free_load_tracks(&response);
}

macro_testCommand()
void commandTestDisconnect(tnic_application app, const struct discord_interaction *event) {
    coglink_leave_voice_channel(app.client, app.bot, event->guild_id);
}

// Public functions
void tnic_proccessApplicationCommand(tnic_application app, const struct discord_interaction *event) {
    if (strcmp(event->data->name, "ping") == 0) {
        commandPing(app.bot, event);
        return;
    }

    if (strcmp(event->data->name, "connect") == 0) {
        commandTestConnect(app, event);
        return;
    }

    if (strcmp(event->data->name, "disconnect") == 0) {
        commandTestDisconnect(app, event);
        return;
    }
}

void tnic_registerMusicCommands(struct discord *bot, const struct discord_ready *event) {
    struct discord_create_global_application_command commandTestConnect = {
        .name = "connect",
        .description = "Connection to voice channel test"
    };

    struct discord_create_global_application_command commandTestDisconnect = {
        .name = "disconnect",
        .description = "Connection to voice channel test"
    };

    discord_create_global_application_command(bot, event->application->id, &commandTestConnect, NULL);
    discord_create_global_application_command(bot, event->application->id, &commandTestDisconnect, NULL);
}