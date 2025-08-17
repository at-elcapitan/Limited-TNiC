#include "music.h"
#include <coglink/codecs.h>

#define GET_TRACK_CURL_ERROR 1
#define GET_TRACK_ALLCATION_ERORR 2
#define GET_TRACK_CURL_ESCAPE_ERROR 3
/**
 * @brief Retrieves a track from a given query using the Coglink library.
 *
 * This function initializes a libcurl session, escapes the query, and
 * constructs a search query for the Coglink library. It then loads tracks using
 * the Coglink library and returns the first track found.
 *
 * @param query The search query for the track.
 * @param client The Coglink client used for track retrieval.
 *
 * .additionalNumber used for error details
 * - 1: Curl process error tnic_IS_NULL
 * - 2: Curl esacape processing error tnic_IS_NULL
 * - 3: Error while allocation memory tnic_IS_NULL
 *
 * @return struct tnic_errnoReturn
 *
 * @note void* Ok have to be converted to tnic_track
 */
tnic_errnoReturn getTrackFromQuery(const char *query,
                                   struct coglink_client *client,
                                   struct coglink_player *player,
                                   char *username) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    log_debug("[TNIC/Errno] Errno: tnic_IS_NULL 1, position: %d, at %s",
              __LINE__, __FILE__);
    return (tnic_errnoReturn){.Err = tnic_IS_NULL,

                              .additionalNumber = 1,
                              .Ok = NULL};
  }

  log_debug("Query loaded: %s by %s", query, username);

  char *search = curl_easy_escape(curl, query, strlen(query));
  if (!search) {
    log_debug("[TNIC/Errno] Errno: tnic_IS_NULL 3, position: %d, at %s",
              __LINE__, __FILE__);
    curl_easy_cleanup(curl);
    return (tnic_errnoReturn){
        .Err = tnic_IS_NULL, .additionalNumber = 3, .Ok = NULL};
  }

  char *searchQuery = malloc(strlen(search) + sizeof("ytmsearch:") + 1);
  if (!searchQuery) {
    log_debug("[TNIC/Errno] Errno: tnic_IS_NULL 2, position: %d, at %s",
              __LINE__, __FILE__);
    curl_free(search);
    curl_easy_cleanup(curl);
    return (tnic_errnoReturn){
        .Err = tnic_IS_NULL, .additionalNumber = 2, .Ok = NULL};
  }

  if (strncmp(query, "http://", sizeof("http://") - 1) != 0 &&
      strncmp(query, "https://", sizeof("https://") - 1) != 0) {
    snprintf(searchQuery, strlen(search) + sizeof("ytmsearch:") + 1,
             "ytmsearch:%s", search);
  } else {
    strcpy(searchQuery, search);
  }

  struct coglink_load_tracks *response = (struct coglink_load_tracks *)malloc(
      sizeof(struct coglink_load_tracks *));

  int status = coglink_load_tracks(
      client, coglink_get_player_node(client, player), searchQuery, response);

  curl_free(search);
  curl_easy_cleanup(curl);
  free(searchQuery);

  if (status == COGLINK_FAILED) {
    log_debug("[TNIC/Errno] Errno: tnic_SEARCH_FAILED, position: %d, at %s",
              __LINE__, __FILE__);
    return (tnic_errnoReturn){
        .Err = tnic_SEARCH_FAILED, .additionalNumber = 0, .Ok = NULL};
  }

  tnic_track *track = malloc(sizeof(tnic_track));
  track->response = response;
  track->playingTimeDelta = 0;
  track->username = (char *)malloc(strlen(username));
  strncpy(track->username, username, strlen(username) + 1);

  switch (response->type) {
  case COGLINK_LOAD_TYPE_TRACK:
    struct coglink_load_tracks_track *trackResponse = response->data;
    track->encodedTrack = trackResponse->encoded;
    track->trackInfo = trackResponse->info;
    break;

  case COGLINK_LOAD_TYPE_PLAYLIST:
    struct coglink_load_tracks_playlist *data = response->data;
    track->encodedTrack = data->tracks->array[0]->encoded;
    track->trackInfo = data->tracks->array[0]->info;
    break;

  case COGLINK_LOAD_TYPE_SEARCH:
    struct coglink_load_tracks_search *searchResponse = response->data;
    track->encodedTrack = searchResponse->array[0]->encoded;
    track->trackInfo = searchResponse->array[0]->info;
    break;

  case COGLINK_LOAD_TYPE_EMPTY:
  case COGLINK_LOAD_TYPE_ERROR:
    log_debug("[TNIC/Errno] Errno: tnic_SEARCH_FAILED, position: %d, at %s",
              __LINE__, __FILE__);
    coglink_free_load_tracks(response);
    free(response);

    return (tnic_errnoReturn){
        .Err = tnic_SEARCH_FAILED, .additionalNumber = 0, .Ok = NULL};
  }

  return (tnic_errnoReturn){.Err = tnic_OK, .additionalNumber = 0, .Ok = track};
}

