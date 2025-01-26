#ifndef __TNIC_ERROR_H
#define __TNIC_ERROR_H

enum tnic_errorTypes {
    tnic_OK,
    tnic_VALUE_NOT_FOUND
};

typedef struct {
    enum tnic_errorTypes errno;
    void *data;
} tnic_errnoReturn;

#endif