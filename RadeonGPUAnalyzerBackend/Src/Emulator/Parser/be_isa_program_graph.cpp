//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

// Local.
#include "be_isa_program_graph.h"
#include "be_parser_si.h"

const int kDefaultIterationCount = 10;
const int kDefaultIterationCountHwLoops = 64;

ISAProgramGraph::ISAProgramGraph()
{
    next_label_ = kNoLabel - 1 ;
    isa_code_block_ = NULL;
    loop_iteration_count_ = kDefaultIterationCount;
}


IsaCodeBlock* ISAProgramGraph::CreateNewNode(int label)
{
    LabelNodeSet new_node_set;
    IsaCodeBlock* new_node_isa = new IsaCodeBlock();

    // Case this is a chunk of instruction not under specific label.
    if (label == kNoLabel)
    {
        new_node_isa->label_ = next_label_;
        new_node_set.label_ = next_label_;
        next_label_--;
    }
    else
    {
        // There is a label parsed from the ISA.
        new_node_set.label_ = label;
        new_node_isa->label_ = label;
    }

    new_node_set.node_code_block_ = new_node_isa;
    node_set_.insert(new_node_set);

    // Check if this is the head of the graph- if so- save it.
    if (NULL == isa_code_block_)
    {
        isa_code_block_ = new_node_isa;
    }

    return new_node_isa;
}

void ISAProgramGraph::DestroyISAProgramStructure()
{

    if (!node_set_.empty())
    {
        std::set<LabelNodeSet, LabelNodeSetCompare>::const_iterator iterBegin = node_set_.begin();

        for (; iterBegin != node_set_.end(); ++iterBegin)
        {
            if (iterBegin->node_code_block_)
            {
                delete iterBegin->node_code_block_;
            }
        }

        node_set_.clear();

        isa_code_block_ = NULL;
        next_label_ = kNoLabel - 1;
    }
}

IsaCodeBlock* ISAProgramGraph::LabelSearcher(int label)
{
    IsaCodeBlock* ret = NULL;
    LabelNodeSet new_node_set;
    new_node_set.label_ = label;

    std::set<LabelNodeSet, LabelNodeSetCompare>::const_iterator iter_begin = node_set_.find(new_node_set);
    if (iter_begin != node_set_.end())
    {
        ret = iter_begin->node_code_block_;
    }

    return ret;
}

bool ISAProgramGraph::BuildISAProgramStructure(std::vector<Instruction*>& instructions)
{
    std::vector<Instruction*>::const_iterator iIterator = instructions.begin();
    return BuildISAProgramStructureInternal(instructions, iIterator, this->isa_code_block_);
}

