#ifdef QTQUICKCOMMONLIB_USE_DLL
#ifdef QTQUICKCOMMONLIB_BUILDING_PROJECT
#define QTQUICKCOMMONLIB_API __declspec(dllexport)
#else
#define QTQUICKCOMMONLIB_API __declspec(dllimport)
#endif
#else
#define QTQUICKCOMMONLIB_API
#endif