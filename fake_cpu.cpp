#include <iostream>
#include <sstream>
#include <string>

#include "fake_cpu.hpp"
#include "register.cpp"


FakeCPU::FakeCPU() {
    constructRegisters();
    registerOperations();
}

void FakeCPU::executeProgram(std::ifstream& inputFile) {
    parseFile(inputFile);
    while(eip.read() < programInstructions.size()) {
        executeSingleInstruction(programInstructions[eip.read()]);
    }
}

void FakeCPU::constructRegisters() {
     eip = Register<unsigned int>("EIP", 0);
     zf = Register<bool>("ZF", 0);
     sf = Register<bool>("SF", 0);   

    for(int i = 0; i < 10; ++i) {
        dataRegisters[i] = Register<int>("r" + std::to_string(i), 0);
    }

}

void FakeCPU::registerOperations() {
    operations["mov"] = &FakeCPU::MOV;

    operations["cmp"] = &FakeCPU::CMP;
    operations["test"] = &FakeCPU::TEST;

    operations["add"] = &FakeCPU::ADD;
    operations["sub"] = &FakeCPU::SUB;
    operations["mul"] = &FakeCPU::MUL;
    operations["div"] = &FakeCPU::DIV;

    operations["and"] = &FakeCPU::AND;
    operations["or"] = &FakeCPU::OR;
    operations["xor"] = &FakeCPU::XOR;
    operations["not"] = &FakeCPU::NOT;

    operations["jmp"] = &FakeCPU::JMP;
    operations["je"] = &FakeCPU::JE;
    operations["jl"] = &FakeCPU::JL;
    operations["jle"] = &FakeCPU::JLE;
    operations["jg"] = &FakeCPU::JG;
    operations["jge"] = &FakeCPU::JGE;
}

void FakeCPU::parseFile(std::ifstream& inputFile) {
    int instructionCount = 0;

    std::string line;
    while (std::getline(inputFile, line)) {
        if(line[line.size() - 1] == ':') {
            line.pop_back();
            addLabel(line, instructionCount - 1);
        }
        else {
            programInstructions[instructionCount++] = parseInstruction(line);
        }
    }
}

FakeCPU::Instruction FakeCPU::parseInstruction(const std::string& line) {
    std::istringstream ss(line);
    std::string token;

    std::getline(ss,token,' ');
    std::string operationName = token;
    std::vector<std::string> instructionArguments;
    while(std::getline(ss, token,',')){
        ss.ignore();
        instructionArguments.push_back(token);
    }
    
    return Instruction{operationName, instructionArguments};
}


void FakeCPU::addLabel(const std::string& name, int instructionIndex) {
    if(labels.find(name) == labels.end()) {
        labels[name] = instructionIndex;
    }
    else {
        std::cerr << "Label with name '" << name << "' already exists!" << std::endl;
        exit(1);
    }
}

void FakeCPU::executeSingleInstruction(const Instruction& instruction) {
    int res = (this->*operations[instruction.first])(instruction.second);
    changeFlags(res);
    eip.write(eip.read() + 1);
}

void FakeCPU::changeFlags(int recentOperationResult) {
    zf.write(recentOperationResult ? false : true);
    sf.write((recentOperationResult < 0) ? true : false);
}

bool FakeCPU::isDataRegister(const std::string& name) {
    return name.size() == 2 && name[0] == 'r' && isdigit(name[1]);
}

auto FakeCPU::getArgumentValue(const std::string& arg) {
    if(isDataRegister(arg)) {
        return dataRegisters[arg[1] - '0'].read();
    }
    else if(std::find_if(arg.begin(), 
        arg.end(), [](unsigned char c) { return !std::isdigit(c); }) == arg.end()) {
        return std::stoi(arg);
    }
    else {
        std::cerr << "Cannot determine the value of '" << arg << "'!" << std::endl;
        exit(1);
    }
}