void playTrack(tnic_application app, struct coglink_player *player,
               tnic_track *track, u64snowflake guildId) {
  struct coglink_update_player_params params = {
      .track =
          &(struct coglink_update_player_track_params){
              .encoded = track->encodedTrack,
          },
      .volume = app.playlistController->playlist->volume};

  track->lastPlayStartUnixTime = time(NULL);

  coglink_update_player(app.client, player, &params, NULL);
}

void updateMessage(tnic_application app, const u64snowflake channelId) {
  tnic_playlist *playlist = app.playlistController->playlist;

  if (playlist->channelId == 0) {
    playlist->channelId = channelId;
  }

  tnic_track *track = playlist->currentTrack;

  char *footerText = (char *)malloc(80);

  if (!footerText) {
    log_error("Unable to allocate memory for variable");
    return;
  }

  char *loopState = (char *)malloc(14);

  if (!loopState) {
    log_error("Unable to allocate memory for variable");
    free(footerText);
    return;
  }

  switch (playlist->currentState) {
  case PLAYLIST_NORMAL:
    strcpy(loopState, "turned off");
    break;

  case PLAYLIST_LOOP_PLAYLIST:
    strcpy(loopState, "on playlist");
    break;

  case PLAYLIST_REPEAT_SINGLE_TRACK:
    strcpy(loopState, "current track");
    break;
  }

  if (playlist->tracks == NULL) {
    snprintf(footerText, 80, "Loop: %s\nPosition: %d of %ld\nVolume: %d%%",
             loopState, 0, playlist->size, playlist->volume);

    struct discord_embed_footer footer = {
        .text = footerText, .icon_url = NULL, .proxy_icon_url = NULL};

    struct discord_embed embed = {.description =
                                      "Length: 00:00\n\n> URL:\n> Ordered by:",
                                  .title = "Music is not playing",
                                  .color = 0xa31eff,
                                  .footer = &footer};

    struct discord_edit_message params = {
        .embeds = &(struct discord_embeds){.array = &embed, .size = 1}};

    discord_edit_message(app.bot, playlist->channelId, playlist->messageId,
                         &params, NULL);
    free(footerText);
    free(loopState);
    return;
  }

  snprintf(footerText, 80, "Loop: %s\nPosition: %d of %ld\nVolume: %d%%",
           loopState, playlist->position + 1, playlist->size, playlist->volume);

  struct discord_embed_footer footer = {
      .text = footerText, .icon_url = NULL, .proxy_icon_url = NULL};

  uint32_t length = 840 + snprintf(NULL, 0, "%ld", track->trackInfo->length);
  char *description = (char *)malloc(length);

  if (!description) {
    log_error("Couldn't allocate memory for variable");
    free(footerText);
    free(loopState);
    return;
  }

  int totalSecs = track->trackInfo->length / 1000;
  int minutes = totalSecs / 60;
  int seconds = totalSecs % 60;

  snprintf(description, length,
           "Length: %02d:%02d\n\n> URL: [link](%s)\n> Ordered by: `%s`",
           minutes, seconds, track->trackInfo->uri, track->username);

  int titleLength =
      strlen(track->trackInfo->title) + strlen(track->trackInfo->author) + 4;
  char *title = (char *)malloc(titleLength);

  if (!title) {
    log_error("Couldn't allocate memory for variable");
    free(footerText);
    free(loopState);
    return;
  }

  snprintf(title, titleLength, "%s - %s", track->trackInfo->author,
           track->trackInfo->title);

  struct discord_embed embed = {.color = 0xa31eff,
                                .title = title,
                                .description = description,
                                .footer = &footer};

  if (playlist->messageId == 0) {
    struct discord_create_message params = {
        .embeds = &(struct discord_embeds){.array = &embed, .size = 1}};

    struct discord_message msg;

    struct discord_ret_message ret = {.sync = &msg};

    discord_create_message(app.bot, playlist->channelId, &params, &ret);
    playlist->messageId = msg.id;

    log_debug("Message ID: %ld", playlist->messageId);

    free(description);
    free(footerText);
    free(loopState);
    return;
  }

  struct discord_edit_message params = {
      .embeds = &(struct discord_embeds){.array = &embed, .size = 1}};

  discord_edit_message(app.bot, playlist->channelId, playlist->messageId,
                       &params, NULL);
  free(description);
  free(footerText);
  free(loopState);
  free(title);
}

