#ifndef ARTICLE_H
#define ARTICLE_H

#include <filesystem>
#include <map>
#include <string>

static std::string readWholeFile(const std::string& filename);

struct Article
{
    std::string path;
    std::string year;
    std::string month;
    std::string day;
    std::string number;
    std::string name;
    std::string projectName;
    std::string linkSuffix;

    static Article fromFilename(const std::string& filename);
    static std::string transformFilenameToTitle(const std::string& filename);
};


using ArticleDictionary = std::map<std::string, Article>;
ArticleDictionary buildIndexForArticles(const std::filesystem::path& path);

struct ArticleData
{
    std::string content;
    std::string title;
    std::string date;
};
ArticleData retrieve(const ArticleDictionary& articles, const std::string& key);
std::string buildArticleKey(const std::string& year, const std::string& month, const std::string& day,
                            const std::string& nr, const std::string& filename, const std::string& projectName = "");

#endif // ARTICLE_H
