#include "playlist.h"

macro_testCommand()
// Make check if malloc fails
bool playlist_addTrack(tnic_playlist *playlist, tnic_track *track) {
    tnic_track **newTracksArr = (tnic_track**) malloc(sizeof(tnic_track*) * (playlist->size + 1));

    /* if (!newTracksArr) {
        return (tnic_errnoReturn) { .Err = tnic_IS_NULL, .Ok = NULL };
    } */

    if (playlist->size == 0) {
        newTracksArr[0] = track;
        playlist->tracks = newTracksArr;
        playlist->currentTrack = track;
        playlist->isPaused = false;
        playlist->position = 0;
        playlist->size = 1;
        
        //return (tnic_errnoReturn) { .Err = tnic_OK, .Ok = (void *)(uintptr_t)true };
        return true;
    }

    for (size_t i = 0; i < playlist->size; i++) {
        newTracksArr[i] = playlist->tracks[i];
    }
    free(playlist->tracks);
    playlist->tracks = newTracksArr;

    playlist->size += 1;
    playlist->tracks[playlist->size - 1] = track;
    // return (tnic_errnoReturn) { .Err = tnic_OK, .Ok = (void *)(uintptr_t)false };
    return false;
}

void playlist_changeState(tnic_playlist *playlist, const enum tnic_playlistStates state) {

}

void playlist_clearPlaylist(tnic_playlist *playlist) {
    for (int i = 0; i < playlist->size; i++) {
        coglink_free_load_tracks(playlist->tracks[i]->response);
        free(playlist->tracks[i]->response);
        free(playlist->tracks[i]);
    }

    playlist->size = 0;
    playlist->position = 0;
    playlist->currentTrack = NULL;
    playlist->currentState = PLAYLIST_NORMAL;

    free(playlist->tracks);
}

tnic_errnoReturn playlist_getTrack(tnic_playlist *playlist, const uint32_t position) {
    tnic_errnoReturn errno;
    errno.additionalNumber = 0;

    if (position + 1 > playlist->size) {
        errno.Err = tnic_OUT_OF_RANGE;
        errno.Ok = NULL;
        return errno;
    }

    errno.Ok = playlist->tracks[position];
    errno.Err = tnic_OK;
    return errno;
}

tnic_errnoReturn playlist_changeTrack(tnic_playlist *playlist, const bool reverse, bool force) {
    enum tnic_playlistStates localStateCopy = playlist->currentState;

    // Checking if we want to force changing track (for repeating single track)
    if (force && playlist->currentState == PLAYLIST_REPEAT_SINGLE_TRACK) {
        localStateCopy = PLAYLIST_LOOP_PLAYLIST;
    }
    
    // If we are repeating single track, we'll just return this track
    if (localStateCopy == PLAYLIST_REPEAT_SINGLE_TRACK)
        return (tnic_errnoReturn) { .Err = tnic_OK, .Ok = playlist->currentTrack };

    if (reverse && playlist->position == 1) {                          // If we are getting previous track 
        playlist->position = playlist->size - 1;                       // and we are at the beginning
    } else if (!reverse && playlist->position + 1 == playlist->size && 
               localStateCopy == PLAYLIST_LOOP_PLAYLIST) {             // If we are at the end of the playlist
        playlist->position = 0;                                        // and state is set to LOOP_PLAYLIST
    } else {
        if (playlist->position + 1 == playlist->size) {                // If we are at the end of the playlist
            playlist_clearPlaylist(playlist);
            return (tnic_errnoReturn) { .Err = tnic_PLAYLIST_END, .Ok = NULL };
        }

        // We are not at the end, we are not repeating single track
        // so we can return next track
        playlist->position += 1;
    }

    return playlist_getTrack(playlist, playlist->position);
}

tnic_playlist* playlist_init(tnic_track *track) {
    tnic_playlist *playlist = (tnic_playlist*)malloc(sizeof(tnic_playlist));
    playlist->tracks = malloc(sizeof(tnic_track*));

    playlist->currentTrack = track;
    playlist->tracks[0] = track;
    playlist->position = 0;
    playlist->size = 1;

    return playlist;
}