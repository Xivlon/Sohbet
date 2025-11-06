#pragma once

#include <string>
#include <set>
#include <vector>
#include <regex>

namespace sohbet {
namespace utils {

class TextParser {
public:
    // Extract hashtags from text (e.g., #study #programming)
    static std::set<std::string> extractHashtags(const std::string& text) {
        std::set<std::string> hashtags;
        // Match hashtags: # followed by alphanumeric characters and underscores
        std::regex hashtag_regex(R"(#([a-zA-Z0-9_]+))");
        std::sregex_iterator iter(text.begin(), text.end(), hashtag_regex);
        std::sregex_iterator end;

        while (iter != end) {
            std::string tag = (*iter)[1].str();
            // Convert to lowercase for consistency
            std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
            hashtags.insert(tag);
            ++iter;
        }

        return hashtags;
    }

    // Extract user mentions from text (e.g., @username)
    static std::set<std::string> extractMentions(const std::string& text) {
        std::set<std::string> mentions;
        // Match mentions: @ followed by alphanumeric characters and underscores
        std::regex mention_regex(R"(@([a-zA-Z0-9_]+))");
        std::sregex_iterator iter(text.begin(), text.end(), mention_regex);
        std::sregex_iterator end;

        while (iter != end) {
            std::string username = (*iter)[1].str();
            mentions.insert(username);
            ++iter;
        }

        return mentions;
    }

    // Make text clickable by wrapping hashtags and mentions in HTML
    static std::string makeClickable(const std::string& text) {
        std::string result = text;

        // Replace hashtags with links
        std::regex hashtag_regex(R"(#([a-zA-Z0-9_]+))");
        result = std::regex_replace(result, hashtag_regex,
            "<a href=\"/hashtags/$1\" class=\"hashtag\">#$1</a>");

        // Replace mentions with links
        std::regex mention_regex(R"(@([a-zA-Z0-9_]+))");
        result = std::regex_replace(result, mention_regex,
            "<a href=\"/users/$1\" class=\"mention\">@$1</a>");

        return result;
    }
};

} // namespace utils
} // namespace sohbet
