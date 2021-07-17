#pragma once

#include "filesystem.h"
#include "processedcommand.h"
#include "sourcetype.h"
#include "translateconfig.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <map>
#include <sstream>
#include <string>
#include <vector>

enum class TaskState {
    NotCalculated,
    Raw, // Input file: nothing to do
    Fresh,
    DirtyReady,
    DirtyWaiting,
    Done,
};

inline std::string join(std::string a, std::string b) {
    if (a.empty()) {
        return b;
    }
    else if (b.empty()) {
        return a;
    }
    return a + " " + b;
}

enum class BuildLocation : size_t {
    Real,         // Build results that should be part of the final product
    Intermediate, // Object files and others not part of final product
    Count,        // Put last, counts possibilities

    Inherit, // Select depending on target
};

class Task {
public:
    using TimePoint = filesystem::file_time_type;

    Task() = default;
    Task(const class Json &json) {
        parse(json);
    }
    Task(const Task &) = delete;
    Task(Task &&) = default;
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = default;

    std::string name() const {
        if (_name.empty()) {
            return _out.string();
        }
        else {
            return _name;
        }
    }

    void name(std::string value) {
        _name = std::move(value);
    }

    filesystem::path out() const {
        if (_out.empty()) {
            return {};
        }
        else if (*_out.begin() == ".") {
            return _out;
        }
        else {
            auto path = dir() / _out;
            auto ext = extensionFromCommandType(_command, flagStyle());
            if (!ext.empty()) {
                path.replace_extension(ext);
            }
            return path;
        }
    }

    filesystem::path rawOut() const {
        return _out;
    }

    void out(filesystem::path path) {
        _out = path;
    }

    void pushIn(Task *in) {
        if (!in) {
            return;
        }
        if (std::find(_in.begin(), _in.end(), in) == _in.end()) {
            _in.push_back(in);
            if (in->shouldLinkFile()) {
                _triggers.push_back(in);
            }
            in->parent(this);
            in->addSubscriber(this);
        }
    }

    void pushTrigger(Task *trigger) {
        if (!trigger) {
            return;
        }
        else if (std::find(_triggers.begin(), _triggers.end(), trigger) ==
                 _triggers.end()) {
            _triggers.push_back(trigger);
            trigger->parent(this);
        }
    }

    auto &in() const {
        return _in;
    }

    auto &triggers() const {
        return _triggers;
    }

    std::string concatIn() const {
        if (_in.empty()) {
            return {};
        }

        std::string ret;

        for (auto &in : _in) {
            auto fname = in->out();
            auto ext = fname.extension();
            if (!in->shouldLinkFile()) {
                continue;
            }
            //            else if (ext == ".gch" || ext == ".pch" || ext == "")
            //            {
            //                fname.replace_extension("");
            //                ret += " -include " + fname.string() + " ";
            //            }
            else {
                ret += fname.string() + " ";
            }
        }

        return ret;
    }

    void parent(Task *parent) {
        _parent = parent;
    }

    auto parent() {
        return _parent;
    }

    auto parent() const {
        return _parent;
    }

    //! Find the originating source file for this file
    //! This is assumed to be run on ".o" files so that there is no ambiguity
    Task *findSource() {
        if (!_in.empty()) {
            return _in.front()->findSource();
        }
        else if (_state == TaskState::Raw) {
            return this;
        }
        else {
            return nullptr;
        }
    }

    void dir(BuildLocation loc, filesystem::path dir) {
        _dir.at(static_cast<size_t>(loc)) = dir;
    }

    //! Returns the output path
    //! If buildLocation() is set to use intermediate location that is
    //! chosen when traversing the tree
    //! External users should not specify the variable loc
    filesystem::path dir(BuildLocation loc = BuildLocation::Inherit) const {
        if (loc == BuildLocation::Inherit) {
            loc = _buildLocation;
        }

        auto dir = [this, loc] {
            if (loc == BuildLocation::Real) {
                return _dir.front();
            }

            auto &dir =
                _dir.at(static_cast<size_t>(BuildLocation::Intermediate));

            // If no intermediate location exist, just use the real instead
            if (dir.empty()) {
                return _dir.front();
            }

            return dir;
        }();

        if (_parent) {
            if (dir.empty()) {
                return _parent->dir(loc);
            }
            return _parent->dir(loc) / dir;
        }
        else {
            return dir;
        }
    }

