#include "io/PlyLoader.h"

#include "core/PathUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/geometric.hpp>

namespace gs
{
	namespace
	{

		struct PlyProperty
		{
			std::string type;
			std::string name;
		};

		struct HeaderInfo
		{
			bool binaryLittleEndian{ false };
			std::size_t vertexCount{ 0 };
			std::vector<PlyProperty> properties;
		};

		std::size_t typeSize(const std::string& type)
		{
			if (type == "char" || type == "uchar" || type == "int8" || type == "uint8")
			{
				return 1;
			}
			if (type == "short" || type == "ushort" || type == "int16" || type == "uint16")
			{
				return 2;
			}
			if (type == "int" || type == "uint" || type == "float" || type == "int32" || type == "uint32" || type == "float32")
			{
				return 4;
			}
			if (type == "double" || type == "float64")
			{
				return 8;
			}
			return 0;
		}

		float readAsFloatBinary(std::ifstream& ifs, const std::string& type)
		{
			if (type == "float" || type == "float32")
			{
				float v = 0.0f;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(float));
				return v;
			}
			if (type == "double" || type == "float64")
			{
				double v = 0.0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(double));
				return static_cast<float>(v);
			}
			if (type == "uchar" || type == "uint8")
			{
				std::uint8_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}
			if (type == "char" || type == "int8")
			{
				std::int8_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}
			if (type == "ushort" || type == "uint16")
			{
				std::uint16_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}
			if (type == "short" || type == "int16")
			{
				std::int16_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}
			if (type == "uint" || type == "uint32")
			{
				std::uint32_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}
			if (type == "int" || type == "int32")
			{
				std::int32_t v = 0;
				ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
				return static_cast<float>(v);
			}