void youtube(tnic_application app, const struct discord_interaction *event) {
  struct coglink_player *player =
      coglink_create_player(app.client, event->guild_id);
  if (!player) {
    tnic_sendErrorEmbed(app, event, "#e001x2", "Can't create coglink player");
    return;
  }

  struct coglink_user *user =
      coglink_get_user(app.client, event->member->user->id);
  if (!user) {
    tnic_sendErrorEmbed(app, event, "#e002",
                        "You are not connected to voice channel");
    return;
  }

  tnic_errnoReturn getTrackErrno =
      getTrackFromQuery(event->data->options->array[0].value, app.client,
                        player, event->member->user->username);

  if (getTrackErrno.Err == tnic_IS_NULL) {
    switch (getTrackErrno.additionalNumber) {
    case GET_TRACK_CURL_ERROR:
      tnic_sendErrorEmbed(
          app, event, "#e003x1",
          "Failed to process request. Report this issue to administrator");
      break;

    case GET_TRACK_ALLCATION_ERORR:
      tnic_sendErrorEmbed(
          app, event, "#eAx1",
          "Failed to process request. Report this issue to administrator");
      break;

    case GET_TRACK_CURL_ESCAPE_ERROR:
      tnic_sendErrorEmbed(
          app, event, "#e001x3",
          "Failed to process request. Report this issue to administrator");
      break;
    }

    return;
  }

  if (getTrackErrno.Err == tnic_SEARCH_FAILED) {
    tnic_sendErrorEmbed(app, event, "#e004", "Failed to load track");
    return;
  }

  tnic_track *track = (tnic_track *)getTrackErrno.Ok;

  if (app.playlistController->playlist == NULL) {
    coglink_join_voice_channel(app.client, app.bot, event->guild_id,
                               user->channel_id);
    app.playlistController->playlist = playlist_init(track);
    app.playlistController->playlist->currentTrack->lastPlayStartUnixTime =
        time(NULL);

    tnic_sendInfoEmbed(app, event, "Processing...");

    playTrack(app, player, track, event->guild_id);
    updateMessage(app, event->channel_id);
    return;
  }

  tnic_errnoReturn errno =
      playlist_addTrack(app.playlistController->playlist, track);
  updateMessage(app, event->channel_id);

  if (errno.Err == tnic_IS_NULL) {
    tnic_sendErrorEmbed(
        app, event, "#eAx1",
        "Failed to process request. Report this issue to administrator");
    return;
  }

  if (errno.additionalNumber == 1) {
    playTrack(app, player, track, event->guild_id);
  }

  tnic_sendInfoEmbed(app, event, "Processing...");
}

void disconnect(tnic_application app, const struct discord_interaction *event) {
  struct coglink_player *player =
      coglink_get_player(app.client, event->guild_id);

  if (!player) {
    tnic_sendErrorEmbed(app, event, "#e001x2",
                        "You are not playing music or not connected");
    return;
  }

  discord_delete_message(app.bot, app.playlistController->playlist->channelId,
                         app.playlistController->playlist->messageId, NULL,
                         NULL);
  app.playlistController->playlist->messageId = 0;
  app.playlistController->playlist->channelId = 0;
  coglink_destroy_player(app.client, player);
  coglink_remove_player(app.client, player);
  coglink_leave_voice_channel(app.client, app.bot, event->guild_id);
  playlist_clearPlaylist(app.playlistController->playlist);
  free(app.playlistController->playlist);
  app.playlistController->playlist = NULL;

  struct discord_interaction_response interactionRespParams = {
      .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
      .data = &(struct discord_interaction_callback_data){
          .content = "Disconnecting", .flags = DISCORD_MESSAGE_EPHEMERAL}};

  discord_create_interaction_response(app.bot, event->id, event->token,
                                      &interactionRespParams, NULL);
}

