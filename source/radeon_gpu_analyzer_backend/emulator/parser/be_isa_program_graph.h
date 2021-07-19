//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PROGRAM_GRAPH_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PROGRAM_GRAPH_H_

#include "be_instruction.h"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

// Basic node in the isa program.
class IsaCodeBlock
{
private:
    // The label number of the loop, if any.
    int label_ = kNoLabel;

    // How many times to do the loop, if applicable.
    int iteration_count_ = 1;

    // How many nodes points on me.
    std::vector<Instruction*> instructions_;

    // Next code block.
    IsaCodeBlock* next_ = nullptr;

    // Next code block in case of branch is true. null if point to the upper loop to avoid loops in the graph.
    IsaCodeBlock* true_ = nullptr;

    // Next code block in case of branch is false.
    IsaCodeBlock* false_ = nullptr;

public:
    ~IsaCodeBlock() = default;

    int GetLabel() const {return label_;}
    IsaCodeBlock* GetNext() {return next_;}
    IsaCodeBlock* GetTrue() {return true_;}
    IsaCodeBlock* GetFalse() {return false_;}
    int GetIterationCount() const {return iteration_count_;}
    const std::vector<Instruction*>& GetIsaCodeBlockInstructions() const { return instructions_;}

    void SetNext(IsaCodeBlock* p) { next_ = p; }
    void SetTrue(IsaCodeBlock*  p)  { true_ = p; }
    void SetFalse(IsaCodeBlock* p) { false_ = p; }
    void SetIterationCount(int iteration_count) { iteration_count_ = iteration_count; }

    friend class ISAProgramGraph;
};

// This class is a utility class that can build, destroy, search and traverse ISAProgramGraph.
// The LabelNodeSet is saved as well so finding a label will be efficient
class ISAProgramGraph
{
public:
    enum AnalyzeDataPath
    {
        kCalcAll = 0,
        kCalcTrue = 1,
        kCalcFalse = 2,
        kCalcPathCount = 3,
    };

    // The number of instruction in each category
    struct NumOfInstructionsInCategory
    {
        unsigned int instruction_count_scalar_memory_read = 0;
        unsigned int instruction_count_scalar_memory_write = 0;
        unsigned int instruction_count_scalar_alu = 0;
        unsigned int instruction_count_vector_memory_read = 0;
        unsigned int instruction_count_vector_memory_write = 0;
        unsigned int instruction_count_vector_alu = 0;
        unsigned int instruction_count_lds = 0;
        unsigned int instruction_count_gds = 0;
        unsigned int instruction_count_export = 0;
        unsigned int instruction_count_atomics = 0;
        unsigned int calculated_cycles = 0;
        unsigned int calculated_cycles_per_wavefront = 0;

        NumOfInstructionsInCategory& operator=(const NumOfInstructionsInCategory& original)
        {
            instruction_count_scalar_memory_read = original.instruction_count_scalar_memory_read;
            instruction_count_scalar_memory_write = original.instruction_count_scalar_memory_write;
            instruction_count_scalar_alu = original.instruction_count_scalar_alu;
            instruction_count_vector_memory_read = original.instruction_count_vector_memory_read;
            instruction_count_vector_memory_write = original.instruction_count_vector_memory_write;
            instruction_count_vector_alu = original.instruction_count_vector_alu;
            instruction_count_lds = original.instruction_count_lds;
            instruction_count_gds = original.instruction_count_gds;
            instruction_count_export = original.instruction_count_export;
            instruction_count_atomics = original.instruction_count_atomics;
            calculated_cycles = original.calculated_cycles;
            calculated_cycles_per_wavefront = original.calculated_cycles_per_wavefront;
            return *this;
        };
    };

private:
    class LabelNodeSet
    {
    public:
        int label_;
        int iteration_count_;
        IsaCodeBlock* node_code_block_;

        LabelNodeSet()
        {
            label_ = kNoLabel;
            node_code_block_ = NULL;
            iteration_count_ = 1;
        }
    };

    class LabelNodeSetCompare
    {
    public:
        bool operator()(const LabelNodeSet& lhs, const LabelNodeSet& rhs) const
        {
            if (lhs.label_ < rhs.label_)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

private:
    // Head of the entire ISA graph.
    IsaCodeBlock* isa_code_block_;

    // Container for quick node search.
    std::set<LabelNodeSet, LabelNodeSetCompare> node_set_;

    // Fake label for nodes with no original label in the ISA.
    int next_label_;
    int loop_iteration_count_;

    // Internal function that builds the graph out of the instruction vector.
    bool BuildISAProgramStructureInternal(std::vector<Instruction*>& Instructions, std::vector<Instruction*>::const_iterator instIterator, IsaCodeBlock* pHeadIsaNode);

    // Part of ISA program graph. when a loop is discovered update the default num of iteration.
    bool UpdateNumOfIteration(std::vector<Instruction*>& Instructions, IsaCodeBlock* pLoop, const std::vector<Instruction*>::const_iterator iIterator);

    // Internal recursive search.
    void GetInstructionsOfProgramPathInternal(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, IsaCodeBlock* pHeadISACodeBlock, int iPath, int iNumOfIterations);


    // Create a new node, updates it's label and update the node set.
    IsaCodeBlock* CreateNewNode(int label_);

    // The main function that counts the instruction in the specific ISA graph. this is the main idea of the entire analysis.
    void CountInstructions(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, ISAProgramGraph::NumOfInstructionsInCategory& NumOfInstructionsInCategory);

public:
    ISAProgramGraph();
    ~ISAProgramGraph() = default;

    // Go through the instructions parsed in ParseToVector and build the ISA program graph.
    bool BuildISAProgramStructure(std::vector<Instruction*>& Instructions);

     // Destroy ALL the program graph.
    void DestroyISAProgramStructure();

    // Return the node with the desired label
    IsaCodeBlock* LabelSearcher(int label_);

    // Return the vector of instruction in a specific path (all/true/false)
    void GetInstructionsOfProgramPath(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, int iPath);

    // Save the graph in GRAPHVIZ format.
    void DumpGraph(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, std::string sFileName);

    // Traverse through all Graph Paths (ALL/TRUE/FALSE) and count the instructions.
    void GetNumOfInstructionsInCategory(ISAProgramGraph::NumOfInstructionsInCategory NumOfInstructionsInCategory[kCalcPathCount], std::string sDumpGraph);

    void SetNumOfLoopIteration(int iNumOfLoopIteration);
    int GetNumOfLoopIteration();
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PROGRAM_GRAPH_H_