#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cctype>

#include "cache/SplatCacheWriter.h"
#include "core/PathUtils.h"
#include "import/PlySceneImporter.h"

namespace
{
	std::string trim_copy(std::string value)
	{
		const auto is_space = [](unsigned char c)
		{
			return std::isspace(c) != 0;
		};

		auto first = value.begin();
		auto last = value.end();
		while (first != last && is_space(static_cast<unsigned char>(*first)))
		{
			++first;
		}
		while (first != last && is_space(static_cast<unsigned char>(*(last - 1))))
		{
			--last;
		}
		value = std::string(first, last);

		if (value.size() >= 3 &&
			static_cast<unsigned char>(value[0]) == 0xEF &&
			static_cast<unsigned char>(value[1]) == 0xBB &&
			static_cast<unsigned char>(value[2]) == 0xBF)
		{
			value.erase(0, 3);
		}

		return value;
	}

	bool read_first_config_line(const std::filesystem::path& txt_path, std::string& out_line)
	{
		std::ifstream stream(txt_path);
		if (!stream.is_open())
		{
			return false;
		}

		std::string line;
		while (std::getline(stream, line))
		{
			line = trim_copy(line);
			if (!line.empty() && line[0] != '#')
			{
				out_line = line;
				return true;
			}
		}

		return false;
	}

	std::filesystem::path make_default_output_path(const std::filesystem::path& ply_path)
	{
		return ply_path.parent_path() / (gs::pathToUtf8(ply_path.stem()) + ".gsplatcache");
	}

	std::filesystem::path resolve_output_path(
		const std::filesystem::path& resolved_input,
		const std::filesystem::path& requested_output,
		bool has_requested_output)
	{
		const std::filesystem::path source_directory = resolved_input.parent_path();
		const std::string default_filename = gs::pathToUtf8(resolved_input.stem()) + ".gsplatcache";
		if (!has_requested_output)
		{
			return source_directory / default_filename;
		}

		std::error_code ec;
		const bool existing_directory = std::filesystem::is_directory(requested_output, ec);
		if (existing_directory || requested_output.filename().empty())
		{
			std::cerr << "Cache output directory is fixed to the source PLY directory: "
				<< gs::pathToUtf8(source_directory) << "\n";
			return source_directory / default_filename;
		}

		std::filesystem::path output_filename = requested_output.filename();
		if (requested_output.has_parent_path())
		{
			std::cerr << "Ignoring requested output directory and writing cache files next to source PLY: "
				<< gs::pathToUtf8(source_directory) << "\n";
		}

		if (!output_filename.has_extension())
		{
			output_filename.replace_extension(".gsplatcache");
			if (output_filename == ".gsplatcache")
			{
				output_filename = default_filename;
			}
			return source_directory / output_filename;
		}

		if (output_filename.extension() != ".gsplatcache")
		{
			std::filesystem::path normalized = output_filename;
			normalized.replace_extension(".gsplatcache");
			std::cerr << "Output filename normalized to .gsplatcache: " << gs::pathToUtf8(normalized) << "\n";
			return source_directory / normalized;
		}

		return source_directory / output_filename;
	}

	int run_builder(
		const std::filesystem::path& input_path,
		const std::filesystem::path& requested_output,
		bool has_requested_output,
		const gs::SplatCacheWriteOptions& options)
	{
		std::filesystem::path resolved_input = input_path;
		if (resolved_input.extension() == ".txt")
		{
			std::string line;
			if (!read_first_config_line(resolved_input, line))
			{
				std::cerr << "Failed to read PLY path from txt: " << gs::pathToUtf8(resolved_input) << "\n";
				return 2;
			}

			resolved_input = gs::pathFromText(line);
			if (resolved_input.is_relative())
			{
				resolved_input = (input_path.parent_path() / resolved_input).lexically_normal();
			}
		}

		if (resolved_input.extension() != ".ply")
		{
			std::cerr << "Builder expects input .ply file, got: " << gs::pathToUtf8(resolved_input) << "\n";
			return 2;
		}
		if (!std::filesystem::exists(resolved_input))
		{
			std::cerr << "Input PLY does not exist: " << gs::pathToUtf8(resolved_input) << "\n";
			return 2;
		}

		const std::filesystem::path output_path = resolve_output_path(resolved_input, requested_output, has_requested_output);
		const std::filesystem::path output_parent = output_path.parent_path();
		if (!output_parent.empty())
		{
			std::error_code ec;
			std::filesystem::create_directories(output_parent, ec);
			if (ec)
			{
				std::cerr << "Failed to prepare output directory: " << gs::pathToUtf8(output_parent) << " (" << ec.message() << ")\n";
				return 3;
			}
		}
		if (std::filesystem::exists(output_path))
		{
			std::cerr << "Warning: overwriting existing cache file: " << gs::pathToUtf8(output_path) << "\n";
		}

		gs::GaussianScene scene;
		if (!gs::PlySceneImporter::import_scene(resolved_input, scene))
		{
			std::cerr << "Failed to import input scene: " << gs::pathToUtf8(resolved_input) << "\n";
			return 2;
		}

		if (!gs::SplatCacheWriter::write_cache_bundle(scene, output_path, options))
		{
			std::cerr << "Failed to write cache file: " << gs::pathToUtf8(output_path) << "\n";
			return 3;
		}

		std::cout << "Cache written: " << gs::pathToUtf8(output_path) << "\n";
		std::cout << "Splats: " << scene.splats.size() << ", SH degree: " << scene.max_sh_degree << "\n";
		std::cout << "Chunk payload: " << (options.include_chunk ? "enabled" : "disabled") << "\n";
		return 0;
	}

} // namespace

int main(int argc, char** argv)
{
	gs::SplatCacheWriteOptions options{};
	std::vector<std::string> positional_args;
	positional_args.reserve(static_cast<std::size_t>(std::max(argc - 1, 0)));
	for (int index = 1; index < argc; ++index)
	{
		const std::string arg = argv[index];
		if (arg == "--no-chunk")
		{
			options.include_chunk = false;
			continue;
		}
		if (arg == "--chunk")
		{
			options.include_chunk = true;
			continue;
		}
		positional_args.push_back(arg);
	}

	if (positional_args.empty() || positional_args.size() > 2)
	{
		std::cerr << "Usage: gs_cache_builder [--chunk|--no-chunk] <input.ply|input.txt> [output_filename.gsplatcache]\n";
		std::cerr << "All cache files are written next to the source PLY. Optional output only controls the manifest filename.\n";
		return 1;
	}

	const std::filesystem::path input_path = gs::pathFromText(positional_args[0]);
	const bool has_output = positional_args.size() >= 2;
	const std::filesystem::path output_path = has_output ? gs::pathFromText(positional_args[1]) : std::filesystem::path{};
	return run_builder(input_path, output_path, has_output, options);
}
