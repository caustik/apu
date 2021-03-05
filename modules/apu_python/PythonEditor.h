//
// File: PythonEditor.h
// Desc: Declarations for PythonEditor JUCE component
//

#ifndef PYTHON_EDITOR_H
#define PYTHON_EDITOR_H

#include "apu_python.h"

#include <filesystem>
#include <thread>
#include <chrono>

//
// PythonEditor
//
// Main class for PythonEditor JUCE component. Allows Python code to be opened/saved and executed internally
// to define Python function(s) for external code to call.
//

class PythonEditor : public Component, private MenuBarModel, private ApplicationCommandTarget, private FilenameComponentListener, private CodeDocument::Listener
{
public:
    enum CommandIDs
    {
        MenuItemFileSave = 1,
        MenuItemFileSaveAs
    };

    PythonEditor(PythonExecutor& pythonExecutor, FilenameComponentListener* listener = nullptr, std::string filename = "");
    ~PythonEditor() override;

    // Component interface implementation
    void paint(Graphics& graphics) override;
    void resized() override;
    void lookAndFeelChanged() override;

    void setFilename(const char* filename) { m_fileChooser.setCurrentFile(File(filename), true); }
    std::string getFilename() { return m_fileChooser.getCurrentFileText().toStdString(); }

private:
    // color scheme utility functions
    CodeEditorComponent::ColourScheme getDarkCodeEditorColourScheme();
    CodeEditorComponent::ColourScheme getLightCodeEditorColourScheme();
    Colour getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour(0xff4d4d4d)) noexcept;

    // script monitoring utility functions
    void stopMonitoring();
    void startMonitoring();

    // MenuBarModel interface implementation
    StringArray getMenuBarNames() override { return { "File" }; }
    PopupMenu getMenuForIndex(int menuIndex, const String& menuName) override;
    void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}

    // ApplicationCommandTarget interface implementation
    ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
    void getAllCommands(Array<CommandID>& c) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    // FilenameComponentListener interface implementation
    void filenameComponentChanged(FilenameComponent* filenameComponent) override;

    // CodeDocument::Listerner interface implementation
    void codeDocumentTextInserted(const String& newText, int insertIndex) override { codeDocumentTextChanged(); }
    void codeDocumentTextDeleted(int startIndex, int endIndex) override { codeDocumentTextChanged(); }
    void codeDocumentTextChanged();

    // python executor
    PythonExecutor& m_pythonExecutor;

    // menu resources
    MenuBarComponent m_menu;
    ApplicationCommandManager m_commandManager;

    // python editor resources
    CodeDocument m_document;
    PythonCodeTokeniser m_tokeniser;
    CodeEditorComponent m_editor;
    FilenameComponent m_fileChooser;

    // python source monitor resources
    std::filesystem::file_time_type m_lastModified;
    std::thread m_monitor;
    std::atomic<bool> m_monitorQuit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonEditor)
};

#endif /* PYTHON_EDITOR_H */
