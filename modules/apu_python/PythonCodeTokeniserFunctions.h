//
// File: PythonCodeTokeniserFunctions.h
// Desc: Utility functions for parsing Python tokens
//

struct PythonTokeniserFunctions : public CppTokeniserFunctions
{
    static bool isReservedKeyword(String::CharPointerType token, const int tokenLength) noexcept
    {
        static const char* const keywords[] = { "and", "as", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "exec", "False",
            "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "None", "not", "or", "pass", "print", "raise", "return", "triple", "True",
            "try", "while", "with", "yield", nullptr };

        for (int i = 0; keywords[i] != nullptr; ++i)
            if (token.compare(CharPointer_ASCII(keywords[i])) == 0)
                return true;

        return false;
    }

    template <typename Iterator>
    static int parseIdentifier(Iterator& source) noexcept
    {
        int tokenLength = 0;
        String::CharPointerType::CharType possibleIdentifier[100];
        String::CharPointerType possible(possibleIdentifier);

        while (isIdentifierBody(source.peekNextChar())) {
            auto c = source.nextChar();

            if (tokenLength < 20)
                possible.write(c);

            ++tokenLength;
        }

        if (tokenLength > 1 && tokenLength <= 16) {
            possible.writeNull();

            if (isReservedKeyword(String::CharPointerType(possibleIdentifier), tokenLength))
                return CPlusPlusCodeTokeniser::tokenType_keyword;
        }

        return CPlusPlusCodeTokeniser::tokenType_identifier;
    }

    template <typename Iterator>
    static int readNextToken(Iterator& source)
    {
        source.skipWhitespace();
        auto firstChar = source.peekNextChar();

        switch (firstChar) {
            case 0:
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.': {
                auto result = parseNumber(source);

                if (result == CPlusPlusCodeTokeniser::tokenType_error) {
                    source.skip();

                    if (firstChar == '.')
                        return CPlusPlusCodeTokeniser::tokenType_punctuation;
                }

                return result;
            }

            case '"':
                skipQuotedString(source);
                return CPlusPlusCodeTokeniser::tokenType_string;

            case '#':
                source.skipToEndOfLine();
                return CPlusPlusCodeTokeniser::tokenType_comment;

            default:
                if (isIdentifierStart(firstChar))
                    return parseIdentifier(source);

                source.skip();
                break;
        }

        return CPlusPlusCodeTokeniser::tokenType_punctuation;
    }
};
