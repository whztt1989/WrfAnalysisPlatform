#ifndef WRFCOMMON_GLOBAL_H
#define WRFCOMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef WRFCOMMON_LIB
# define WRFCOMMON_EXPORT Q_DECL_EXPORT
#else
# define WRFCOMMON_EXPORT Q_DECL_IMPORT
#endif

#endif // WRFCOMMON_GLOBAL_H
