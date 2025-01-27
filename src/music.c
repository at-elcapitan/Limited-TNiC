#include "music.h"

#define GET_TRACK_CURL_ERROR        1
#define GET_TRACK_ALLCATION_ERORR   2
#define GET_TRACK_CURL_ESCAPE_ERROR 3

// Private functions
/**
 * @brief Retrieves a track from a given query using the Coglink library.
 *
 * This function initializes a libcurl session, escapes the query, and constructs a search query for the Coglink library.
 * It then loads tracks using the Coglink library and returns the first track found.
 *
 * @param query The search query for the track.
 * @param client The Coglink client used for track retrieval.
 * 
 * Error states:
 * - 1: Curl process error
 * - 2: Curl esacape processing error
 * - 3: Error while allocation memory
 *
 * @return struct tnic_errnoReturn
 *
 * @note void* Ok have to be converted to tnic_track
 */
tnic_errnoReturn getTrackFromQuery(const char *query, struct coglink_client *client,
                                   struct coglink_player *player) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return (tnic_errnoReturn) {
            .Err = tnic_IS_NULL,
            .additionalNumber = 1,
            .Ok = NULL
        };
    }

    char *search = curl_easy_escape(curl, query, strlen(query));
    if (!search) {
        curl_easy_cleanup(curl);
        return (tnic_errnoReturn) {
            .Err = tnic_IS_NULL,
            .additionalNumber = 3,
            .Ok = NULL
        };
    }

    char *searchQuery = malloc(strlen(search) + sizeof("ytsearch:") + 1);
    if (!searchQuery) {
        curl_free(search);
        curl_easy_cleanup(curl);
        return (tnic_errnoReturn) {
            .Err = tnic_IS_NULL,
            .additionalNumber = 2,
            .Ok = NULL
        };
    }

    snprintf(searchQuery, strlen(search) + sizeof("ytsearch:") + 1, "ytsearch:%s", search);

    struct coglink_load_tracks response = { 0 };

    int status = coglink_load_tracks(client, coglink_get_player_node(client, player), searchQuery, &response);

    curl_free(search);
    curl_easy_cleanup(curl);
    free(searchQuery);

    if (status == COGLINK_FAILED) {
        return (tnic_errnoReturn) {
            .Err = tnic_SEARCH_FAILED,
            .additionalNumber = 0,
            .Ok = NULL
        };
    }

    tnic_track track;
    track.response = &response;

    switch (response.type) {
        case COGLINK_LOAD_TYPE_TRACK:
            struct coglink_load_tracks_track *trackResponse = response.data;
            track.encodedTrack = trackResponse->encoded;
            track.trackInfo = trackResponse->info;
            break;
        
        case COGLINK_LOAD_TYPE_PLAYLIST:
            struct coglink_load_tracks_playlist *data = response.data;
            track.encodedTrack = data->tracks->array[0]->encoded;
            track.trackInfo = data->tracks->array[0]->info;
            break;

        case COGLINK_LOAD_TYPE_SEARCH:
            struct coglink_load_tracks_search *searchResponse = response.data;
            track.encodedTrack = searchResponse->array[0]->encoded;
            track.trackInfo = searchResponse->array[0]->info;
            break;
        
        case COGLINK_LOAD_TYPE_EMPTY:        
        case COGLINK_LOAD_TYPE_ERROR:
            coglink_free_load_tracks(&response);
            return (tnic_errnoReturn) {
                .Err = tnic_SEARCH_FAILED,
                .additionalNumber = 0,
                .Ok = NULL
            };
    }

    return (tnic_errnoReturn) {
        .Err = tnic_OK,
        .additionalNumber = 0,
        .Ok = &track
    };
}

