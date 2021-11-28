#include "article.h"

#include <algorithm>
#include <fstream>

std::string readWholeFile(const std::string &filename)
{
    std::fstream article(filename);
    std::stringstream stream;
    stream << article.rdbuf();
    return stream.str();
}

Article Article::fromFilename(const std::string &filename)
{
    Article article;

    // Format: YYYYMMDD_NR_Name
    article.year = filename.substr(0, 4);
    article.month = filename.substr(4, 2);
    article.day = filename.substr(6, 2);
    article.number = filename.substr(9, 2);
    article.name = transformFilenameToTitle(filename.substr(12));

    // Link suffix is the /YYYY/MM/DD/NR/name part that is same for loose articles and articles in projects
    article.linkSuffix = "/" + article.year + "/" + article.month + "/" + article.day + "/" +
                         article.number + "/" + filename.substr(12);

    return article;
}

std::string Article::transformFilenameToTitle(const std::string &filename)
{
    auto title = filename;
    std::replace_if(title.begin(), title.end(), [](char c) { return c == '_'; }, ' ');
    return title;
}

ArticleData retrieve(const ArticleDictionary &articles, const std::string &key)
{
    auto value = articles.find(key);
    if(value == articles.end())
    {
        return ArticleData{"Article could not be found.", "404", ""};
    }

    auto article = value->second;
    auto content = readWholeFile(article.path);
    auto title = article.name;
    auto date = article.year + "-" + article.month + "-" + article.day;

    return {content, title, date};
}

std::string buildArticleKey(const std::string &year, const std::string &month, const std::string &day, const std::string &nr, const std::string &filename, const std::string &projectName)
{
    return year + month + day + "_" + nr + "_" + filename + projectName;
}

ArticleDictionary buildIndexForArticles(const std::filesystem::__cxx11::path &path)
{
    ArticleDictionary articles;

    for(const auto& file : std::filesystem::directory_iterator(path))
    {
        auto filename = file.path().filename().string();

        if(file.is_directory())
        {
            for(const auto& fileInProject : std::filesystem::directory_iterator(file.path()))
            {
                auto articleFilename = fileInProject.path().filename().string();

                auto articleObject = Article::fromFilename(articleFilename);
                articleObject.path = fileInProject.path().string();
                articleObject.projectName = Article::transformFilenameToTitle(filename);

                articles.insert({articleFilename + filename, articleObject});
            }
        }
        else
        {
            auto articleObject = Article::fromFilename(filename);
            articleObject.path = file.path().string();

            articles.insert({filename, articleObject});
        }
    }

    return articles;
}