    void buildLocation(BuildLocation value) {
        _buildLocation = value;
    }

    std::string property(std::string name) const {
        if (name.empty()) {
            return {};
        }
        if (name.rfind("parent.") == 0) {
            if (_parent) {
                return _parent->property(name.substr(7));
            }
        }
        else if (name.front() == '.') {
            return ::extension(name, flagStyle());
        }
        else if (name == "command") {
            return _command;
        }
        else if (name == "out") {
            return out().string();
        }
        else if (name == "dir") {
            return dir().string();
        }
        else if (name == "depfile") {
            auto d = depfile().string();
            if (!d.empty()) {
                return depprefix() + d;
            }
        }
        else if (name == "in") {
            return concatIn();
        }
        else if (name == "src") {
            if (!_in.empty()) {
                return _in.front()->out().string();
            }
        }
        else if (name == "c++") {
            return cxx().string();
        }
        else if (name == "cc") {
            return cc().string();
        }
        else if (name == "static" || name == "ar") {
            return ar().string();
        }
        else if (name == "flags") {
            return join(flags(), config());
        }
        else if (name == "ldflags") {
            return ldflags();
        }
        else if (name == "eflags") {
            return eflags();
        }
        else if (name == "modules") {
            return modulesString();
        }
        else if (name == "includes") {
            return includes();
        }
        else if (name == "standard") {
            return standard();
        }
        return {};
    }

    std::string includes() const {
        if (_parent) {
            auto parentIncludes = parent()->includes();
            auto includeStr = concatIncludes();
            if (parentIncludes.empty()) {
                return includeStr;
            }
            if (includeStr.empty()) {
                return parentIncludes;
            }
            return parentIncludes + " " + includeStr;
        }
        else {
            return concatIncludes();
        }
    }

    std::string concatIncludes() const {
        if (_includes.empty()) {
            return {};
        }
        std::ostringstream ss;

        auto includePrefix = ::includePrefix(flagStyle());

        for (auto &i : _includes) {
            ss << includePrefix << i << " ";
        }

        auto str = ss.str();

        if (!str.empty() && str.back() == ' ') {
            str.pop_back();
        }
        return str;
    }

    void pushInclude(std::string includes) {
        _includes.push_back(includes);
    }

    void depfile(filesystem::path path) {
        _depfile = path;
    }

    filesystem::path depfile() const {
        if (!_depfile.empty()) {
            return dir() / _depfile;
        }
        else {
            return {};
        }
    }

    void generateDepName() {
        if (!_out.empty()) {
            _depfile = _out.string() + ".d";
        }
    }

    void command(std::string command) {
        shouldLinkFile(command != "[copy]");
        _command = command;
    }

    std::string command() const {
        if (_command.empty()) {
            if (_parent) {
                return _parent->command();
            }
        }
        else {
            if (_command.front() == '[' && _command.back() == ']') {
                return commandAt(_command.substr(1, _command.size() - 2));
            }
            else {
                return _command;
            }
        }
        throw std::runtime_error{"no command specified for target " + name()};
    }

    void commands(std::map<std::string, std::string> commands) {
        _commands = std::move(commands);
    }

    const std::map<std::string, std::string> &commands() const {
        if (_commands.empty() && _parent) {
            return _parent->commands();
        }
        return _commands;
    }

    // Get a single command from the commands-list
    std::string commandAt(std::string name) const {
        if (name == "none") {
            return "";
        }
        else if (name == "test") {
            return commandAt("exe");
        }
        else if (auto f = _commands.find(name); f != _commands.end()) {
            return f->second;
        }
        else if (_parent) {
            return _parent->commandAt(name);
        }
        return name;
    }

    std::string extension() const {
        auto command = this->command();
        if (!command.empty()) {
            extensionFromCommandType(command, flagStyle());
        }
        return {};
    }

    void cxx(filesystem::path cxx) {
        _cxx = cxx;
    }

    filesystem::path cxx() const {
        if (!_cxx.empty()) {
            return _cxx;
        }
        else if (_parent) {
            return _parent->cxx();
        }
        else {
            return {};
        }
    }

