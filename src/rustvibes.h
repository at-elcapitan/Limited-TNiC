#ifndef __TNIC_RUST_VIBES_H
#define __TNIC_RUST_VIBES_H

#define macro_todo() \
    _Pragma("GCC error \"Function not implemented!\"")

#ifdef DEBUG
    #define macro_testCommand()
#else
    #define macro_testCommand() \
        _Pragma("GCC error \"Test function cannot be compiled for release version\"")
#endif

enum tnic_errorTypes {
    tnic_OK,
    tnic_VALUE_NOT_FOUND,
    tnic_IS_NULL,
    tnic_SEARCH_FAILED,
    tnic_FAILED,
    tnic_OUT_OF_RANGE,
    tnic_PLAYLIST_END
};

typedef struct {
    enum tnic_errorTypes Err;
    int additionalNumber;
    void *Ok;
} tnic_errnoReturn;

#endif