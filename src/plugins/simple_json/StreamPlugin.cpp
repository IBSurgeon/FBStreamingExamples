#ifdef _WINDOWS
#include <windows.h>
#endif
#include "SimpleJsonPlugin.h"

using namespace Firebird;

extern "C"
{
	FB_DLL_EXPORT void fb_stream_plugin(IMaster* master, IStreamPluginManager* pm)
	{
		auto factory = new SimpleJsonPlugin::SimpleJsonPluginFactory(master);
		pm->registerPluginFactory("SimpleJsonPlugin", factory);
	}
}