Register<int>* FakeCPU::destRegisterForArtihmeticLogic(const std::vector<std::string>& args) {
    Register<int>* dest = nullptr;

    if(args.size() == 3) {
        if(isDataRegister(args.at(2))) {
            dest = &dataRegisters[(args.at(2)[1]) - '0'];
        }
        else {
            std::cerr << "Cannot have '" << args.at(2) << "' as destination!" << std::endl;
            exit(1);
        }
    }
    else if(isDataRegister(args.at(0))) {
        dest = &dataRegisters[(args.at(0)[1]) - '0'];
    }
    else {
        std::cerr << "Cannot have '" << args.at(0) << "' as destination!" << std::endl;
        exit(1);
    }

    return dest;
}

int FakeCPU::MOV(const std::vector<std::string>& args) {
    if(args.size() != 2) {
        std::cerr << "Invalid amount of arguments for operation 'mov'" << std::endl;
        exit(1); 
    }

    if(!isDataRegister(args.at(0))) {
        std::cerr << "Cannot have '" << args.at(0) << "' as destination!" << std::endl;
        exit(1);
    }

    int sourceValue = getArgumentValue(args.at(1));
    dataRegisters[(args.at(0)[1]) - '0'].write(sourceValue);

    return 0;
}

int FakeCPU::CMP(const std::vector<std::string>& args) {
    if(args.size() != 2) {
        std::cerr << "Invalid amount of arguments for operation 'cmp'" << std::endl;
        exit(1); 
    }
    return getArgumentValue(args.at(0)) - getArgumentValue(args.at(1));
}

int FakeCPU::TEST(const std::vector<std::string>& args) {
    if(args.size() != 2) {
        std::cerr << "Invalid amount of arguments for operation 'test'" << std::endl;
        exit(1); 
    }

    return getArgumentValue(args.at(0)) & getArgumentValue(args.at(1));
}

int FakeCPU::ADD(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'add'" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);

    int result = firstArgValue + secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::SUB(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'sub'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue - secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::MUL(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'mul'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue * secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::DIV(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'div'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    if(!secondArgValue) {
        std::cerr << "Cannot divide by zero!" << std::endl;
        exit(1); 
    }

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue / secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::AND(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'and'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue & secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::OR(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'and'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue | secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::XOR(const std::vector<std::string>& args) {
    if(args.size() < 2 || args.size() > 3) {
        std::cerr << "Invalid amount of arguments for operation 'and'!" << std::endl;
        exit(1); 
    }

    int firstArgValue = getArgumentValue(args.at(0));
    int secondArgValue = getArgumentValue(args.at(1));

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = firstArgValue ^ secondArgValue;
    destination->write(result);

    return result;
}

int FakeCPU::NOT(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'not'!" << std::endl;
        exit(1);
    }

    if(!isDataRegister(args.at(0))) {
        std::cerr << "Cannot have '" << args.at(0) << "' as destination!" << std::endl;
        exit(1);
    }

    Register<int>* destination = destRegisterForArtihmeticLogic(args);
    int result = !destination->read();
    destination->write(result);

    return result;
}

int FakeCPU::JMP(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    std::string labelName = args.at(0);

    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }
    eip.write(labels[labelName]);
    return 1;
}

int FakeCPU::JE(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    std::string labelName = args.at(0);

    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }

    if(!zf.read()) {
        eip.write(labels[labelName]);
        return 1;
    }

    return 0;
}

int FakeCPU::JL(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    
    std::string labelName = args.at(0);
    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }

    if(sf.read() && !zf.read()) {
        eip.write(labels[labelName]);
        return 1;
    }

    return 0;
}

int FakeCPU::JLE(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    std::string labelName = args.at(0);

    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }

    if(sf.read() && zf.read()) {
        eip.write(labels[labelName]);
        return 1;
    }

    return 0;
}

int FakeCPU::JG(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    std::string labelName = args.at(0);

    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }

    if(!sf.read() && !zf.read()) {
        eip.write(labels[labelName]);
    }

    return 0;
}

int FakeCPU::JGE(const std::vector<std::string>& args) {
    if(args.size() != 1) {
        std::cerr << "Invalid amount of arguments for operation 'jmp'!" << std::endl;
        exit(1);
    }

    std::string labelName = args.at(0);

    if(labels.find(labelName) == labels.end()) {
        std::cerr << "No label named '" << labelName << "'!" << std::endl;
        exit(1);
    }

    if(!sf.read() && zf.read()) {
        eip.write(labels[labelName]);
        return 1;
    }

    return 0;
}
