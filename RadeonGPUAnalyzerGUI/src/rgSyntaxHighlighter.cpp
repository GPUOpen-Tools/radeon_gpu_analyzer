
// C++
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgSyntaxHighlighter.h>

// Static types and constants

// Indicates whether a text block has some "open" state that continues in the next block.
enum class BlockState {
    None,
    SingleLineComment,
    MultiLineComment,
    SingleQuoteString,
    DoubleQuoteString
};


// Definitions of language descriptors.
static const std::map<rgSyntaxHighlight::Language, rgSyntaxHighlight::LangDesc> LANG_DESC_MAP = {
    {
    /* language */ rgSyntaxHighlight::Language::OpenCL, {
    /* keywords */ { "if", "else", "for", "while", "do", "repeat", "switch", "case", "default", "break", "continue", "return",  "kernel", "__kernel" , "__attribute__",
                     "static_cast", "reinterpret_cast", "struct", "class", "typedef", "enum", "namespace", "using", "template", "explicit", "constexpr",
                     "inline", "__inline", "private", "public", "protected" },
    /* types */    { "void", "signed", "unsigned", "size_t", "ptrdiff_t", "intptr_t", "uintptr_t", "auto", "const", "__const", "static",
                     "global", "__global", "local", "__local", "private", "__private", "constant", "__constant",
                     "read_only", "__read_only", "write_only", "__write_only", "read_write", "__read_write",
                     "bool", "bool2", "bool3", "bool4", "bool8", "bool16",
                     "char", "char2", "char3", "char4", "char8", "char16", "uchar", "uchar2", "uchar3", "uchar4", "uchar8", "uchar16",
                     "short", "short2", "short3", "short4", "short8", "short16", "ushort", "ushort2", "ushort3", "ushort4", "ushort8", "ushort16",
                     "int", "int2", "int3", "int4", "int8", "int16", "uint", "uint2", "uint3", "uint4", "uint8", "uint16",
                     "long", "long2", "long3", "long4", "long8", "long16", "ulong", "ulong2", "ulong3", "ulong4", "ulong8", "ulong16",
                     "float", "float2", "float3", "float4", "float8", "float16",
                     "double", "double2", "double3", "double4", "double8", "double16",
                     "half", "half2", "half3", "half4", "half8", "half16",
                     "sampler", "sampler_t", "image1d", "image1d_t", "image2d", "image2d_t", "image3d", "image3d_t",
                     "image1d_buffer", "image1d_buffer_t", "image1d_array", "image1d_array_t", "image2d_ms", "image2d_array", "image2d_array_t", "image2d_array_ms",
                     "image2d_depth", "image2d_depth_ms", "image2d_array_depth", "image2d_array_depth_ms", "image3d", "image3d_t",
                     "pipe", "device_queue", "queue_t", "ndrange_t", "clk_event_t", "reserve_id_t", "event_t", "cl_mem_fence_flags"},
    /* preproc */  { "#include", "#define", "#undef", "#if", "#ifdef", "#ifndef", "#else", "#endif", "#error", "#pragma" },
    /* comments */ { "//", "/*", "*/" }
    } },
    {
    /* language */ rgSyntaxHighlight::Language::GLSL, {
    /* keywords */ {/* TODO: Add GLSL keywords. */},
    /* types */    {/* TODO: Add GLSL types. */},
    /* preproc */  {/* TODO: Add preprocessor directives. */},
    /* comments */ {/* TODO: Add comment start/end tokens. */}
    } },
    // TODO: Add HLSL and Vulkan.
};

rgSyntaxHighlight::rgSyntaxHighlight(QTextDocument* pDoc, rgSyntaxHighlight::Language lang, const rgSyntaxHighlight::Style& style)
    : QSyntaxHighlighter(pDoc), m_style(style)
{
    InitTokens(lang);
}

void rgSyntaxHighlight::InitTokens(rgSyntaxHighlight::Language lang)
{
    assert(LANG_DESC_MAP.count(lang) == 1);
    m_langDesc = LANG_DESC_MAP.at(lang);

    // Initialize "keyword" tokens
    for (const QString& keyword : m_langDesc.keywords)
    {
        m_tokenTable[keyword[0]].push_back({keyword, m_style.keywords});
    }

    // Initialize "type" tokens
    for (const QString& type : m_langDesc.types)
    {
        m_tokenTable[type[0]].push_back({type, m_style.types});
    }

    // Initialize "preprocessor directive" tokens
    for (const QString& prepDir : m_langDesc.preproc)
    {
        m_tokenTable[prepDir[0]].push_back({prepDir, m_style.preproc});
    }
}

rgSyntaxHighlight::Style  rgSyntaxHighlight::GetDefaultStyle()
{
    Style  style;
    style.keywords.setForeground(Qt::blue);
    style.types.setForeground(Qt::blue);
    style.functions.setForeground(Qt::darkRed);
    style.comments.setForeground(Qt::darkGreen);
    style.strings.setForeground(Qt::darkRed);
    style.preproc.setForeground(Qt::darkBlue);

    return style;
}

bool rgSyntaxHighlight::ProcessFuncName(const QString & text, int beginOffset, int endOffset)
{
    bool  ret = false;
    // Check if the next symbol after endOffset is "(".
    int size = text.size();
    int offset = endOffset + 1;
    while (offset < size && text[offset] == ' ') offset++;
    if (offset < size && text[offset] == '(')
    {
        setFormat(beginOffset, endOffset - beginOffset + 1, m_style.functions);
        ret = true;
    }
    return ret;
}

