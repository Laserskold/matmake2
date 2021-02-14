#pragma once

#include "filesystem.h"
#include "processedcommand.h"
#include "sourcetype.h"
#include "translateconfig.h"
#include <algorithm>
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
            auto ext = extensionFromCommandType(_command, _flagStyle);
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
            _triggers.push_back(in);
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
            if (ext == ".gch" || ext == ".pch" || ext == "") {
                fname.replace_extension("");
                ret += " -include " + fname.string() + " ";
            }
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

    void dir(filesystem::path dir) {
        _dir = dir;
    }

    filesystem::path dir() const {
        if (_parent) {
            return _parent->dir() / _dir;
        }
        else {
            return _dir;
        }
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
            return ::extension(name, _flagStyle);
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
        else if (name == "flags") {
            return join(flags(), config());
        }
        else if (name == "ldflags") {
            return ldflags();
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
        if (_includes.empty() && _parent) {
            return parent()->includes();
        }
        else {
            return concatIncludes();
        }
    }

    std::string concatIncludes() const {
        std::ostringstream ss;

        auto includePrefix = ::includePrefix(_flagStyle);

        for (auto &i : _includes) {
            ss << includePrefix << i << " ";
        }

        return ss.str();
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
        else {
            throw std::runtime_error{"could not find " + name + " on target " +
                                     this->name()};
        }
    }

    std::string extension() const {
        auto command = this->command();
        if (!command.empty()) {
            extensionFromCommandType(command, _flagStyle);
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
                ss << "-fmodule-file=" << in->out() << " ";
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

    void config(std::vector<std::string> value) {
        _config = value;
    }

    std::string config() const {
        std::ostringstream ss;
        if (_parent) {
            ss << _parent->config();
        }

        for (auto &c : _config) {
            ss << translateConfig(c, _flagStyle) << " ";
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
                throw std::runtime_error{"input file " + out().string() +
                                         " not found"};
            }
            _state = TaskState::Raw;
            for (auto &t : _triggers) {
                t->subscribtionNotice(this);
            }
            return;
        }

        for (auto &in : _in) {
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

    Json dump();

    //! Print tree view from node
    void print(bool verbose = false, size_t indentation = 0);

private:
    Task *_parent = nullptr;
    filesystem::path _out;
    filesystem::path _dir;
    filesystem::path _depfile;
    filesystem::path _cxx;
    std::string _name;                            // If empty-same as out
    std::string _command;                         // If empty use parents
    std::map<std::string, std::string> _commands; // Parents build command
    std::string _flags;
    std::string _ldflags;
    std::string _depprefix;
    std::vector<std::string> _includes;
    std::vector<std::string> _config;
    FlagStyle _flagStyle = FlagStyle::Inherit;

    // Fixed during connection step
    std::vector<Task *> _in; // Files that needs to be built before this file

    // Others used to calculate state
    std::vector<Task *> _triggers; // Files that mark this task as dirty
    std::vector<Task *> _subscribers;
    TimePoint _changedTime;
    bool _isChangedTimeCurrent = false;
    TaskState _state = TaskState::NotCalculated;
};
