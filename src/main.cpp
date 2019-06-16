
#include "pba.hpp"

int main(int argc, char *argv[])
{
    Options::options_description generic("Generic Options");
    generic.add_options()
        ("help,h", "Display this help menu and exit")
        ("version", "Display program version information")
    ;

    Options::options_description exec("Execution Options");
    exec.add_options()
        ("filename,f", Options::value<std::string>(), "Name of file to analyze")
        ("section,S", Options::value<std::string>()->default_value("all"), "Section(s) to analyze")
    ;

    Options::positional_options_description p;
    p.add("filename", -1);

    Options::options_description program_options("Program Options");
    program_options
        .add(generic)
        .add(exec);
    
    Options::variables_map vm;
    Options::store(
        Options::command_line_parser(argc, argv)
            .options(program_options)
            .positional(p)
            .run(),
        vm);
    Options::notify(vm);

    if (vm.count("help")) {
        // TODO: Help option
        std::cout << program_options << std::endl;

        return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
        // TODO: Version option
        std::cout << "<Program Version Info>" << std::endl;

        return EXIT_SUCCESS;
    }

    /** For obvious reasons, we need to verify the user actually supplied the
     *  filename of the binary to analyze. We verify that by querying the
     *  variables_map variable 'vm'. If the 'filename' option was not set, we
     *  need to tell the user and exit with an error status code.
     *
     */
    if (!vm.count("filename")) {
        std::cerr << "No filename supplied." << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "File name: " << vm["filename"].as<std::string>() << std::endl;

    size_t i;
    Binary bin;
    Section* sec;
    Symbol* sym;
    std::string filename;
    
    filename.assign(vm["filename"].as<std::string>());

    if (load_binary(filename, &bin) < 0) {
        return EXIT_FAILURE;
    }

    std::cout << "Loaded binary: " << filename << std::endl;
    std::cout << "\t" << bin.type_str << std::endl;
    std::cout << "\t" << bin.arch_str << std::endl;
    std::cout << "\t" << bin.bits << "bit" << std::endl;
    std::cout << "\tentry@0x" << std::hex << bin.entry << std::endl << std::endl;

    for (i = 0; i < bin.sections.size(); ++i) {
        sec = &bin.sections[i];

        std::cout << "\n\t\tSection Name: " << sec->name << std::endl;
        std::cout << "\t\tAddr: 0x" << std::hex << sec->vma << std::endl;
        std::cout << "\t\tSize: " << sec->size << std::endl;
        std::cout << "\t\tType: " << sec->type << std::endl << std::endl;
    }

    if (bin.symbols.size() > 0) {
        std::cout << "\n\tScanned symbol tables" << std::endl;

        for (i = 0; i < bin.symbols.size(); ++i) {
            sym = &bin.symbols[i];

            std::cout << "\t\t" << std::setw(80) << sym->name << " - " << std::setw(20) << std::left << sym->addr << "(" << sym->type << ")" << std::endl;
        }
    }

    unload_binary(&bin);

    return EXIT_SUCCESS;
}

