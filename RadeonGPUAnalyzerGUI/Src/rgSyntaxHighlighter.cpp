
// C++
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSyntaxHighlighter.h>

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
static const std::map<rgSrcLanguage, rgSyntaxHighlighter::LangDesc> LANG_DESC_MAP = {
    // ----------------
    //     OpenCL
    // ----------------
    {
    /* language */ rgSrcLanguage::OpenCL, {
    /* keywords */ { "if", "else", "for", "while", "do", "repeat", "switch", "case", "default", "break", "continue", "return",  "kernel", "__kernel" , "__attribute__",
                     "static_cast", "reinterpret_cast", "struct", "class", "typedef", "enum", "namespace", "using", "template", "explicit", "constexpr",
                     "inline", "__inline", "private", "public", "protected" },
    /* prefixes */ {},
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

    // ----------------
    //     GLSL
    // ----------------
    {
    /* language */ rgSrcLanguage::GLSL, {
    /* keywords */ { "const", "uniform", "buffer", "shared", "attribute", "varying", "coherent", "volatile", "restrict", "readonly", "writeonly",
                     "atomic_uint", "layout", "centroid", "flat", "smooth", "noperspective", "patch", "sample", "invariant", "precise", "break",
                     "continue", "do", "for", "while", "switch", "case", "default", "if", "else", "subroutine", "in", "out", "inout",
                     "discard", "return", "struct", "common", "partition", "active", "asm", "class", "union", "enum", "typedef", "template",
                     "this", "resource", "goto", "inline", "noinline", "public", "static", "extern", "external", "interface", "input", "output",
                     "filter", "sizeof", "cast", "namespace", "using", "require", "enable"},
    /* prefixes */ {},
    /* types */    { "void", "bool", "int", "uint", "float", "double", "vec2", "vec3", "vec4", "dvec2", "dvec3", "dvec4", "bvec2", "bvec3", "bvec4",
                     "ivec2", "ivec3", "ivec4", "uvec2", "uvec3", "uvec4", "mat2", "mat3", "mat4", "mat2x2", "mat2x3", "mat2x4", "mat3x2", "mat3x3", "mat3x4",
                     "mat4x2", "mat4x3", "mat4x4", "dmat2", "dmat3", "dmat4", "dmat2x2", "dmat2x3", "dmat2x4", "dmat3x2", "dmat3x3", "dmat3x4",
                     "dmat4x2", "dmat4x3", "dmat4x4", "sampler1D", "image1D", "sampler1DShadow", "sampler1DArray", "image1DArray", "sampler1DArrayShadow",
                     "sampler2D", "image2D", "sampler2DShadow", "sampler2DArray", "image2DArray", "sampler2DArrayShadow", "sampler2DMS", "image2DMS",
                     "sampler2DMSArray", "image2DMSArray", "sampler2DRect", "image2DRect", "sampler2DRectShadow", "sampler3D", "image3D", "samplerCube",
                     "imageCube", "samplerCubeShadow", "samplerCubeArray", "imageCubeArray", "samplerCubeArrayShadow", "samplerBuffer", "imageBuffer",
                     "isampler1D", "iimage1D", "isampler1DArray", "iimage1DArray", "isampler2D", "iimage2D", "isampler2DArray", "iimage2DArray",
                     "isampler2DMS", "iimage2DMS", "isampler2DMSArray", "iimage2DMSArray", "isampler2DRect", "iimage2DRect", "isampler3D", "iimage3D",
                     "isamplerCube", "iimageCube", "isamplerCubeArray", "iimageCubeArray", "isamplerBuffer", "iimageBuffer", "usampler1D", "uimage1D",
                     "usampler1DArray", "uimage1DArray", "usampler2D", "uimage2D", "usampler2DArray", "uimage2DArray", "usampler2DMS", "uimage2DMS",
                     "usampler2DMSArray", "uimage2DMSArray", "usampler2DRect", "uimage2DRect", "usampler3D", "uimage3D", "usamplerCube", "uimageCube",
                     "usamplerCubeArray", "uimageCubeArray", "usamplerBuffer", "uimageBuffer", "atomic_uint" },
    /* preproc */  { "#include, #define", "#undef", "#if", "#ifdef", "#ifndef", "#else", "#elif", "#endif", "#error", "#pragma", "#extension", "#version", "#line" },
    /* comments */ { "//", "/*", "*/" }
    } },

    // ----------------
    //     HLSL
    // ----------------
    {
    /* language */ rgSrcLanguage::HLSL, {
    /* keywords */ { "extern", "nointerpolation", "precise", "shared", "groupshared", "static", "uniform", "volatile", "const", "row_major", "column_major",
                     "asm", "asm_fragment", "BlendState", "break", "case", "cbuffer", "centroid", "class", "compile", "compile_fragment", "CompileShader",
                     "continue", "ComputeShader", "default", "DepthStencilState", "DepthStencilView", "discard", "do", "else", "export", "extern", "false",
                     "for", "fxgroup", "GeometryShader", "groupshared", "Hullshader", "if", "in", "inline", "inout", "InputPatch", "interface", "line",
                     "lineadj", "linear", "LineStream", "namespace", "nointerpolation", "noperspective", "NULL", "out", "OutputPatch", "packoffset", "pass",
                     "pixelfragment", "PixelShader", "point", "PointStream", "precise", "RasterizerState", "RenderTargetView", "return", "register",
                     "row-major", "sample", "shared", "snorm", "stateblock", "stateblock_state", "static", "struct", "switch", "technique", "technique10",
                     "technique11", "true", "typedef", "triangle", "triangleadj", "uniform", "unorm", "unroll", "vertexfragment", "VertexShader", "volatile",
                     "while", "branch", "flatten", },
    /* prefixes */ {},
    /* types */    { "Buffer", "bool", "int", "uint", "dword", "half", "float", "double", "vector", "bool1", "bool2", "bool3", "bool4", "int1", "int2",
                     "int3", "int4", "uint1", "uint2", "uint3", "uint4", "dword1", "dword2", "dword3", "dword4", "half1", "half2", "half3", "half4",
                     "float1", "float2", "float3", "float4", "double1", "double2", "double3", "double4", "matrix", "bool1x1", "bool1x2", "bool1x3", "bool1x4",
                     "bool2x1", "bool2x2", "bool2x3", "bool2x4", "bool3x1", "bool3x2", "bool3x3", "bool3x4", "bool4x1", "bool4x2", "bool4x3", "bool4x4",
                     "int1x1", "int1x2", "int1x3", "int2x4", "int2x1", "int2x2", "int2x3", "int2x4", "int3x1", "int3x2", "int3x3", "int3x4",
                     "int4x1", "int4x2", "int4x3", "int4x4", "uint1x1", "uint1x2", "uint1x3", "uint2x4", "uint2x1", "uint2x2", "uint2x3", "uint2x4",
                     "uint3x1", "uint3x2", "uint3x3", "uint3x4", "uint4x1", "uint4x2", "uint4x3", "uint4x4", "dword1x1", "dword1x2", "dword1x3", "dword1x4",
                     "dword2x1", "dword2x2", "dword2x3", "dword2x4", "dword3x1", "dword3x2", "dword3x3", "dword3x4", "dword4x1", "dword4x2", "dword4x3",
                     "dword4x4", "half1x1", "half1x2", "half1x3", "half1x4", "half2x1", "half2x2", "half2x3", "half2x4", "half3x1", "half3x2", "half3x3",
                     "half3x4", "half4x1", "half4x2", "half4x3", "half4x4", "float1x1", "float1x2", "float1x3", "float1x4", "float2x1", "float2x2",
                     "float2x3", "float2x4", "float3x1", "float3x2", "float3x3", "float3x4", "float4x1", "float4x2", "float4x3", "float4x4",
                     "double1x1", "double1x2", "double1x3", "double1x4", "double2x1", "double2x2", "double2x3", "double2x4", "double3x1", "double3x2",
                     "double3x3", "double3x4", "double4x1", "double4x2", "double4x3", "double4x4", "sampler", "texture", "Texture1D", "Texture1DArray",
                     "Texture2D", "Texture2DArray", "Texture3D", "TextureCube", "struct" },
    /* preproc */  { "#define", "#undef", "#if", "#ifdef", "#ifndef", "#else", "#elif", "#endif", "#error", "#pragma", "#line", "#include" },
    /* comments */ { "//", "/*", "*/" }
    } },

    // ----------------
    //     SPIRV
    // ----------------
    {
    /* language */ rgSrcLanguage::SPIRV_Text, {
    /* keywords */ {},
    /* prefixes */ { "Op" },
    /* types */    {},
    /* preproc */  {},
    /* comments */ {";"}
    } },
};

rgSyntaxHighlighter::rgSyntaxHighlighter(QTextDocument* pDoc, rgSrcLanguage lang, const rgSyntaxHighlighter::Style& style)
    : QSyntaxHighlighter(pDoc), m_style(style)
{
    InitTokens(lang);
}

void rgSyntaxHighlighter::InitTokens(rgSrcLanguage lang)
{
    bool foundLangDesc = (LANG_DESC_MAP.count(lang) == 1);
    assert(foundLangDesc);
    if (foundLangDesc)
    {
        m_langDesc = LANG_DESC_MAP.at(lang);

        // Initialize "keyword" tokens
        for (const QString& keyword : m_langDesc.keywords)
        {
            m_tokenTable[keyword[0]].push_back({ keyword, m_style.keywords });
        }

        // Initialize "type" tokens
        for (const QString& type : m_langDesc.types)
        {
            m_tokenTable[type[0]].push_back({ type, m_style.types });
        }

        // Initialize "preprocessor directive" tokens
        for (const QString& prepDir : m_langDesc.preproc)
        {
            m_tokenTable[prepDir[0]].push_back({ prepDir, m_style.preproc });
        }
    }
}

rgSyntaxHighlighter::Style  rgSyntaxHighlighter::GetDefaultStyle()
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

bool rgSyntaxHighlighter::ProcessFuncName(const QString & text, int beginOffset, int endOffset)
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

int  rgSyntaxHighlighter::ProcessComment(const QString& text, int offset, bool isMultiLine)
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

int  rgSyntaxHighlighter::ProcessString(const QString& text, int offset, bool doubleQuote)
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

bool  rgSyntaxHighlighter::ProcessToken(const QString& text, int begin, int end)
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
    const QStringRef token = QStringRef(&text, begin, tokenSize);

    // Check if the token starts with one of predefined keyword prefixes.
    for (const QString& prefix : m_langDesc.keywordPrefixes)
    {
        if (token.startsWith(prefix))
        {
            setFormat(begin, tokenSize, m_style.keywords);
            ret = true;
            break;
        }
    }

    if (!ret)
    {
        // Look through all tokens from the TokenTable with matching 1st symbol
        // and highlight the token if found the matching one.
        for (Token& langToken : m_tokenTable[symbol])
        {
            if (langToken.token.size() == tokenSize && langToken.token.compare(token) == 0)
            {
                setFormat(begin, tokenSize, langToken.format);
                ret = true;
                break;
            }
        }
    }

    return ret;
}

void  rgSyntaxHighlighter::highlightBlock(const QString& text)
{
    int  tokenStart = 0, offset = 0;
    int  textSize = text.size();
    bool inToken = false;

    // Comment starting strings: single-line & multi-line
    QString& SLCommentStart = m_langDesc.comments.singleLineStart;
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
