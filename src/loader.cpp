
#include "pba.hpp"

static bfd* open_bfd(std::string& filename)
{
    static int bfd_inited = 0;

    if (bfd_inited == 0) {
        bfd_init();
        bfd_inited = 1;
    }

    bfd* bfd_h = bfd_openr(filename.c_str(), nullptr);

    if (bfd_h == nullptr) {
        std::cerr << "[Error] Failed to open binary: " << filename << ". [";
        std::cerr << bfd_errmsg(bfd_get_error()) << "]" << std::endl;

        return nullptr;
    }

    if (!bfd_check_format(bfd_h, bfd_object)) {
        std::cerr << filename << " does not look like an executable. [";
        std::cerr << bfd_errmsg(bfd_get_error()) << "]" << std::endl;

        return nullptr;
    }

    /** Some version of bfd_check_format pessimistically set a wrong_format
     *  error before detecting the format and then neglect to unset it once
     *  the format has been detected. We unset it manually to prevent problems.
     */
    bfd_set_error(bfd_error_no_error);

    if (bfd_get_flavour(bfd_h) == bfd_target_unknown_flavour) {
        std::cerr << "Unrecognized format for binary: " << filename << ". [";
        std::cerr << bfd_errmsg(bfd_get_error()) << "]" << std::endl;

        return nullptr;
    }

    return bfd_h;
}

static int load_symbols_bfd(bfd* bfd_h, Binary* bin)
{
    int ret;
    long n, nsyms, i;
    asymbol** bfd_symtab;
    Symbol* sym;

    bfd_symtab = nullptr;

    n = bfd_get_symtab_upper_bound(bfd_h);

    if (n < 0) {
        std::cerr << "Failed to read symtab: " << bfd_errmsg(bfd_get_error()) << std::endl;
        goto FAIL;
    } else if (n) {
        bfd_symtab = static_cast<asymbol**>(malloc(n));

        if (!bfd_symtab) {
            std::cerr << "Error: Out of memory" << std::endl;
            goto FAIL;
        }

        nsyms = bfd_canonicalize_symtab(bfd_h, bfd_symtab);

        if (nsyms < 0) {
            std::cerr << "Failed to read symtab: " << bfd_errmsg(bfd_get_error()) << std::endl;
            goto FAIL;
        }

        for (i = 0; i < nsyms; ++i) {
            if (bfd_symtab[i]->flags & BSF_FUNCTION) {
                bin->symbols.emplace_back(Symbol());
                
                sym = &bin->symbols.back();
                
                sym->type = SymbolType::Function;
                sym->name = std::string(bfd_symtab[i]->name);
                sym->addr = bfd_asymbol_value(bfd_symtab[i]);
            }
        }
    }

    ret = 0;
    goto CLEANUP;

FAIL:
    ret = -1;

CLEANUP:
    if (bfd_symtab) {
        free(bfd_symtab);
    }

    return ret;
}

static int load_dynsym_bfd(bfd* bfd_h, Binary* bin)
{
    int ret;
    long n, nsyms, i;
    asymbol** bfd_dynsym;
    Symbol* sym;

    bfd_dynsym = nullptr;

    n = bfd_get_dynamic_symtab_upper_bound(bfd_h);

    if (n < 0) {
        std::cerr << "Error: Failed to read dynamic symtab: " << bfd_errmsg(bfd_get_error()) << std::endl;
        goto FAIL;
    } else if (n) {
        bfd_dynsym = static_cast<asymbol**>(malloc(n));

        if (!bfd_dynsym) {
            std::cerr << "Error: Out of memory" << std::endl;
            goto FAIL;
        }

        nsyms = bfd_canonicalize_dynamic_symtab(bfd_h, bfd_dynsym);

        if (nsyms < 0) {
            std::cerr << "Error: Failed to read dynamic symtab: " << bfd_errmsg(bfd_get_error()) << std::endl;
            goto FAIL;
        }

        for (i = 0; i < nsyms; ++i) {
            if (bfd_dynsym[i]->flags & BSF_FUNCTION) {
                bin->symbols.emplace_back(Symbol());

                sym = &bin->symbols.back();

                sym->type = SymbolType::Function;
                sym->name = std::string(bfd_dynsym[i]->name);
                sym->addr = bfd_asymbol_value(bfd_dynsym[i]);
            }
        }
    }

    ret = 0;
    goto CLEANUP;

FAIL:
    ret = -1;

CLEANUP:
    if (bfd_dynsym) {
        free(bfd_dynsym);
    }

    return ret;
}

