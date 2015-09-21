#ifndef ISOLINECREATORCONFIG_H
#define ISOLINECREATORCONFIG_H

#ifdef WIN32
#ifdef  ISOLINECREATOR_EXPORTS
#define EXPORT_CLASS   __declspec(dllexport)
#define EXPORT_API  extern "C" __declspec(dllexport)
#else
#define EXPORT_CLASS   __declspec(dllimport )
#define EXPORT_API  extern "C" __declspec(dllimport )
#endif
#else
#define EXPORT_CLASS
#define EXPORT_API
#endif

#endif