#ifndef __TNIC_PLAYLIST_H
#define __TNIC_PLAYLIST_H

#include <concord/discord.h>
#include <coglink/lavalink.h>

typedef struct {
    char *username;
    char *encodedTrack;
    struct coglink_track_info *trackInfo;
    struct coglink_load_tracks *response;
} tnic_track;

#endif