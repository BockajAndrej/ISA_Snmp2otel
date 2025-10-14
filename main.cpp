
#include "src/ArgumentParser.h"
#include "src/Structs/SnmpOtelErrors.h"
#include "src/SnmpOtel_BL.h"

int main(int argc, char *argv[]) {
    ArgumentParser parser;
    SnmpOtelConfig config;

    try{
        config = parser.Parse(argc, argv);
        SnmpOtel_BL BL_logic;

        BL_logic.MainFnc(config);
    }catch (const MissingArgumentError& e){
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }catch(const InvalidValueError& e){
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }catch(const std::runtime_error& e){
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