bool ISAProgramGraph::BuildISAProgramStructureInternal(std::vector<Instruction*>& instructions,
    std::vector<Instruction*>::const_iterator instruction_iter, IsaCodeBlock* head_isa_node)
{
    if (instruction_iter == instructions.end())
    {
        return false;
    }

    std::vector<Instruction*>::const_iterator iter = instruction_iter;
    Instruction* curr_instruction = *instruction_iter;

    // Set the label.
    if (NULL == head_isa_node)
    {
        head_isa_node = CreateNewNode(curr_instruction->GetLabel());
    }

    // Iterate through the instructions and push them to the node.
    for (; iter != instructions.end(); ++iter)
    {
        // First- add it to the instructions vector of the node.
        head_isa_node->instructions_.push_back(*iter);

        // A branch.
        if ((*iter)->GetGotoLabel() != kNoLabel)
        {
            // this is the info we want to gather in order to decide how to build the graph
            bool does_node_exist = false;
            bool is_loop = false;
            bool is_branch = false;

            std::vector<Instruction*>::const_iterator instruction_begin = instructions.begin();

            // Case 1: the label is already in the graph: search for the label in the IsaTree if found- set the true pointer if false call with the label.
            IsaCodeBlock* loop = LabelSearcher((*iter)->GetGotoLabel());

            if (loop)
            {
                // The node is already in the graph.
                // This check if we have a real loop, and if so, update the num of iterations.
                is_loop = UpdateNumOfIteration(instructions, loop, iter);
                does_node_exist = true;
            }
            else // need to create a new node
            {
                instruction_begin = instructions.begin();

                for (; instruction_begin != instructions.end(); ++instruction_begin)
                {
                    if (((*instruction_begin)->GetLabel() != kNoLabel) && ((*instruction_begin)->GetLabel() == (*iter)->GetGotoLabel()))
                    {
                        break;
                    }
                }

                if (instruction_begin != instructions.end())
                {
                    loop = CreateNewNode((*iter)->GetGotoLabel());

                }

            }

            // Now see if we are facing an S_Branch  or S_CBranch (unconditional branch vs. conditional).
            curr_instruction = *iter;

            if (curr_instruction->GetInstructionCategory() == Instruction::kScalarAlu)
            {
                if (curr_instruction->GetInstructionFormat() == Instruction::kInstructionSetSopp)
                {
                    SISOPPInstruction* instruction_si_sopp = dynamic_cast<SISOPPInstruction*>(curr_instruction);

                    if (instruction_si_sopp != NULL)
                    {
                        SISOPPInstruction::OP opSOPP = instruction_si_sopp->GetOp();

                        if (SISOPPInstruction::kBranch == opSOPP)
                        {
                            is_branch = true;
                        }
                    }
                    else
                    {
                        VISOPPInstruction* instruction_vi_sopp = dynamic_cast<VISOPPInstruction*>(curr_instruction);

                        if (instruction_vi_sopp != NULL)
                        {
                            VISOPPInstruction::OP opSOPP = instruction_vi_sopp->GetOp();

                            if (VISOPPInstruction::kBranch == opSOPP)
                            {
                                is_branch = true;
                            }
                        }
                    }
                }
            }

            // Now we have all the information we need, just need to set the right sequential and call function again.
            if (is_branch)
            {
                if (!is_loop) // this is a case where we will mark the next because this is unconditional branch and we do it always
                {
                    head_isa_node->SetNext(loop);
                }
                else
                {
                    // We shouldn't get it, but just in case.
                    head_isa_node->SetNext(NULL);
                    head_isa_node->SetFalse(NULL);
                    head_isa_node->SetTrue(NULL);
                }

                if (!does_node_exist)
                {
                    BuildISAProgramStructureInternal(instructions, instruction_begin, loop);
                }

                // We have an unconditional branch we break here because there is no false.
                break;
            }
            else
            {
                if (is_loop)
                {
                    // in case of a loop, we mark the TRUE=NULL because we already know it is a loop and we don't want loops in the graph
                    head_isa_node->true_ = NULL; //pLoop;
                }
                else
                {
                    // this is the case where we have a branch that points to a node which exist in the graph and it is not a loop.
                    head_isa_node->SetTrue(loop);
                }

                if (!does_node_exist)
                {
                    BuildISAProgramStructureInternal(instructions, instruction_begin, loop);
                }

                std::vector<Instruction*>::const_iterator next_instruction = (iter + 1);
                if (next_instruction != instructions.end())
                {
                    IsaCodeBlock* pFalseIsaNode = CreateNewNode((*next_instruction)->GetLabel());
                    head_isa_node->SetFalse(pFalseIsaNode);
                    BuildISAProgramStructureInternal(instructions, next_instruction, pFalseIsaNode);
                }

                break;
            }
        }
        else
        {
            // Not a branch- is next instruction is with a label?
            std::vector<Instruction*>::const_iterator next_instruction = (iter + 1);

            if ((next_instruction != instructions.end()) && ((*next_instruction)->GetLabel() != kNoLabel))
            {
                IsaCodeBlock* next_code_block = LabelSearcher((*next_instruction)->GetLabel());
                if (next_code_block)
                {
                    head_isa_node->SetNext(next_code_block);
                    return true;
                }
                else
                {
                    std::vector<Instruction*>::const_iterator instruction_begin = instructions.begin();

                    for (; instruction_begin != instructions.end(); ++instruction_begin)
                    {
                        if (((*instruction_begin)->GetLabel() != kNoLabel) && ((*instruction_begin)->GetLabel() == (*next_instruction)->GetLabel()))
                        {
                            break;
                        }
                    }

                    if (instruction_begin != instructions.end())
                    {
                        IsaCodeBlock* pNextIsaNode = CreateNewNode((*instruction_begin)->GetLabel());
                        head_isa_node->SetNext(pNextIsaNode);
                        BuildISAProgramStructureInternal(instructions, instruction_begin, pNextIsaNode);
                        break;
                    }
                }
            }
        }
    }

    return true;
}


