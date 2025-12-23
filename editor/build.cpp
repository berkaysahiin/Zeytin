#include "../Entry/entry/entry.hpp" // IWYU pragma: keep

static constexpr std::string_view DEFAULT_CONFIG = "Debug";

int main() {
	Entry::Platform platform = Entry::GetPlatform();

	ENTRY_MAYBE_UNUSED
	std::string_view config = Entry::GetOptionStringOr("config", DEFAULT_CONFIG);

	Entry::Target editor;
	editor.name = "ZeytinEditor";

	editor.includeDirs = {
		"include",
        "3rdparty",
        "3rdparty/imgui",
        "3rdparty/zmq",
        "3rdparty/rapidjson",
        "3rdparty/raylib",
        "3rdparty/imgui_test_engine",
	};

	editor.libraryDirs = {
		"3rdparty/raylib/lib/linux",
		"3rdparty/zmq/linux",
	};

	editor.libraries = { 
		"raylib",
		"m",
		"pthread",
		"dl",
		"rt",
		"X11",
		"asound",
		"zmq"
	};

	// TODO: remove this ?
	editor.AppendSource("3rdparty/backward-cpp/backward.cpp");

	// Actual sources
	editor.AppendSourceRange(Entry::CollectSourcesRecursive("source"));

	editor.AppendSourceRange(Entry::CollectSourcesRecursive("3rdparty/imgui"));
	editor.AppendSourceRange(Entry::CollectSourcesRecursive("3rdparty/imgui_test_engine"));
	editor.AppendSourceRange(Entry::CollectSourcesRecursive("3rdparty/rlimgui"));

	editor.AppendFlagRange({
		"-std=c++17",
		"-w",
		"-DDEBUG=1",
		"-DIMGUI_ENABLE_TEST_ENGINE",
		"-DIMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1"
	});

	// Exporting compile commands is intentionally done before building the actual project
	// Works better with incremental builds, as we may export the command even if we skip compiling the file
	Entry::ExportCompileCommands(editor);

	return Entry::Build(editor);
}