int  rgSyntaxHighlight::ProcessComment(const QString& text, int offset, bool isMultiLine)
{
    int size = text.size();
    assert(offset >= 0 && offset <= size);
    int endOffset = offset;
    BlockState  state = BlockState::None;

    if (isMultiLine)
    {
        QString& MLCommentEnd = m_langDesc.comments.multiLineEnd;
        while (endOffset < size && MLCommentEnd.compare(QStringRef(&text, endOffset, MLCommentEnd.size())) != 0)
        {
            endOffset++;
        }
        // If there are no comment ending symbols within the current text block, mark this block with
        // "MultiLineComment" state so that the next block will know that it should continue looking for the comment end.
        state = endOffset == size ? BlockState::MultiLineComment : BlockState::None;
        endOffset += (endOffset == size ? 0 : MLCommentEnd.size());
    }
    else
    {
        // Skip symbols till the end of line or the end of text.
        while (endOffset < size)
        {
            // If found line break ('\' symbol is the last symbon in the line), mark this block with
            // "SingleLineComment" so that the next block knows that it should continue looking for the "real" EOL.
            if (endOffset == (size - 1) && text[endOffset] == '\\')
            {
                state = BlockState::SingleLineComment;
            }
            endOffset++;
        }
    }

    setFormat(offset, endOffset - offset, m_style.comments);
    setCurrentBlockState(static_cast<int>(state));

    return endOffset;
}

int  rgSyntaxHighlight::ProcessString(const QString& text, int offset, bool doubleQuote)
{
    int  size = text.size();
    assert(offset >= 0 && offset <= size);
    int  endOffset = offset + 1;
    QChar  endSymbol = (doubleQuote ? '"' : '\'');
    BlockState state = BlockState::None;

    // Skip symbols till the end symbol, the end of line or the end of text.
    while (endOffset < size && text[endOffset] != endSymbol)
    {
        // If found line break ('\' symbol is the last symbol in the line), mark this block with
        // "SingleQuoteString" or "DoubleQuoteString".
        if (endOffset == (size - 1) && text[endOffset] == '\\')
        {
            state = (doubleQuote ? BlockState::DoubleQuoteString : BlockState::SingleQuoteString);
        }
        endOffset++;
    }

    setFormat(offset, endOffset - offset + 1, m_style.strings);
    setCurrentBlockState(static_cast<int>(state));

    return endOffset;
}

bool  rgSyntaxHighlight::ProcessToken(const QString& text, int begin, int end)
{
    // text
    //  |
    //  v
    //  static float foo(...)
    //         ^   ^
    //         |   |
    //  begin -'   `- end

    assert(begin >= 0 && begin <= end && end < text.size());
    bool  ret = false;
    QChar symbol = text[begin];
    int tokenSize = end - begin + 1;
    // Look through all tokens from the TokenTable with matching 1st symbol
    // and highlight the token if found the matching one.
    for (Token& token : m_tokenTable[symbol])
    {
        if (token.token.size() == tokenSize && token.token.compare(QStringRef(&text, begin, tokenSize)) == 0)
        {
            setFormat(begin, tokenSize, token.format);
            ret = true;
            break;
        }
    }

    return ret;
}

void  rgSyntaxHighlight::highlightBlock(const QString& text)
{
    int  tokenStart = 0, offset = 0;
    int  textSize = text.size();
    bool inToken = false;

    // Comment starting strings: single-line & multi-line
    QString& SLCommentStart = m_langDesc.comments.singleLine;
    QString& MLCommentStart = m_langDesc.comments.multiLineStart;

    // First, check if previous block has unfinished state that has to be continued in this block.
    BlockState  prevState = static_cast<BlockState>(previousBlockState());
    if (prevState == BlockState::SingleLineComment || prevState == BlockState::MultiLineComment)
    {
        offset = ProcessComment(text, offset, prevState == BlockState::MultiLineComment);
    }
    else if (prevState == BlockState::SingleQuoteString || prevState == BlockState::DoubleQuoteString)
    {
        offset = ProcessString(text, offset, prevState == BlockState::DoubleQuoteString);
    }
    else
    {
        setCurrentBlockState(static_cast<int>(BlockState::None));
    }

    // Parse tokens.
    while (offset < textSize)
    {
        QChar  symbol = text[offset];
        if (inToken)
        {
            // Check if current symbol terminates a token or it's the last symbol in the block.
            if (!symbol.isLetterOrNumber() && symbol != '_' && symbol != '#')
            {
                offset--;
                // Check if it's a function or macro first.
                if (!ProcessToken(text, tokenStart, offset))
                {
                    ProcessFuncName(text, tokenStart, offset);
                }
                inToken = false;
            }
            else if (offset == textSize - 1)
            {
                ProcessToken(text, tokenStart, offset);
                inToken = false;
            }
        }
        else
        {
            // Check if current symbol starts a comment.
            if (SLCommentStart.compare(QStringRef(&text, offset, SLCommentStart.size())) == 0)
            {
                offset = ProcessComment(text, offset, false);
            }
            else if (MLCommentStart.compare(QStringRef(&text, offset, MLCommentStart.size())) == 0)
            {
                offset = ProcessComment(text, offset, true);
            }

            // Check if current symbol starts a string.
            if (symbol == '"' || symbol == '\'')
            {
                offset = ProcessString(text, offset, symbol == '"');
            }

            // Check if current symbol starts new token.
            if (symbol.isLetter() || symbol == '_' || symbol == '#')
            {
                tokenStart = offset;
                inToken = true;
            }
        }
        offset++;
    } // while
}
