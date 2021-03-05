//
// File: PythonEditor.cpp
// Desc: Definitions for PythonEditor JUCE module
//

#include "apu_python.h"

PythonEditor::PythonEditor(PythonExecutor& pythonExecutor, FilenameComponentListener* listener, std::string filename)
  : m_pythonExecutor(pythonExecutor),
    m_menu(this),
    m_editor(m_document, &m_tokeniser),
    m_fileChooser("File", {}, true, false, false, "*.py", {}, "Choose a Python file to open it in the editor"),
    m_monitorQuit(true)
{
    // initialize command manager
    m_commandManager.registerAllCommandsForTarget(this);
    m_commandManager.setFirstCommandTarget(this);

    // initialize python editor
    m_fileChooser.addListener(listener);
    m_fileChooser.addListener(this);
    m_document.addListener(this);

    // set filename if one was specified
    if (filename != "")
        m_fileChooser.setCurrentFile(File(filename), true);

    // initialize MenuBarModel
    MenuBarModel::setApplicationCommandManagerToWatch(&m_commandManager);

    // initialize gui components
    Component::setOpaque(true);
    Component::addAndMakeVisible(&m_menu);
    Component::addAndMakeVisible(m_fileChooser);
    Component::addAndMakeVisible(m_editor);
    Component::addKeyListener(m_commandManager.getKeyMappings());
    Component::setSize(640, 480);

    // initialize default look and feel
    PythonEditor::lookAndFeelChanged();
}

PythonEditor::~PythonEditor()
{
    m_fileChooser.removeListener(this);
    stopMonitoring();
}

void PythonEditor::paint(Graphics& graphics)
{
    Colour color = getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground, Colours::lightgrey);
    graphics.fillAll(color);
}

void PythonEditor::resized()
{
    // calculate component bounds
    juce::Rectangle<int> local_bounds = getLocalBounds();
    juce::Rectangle<int> menu_bounds = local_bounds.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight());
    juce::Rectangle<int> body_bounds = local_bounds.reduced(8);
    juce::Rectangle<int> chooser_bounds = body_bounds.removeFromTop(25);
    juce::Rectangle<int> editor_bounds = body_bounds.withTrimmedTop(8);

    // apply bounds
    m_menu.setBounds(menu_bounds);
    m_fileChooser.setBounds(chooser_bounds);
    m_editor.setBounds(editor_bounds);
}

void PythonEditor::lookAndFeelChanged()
{
    LookAndFeel_V4* v4 = dynamic_cast<LookAndFeel_V4*>(&LookAndFeel::getDefaultLookAndFeel());
    if (v4) {
        const bool useLight = v4->getCurrentColourScheme() == LookAndFeel_V4::getLightColourScheme();
        m_editor.setColourScheme(useLight ? getLightCodeEditorColourScheme() : getDarkCodeEditorColourScheme());
    }
    else {
        m_editor.setColourScheme(m_tokeniser.getDefaultColourScheme());
    }
}

CodeEditorComponent::ColourScheme PythonEditor::getDarkCodeEditorColourScheme()
{
    CodeEditorComponent::ColourScheme cs;

    cs.set("Error", Colour(0xffe60000));
    cs.set("Comment", Colour(0xff72d20c));
    cs.set("Keyword", Colour(0xffee6f6f));
    cs.set("Operator", Colour(0xffc4eb19));
    cs.set("Identifier", Colour(0xffcfcfcf));
    cs.set("Integer", Colour(0xff42c8c4));
    cs.set("Float", Colour(0xff885500));
    cs.set("String", Colour(0xffbc45dd));
    cs.set("Bracket", Colour(0xff058202));
    cs.set("Punctuation", Colour(0xffcfbeff));
    cs.set("Preprocessor Text", Colour(0xfff8f631));

    return cs;
}

CodeEditorComponent::ColourScheme PythonEditor::getLightCodeEditorColourScheme()
{
    CodeEditorComponent::ColourScheme cs;

    cs.set("Error", Colour(0xffcc0000));
    cs.set("Comment", Colour(0xff00aa00));
    cs.set("Keyword", Colour(0xff0000cc));
    cs.set("Operator", Colour(0xff225500));
    cs.set("Identifier", Colour(0xff000000));
    cs.set("Integer", Colour(0xff880000));
    cs.set("Float", Colour(0xff885500));
    cs.set("String", Colour(0xff990099));
    cs.set("Bracket", Colour(0xff000055));
    cs.set("Punctuation", Colour(0xff004400));
    cs.set("Preprocessor Text", Colour(0xff660000));

    return cs;
}

