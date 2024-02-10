#include "filesystem"
#include <format>
#include <fstream>
#include <print>
#include <sstream>
#include <string>


int main(int argc, char *argv[]) {
	if (argc != 5) return 1;
	std::string compiledDir = argv[1];
	std::string shadersDir = argv[2];
	std::string fileName = argv[3];
	std::string outputDir = argv[4];

	const std::string newName = std::string(fileName).replace(fileName.find('.'), 1, "");

    const auto shaderFileComplete = std::format("{}/{}", shadersDir, fileName);
	const auto compiledFileComplete = std::format("{}/{}.spv", compiledDir, fileName);
	const auto headerFileComplete = std::format("{}/{}.hpp", outputDir, newName);

	// Check if the header file is already created and its last modified time
	// Rewriting the header file when it is newer than the shader file would cause a useless rebuild
	if (std::filesystem::exists(headerFileComplete)) {
		auto lastTimeShader = std::filesystem::last_write_time(shaderFileComplete);
		auto lastTimeHeader = std::filesystem::last_write_time(headerFileComplete);

		if (lastTimeHeader > lastTimeShader) return 0;
	}

	std::ifstream file{compiledFileComplete, file.binary | file.in};
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	const std::string shaderContents = buffer.str();


	std::stringstream output;
	output << "#pragma once\n";
	output << "#include <array>\n\n";
	output << "namespace Engine {\n";
	output << "\tnamespace Shaders {\n";
	output << "\t\tconstexpr std::array<char, " << shaderContents.size() << "> " << newName << " = {";
	for (const char &c: shaderContents) {
		output << std::format("{:#x},", c);
	}
	output << "};\n";
	output << "\t}\n";
	output << "}";

	std::ofstream outputFile{headerFileComplete, outputFile.out};
	outputFile << output.str();
	outputFile.close();
}