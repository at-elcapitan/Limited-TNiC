#ifndef __TNIC_PLAYLIST_H
#define __TNIC_PLAYLIST_H

#include <time.h>
#include <stdlib.h>
#include <concord/discord.h>
#include <coglink/lavalink.h>

#include "map/map.h"
#include "rustvibes.h"

typedef struct {
	char *username;
	char *encodedTrack;

	int playingTimeDelta;	   // Time since last playing before pausing
	int lastPlayStartUnixTime; // Timestamp since start playing or last unpausing

	struct coglink_track_info *trackInfo;
	struct coglink_load_tracks *response;
} tnic_track;

enum tnic_playlistStates {
	PLAYLIST_NORMAL,
	PLAYLIST_LOOP_PLAYLIST,
	PLAYLIST_REPEAT_SINGLE_TRACK
};

typedef struct {
	bool isPaused;
	enum tnic_playlistStates currentState;

	int volume;
	size_t size;
	uint32_t position;

	u64snowflake channelId;
	u64snowflake messageId;

	tnic_track **tracks;
	tnic_track *currentTrack;
} tnic_playlist;

typedef struct {
	tnic_playlist *playlist;
} tnic_playlist_controller;

void playlist_clearPlaylist(tnic_playlist *playlist);
tnic_errnoReturn playlist_addTrack(tnic_playlist *playlist, tnic_track *track);
tnic_errnoReturn playlist_getTrack(tnic_playlist *playlist, const uint32_t position);
tnic_errnoReturn playlist_changeTrack(
	tnic_playlist *playlist, const bool reverse, bool force);
tnic_playlist *playlist_init(tnic_track *track);
void playlist_updatePaused(tnic_playlist *playlist);
void playlist_updateUnpaused(tnic_playlist *playlist);
int playlist_currentTrackPosition(tnic_playlist *playlist);
void playlist_updateSeek(tnic_playlist *playlist, int seekTime, bool reset);

#endif