    void cc(filesystem::path cc) {
        _cc = cc;
    }

    filesystem::path cc() const {
        if (!_cc.empty()) {
            return _cc;
        }
        else if (_parent) {
            return _parent->cc();
        }
        else {
            return {};
        }
    }

    void ar(filesystem::path ar) {
        _ar = ar;
    }

    filesystem::path ar() const {
        if (!_ar.empty()) {
            return _ar;
        }
        else if (_parent) {
            return _parent->ar();
        }
        else {
            return {};
        }
    }

    TimePoint changedTime() {
        if (!_isChangedTimeCurrent) {
            updateChangedTime();
        }
        return _changedTime;
    }

    bool exists() {
        return filesystem::exists(out());
    }

    TaskState state() {
        return _state;
    }

    bool isDirty() {
        updateState();
        return _state != TaskState::Fresh && _state != TaskState::Raw;
    }

    void setState(TaskState state) {
        _state = state;
    }

    // Let this task know that one of its in-tasks is ready
    void subscribtionNotice(Task *task) {
        auto it = std::remove(_triggers.begin(), _triggers.end(), task);
        _triggers.erase(it, _triggers.end());
        if (_triggers.empty()) {
            if (_state == TaskState::DirtyWaiting) {
                _state = TaskState::DirtyReady;
            }
        }
    }

    bool isModule() {
        return getType(_out) == SourceType::PrecompiledModule;
    }

    std::string modulesString() const {
        std::ostringstream ss;

        for (auto &in : in()) {
            if (in->isModule()) {
                ss << translateString(TranslatableString::IncludeModuleString,
                                      flagStyle())
                   << in->out() << " ";
            }
        }

        return ss.str();
    }

    std::vector<Task *> subscribers() {
        return _subscribers;
    }

    void parse(const class Json &jtask);

    void addSubscriber(Task *task) {
        _subscribers.push_back(task);
    }

    void updateChangedTime() {
        auto filename = out();
        if (filesystem::exists(filename)) {
            _changedTime = filesystem::last_write_time(filename);
        }
        else {
            _changedTime = {};
        }
        _isChangedTimeCurrent = true;
    }

    std::string ldflags() const {
        if (_ldflags.empty() && _parent) {
            return _parent->ldflags();
        }

        return _ldflags;
    }

    void ldflags(std::string value) {
        _ldflags = value;
    }

    //! Flags used when preprocessing files
    std::string eflags() const {
        if (_eflags.empty() && _parent) {
            return _parent->eflags();
        }

        return _eflags;
    }

    void eflags(std::string value) {
        _eflags = value;
    }

    void config(std::vector<std::string> value) {
        _config = value;
    }

    std::string config() const {
        std::ostringstream ss;
        if (_parent) {
            auto parentConfig = _parent->config();
            if (!parentConfig.empty()) {
                ss << parentConfig << " ";
            }
        }

        auto flagStyle = this->flagStyle();

        for (auto &c : _config) {
            ss << translateConfig(c, flagStyle) << " ";
        }

        auto extraConfig = commandSpecificConfig(_command, flagStyle);
        if (!extraConfig.empty()) {
            ss << extraConfig << " ";
        }

        auto str = ss.str();
        if (!str.empty() && isspace(str.back())) {
            str.pop_back();
        }
        return str;
    }

    FlagStyle flagStyle() const {
        if (_flagStyle == FlagStyle::Inherit && _parent) {
            return _parent->flagStyle();
        }

        return _flagStyle;
    }

    void flagStyle(FlagStyle value) {
        _flagStyle = value;
    }

    void flagStyle(std::string value) {
        if (value == "msvc") {
            _flagStyle = FlagStyle::Msvc;
        }
        else {
            _flagStyle = FlagStyle::Gcc;
        }
    }

    //! In some cases you only need to know the c++-standard
    //! Notice the reverse order, because we want to catch the latest
    //! config that is specified
    std::string standard() const {
        auto f = std::find_if(_config.rbegin(), _config.rend(), [](auto &x) {
            return x.rfind("c++") != std::string::npos;
        });

        if (f != _config.rend()) {
            return translateConfig(*f, flagStyle());
        }
        else if (_parent) {
            return _parent->standard();
        }

        return {};
    }

