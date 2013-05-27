#ifndef ipod_common_h
#define ipod_common_h

#define IPOD_RECONNECT_KEY 0xFEFF
#define IPOD_REQUEST_LIBRARY_KEY 0xFEFE
#define IPOD_REQUEST_OFFSET_KEY 0xFEFB
#define IPOD_LIBRARY_RESPONSE_KEY 0xFEFD
#define IPOD_NOW_PLAYING_KEY 0xFEFA
#define IPOD_REQUEST_PARENT_KEY 0xFEF9
#define IPOD_PLAY_TRACK_KEY 0xFEF8
#define IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY 0xFEF7
#define IPOD_ALBUM_ART_KEY 0xFEF6
#define IPOD_STATE_CHANGE_KEY 0xFEF5

typedef enum {
    MPMediaGroupingTitle,
    MPMediaGroupingAlbum,
    MPMediaGroupingArtist,
    MPMediaGroupingAlbumArtist,
    MPMediaGroupingComposer,
    MPMediaGroupingGenre,
    MPMediaGroupingPlaylist,
    MPMediaGroupingPodcastTitle,
} MPMediaGrouping;

typedef enum {
    NowPlayingTitle,
    NowPlayingArtist,
    NowPlayingAlbum,
    NowPlayingTitleArtist
} NowPlayingType;

#endif