bool ISAProgramGraph::UpdateNumOfIteration(std::vector<Instruction*>& instructions, IsaCodeBlock* loop, const std::vector<Instruction*>::const_iterator iter)
{
    std::vector<Instruction*>::const_iterator instruction_begin = instructions.begin();
    bool isFound = false;

    for (; (instruction_begin != instructions.end()) && (instruction_begin < iter); ++instruction_begin)
    {
        if (((*instruction_begin)->GetLabel() != kNoLabel) && ((*instruction_begin)->GetLabel() == loop->label_))
        {
            isFound = true;
            loop->SetIterationCount(loop_iteration_count_);
            break;
        }
    }

    // It is a loop, check if this is HW loop or regular and update num of iteration.
    if (isFound)
    {
        instruction_begin = loop->instructions_.begin();

        for (; instruction_begin < loop->instructions_.end(); ++instruction_begin)
        {
            SIVOP1Instruction* curr_instruction = dynamic_cast<SIVOP1Instruction*>(*instruction_begin);

            if (curr_instruction != NULL)
            {
                if ((curr_instruction->GetInstructionType() == SIVOP1Instruction::kEncodingVop1) &&
                    ((curr_instruction->GetOp() == SIVOP1Instruction::kReadfirstlaneB32) || (curr_instruction->GetOp() == SIVOP1Instruction::kMovereldB32)))
                {
                    loop->SetIterationCount(kDefaultIterationCountHwLoops);
                    break;
                }
            }
            else
            {
                VIVOP1Instruction* curr_instruction_vivo = dynamic_cast<VIVOP1Instruction*>(*instruction_begin);

                if (curr_instruction_vivo != NULL)
                {
                    if ((curr_instruction_vivo->GetInstructionType() == VIVOP1Instruction::kEncodingVop1) && (curr_instruction_vivo->GetOp() == VIVOP1Instruction::kReadfirstlaneB32))
                    {
                        loop->SetIterationCount(kDefaultIterationCountHwLoops);
                        break;
                    }
                }
            }
        }
    }

    return isFound;
}

void ISAProgramGraph::GetInstructionsOfProgramPath(std::set<LabelNodeSet, LabelNodeSetCompare>& path_instruction_set, int iPath)
{
    GetInstructionsOfProgramPathInternal(path_instruction_set, this->isa_code_block_, iPath, 1);
}

void ISAProgramGraph::GetInstructionsOfProgramPathInternal(std::set<LabelNodeSet, LabelNodeSetCompare>& path_instruction_set, IsaCodeBlock* head_isa_code_block, int path_num, int iteration_count)
{
    if (NULL == head_isa_code_block)
    {
        return;
    }

    // If the node is in the set- return. we visit only once.
    LabelNodeSet label_node_set_temp;
    label_node_set_temp.label_ = head_isa_code_block->GetLabel();
    if (path_instruction_set.find(label_node_set_temp) != path_instruction_set.end())
    {
        return;
    }

    // We got here with the node- put it in the set.
    LabelNodeSet new_label_node_set;
    new_label_node_set.label_ = head_isa_code_block->GetLabel();
    new_label_node_set.iteration_count_ = iteration_count * (head_isa_code_block->GetIterationCount());
    new_label_node_set.node_code_block_ = head_isa_code_block;
    path_instruction_set.insert(new_label_node_set);

    // Check where we go next.
    int iteration_count_temp = new_label_node_set.iteration_count_;
    if (head_isa_code_block->GetTrue())
    {
        // If this is true, we probably going out of a loop- reduce the iteration count.
        if (head_isa_code_block->GetFalse() && (head_isa_code_block->GetIterationCount() > 1))
        {
            if (iteration_count_temp > 1)
            {
                iteration_count_temp /= loop_iteration_count_;
            }

            GetInstructionsOfProgramPathInternal(path_instruction_set, head_isa_code_block->GetTrue(), path_num, iteration_count_temp);
        }

        // We do the true unless we are in the false path.
        if (path_num != 2)
        {
            GetInstructionsOfProgramPathInternal(path_instruction_set, head_isa_code_block->GetTrue(), path_num, new_label_node_set.iteration_count_);
        }
    }

    if (head_isa_code_block->GetFalse())
    {
        iteration_count_temp = new_label_node_set.iteration_count_;

        // we do the false always if the true is null because this is probably an end of a loop
        if (!head_isa_code_block->GetTrue())
        {
            if ((head_isa_code_block->GetIterationCount() > 1) && (iteration_count_temp > 1))
            {
                iteration_count_temp /= loop_iteration_count_;
            }

            GetInstructionsOfProgramPathInternal(path_instruction_set, head_isa_code_block->GetFalse(), path_num, iteration_count_temp);
        }
        else if (path_num != 1)
        {
            GetInstructionsOfProgramPathInternal(path_instruction_set, head_isa_code_block->GetFalse(), path_num, iteration_count_temp);
        }

    }


    if (head_isa_code_block->GetNext())
    {
        // do it always
        GetInstructionsOfProgramPathInternal(path_instruction_set, head_isa_code_block->GetNext(), path_num, new_label_node_set.iteration_count_);
    }

}

