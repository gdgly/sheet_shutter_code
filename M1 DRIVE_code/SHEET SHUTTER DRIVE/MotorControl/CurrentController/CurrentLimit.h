
#ifndef CURRENT_LIMIT_H
#define CURRENT_LIMIT_H
#include "./Common/Typedefs/Typedefs.h"

VOID initCurrentLimitPI(VOID);
VOID runCurrentLimitPI(VOID);
VOID checkSustainedOvercurrent(VOID);

#endif
