#pragma once

// C++
#include <unordered_map>

// Qt
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

// Local
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Hash class for QChar
namespace std
{
template<>
struct hash<QChar>
{
    size_t  operator()(const QChar& c) const {
        return static_cast<size_t>(c.toLatin1());
    }
};
}

//
// RGA GUI Syntax Highlighter
//
// The Syntax Highlighter uses single-pass parsing of the text, very similar to lexical parsing.
// When parsing the text, the Highlighter divides it into lexical tokens and tries to match these tokens
// with ones from the "Token Table".
// The Token Table contains language-defined identifiers that must be highlighted (keywords, types, etc.)
// To accelerate matching tokens, the Token Table is organized as a hash-map addressed by 1st symbol of token.
// Each element of Token Table contains an identifier string and corresponding font style.
//
// Example (OpenCL):
//
// -----
// | a |
// |---|
// | b |--> "bool" --> "bool2" --> "bool3" --> ...
// |---|
// | c |--> "const" --> "case" --> "continue" --> "char" --> ...
// |---|
// | d |--> "do" --> "default" --> "double" --> ...
// |---|
//   .
//   .
//   .
//
class rgSyntaxHighlight : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    // -- Types --

    // Syntax highlighting style
    struct Style {
        QTextCharFormat    keywords;
        QTextCharFormat    types;
        QTextCharFormat    functions;
        QTextCharFormat    comments;
        QTextCharFormat    strings;
        QTextCharFormat    preproc;
    };

    // A Language Descriptor structure that contains lists of keywords, types, comment specifiers etc. to be highlighted.
    struct LangDesc {
        // List of language keywords.
        std::vector<QString>  keywords;

        // List of keyword prefixes. All tokens that start with these prefixes will be highlighted as keywords.
        std::vector<QString>  keywordPrefixes;

        // List of language types.
        std::vector<QString>  types;

        // List of preprocessor directives.
        std::vector<QString>  preproc;

        // Comments: single-line and multi-line.
        struct {
            QString    singleLineStart;
            QString    multiLineStart;
            QString    multiLineEnd;
        } comments;
    };

    // -- Methods --

    // Constructors
    rgSyntaxHighlight(QTextDocument *pDoc, rgSrcLanguage lang)
        : rgSyntaxHighlight(pDoc, lang, GetDefaultStyle()) {}

    rgSyntaxHighlight(QTextDocument *pDoc, rgSrcLanguage lang, const Style& style);

    // Destructor
    ~rgSyntaxHighlight() = default;

    // Change highlighting style
    void SetStyle(Style& style);

private:
    // -- Methods --

    // This function is called by the Qt framework when a fragment of text needs to be reformatted.
    void highlightBlock(const QString& text) override final;

    // Return the default style.
    Style GetDefaultStyle();

    // Process string. Highlights a string enclosed in quotes (') or double quotes (").
    // Returns offset of first symbol after the closing quote.
    int ProcessString(const QString& text, int offset, bool doubleQuote);

    // Process a function or macro name. Returns "true" and highlight the token if it's a function or macro name.
    // Returns "false" otherwise.
    // "endOffset" is the offset of the last symbol in the token.
    bool ProcessFuncName(const QString& text, int beginOffset, int endOffset);

    // Process comment. Highlights a single-line comment or a part of multi-line comment belonging to the current block.
    // Returns offset of first symbol after the comment end.
    int ProcessComment(const QString& text, int offset, bool isMultiLine);

    // Process general token. Highlights the token between "begin" and "end" offsets if it matches with any
    // token from the Token Table.
    // Returns "true" if the token was recognized and highlighted.
    bool ProcessToken(const QString& text, int begin, int end);

    // Initialize the Token Table with language elements from corresponding LangDesc and Style structures.
    void InitTokens(rgSrcLanguage lang);

    // -- Types & Data --

    // An element of Token Table: lexical token and corresponding Qt char format.
    struct Token {
        QString          token;
        QTextCharFormat  format;
    };

    typedef  std::unordered_map<QChar, std::vector<Token>>  TokenTable;

    LangDesc        m_langDesc;
    TokenTable      m_tokenTable;
    Style           m_style;
};
