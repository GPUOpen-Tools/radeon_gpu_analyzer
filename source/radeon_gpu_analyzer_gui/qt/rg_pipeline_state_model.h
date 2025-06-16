//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso base model.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_H_

// C++.
#include <sstream>

// Qt.
#include <QAbstractItemModel>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_remove.h"
#include "source/radeon_gpu_analyzer_gui/rg_definitions.h"
#include "source/radeon_gpu_analyzer_gui/rg_pipeline_state_searcher.h"

// Forward declarations.
class RgPsoCreateInfo;
enum class RgPipelineType : char;
enum class RgEditorDataType : char;

// RgPipelineStateModel is the object that manages the PipelineStateView's tree data structure.
class RgPipelineStateModel : public QObject
{
    Q_OBJECT

public:
    RgPipelineStateModel(QWidget* parent = nullptr);
    virtual ~RgPipelineStateModel() = default;

    // Get the root element for the tree model.
    RgEditorElement* GetRootElement() const;

    // Initialize the model with API-specific CreateInfo structures.
    void InitializeDefaultPipelineState(QWidget* parent, RgPipelineType pipeline_type);

    // Search the model for the given string.
    void Search(const QString& search_string, RgPipelineStateSearcher::SearchResultData& search_results, RgEditorElement* root_element = nullptr);

    // Return the current pipeline type.
    RgPipelineType GetPipelineType() const;

signals:
    // A signal emitted when a node should be expanded.
    void ExpandNode(RgEditorElementArrayElementAdd* array_root);

protected:
    // Check that the pipeline state is valid.
    virtual bool CheckValidPipelineState(std::string& error_string) const = 0;

    // Initialize the model with the default graphics pipeline state.
    virtual void InitializeDefaultGraphicsPipeline() = 0;

    // Initialize the model with the default compute pipeline state.
    virtual void InitializeDefaultComputePipeline() = 0;

    // Initialize the Graphics Pipeline CreateInfo tree structure.
    virtual RgEditorElement* InitializeGraphicsPipelineCreateInfo(QWidget* parent) = 0;

    // Initialize the Compute Pipeline CreateInfo tree structure.
    virtual RgEditorElement* InitializeComputePipelineCreateInfo(QWidget* parent) = 0;

    // Resize the given array, and copy existing element data into the new array where possible.
    template <typename T>
    T* ResizeArray(const T* original_array, uint32_t old_size, uint32_t new_size)
    {
        // Create an array of object with the new dimension.
        T* resized_array = new T[new_size]{};

        // If the element count was increased, copy all old contents.
        // If it was reduced, copy as many as will fit in the new array.
        uint32_t element_count = new_size > old_size ? old_size : new_size;

        if (original_array != nullptr)
        {
            // Copy existing element data into the resized elements.
            for (uint32_t index = 0; index < element_count; ++index)
            {
                // Copy all member data into the new array elements.
                memcpy(resized_array + index, original_array + index, sizeof(T));
            }
        }

        return resized_array;
    }

    // Remove the given element in the provided array and shift the following elements forward.
    template <typename T>
    void RemoveElement(const T* original_array, int element_count, int element_index)
    {
        // If we remove the given element, is it necessary to shift other subsequent elements?
        if (element_count > 1 && element_index < element_count - 1)
        {
            // Shift all elements after the removed element.
            for (int copy_index = element_index; copy_index < (element_count - 1); ++copy_index)
            {
                memcpy((void*)(original_array + copy_index), original_array + (copy_index + 1), sizeof(T));
            }
        }
    }

    // A helper function responsible for resizing element arrays while preserving
    // existing element data.
    // root_element is the root element for the array being resized in the model.
    // element_count is the new number of elements in the array.
    // array_pointer is a reference to the existing array that is being resized.
    // element_type_name is the API type name of an element in the array.
    // initialization_handler is a callback that gets invoked to initialize each
    // new element in the resized array.
    // is_first_init is a flag used to indicate if resize is being called as part of
    // a first-time initialization of the tree structure. False indicates that the
    // resize was triggered by an array size element being altered in the view.
    template <typename T, typename U = std::function<void>(RgEditorElement*, T*, int)>
    void ResizeHandler(RgEditorElement* root_element,
        uint32_t element_count,
        const T*& array_pointer,
        const char* element_type_name,
        U initialization_handler,
        bool is_first_init)
    {
        int num_existing_elements = root_element->ChildCount();
        int new_element_count = static_cast<int32_t>(element_count);

        if (new_element_count != num_existing_elements)
        {
            if (is_first_init)
            {
                num_existing_elements = new_element_count;
            }

            RgEditorElementArrayElementAdd* array_root = static_cast<RgEditorElementArrayElementAdd*>(root_element);
            if (array_root != nullptr)
            {
                if (new_element_count == 0)
                {
                    root_element->ClearChildren();
                    RG_SAFE_DELETE(array_pointer);

                    // Let the array root element know that the child elements were resized.
                    array_root->InvokeElementResizedCallback();
                }
                else
                {
                    // Allocate the new element array with the updated size.
                    T* resized =
                        ResizeArray(array_pointer, static_cast<uint32_t>(num_existing_elements), static_cast<uint32_t>(new_element_count));

                    // Remove all existing child element items from the array root node.
                    root_element->ClearChildren();

                    // Create a new element row for each item in the resized array.
                    CreateArrayElements(new_element_count, element_type_name, array_root, initialization_handler, resized);

                    // Destroy the old model data.
                    RG_SAFE_DELETE(array_pointer);
                    array_pointer = resized;

                    // Invoke the array's resized callback.
                    array_root->InvokeElementResizedCallback();

                    if (!is_first_init)
                    {
                        // Expand the array root node that was resized.
                        emit ExpandNode(array_root);
                    }
                }
            }
        }
    }

    // Create a row for each element in a given array.
    // Each new element will be appended to the given array_root row, and use the provided
    // initialization_handler callback to initialize the new element row.
    template <typename T, typename U = std::function<void>(RgEditorElement*, T*, int)>
    void CreateArrayElements(int new_element_count, const char* element_type_name, RgEditorElementArrayElementAdd* array_root, U initialization_handler, T* resized)
    {
        for (int new_child_index = 0; new_child_index < new_element_count; ++new_child_index)
        {
            // Show the child index and the API's name for the structure.
            std::stringstream element_name_stream;
            element_name_stream << new_child_index;
            element_name_stream << " ";
            element_name_stream << element_type_name;

            // Create an element node for each item in the array.
            RgEditorElementArrayElementRemove* array_element = new RgEditorElementArrayElementRemove(array_root, element_name_stream.str());

            assert(array_element != nullptr);
            assert(array_root != nullptr);
            if (array_element != nullptr && array_root != nullptr)
            {
                // Each element in the array needs to know its index and the array root element.
                array_element->SetElementIndex(array_root, new_child_index);

                // Add the element node to the array root node.
                array_root->AppendChildItem(array_element);

                // Invoke the callback used to initialize each array element.
                initialization_handler(array_element, resized, new_child_index);
            }
        }

        // Initialize the newly added rows recursively starting with the parent of all the new rows.
        array_root->InitializeRows();
    }

    // The root editor element item.
    RgEditorElement* root_item_ = nullptr;

    // The parent widget that all enum ListWidgets are attached to.
    QWidget* parent_ = nullptr;

    // The type of pipeline being edited with the model.
    RgPipelineType pipeline_type_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_H_
