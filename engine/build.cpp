#include "../Entry/entry/entry.hpp" // IWYU pragma: keep

static constexpr std::string_view DEFAULT_CONFIG = "weditor";

int main(int argc, char* argv[]) {
	Entry::Parse(argc, argv);

	Entry::Platform platform = Entry::GetPlatform();
	std::string_view config = Entry::GetOptionStringOr("config", DEFAULT_CONFIG);

	Entry::Target engine;

	engine.name = ENTRY_FORMAT("ZeytinEngine_{}", config.data());

	engine.includeDirs = {
		"include",
        "3rdparty",
        "3rdparty/zmq",
        "3rdparty/raylib",
        "3rdparty/rttr",
        "3rdparty/tracy",
	};

	engine.libraryDirs = {
		"3rdparty/raylib/lib/linux",
		"3rdparty/rttr/lib/linux",
		"3rdparty/zmq/linux", // Only needed on EDITOR_MODE, no harm pointing
	};

	engine.libraries = { 
		"raylib",
		"m",
		"pthread",
		"dl",
		"rt",
		"X11",
		"asound",
		"rttr_core",
	};

	engine.flags = {
		//"-Wall",
		//"-Wextra",
		//"-Werror",
		"-std=c++17",
    };

	if(config == "weditor") {
		engine.AppendLib("zmq"); // Speak to editor

		engine.AppendFlag("-g"); // symbols
		engine.AppendFlag("-O0"); // no optimization
		engine.AppendFlag("-DDEBUG=1"); 
		engine.AppendFlag("-DEDITOR_MODE=1"); 

		engine.AppendFlag("-DTRACY_ENABLE=1"); 
		engine.AppendSource("3rdparty/tracy/TracyClient.cpp");
	}
	else if(config == "standalone") {
		engine.AppendFlag("-O3"); // full optimization
		engine.AppendFlag("-DSTANDALONE=1"); // full optimization
		engine.AppendFlag("-DEMBED_SCENE=1"); // Embed scene json as global variable
	}

	engine.AppendSourceRange(Entry::CollectSourcesRecursive("source"));

	// Exporting compile commands is intentionally done before building the actual project
	// Works better with incremental builds, as we may export the command even if we skip compiling the file
	Entry::ExportCompileCommands(engine);
	return Entry::Build(engine);
}
