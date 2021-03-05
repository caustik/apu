//
// File: PythonCodeTokeniser.h
// Desc: Tokeniser for Python source code
//

class JUCE_API PythonCodeTokeniser : public CPlusPlusCodeTokeniser
{
public:
    PythonCodeTokeniser() {}
    ~PythonCodeTokeniser() override {}

    // CodeTokeniser interface implementation
    int readNextToken(CodeDocument::Iterator&) override;

private:
    JUCE_LEAK_DETECTOR(PythonCodeTokeniser)
};
