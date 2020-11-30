#pragma once

// C++.
#include <sstream>

// Qt.
#include <QAbstractItemModel>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementAdd.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementRemove.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgPipelineStateSearcher.h>

// Forward declarations.
class rgPsoCreateInfo;
enum class rgPipelineType : char;
enum class rgEditorDataType : char;

// rgPipelineStateModel is the object manages the PipelineStateView's tree data structure.
class rgPipelineStateModel : public QObject
{
    Q_OBJECT

public:
    rgPipelineStateModel(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateModel() = default;

    // Get the root element for the tree model.
    rgEditorElement* GetRootElement() const;

    // Initialize the model with API-specific CreateInfo structures.
    void InitializeDefaultPipelineState(QWidget* pParent, rgPipelineType pipelineType);

    // Search the model for the given string.
    void Search(const QString& searchString, rgPipelineStateSearcher::SearchResultData& searchResults, rgEditorElement* pRootElement = nullptr);

    // Return the current pipeline type.
    rgPipelineType GetPipelineType() const;

signals:
    // A signal emitted when a node should be expanded.
    void ExpandNode(rgEditorElementArrayElementAdd* pArrayRoot);

protected:
    // Check that the pipeline state is valid.
    virtual bool CheckValidPipelineState(std::string& errorString) const = 0;

    // Initialize the model with the default graphics pipeline state.
    virtual void InitializeDefaultGraphicsPipeline() = 0;

    // Initialize the model with the default compute pipeline state.
    virtual void InitializeDefaultComputePipeline() = 0;

    // Initialize the Graphics Pipeline CreateInfo tree structure.
    virtual rgEditorElement* InitializeGraphicsPipelineCreateInfo(QWidget* pParent) = 0;

    // Initialize the Compute Pipeline CreateInfo tree structure.
    virtual rgEditorElement* InitializeComputePipelineCreateInfo(QWidget* pParent) = 0;

    // Resize the given array, and copy existing element data into the new array where possible.
    template <typename T>
    T* ResizeArray(const T* pOriginalArray, uint32_t oldSize, uint32_t newSize)
    {
        // Create an array of object with the new dimension.
        T* pResizedArray = new T[newSize]{};

        // If the element count was increased, copy all old contents.
        // If it was reduced, copy as many as will fit in the new array.
        uint32_t elementCount = newSize > oldSize ? oldSize : newSize;

        if (pOriginalArray != nullptr)
        {
            // Copy existing element data into the resized elements.
            for (uint32_t index = 0; index < elementCount; ++index)
            {
                // Copy all member data into the new array elements.
                memcpy(pResizedArray + index, pOriginalArray + index, sizeof(T));
            }
        }

        return pResizedArray;
    }

    // Remove the given element in the provided array and shift the following elements forward.
    template <typename T>
    void RemoveElement(const T* pOriginalArray, int elementCount, int elementIndex)
    {
        // If we remove the given element, is it necessary to shift other subsequent elements?
        if (elementCount > 1 && elementIndex < elementCount - 1)
        {
            // Shift all elements after the removed element.
            for (int copyIndex = elementIndex; copyIndex < (elementCount - 1); ++copyIndex)
            {
                memcpy((void*)(pOriginalArray + copyIndex), pOriginalArray + (copyIndex + 1), sizeof(T));
            }
        }
    }

    // A helper function responsible for resizing element arrays while preserving
    // existing element data.
    // pRootElement is the root element for the array being resized in the model.
    // elementCount is the new number of elements in the array.
    // pArrayPointer is a reference to the existing array that is being resized.
    // pElementTypeName is the API type name of an element in the array.
    // initializationHandler is a callback that gets invoked to initialize each
    // new element in the resized array.
    // isFirstInit is a flag used to indicate if resize is being called as part of
    // a first-time initialization of the tree structure. False indicates that the
    // resize was triggered by an array size element being altered in the view.
    template <typename T, typename U = std::function<void>(rgEditorElement*, T*, int)>
    void ResizeHandler(rgEditorElement* pRootElement,
        uint32_t elementCount,
        const T*& pArrayPointer,
        const char* pElementTypeName,
        U initializationHandler,
        bool isFirstInit)
    {
        int numExistingElements = pRootElement->ChildCount();
        int newElementCount = static_cast<int32_t>(elementCount);

        if (newElementCount != numExistingElements)
        {
            if (isFirstInit)
            {
                numExistingElements = newElementCount;
            }

            rgEditorElementArrayElementAdd* pArrayRoot = static_cast<rgEditorElementArrayElementAdd*>(pRootElement);
            if (pArrayRoot != nullptr)
            {
                if (newElementCount == 0)
                {
                    pRootElement->ClearChildren();
                    RG_SAFE_DELETE(pArrayPointer);

                    // Let the array root element know that the child elements were resized.
                    pArrayRoot->InvokeElementResizedCallback();
                }
                else
                {
                    // Allocate the new element array with the updated size.
                    T* pResized =
                        ResizeArray(pArrayPointer, static_cast<uint32_t>(numExistingElements), static_cast<uint32_t>(newElementCount));

                    // Remove all existing child element items from the array root node.
                    pRootElement->ClearChildren();

                    // Create a new element row for each item in the resized array.
                    CreateArrayElements(newElementCount, pElementTypeName, pArrayRoot, initializationHandler, pResized);

                    // Destroy the old model data.
                    RG_SAFE_DELETE(pArrayPointer);
                    pArrayPointer = pResized;

                    // Invoke the array's resized callback.
                    pArrayRoot->InvokeElementResizedCallback();

                    if (!isFirstInit)
                    {
                        // Expand the array root node that was resized.
                        emit ExpandNode(pArrayRoot);
                    }
                }
            }
        }
    }

    // Create a row for each element in a given array.
    // Each new element will be appended to the given pArrayRoot row, and use the provided
    // initializationHandler callback to initialize the new element row.
    template <typename T, typename U = std::function<void>(rgEditorElement*, T*, int)>
    void CreateArrayElements(int newElementCount, const char* pElementTypeName, rgEditorElementArrayElementAdd* pArrayRoot, U initializationHandler, T* pResized)
    {
        for (int newChildIndex = 0; newChildIndex < newElementCount; ++newChildIndex)
        {
            // Show the child index and the API's name for the structure.
            std::stringstream elementNameStream;
            elementNameStream << newChildIndex;
            elementNameStream << " ";
            elementNameStream << pElementTypeName;

            // Create an element node for each item in the array.
            rgEditorElementArrayElementRemove* pArrayElement = new rgEditorElementArrayElementRemove(pArrayRoot, elementNameStream.str());

            assert(pArrayElement != nullptr);
            assert(pArrayRoot != nullptr);
            if (pArrayElement != nullptr && pArrayRoot != nullptr)
            {
                // Each element in the array needs to know its index and the array root element.
                pArrayElement->SetElementIndex(pArrayRoot, newChildIndex);

                // Add the element node to the array root node.
                pArrayRoot->AppendChildItem(pArrayElement);

                // Invoke the callback used to initialize each array element.
                initializationHandler(pArrayElement, pResized, newChildIndex);
            }
        }

        // Initialize the newly added rows recursively starting with the parent of all the new rows.
        pArrayRoot->InitializeRows();
    }

    // The root editor element item.
    rgEditorElement* m_pRootItem = nullptr;

    // The parent widget that all enum ListWidgets are attached to.
    QWidget* m_pParent = nullptr;

    // The type of pipeline being edited with the model.
    rgPipelineType m_pipelineType;
};