    std::string flags() const {
        if (_flags.empty() && _parent) {
            return _parent->flags();
        }

        return _flags;
    }

    void flags(std::string flags) {
        _flags = flags;
    }

    std::string depprefix() const {
        if (_depprefix.empty() && _parent) {
            return _parent->depprefix();
        }

        return _depprefix;
    }

    void depprefix(std::string value) {
        _depprefix = value;
    }

    bool isRoot() {
        return _command == "[root]";
    }

    bool isTest() {
        return _command == "[test]";
    }

    //! Remove triggers that is raw or fresh
    void pruneTriggers() {
        auto it =
            std::remove_if(_triggers.begin(), _triggers.end(), [](auto &x) {
                return x->state() == TaskState::Fresh ||
                       x->state() == TaskState::Raw;
            });

        _triggers.erase(it, _triggers.end());
    }

    // Todo: This needs some unit tests
    void updateState() {
        if (_state != TaskState::NotCalculated) {
            return;
        }

        updateChangedTime();

        if (_in.empty()) {
            if (!exists()) {
                if (_out.empty()) {
                    throw std::runtime_error{"target " + name() +
                                             " does specify 'out'"};
                }
                else {
                    throw std::runtime_error{"input file " + out().string() +
                                             " not found"};
                }
            }
            _state = TaskState::Raw;
            for (auto &t : _triggers) {
                t->subscribtionNotice(this);
            }
            return;
        }

        for (auto &in : _in) {
            if (this == in) {
                throw std::runtime_error{name() +
                                         " is trying to import itself"};
            }

            in->updateState();
            if (in->state() == TaskState::Raw) {
                if (in->changedTime() > changedTime()) {
                    _state = TaskState::DirtyReady;
                }
            }
            else if (in->isDirty()) {
                _state = TaskState::DirtyWaiting;
                return; // No need to check more if other task is blocking
            }
            else if (in->changedTime() >= changedTime()) {
                _state = TaskState::DirtyReady;
                return;
            }
        }

        if (!_out.empty() && !exists()) {
            _state = TaskState::DirtyReady;
            return;
        }

        for (auto &trigger : _triggers) {
            if (trigger->isDirty() || trigger->changedTime() >= changedTime()) {
                _state = TaskState::DirtyReady;
                return;
            }
        }

        if (_state == TaskState::NotCalculated) {
            _state = TaskState::Fresh;
        }
    }

    //! @return true if removed something false else
    bool clean() const {
        if (_state == TaskState::Raw) {
            return false;
        }

        bool removed = false;

        {
            auto d = depfile();
            if (filesystem::exists(d)) {
                filesystem::remove(d);
                removed = true;
            }
        }
        {
            auto o = out();
            if (filesystem::exists(o)) {
                filesystem::remove(o);
                removed = true;
            }
        }

        return removed;
    }

    //! IF the file should be used in its parent task at input
    void shouldLinkFile(bool value) {
        _shouldLinkFile = value;
    }

    bool shouldLinkFile() {
        return _shouldLinkFile;
    }

    Json dump();

    //! Print tree view from node
    void print(bool verbose = false, size_t indentation = 0);

private:
    Task *_parent = nullptr;
    filesystem::path _out;
    std::array<filesystem::path, static_cast<size_t>(BuildLocation::Count)>
        _dir;
    filesystem::path _depfile;
    filesystem::path _cxx;
    filesystem::path _cc;
    filesystem::path _ar;
    std::string _name;                            // If empty-same as out
    std::string _command;                         // If empty use parents
    std::map<std::string, std::string> _commands; // Parents build command
    std::string _flags;
    std::string _ldflags;
    std::string _eflags;
    std::string _depprefix;
    std::vector<std::string> _includes;
    std::vector<std::string> _config;
    FlagStyle _flagStyle = FlagStyle::Inherit;
    BuildLocation _buildLocation = BuildLocation::Real;

    // Fixed during connection step
    std::vector<Task *> _in; // Files that needs to be built before this file

    // Others used to calculate state
    std::vector<Task *> _triggers; // Files that mark this task as dirty
    std::vector<Task *> _subscribers;
    TimePoint _changedTime;
    bool _isChangedTimeCurrent = false;
    bool _shouldLinkFile = true;
    TaskState _state = TaskState::NotCalculated;
};