macro_testCommand() void testRepeat(tnic_application app,
                                    const struct discord_interaction *event) {
  switch (app.playlistController->playlist->currentState) {
  case PLAYLIST_NORMAL:
    app.playlistController->playlist->currentState = PLAYLIST_LOOP_PLAYLIST;
    tnic_sendInfoEmbed(app, event, "Repeating playlist");
    break;

  case PLAYLIST_LOOP_PLAYLIST:
    app.playlistController->playlist->currentState =
        PLAYLIST_REPEAT_SINGLE_TRACK;
    tnic_sendInfoEmbed(app, event, "Repeating single track");
    break;

  case PLAYLIST_REPEAT_SINGLE_TRACK:
    app.playlistController->playlist->currentState = PLAYLIST_NORMAL;
    tnic_sendInfoEmbed(app, event, "Disabling repeat");
    break;
  }

  updateMessage(app, event->channel_id);
}

macro_testCommand() void testPause(const tnic_application app,
                                   const struct discord_interaction *event) {
  struct coglink_player *player =
      coglink_get_player(app.client, event->guild_id);
  if (!player) {
    tnic_sendErrorEmbed(app, event, "#e001x2", "Can't create coglink player");
    return;
  }

  if (app.playlistController->playlist->isPaused) {
    struct coglink_update_player_params params = {
        .paused = COGLINK_PAUSED_STATE_FALSE};
    app.playlistController->playlist->isPaused = false;

    coglink_update_player(app.client, player, &params, NULL);
    playlist_updateUnpaused(app.playlistController->playlist);

    struct discord_interaction_response interactionRespParams = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data){
            .content = "Unpausing", .flags = DISCORD_MESSAGE_EPHEMERAL}};

    discord_create_interaction_response(app.bot, event->id, event->token,
                                        &interactionRespParams, NULL);
    return;
  }

  struct coglink_update_player_params params = {.paused =
                                                    COGLINK_PAUSED_STATE_TRUE};
  app.playlistController->playlist->isPaused = true;

  coglink_update_player(app.client, player, &params, NULL);

  struct discord_interaction_response interactionRespParams = {
      .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
      .data = &(struct discord_interaction_callback_data){
          .content = "Pausing", .flags = DISCORD_MESSAGE_EPHEMERAL}};

  playlist_updatePaused(app.playlistController->playlist);
  log_debug("Paused track position: %d",
            playlist_currentTrackPosition(app.playlistController->playlist));

  discord_create_interaction_response(app.bot, event->id, event->token,
                                      &interactionRespParams, NULL);
}

macro_testCommand() void testChangeTrack(
    tnic_application app, bool reverse,
    const struct discord_interaction *event) {
  tnic_errnoReturn errno =
      playlist_changeTrack(app.playlistController->playlist, reverse, true);

  if (errno.Err == tnic_PLAYLIST_END) {
    updateMessage(app, event->channel_id);
    playlist_clearPlaylist(app.playlistController->playlist);
    tnic_sendErrorEmbed(app, event, "the_end", "End of playlist");

    struct coglink_update_player_params params = {
        .paused = COGLINK_PAUSED_STATE_TRUE};

    coglink_update_player(app.client,
                          coglink_get_player(app.client, event->guild_id),
                          &params, NULL);
    return;
  }

  if (app.playlistController->playlist->tracks == NULL) {
    tnic_sendErrorEmbed(app, event, "#e005", "Not playing anything");
    return;
  }

  tnic_track *track = (tnic_track *)errno.Ok;

  struct coglink_update_player_params params = {
      .track = &(struct coglink_update_player_track_params){
          .encoded = track->encodedTrack}};

  updateMessage(app, event->channel_id);
  tnic_sendInfoEmbed(app, event, "Processing...");

  app.playlistController->playlist->trackEventChangeBlock = true;
  coglink_update_player(app.client,
                        coglink_get_player(app.client, event->guild_id),
                        &params, NULL);
}