Colour PythonEditor::getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback) noexcept
{
    LookAndFeel_V4* v4 = dynamic_cast<LookAndFeel_V4*>(&LookAndFeel::getDefaultLookAndFeel());
    if (v4)
        return v4->getCurrentColourScheme().getUIColour(uiColour);

    return fallback;
}

void PythonEditor::stopMonitoring()
{
    if (!m_monitorQuit && m_monitor.joinable()) {
        m_monitorQuit = true;
        m_monitor.join();
    }
}

void PythonEditor::startMonitoring()
{
    // turn off the quit flag
    m_monitorQuit = false;

    // create monitoring thread
    m_monitor = std::thread([this]() {
        static const uint32_t yield_interval = 25;
        static const uint32_t polling_interval = 2000;

        // poll until signal is sent to quit
        auto previous = std::chrono::steady_clock::now();
        while (!m_monitorQuit.load()) {
            // if polling interval has expired, check for file modification
            auto current = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(current - previous).count() >= polling_interval) {
                std::string filename = m_fileChooser.getCurrentFileText().toStdString();
                try {
                    // check file modification timestamp
                    if (std::filesystem::exists(m_fileChooser.getCurrentFileText().toStdString().c_str())) {
                        auto last_modified = std::filesystem::last_write_time(filename.c_str());
                        // if the file has been modified, asynchronously reload it in message manager thread
                        if (last_modified != m_lastModified) {
                            MessageManager::getInstanceWithoutCreating()->callAsync([this]() {
                                String content = m_fileChooser.getCurrentFile().loadFileAsString();
                                m_editor.loadContent(content);
                            });
                            m_lastModified = last_modified;
                        }
                    }
                    // reset polling interval
                    previous = current;
                }
                catch (...) {
                }
            }
            // yield thread execution
            std::this_thread::sleep_for(std::chrono::milliseconds(yield_interval));
        }
    });
}

PopupMenu PythonEditor::getMenuForIndex(int menuIndex, const String& menuName)
{
    PopupMenu menu;

    if (menuIndex == 0) {
        menu.addCommandItem(&m_commandManager, CommandIDs::MenuItemFileSave);
        menu.addCommandItem(&m_commandManager, CommandIDs::MenuItemFileSaveAs);
    }

    return menu;
}

void PythonEditor::getAllCommands(Array<CommandID>& c)
{
    Array<CommandID> commands{ MenuItemFileSave, MenuItemFileSaveAs };

    c.addArray(commands);
}

void PythonEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID) {
        case CommandIDs::MenuItemFileSave:
            result.setInfo("Save", "Saves the current file", "Menu", 0);
            result.setActive(m_fileChooser.getCurrentFileText() != "");
            result.addDefaultKeypress('s', ModifierKeys::ctrlModifier);
            break;
        case CommandIDs::MenuItemFileSaveAs:
            result.setInfo("Save As", "Saves the current file, prompting for filename", "Menu", 0);
            break;
    }
}

bool PythonEditor::perform(const InvocationInfo& info)
{
    auto save = [&](File file) -> bool {
        FileOutputStream stream(file);
        if (stream.openedOk()) {
            stream.setPosition(0);
            stream.truncate();
            stream.writeText(m_editor.getDocument().getAllContent(), false, false, nullptr);
            return true;
        }
        return false;
    };

    auto saveAs = [&]() {
        FileChooser fc(TRANS("Choose a new file"), m_fileChooser.getCurrentFileText(), "*.py");
        if (fc.browseForFileToSave(false) && save(fc.getResult()))
            m_fileChooser.setCurrentFile(fc.getResult(), true, dontSendNotification);
        menuItemsChanged();
        return;
    };

    switch (info.commandID) {
        case CommandIDs::MenuItemFileSave:
            save(m_fileChooser.getCurrentFile());
            break;
        case CommandIDs::MenuItemFileSaveAs:
            saveAs();
            break;
        default:
            return false;
    }

    return true;
}

void PythonEditor::filenameComponentChanged(FilenameComponent* filenameComponent)
{
    stopMonitoring();

    try {
        // read python source file and load into editor
        String content = filenameComponent->getCurrentFile().loadFileAsString();
        m_editor.loadContent(content);

        // update last modified timestamp
        m_lastModified = std::filesystem::last_write_time(m_fileChooser.getCurrentFileText().toStdString().c_str());
    }
    catch (...) {
    }

    startMonitoring();
}

void PythonEditor::codeDocumentTextChanged()
{
    // retrieve python code
    std::string script = m_editor.getDocument().getAllContent().toStdString();

    // execute python code
    m_pythonExecutor.execute(getFilename().c_str(), script.c_str());
}
