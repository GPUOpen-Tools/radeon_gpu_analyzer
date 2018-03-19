#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgFindTextWidget.h>

// Forward declarations.
class rgSourceCodeEditor;

// An object responsible for searching a source editor using an rgFindTextWidget.
class rgSourceEditorSearcher : public ISearchable
{
public:
    rgSourceEditorSearcher();
    virtual ~rgSourceEditorSearcher() = default;

    // Override FindNext to find the next instance of text within the source editor.
    virtual bool FindNext(const QString& searchString) override;

    // Reset the current search.
    virtual void ResetSearch() override;

    // Select the current search result in the source editor.
    virtual void SelectResults() override;

    // Set the target source editor being searched.
    void SetTargetEditor(rgSourceCodeEditor* pTargetEditor);

private:
    // Set cursor for the current editor without changing kernel/correlation context.
    void SetCodeEditorCursor(const QTextCursor& cursor);

    // The location in the document of the last search result.
    int m_lastFoundPosition;

    // The string to search for in the document.
    QString m_searchString;

    // The flag used to determine whether or not matching case matters when searching.
    bool m_matchCase = false;

    // The source editor instance whose text is being searched.
    rgSourceCodeEditor* m_pTargetEditor = nullptr;
};