void seek(tnic_application app, const struct discord_interaction *event) {
  int seek = strtol(event->data->options->array[0].value, NULL, 10);

  struct coglink_update_player_params params = {
      .position =
          (playlist_currentTrackPosition(app.playlistController->playlist) +
           seek) *
          1000};

  playlist_updateSeek(app.playlistController->playlist, seek, false);

  log_debug("Seek seconds: %d", params.position);

  coglink_update_player(app.client,
                        coglink_get_player(app.client, event->guild_id),
                        &params, NULL);

  struct discord_interaction_response interactionRespParams = {
      .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
      .data = &(struct discord_interaction_callback_data){
          .content = "Seeking", .flags = DISCORD_MESSAGE_EPHEMERAL}};

  discord_create_interaction_response(app.bot, event->id, event->token,
                                      &interactionRespParams, NULL);
}

// Public functions
void tnic_proccessApplicationCommand(tnic_application app,
                                     const struct discord_interaction *event) {
  if (strcmp(event->data->name, "youtube") == 0) {
    youtube(app, event);
    return;
  }

  if (app.playlistController->playlist == NULL) {
    tnic_sendErrorEmbed(app, event, "#e002a",
                        "Bot is not connected to the voice channel");
    return;
  }

  if (strcmp(event->data->name, "disconnect") == 0) {
    disconnect(app, event);
    return;
  }

  if (strcmp(event->data->name, "toggle_pause") == 0) {
    testPause(app, event);
    return;
  }

  if (strcmp(event->data->name, "toggle_repeat") == 0) {
    testRepeat(app, event);
    return;
  }

  if (strcmp(event->data->name, "next") == 0) {
    testChangeTrack(app, false, event);
    return;
  }

  if (strcmp(event->data->name, "prev") == 0) {
    testChangeTrack(app, true, event);
    return;
  }

  if (strcmp(event->data->name, "seek") == 0) {
    seek(app, event);
    return;
  }
}

void tnic_registerMusicCommands(struct discord *bot,
                                const struct discord_ready *event) {
  struct discord_application_command_option youtubeCommandOption = {
      .type = DISCORD_APPLICATION_OPTION_STRING,
      .name = "query",
      .description = "Your youtube query",
      .required = true};

  struct discord_create_global_application_command youtubeCommand = {
      .name = "youtube",
      .description = "Play youtube music",
      .options =
          &(struct discord_application_command_options){
              .size = 1,
              .array = &youtubeCommandOption,
          },
  };

  struct discord_application_command_option seekCommandOption = {
      .type = DISCORD_APPLICATION_OPTION_INTEGER,
      .name = "seek",
      .description = "Seconds to seek",
      .required = true};

  struct discord_create_global_application_command seekCommand = {
      .name = "seek",
      .description = "Seek current track",
      .options =
          &(struct discord_application_command_options){
              .size = 1,
              .array = &seekCommandOption,
          },
  };

  struct discord_create_global_application_command disconnectCommand = {
      .name = "disconnect", .description = "Connection to voice channel test"};

  struct discord_create_global_application_command pauseCommand = {
      .name = "toggle_pause", .description = "Toggle pause for current track"};

  struct discord_create_global_application_command repeatCommand = {
      .name = "toggle_repeat", .description = "Toggle pause for current track"};

  struct discord_create_global_application_command nextCommand = {
      .name = "next", .description = "Skip this track"};

  struct discord_create_global_application_command prevCommand = {
      .name = "prev", .description = "Play previous track"};

  discord_create_global_application_command(bot, event->application->id,
                                            &nextCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &prevCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &seekCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &pauseCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &repeatCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &youtubeCommand, NULL);
  discord_create_global_application_command(bot, event->application->id,
                                            &disconnectCommand, NULL);
}

// Events
macro_testCommand() void tnic_cmusicProcessEvent(
    tnic_application app, struct coglink_client *c_client,
    struct coglink_node *node, struct coglink_track_end *trackEnd) {
  if (trackEnd->reason == COGLINK_TRACK_END_REASON_FINISHED &&
      app.playlistController->playlist->trackEventChangeBlock) {
    app.playlistController->playlist->trackEventChangeBlock = false;
    return;
  }

  if (trackEnd->reason == COGLINK_TRACK_END_REASON_FINISHED) {
    tnic_errnoReturn errno =
        playlist_changeTrack(app.playlistController->playlist, false, false);

    if (errno.Err == tnic_PLAYLIST_END) {
      playlist_clearPlaylist(app.playlistController->playlist);
      updateMessage(app, 0);
      return;
    }

    tnic_track *track = (tnic_track *)errno.Ok;
    updateMessage(app, 0);

    playTrack(app, coglink_get_player(app.client, trackEnd->guildId), track,
              trackEnd->guildId);
  }
}
