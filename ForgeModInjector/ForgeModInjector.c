#include "ForgeModInjector.h"
#include "Injector.h"
#include "argparse.h"

static const char* const usages[] = {
	"basic [options] [[--] args]",
	"basic [options]",
	NULL,
};

int main(int argc, char** argv)
{
	INJECT_PARAMS params={0};
	struct argparse_option options[] = {
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_STRING('w', "window", &params.target_window, "Window name to inject into"),
		OPT_STRING('e', "exec", &params.target_process_executable, "Executable name of the process to inject to (only use this if there is exclusively one process of that executable)"),
		OPT_INTEGER('p', "pid", &params.dwProcId, "Process ID to inject into"),
		OPT_END()
	};
	struct argparse argparse;
	argparse_init(&argparse, options, usages, 0);

	// test 
	params.target_window = "Minecraft 1.8.9";
	// C:\\Users\\baumg\\source\\repos\\ForgeModInjector\\out\\build\\x64 - debug\\Payload\\Payload.dll
	params.payload_file_path = "C:\\Users\\baumg\\Payload.dll";
	params.payload_module_name = "Payload.dll";

	Inject(&params);
	system("pause");
	return 0;
}