			return 0.0f;
		}

		bool parseHeader(std::ifstream& ifs, HeaderInfo& header)
		{
			std::string line;
			if (!std::getline(ifs, line) || line != "ply")
			{
				std::cerr << "Not a PLY file\n";
				return false;
			}

			bool inVertexElement = false;
			while (std::getline(ifs, line))
			{
				if (line == "end_header")
				{
					return true;
				}

				std::istringstream iss(line);
				std::string token;
				iss >> token;
				if (token == "format")
				{
					std::string fmt;
					iss >> fmt;
					header.binaryLittleEndian = (fmt == "binary_little_endian");
				}
				else if (token == "element")
				{
					std::string name;
					std::size_t count = 0;
					iss >> name >> count;
					inVertexElement = (name == "vertex");
					if (inVertexElement)
					{
						header.vertexCount = count;
					}
				}
				else if (token == "property" && inVertexElement)
				{
					std::string type;
					std::string name;
					iss >> type >> name;
					if (type == "list")
					{
						std::cerr << "List property in vertex element is not supported\n";
						return false;
					}
					header.properties.push_back({ type, name });
				}
			}

			return false;
		}

		glm::vec3 sh0ToColor(const glm::vec3& dc)
		{
			constexpr float kC0 = 0.2820947918f;
			glm::vec3 color = dc * kC0 + glm::vec3(0.5f);
			color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
			return color;
		}

	} // namespace

	bool PlyLoader::loadGaussianPly(const std::filesystem::path& path, GaussianModel& outModel)
	{
		outModel.splats.clear();

		std::ifstream ifs(path, std::ios::binary);
		if (!ifs.is_open())
		{
			std::cerr << "Failed to open PLY: " << pathToUtf8(path) << '\n';
			return false;
		}

		HeaderInfo header;
		if (!parseHeader(ifs, header))
		{
			std::cerr << "Failed to parse PLY header\n";
			return false;
		}

		if (header.vertexCount == 0 || header.properties.empty())
		{
			std::cerr << "PLY has no vertex data\n";
			return false;
		}

		std::unordered_map<std::string, std::size_t> propIndex;
		propIndex.reserve(header.properties.size());
		for (std::size_t i = 0; i < header.properties.size(); ++i)
		{
			propIndex[header.properties[i].name] = i;
		}

		const auto hasProp = [&propIndex](const std::string& n)
		{
			return propIndex.find(n) != propIndex.end();
		};

		const bool basicOk = hasProp("x") && hasProp("y") && hasProp("z") && hasProp("opacity") &&
			hasProp("scale_0") && hasProp("scale_1") && hasProp("scale_2") &&
			hasProp("rot_0") && hasProp("rot_1") && hasProp("rot_2") && hasProp("rot_3");
		if (!basicOk)
		{
			std::cerr << "PLY missing required 3DGS properties\n";
			return false;
		}

		outModel.splats.resize(header.vertexCount);

		const bool hasSh1 = hasProp("f_rest_0") && hasProp("f_rest_1") && hasProp("f_rest_2") &&
			hasProp("f_rest_3") && hasProp("f_rest_4") && hasProp("f_rest_5") &&
			hasProp("f_rest_6") && hasProp("f_rest_7") && hasProp("f_rest_8");
		const bool hasSh2 = hasProp("f_rest_9") && hasProp("f_rest_10") && hasProp("f_rest_11") &&
			hasProp("f_rest_12") && hasProp("f_rest_13") && hasProp("f_rest_14") &&
			hasProp("f_rest_15") && hasProp("f_rest_16") && hasProp("f_rest_17") &&
			hasProp("f_rest_18") && hasProp("f_rest_19") && hasProp("f_rest_20") &&
			hasProp("f_rest_21") && hasProp("f_rest_22") && hasProp("f_rest_23");
		const bool hasSh3 = hasProp("f_rest_24") && hasProp("f_rest_25") && hasProp("f_rest_26") &&
			hasProp("f_rest_27") && hasProp("f_rest_28") && hasProp("f_rest_29") &&
			hasProp("f_rest_30") && hasProp("f_rest_31") && hasProp("f_rest_32") &&
			hasProp("f_rest_33") && hasProp("f_rest_34") && hasProp("f_rest_35") &&
			hasProp("f_rest_36") && hasProp("f_rest_37") && hasProp("f_rest_38") &&
			hasProp("f_rest_39") && hasProp("f_rest_40") && hasProp("f_rest_41") &&
			hasProp("f_rest_42") && hasProp("f_rest_43") && hasProp("f_rest_44");
		outModel.setMaxShDegree(hasSh3 ? 3 : (hasSh2 ? 2 : (hasSh1 ? 1 : 0)));

		std::vector<float> values(header.properties.size(), 0.0f);
		for (std::size_t i = 0; i < header.vertexCount; ++i)
		{
			if (header.binaryLittleEndian)
			{
				for (std::size_t p = 0; p < header.properties.size(); ++p)
				{
					values[p] = readAsFloatBinary(ifs, header.properties[p].type);
				}
			}
			else
			{
				for (std::size_t p = 0; p < header.properties.size(); ++p)
				{
					ifs >> values[p];
				}
			}

			GaussianSplat splat;
			splat.position = glm::vec3(
				values[propIndex.at("x")],
				values[propIndex.at("y")],
				values[propIndex.at("z")]);

			splat.opacity = 1.0f / (1.0f + std::exp(-values[propIndex.at("opacity")]));
			splat.scale = glm::exp(glm::vec3(
				values[propIndex.at("scale_0")],
				values[propIndex.at("scale_1")],
				values[propIndex.at("scale_2")]));
			splat.rotation = glm::normalize(glm::vec4(
				values[propIndex.at("rot_1")],
				values[propIndex.at("rot_2")],
				values[propIndex.at("rot_3")],
				values[propIndex.at("rot_0")]));

			glm::vec3 dc(0.0f);
			if (hasProp("f_dc_0") && hasProp("f_dc_1") && hasProp("f_dc_2"))
			{
				dc = glm::vec3(values[propIndex.at("f_dc_0")], values[propIndex.at("f_dc_1")], values[propIndex.at("f_dc_2")]);
			}
			splat.color = sh0ToColor(dc);

			if (hasSh1)
			{
				splat.sh1_0 = glm::vec3(values[propIndex.at("f_rest_0")], values[propIndex.at("f_rest_1")], values[propIndex.at("f_rest_2")]);
				splat.sh1_1 = glm::vec3(values[propIndex.at("f_rest_3")], values[propIndex.at("f_rest_4")], values[propIndex.at("f_rest_5")]);
				splat.sh1_2 = glm::vec3(values[propIndex.at("f_rest_6")], values[propIndex.at("f_rest_7")], values[propIndex.at("f_rest_8")]);
			}
			if (hasSh2)
			{
				splat.sh2_0 = glm::vec3(values[propIndex.at("f_rest_9")], values[propIndex.at("f_rest_10")], values[propIndex.at("f_rest_11")]);
				splat.sh2_1 = glm::vec3(values[propIndex.at("f_rest_12")], values[propIndex.at("f_rest_13")], values[propIndex.at("f_rest_14")]);
				splat.sh2_2 = glm::vec3(values[propIndex.at("f_rest_15")], values[propIndex.at("f_rest_16")], values[propIndex.at("f_rest_17")]);
				splat.sh2_3 = glm::vec3(values[propIndex.at("f_rest_18")], values[propIndex.at("f_rest_19")], values[propIndex.at("f_rest_20")]);
				splat.sh2_4 = glm::vec3(values[propIndex.at("f_rest_21")], values[propIndex.at("f_rest_22")], values[propIndex.at("f_rest_23")]);
			}
			if (hasSh3)
			{
				splat.sh3_0 = glm::vec3(values[propIndex.at("f_rest_24")], values[propIndex.at("f_rest_25")], values[propIndex.at("f_rest_26")]);
				splat.sh3_1 = glm::vec3(values[propIndex.at("f_rest_27")], values[propIndex.at("f_rest_28")], values[propIndex.at("f_rest_29")]);
				splat.sh3_2 = glm::vec3(values[propIndex.at("f_rest_30")], values[propIndex.at("f_rest_31")], values[propIndex.at("f_rest_32")]);
				splat.sh3_3 = glm::vec3(values[propIndex.at("f_rest_33")], values[propIndex.at("f_rest_34")], values[propIndex.at("f_rest_35")]);
				splat.sh3_4 = glm::vec3(values[propIndex.at("f_rest_36")], values[propIndex.at("f_rest_37")], values[propIndex.at("f_rest_38")]);
				splat.sh3_5 = glm::vec3(values[propIndex.at("f_rest_39")], values[propIndex.at("f_rest_40")], values[propIndex.at("f_rest_41")]);
				splat.sh3_6 = glm::vec3(values[propIndex.at("f_rest_42")], values[propIndex.at("f_rest_43")], values[propIndex.at("f_rest_44")]);
			}

			outModel.splats[i] = splat;
		}

		std::cout << "Loaded PLY splats: " << outModel.splats.size() << '\n';
		std::cout << "Detected SH payload: " << (hasSh3 ? "degree-3" : (hasSh2 ? "degree-2" : (hasSh1 ? "degree-1" : "none"))) << '\n';
		return true;
	}

} // namespace gs
