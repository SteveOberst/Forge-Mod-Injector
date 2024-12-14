#include "Entry.h"
#include "Injector.h"
#include "argparse.h"

static const char* const usages[] = {
	"basic [options] [[--] args]",
	"basic [options]",
	NULL,
};

int main(int argc, char** argv)
{
	INJECT_PARAMS params={ 0 };
	PAYLOAD_PARAMS payload_params = { 0 };
	struct argparse_option options[] = {
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_STRING('w', "target-window", &params.target_window, "Window name to inject into"),
		OPT_INTEGER('p', "target-pid", &params.dwProcId, "Process ID to inject into"),
		OPT_STRING('f', "file", &params.payload_file_path, "Path to the payload file"),
		OPT_STRING('c', "main-class", &payload_params.main_class, "Name of the payload module"),
		OPT_STRING('m', "mod-file", &params.mod_file, "Path to the mod file you want to inject"),
		OPT_END()
	};

	struct argparse argparse;
	argparse_init(&argparse, options, usages, 0);

	/*if (!argparse_parse(&argparse, argc, (const char**)argv))
	{
		argparse_usage(&argparse);
		return 1;
	}*/

	/*if (!params.dwProcId && !params.target_window)
	{
		PRINT_LAST_ERROR("No process id or window name provided.");
		return 1;
	}

	if (!payload_params.main_class)
	{
		PRINT_LAST_ERROR("No main class provided.");
		return 1;
	}*/

	// test Minecraft 1.8.9 (SkyblockDLC v0.17.0-beta)
	//params.target_window = "Minecraft 1.8.9 (SkyblockDLC v0.17.0-beta)";
	params.target_window = "Minecraft 1.8.9";
	// C:\\Users\\baumg\\source\\repos\\ForgeModInjector\\out\\build\\x64 - debug\\Payload\\Payload.dll
	params.payload_file_path = "C:\\Users\\baumg\\Payload.dll";
	params.payload_module_name = "Payload.dll";
	params.mod_file = "C:\\Users\\baumg\\source\\repos\\ForgeModInjector\\Mod\\build\\libs\\InjectedMod-0.1.1-alpha.jar";
	payload_params.main_class = "net.sxlver.injectedmod.InjectedMod";

	Inject(&params, &payload_params);
	system("pause");
	return 0;
}
