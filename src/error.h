#ifndef __TNIC_ERROR_H
#define __TNIC_ERROR_H

enum tnic_errorTypes {
    tnic_OK
};

typedef struct {
    enum tnic_errorTypes errno;
    void *data;
} tnic_errnoReturn;

#endif