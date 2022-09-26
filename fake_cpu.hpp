#ifndef FAKE_CPU_HPP
#define FAKE_CPU_HPP

#include <fstream>
#include <map>
#include <vector>

#include "register.hpp"

class FakeCPU {
private:
    using Opcode = uint8_t;
    using Instruction = std::pair<std::string, std::vector<std::string>>;

public:
    FakeCPU();
    
public:
    void executeProgram(std::ifstream& inputFile);

private:
    void constructRegisters();
    void registerOperations();

    void parseFile(std::ifstream& inputFile);
    Instruction parseInstruction(const std::string& line);
    void addLabel(const std::string& name, int instructionIndex);

    void executeSingleInstruction(const Instruction& instruction);
    void changeFlags(int recentOperationResult);

    bool isDataRegister(const std::string& name);
    auto getArgumentValue(const std::string& arg);
    Register<int>* destRegisterForArtihmeticLogic(const std::vector<std::string>& args);
    
    int MOV(const std::vector<std::string>& args);
    int CMP(const std::vector<std::string>& args);
    int ADD(const std::vector<std::string>& args);
    int SUB(const std::vector<std::string>& args);
    int MUL(const std::vector<std::string>& args);
    int DIV(const std::vector<std::string>& args);

    int AND(const std::vector<std::string>& args);
    int OR(const std::vector<std::string>& args);
    int XOR(const std::vector<std::string>& args);
    int TEST(const std::vector<std::string>& args);
    int NOT(const std::vector<std::string>& args);
    int JMP(const std::vector<std::string>& args);
    int JE(const std::vector<std::string>& args);
    int JL(const std::vector<std::string>& args);
    int JLE(const std::vector<std::string>& args);
    int JG(const std::vector<std::string>& args);
    int JGE(const std::vector<std::string>& args);


private:
    std::map<int, Instruction> programInstructions;
    std::map<std::string, int> labels;

    std::map<std::string, Opcode> operationCodes;
    std::map<std::string, int(FakeCPU::*)(const std::vector<std::string>&)> operations;

    Register<int> dataRegisters[10];

    Register<unsigned int> eip;
    Register<bool> zf;
    Register<bool> sf;
};

#endif /* FAKE_CPU_HPP */
