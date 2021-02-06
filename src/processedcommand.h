#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

class ProcessedCommand {
public:
    ProcessedCommand(std::string command) {
        auto f = size_t{0};
        auto old = size_t{0};

        while ((f = command.find('{', old)) != std::string::npos) {
            segments.push_back({
                .value = command.substr(old, f - old),
                .isReference = false,
            });

            if ((old = command.find('}', f)) != std::string::npos) {
                segments.push_back({
                    .value = command.substr(f + 1, old - f - 1),
                    .isReference = true,
                });
                ++old;
            }
            else {
                break;
            }
        }
    }
    ProcessedCommand(const ProcessedCommand &) = default;
    ProcessedCommand(ProcessedCommand &&) = default;
    ProcessedCommand &operator=(const ProcessedCommand &) = default;
    ProcessedCommand &operator=(ProcessedCommand &&) = default;

    template <class T>
    std::string expand(const T &task) {
        std::ostringstream ss;

        for (auto s : segments) {
            if (s.isReference) {
                ss << task.property(s.value);
            }
            else {
                ss << s.value;
            }
        }

        return ss.str();
    }

    std::string expand(const std::map<std::string, std::string> values) {
        std::ostringstream ss;

        for (auto s : segments) {
            if (s.isReference) {
                ss << values.at(s.value);
            }
            else {
                ss << s.value;
            }
        }

        return ss.str();
    }

private:
    struct Segment {
        std::string value;
        bool isReference = false;
    };

    std::vector<Segment> segments;
};
