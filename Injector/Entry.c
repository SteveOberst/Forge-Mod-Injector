#include <stdio.h>
#include "Injector.h"
#include "argparse.h"
#include "Filesystem.h"

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
		OPT_INTEGER('t', "target-pid", &params.dwProcId, "Process ID to inject into"),
		OPT_STRING('p', "payload", &params.payload_file_path, "Path to the payload file"),
		OPT_STRING('m', "mod-file", &params.mod_file, "Path to the mod file you want to inject"),
		OPT_STRING('c', "main-class", &payload_params.main_class, "Name of the mods main class"),
		OPT_END()
	};

	struct argparse argparse;
	argparse_init(&argparse, options, usages, 0);
	argparse_parse(&argparse, argc, (const char**)argv);


	if ((!params.target_window && !params.dwProcId) || !params.payload_file_path || !params.mod_file || !payload_params.main_class)
	{
		argparse_usage(&argparse);
		return 0;
	}

	if (!is_absolute_path(params.payload_file_path))
	{
		params.payload_file_path = make_absolute_path(params.payload_file_path);
	}

	if (!is_absolute_path(params.mod_file))
	{
		params.mod_file = make_absolute_path(params.mod_file);
	}

	params.payload_module_name = malloc(MAX_PATH);
	if (!params.payload_module_name)
	{
		PRINT_LAST_ERROR("Failed to allocate memory for payload module name.");
		return 0;
	}

	get_file_name(params.payload_file_path, params.payload_module_name, MAX_PATH);
	Inject(&params, &payload_params);
	free(params.payload_module_name);
	return 0;
}