void youtube(tnic_application app, const struct discord_interaction *event) {
    struct coglink_player *player = coglink_create_player(app.client, event->guild_id);
    if (!player) {
        tnic_sendErrorEmbed(app, event, "#e001x2", "Can't create coglink player");
        return;
    }

    struct coglink_user *user = coglink_get_user(app.client, event->member->user->id);
    if (!user) {
        tnic_sendErrorEmbed(app, event, "#e002", "You are not connected to voice channel");
        return;
    }

    coglink_join_voice_channel(app.client, app.bot, event->guild_id, user->channel_id);

    tnic_errnoReturn getTrackErrno = getTrackFromQuery(event->data->options->array[0].value, app.client, player);

    if (getTrackErrno.Err == tnic_IS_NULL) {
        switch (getTrackErrno.additionalNumber)
        {
        case GET_TRACK_CURL_ERROR:
            tnic_sendErrorEmbed(app, event, "#e003x1", "Failed to process request. Report this issue to administrator");
            break;
        
        case GET_TRACK_ALLCATION_ERORR:
            tnic_sendErrorEmbed(app, event, "#eAx1", "Failed to process request. Report this issue to administrator");
            break;

        case GET_TRACK_CURL_ESCAPE_ERROR:
            tnic_sendErrorEmbed(app, event, "#e001x3", "Failed to process request. Report this issue to administrator");
            break;
        }

        return;
    }

    if (getTrackErrno.Err == tnic_SEARCH_FAILED) {
        tnic_sendErrorEmbed(app, event, "#e004", "Failed to load track");
        return;
    }

    tnic_track *track = (tnic_track*)getTrackErrno.Ok;
    
    struct coglink_update_player_params params = {
        .track = &(struct coglink_update_player_track_params) {
            .encoded = track->encodedTrack
        }
    };

    coglink_update_player(app.client, player, &params, NULL);
    coglink_free_load_tracks(track->response);

    struct discord_interaction_response interactionRespParams = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data) {
            .content = "Processing...",
            .flags = DISCORD_MESSAGE_EPHEMERAL
        }
    };

    discord_create_interaction_response(app.bot, event->id, event->token, &interactionRespParams, NULL);
}

macro_testCommand()
void commandTestDisconnect(tnic_application app, const struct discord_interaction *event) {
    struct coglink_player *player = coglink_get_player(app.client, event->guild_id);

    if (!player) {
        tnic_sendErrorEmbed(app, event, "#e001x2", "You are not playing music or not connected");
        return;
    }

    coglink_destroy_player(app.client, player);
    coglink_remove_player(app.client, player);
    coglink_leave_voice_channel(app.client, app.bot, event->guild_id);

    struct discord_interaction_response interactionRespParams = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data) {
            .content = "Disconnecting",
            .flags = DISCORD_MESSAGE_EPHEMERAL
        }
    };

    discord_create_interaction_response(app.bot, event->id, event->token, &interactionRespParams, NULL);
}

// Public functions
void tnic_proccessApplicationCommand(tnic_application app, const struct discord_interaction *event) {
    if (strcmp(event->data->name, "youtube") == 0) {
        youtube(app, event);
        return;
    }

    if (strcmp(event->data->name, "disconnect") == 0) {
        commandTestDisconnect(app, event);
        return;
    }
}

void tnic_registerMusicCommands(struct discord *bot, const struct discord_ready *event) {
    struct discord_application_command_option youtubeCommandOption = {
            .type = DISCORD_APPLICATION_OPTION_STRING,
            .name = "query",
            .description = "Your youtube query",
            .required = true
    };

    struct discord_create_global_application_command youtubeCommand = {
        .name = "youtube",
        .description = "Play youtube music",
        .options = &(struct discord_application_command_options){
            .size = 1,
            .array = &youtubeCommandOption,
        },
    };

    struct discord_create_global_application_command commandTestDisconnect = {
        .name = "disconnect",
        .description = "Connection to voice channel test"
    };

    discord_create_global_application_command(bot, event->application->id, &youtubeCommand, NULL);
    discord_create_global_application_command(bot, event->application->id, &commandTestDisconnect, NULL);
}