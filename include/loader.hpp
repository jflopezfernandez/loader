
#ifndef PROJECT_INCLUDES_LOADER_HPP_
#define PROJECT_INCLUDES_LOADER_HPP_

class Binary;
class Section;
class Symbol;

enum class SymbolType {
	Unknown,
	Function
};

std::ostream& operator<<(std::ostream& outputStream, SymbolType type);

struct Symbol {
	SymbolType type;
	std::string name;
	uint64_t addr;

public:
	Symbol()
		: type { SymbolType::Unknown }, name { }, addr { NULL }
	{
		/* Default constructor */
	}
};

enum class SectionType {
	None,
	Code,
	Data
};

std::ostream& operator<<(std::ostream& outputStream, SectionType type);

struct Section
{
	Binary* binary;
	std::string name;
	SectionType type;
	uint64_t vma;
	uint64_t size;
	uint8_t* bytes;

public:
	Section()
		: binary { nullptr }, name { }, type { SectionType::None }, vma { 0 }, size { 0 }, bytes { nullptr }
	{
		/* Default constructor */
	}

	Section(const Section& section)
		: binary { section.binary }, name { section.name }, type { section.type }, vma { section.vma }, size { section.size }, bytes { section.bytes }
	{
		/* Copy constructor */
	}

	Section& operator=(const Section& section)
	{
		binary = section.binary;
		name = section.name;
		type = section.type;
		vma = section.vma;
		size = section.size;
		bytes = section.bytes;

		return *this;
	}

	bool contains(uint64_t addr)
	{
		return ((addr >= vma) && (addr - vma < size));
	}
};

enum class BinaryType {
	Auto,
	ELF,
	PE
};

// TODO: Implement BinaryType -> String

enum class BinaryArch {
	None,
	x86
};

// TODO: Implement BinaryArch -> String

struct Binary
{
	std::string filename;
	BinaryType type;
	std::string type_str;
	BinaryArch arch;
	std::string arch_str;
	unsigned int bits;
	uint64_t entry;
	std::vector<Section> sections;
	std::vector<Symbol> symbols;

public:
	Binary()
		: filename { }, type { BinaryType::Auto }, type_str { }, arch { BinaryArch::None }, arch_str { }, bits { 0 }, entry { 0 }, sections { }, symbols { }
	{
		/* Default constructor */
	}

	Section* get_text_section()
	{
		for (auto& section : sections) {
			if (section.name == ".text") {
				return &section;
			}
		}

		return nullptr;
	}
};

int load_binary(std::string& filename, Binary* bin);

void unload_binary(Binary* bin);

#endif // PROJECT_INCLUDES_LOADER_HPP_