static int load_sections_bfd(bfd* bfd_h, Binary* bin)
{
    int bfd_flags;
    uint64_t vma, size;
    const char* secname;
    asection* bfd_sec;
    Section* sec;
    SectionType sectype;

    for (bfd_sec = bfd_h->sections; bfd_sec; bfd_sec = bfd_sec->next) {
        bfd_flags = bfd_get_section_flags(bfd_h, bfd_sec);

        sectype = SectionType::None;

        if (bfd_flags & SEC_CODE) {
            sectype = SectionType::Code;
        } else if (bfd_flags & SEC_DATA) {
            sectype = SectionType::Data;
        } else {
            continue;
        }

        vma = bfd_section_vma(bfd_h, bfd_sec);
        size = bfd_section_size(bfd_h, bfd_sec);
        secname = bfd_section_name(bfd_h, bfd_sec);

        if (!secname) {
            secname = "<unnamed>";
        }

        bin->sections.emplace_back(Section());
        
        sec = &bin->sections.back();

        sec->binary = bin;
        sec->name = std::string(secname);
        sec->type = sectype;
        sec->vma = vma;
        sec->size = size;
        sec->bytes = static_cast<uint8_t*>(malloc(size));

        if (!sec->bytes) {
            std::cerr << "Error: Out of memory" << std::endl;
            return -1;
        }

        if (!bfd_get_section_contents(bfd_h, bfd_sec, sec->bytes, 0, size)) {
            std::cerr << "Error: Failed to read section '" << secname << "' (" << bfd_errmsg(bfd_get_error()) << ")" << std::endl;
            return -1;
        }
    }

    return 0;
}

static int load_binary_bfd(std::string& filename, Binary* bin)
{
    bfd* bfd_h = open_bfd(filename);

    if (bfd_h == nullptr) {
        std::cerr << "Exiting..." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    bin->filename = filename;
    bin->entry = bfd_get_start_address(bfd_h);
    bin->type_str = std::string(bfd_h->xvec->name);
    
    switch (bfd_h->xvec->flavour) {
        case bfd_target_elf_flavour: {
            bin->type = BinaryType::ELF;
        } break;

        case bfd_target_coff_flavour: {
            bin->type = BinaryType::PE;
        } break;

        default: {
            if (bfd_h) {
                bfd_close(bfd_h);
            }

            std::cerr << "Error: Unsupported binary type: " << bfd_h->xvec->name << std::endl;
            std::exit(EXIT_FAILURE);
        } break;
    }

    const bfd_arch_info_type* bfd_info = bfd_get_arch_info(bfd_h);
    bin->arch_str = std::string(bfd_info->printable_name);

    switch (bfd_info->mach) {
        case bfd_mach_i386_i386: {
            bin->arch = BinaryArch::x86;
            bin->bits = 32;
        } break;

        case bfd_mach_x86_64: {
            bin->arch = BinaryArch::x86;
            bin->bits = 64;
        } break;

        default: {
            if (bfd_h) {
                bfd_close(bfd_h);
            }

            std::cerr << "Error: Unsupported architecture: " << bfd_info->printable_name << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    /* Symbol handling is best-effort only (they may not even be present) */
    load_symbols_bfd(bfd_h, bin);
    load_dynsym_bfd(bfd_h, bin);

    if (load_sections_bfd(bfd_h, bin) < 0) {
        if (bfd_h) {
            bfd_close(bfd_h);
        }

        std::cerr << "Error: Failed to load sections from binary: " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (bfd_h) {
        bfd_close(bfd_h);
    }

    return EXIT_SUCCESS;
}

std::ostream& operator<<(std::ostream& outputStream, SymbolType type)
{
	const auto symbolStr = [](SymbolType type)
	{
		if (type == SymbolType::Function) {
			return "Function";
		}

		return "Unknown";
	}(type);

	return outputStream << symbolStr;
}

std::ostream& operator<<(std::ostream& outputStream, SectionType type)
{
	const auto typeStr = [](SectionType type)
	{
		if (type == SectionType::None) {
			return "None";
		} else if (type == SectionType::Code) {
			return "Code";
		} else if (type == SectionType::Data) {
			return "Data";
		}

		return "Unknown";
	}(type);

	return outputStream << typeStr;
}

int load_binary(std::string& filename, Binary* bin)
{
    return load_binary_bfd(filename, bin);
}

void unload_binary(Binary* bin)
{
    /* Verify 'bin' pointer is not NULL */
    if (bin == nullptr) {
        return;
    }

    for (std::size_t i = 0; i < bin->sections.size(); ++i) {
        Section* section = &bin->sections[i];

        if (section->bytes) {
            free(section->bytes);
        }
    }
}