void ISAProgramGraph::DumpGraph(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> path_instruction_set, std::string sFileName)
{
    // open the file:
    std::ofstream ofs;
    ofs.open(sFileName.c_str(), std::ofstream::out);
    ofs << "digraph G {\n";


    std::set<LabelNodeSet, LabelNodeSetCompare>::iterator iter = path_instruction_set.begin();

    for (; iter != path_instruction_set.end(); ++iter)
    {
        //loops gets double circle shape, also the arrows are red and mark the number of times it will be done
        if ((*iter).iteration_count_ > 1)
        {
            ofs << (*iter).label_ << " " << "[shape=doublecircle,style=filled,color=\".7 .3 1.0\", label=\" " << (*iter).label_ << " X " << (*iter).iteration_count_ << "\"];\n";
        }
        else
        {
            ofs << (*iter).label_ << " " << "[shape=box];\n";
        }
    }

    for (iter = path_instruction_set.begin(); iter != path_instruction_set.end(); ++iter)
    {
        if ((*iter).node_code_block_->GetTrue())
        {
            LabelNodeSet temp;
            temp.label_ = (*iter).node_code_block_->GetTrue()->GetLabel();

            if (path_instruction_set.find(temp) != path_instruction_set.end())
            {
                ofs << (*iter).label_ << " -> " << (*iter).node_code_block_->GetTrue()->GetLabel() << "[label=\"T\"]" << ";\n ";
            }
        }

        if ((*iter).node_code_block_->GetFalse())
        {
            LabelNodeSet temp;
            temp.label_ = (*iter).node_code_block_->GetFalse()->GetLabel();

            if (path_instruction_set.find(temp) != path_instruction_set.end())
            {
                ofs << (*iter).label_ << " -> " << (*iter).node_code_block_->GetFalse()->GetLabel() <<  "[label=\"F\"]" << ";\n ";
            }
        }

        if ((*iter).node_code_block_->GetNext())
        {
            LabelNodeSet temp;
            temp.label_ = (*iter).node_code_block_->GetNext()->GetLabel();

            if (path_instruction_set.find(temp) != path_instruction_set.end())
            {
                ofs << (*iter).label_ << " -> " << (*iter).node_code_block_->GetNext()->GetLabel() << "[label=\"N\"]" << ";\n ";
            }
        }
    }

    ofs << "}";
    ofs.close();
}

void ISAProgramGraph::CountInstructions(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> path_instruction_set,
    ISAProgramGraph::NumOfInstructionsInCategory& category_instruction_count)
{
    (void)(path_instruction_set);
    (void)(category_instruction_count);
}

void ISAProgramGraph::GetNumOfInstructionsInCategory(ISAProgramGraph::NumOfInstructionsInCategory instruction_category_count[kCalcPathCount], std::string graph_text)
{
    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> path_instruction_set_all;
    GetInstructionsOfProgramPath(path_instruction_set_all, kCalcAll); // ALL
    CountInstructions(path_instruction_set_all, instruction_category_count[kCalcAll]);

    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> path_instruction_set_true;
    GetInstructionsOfProgramPath(path_instruction_set_true, kCalcTrue); // TRUE
    CountInstructions(path_instruction_set_true, instruction_category_count[kCalcTrue]);

    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> path_instruction_set_false;
    GetInstructionsOfProgramPath(path_instruction_set_false, kCalcFalse); // FALSE
    CountInstructions(path_instruction_set_false, instruction_category_count[kCalcFalse]);

    if (graph_text.length() > 0)
    {
        size_t pos = graph_text.rfind(".");

        if (pos == std::string::npos)
        {
            pos  = graph_text.length() + 1;
        }

        std::string file_all = graph_text.substr(0, pos - 1) + "ALL.txt";
        std::string file_true = graph_text.substr(0, pos - 1) + "TRUE.txt";
        std::string file_false = graph_text.substr(0, pos - 1) + "FALSE.txt";
        DumpGraph(path_instruction_set_all, file_all);
        DumpGraph(path_instruction_set_true, file_true);
        DumpGraph(path_instruction_set_false, file_false);
    }
}

void ISAProgramGraph::SetNumOfLoopIteration(int loop_iteration_count)
{
    if (loop_iteration_count > 0)
    {
        loop_iteration_count_ = loop_iteration_count;
    }
}

int ISAProgramGraph::GetNumOfLoopIteration()
{
    return loop_iteration_count_;
}
