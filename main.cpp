#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#define CROW_MAIN
#include <crow_all.h>


constexpr std::uint16_t PORT_NUMBER = 19283;

std::string readWholeFile(const std::string& filename)
{
    std::fstream article(filename);
    std::stringstream stream;
    stream << article.rdbuf();
    return stream.str();
}

std::string transformFilenameToTitle(const std::string& filename)
{
    auto title = filename;
    for(auto& character : title)
    {
        if(character == '_')
        {
            character = ' ';
        }
    }
    return title;
}

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

    static Article fromFilename(const std::string& filename)
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
};

using ArticleDictionary = std::map<std::string, Article>;
// TODO: Function buildIndexForArticles(...)

int main()
{
    auto executablePath = std::filesystem::current_path();
    auto articlesPath = executablePath;
    articlesPath += "/articles";

    ArticleDictionary articles;

    for(const auto& article : std::filesystem::directory_iterator(articlesPath))
    {
        if(article.is_directory())
        {
            auto projectName = article.path().filename().string();
            for(const auto& articleInProject : std::filesystem::directory_iterator(article.path()))
            {
                auto articleFilename = articleInProject.path().filename().string();

                auto articleObject = Article::fromFilename(articleFilename);
                articleObject.path = articleInProject.path().string();
                articleObject.projectName = transformFilenameToTitle(projectName);

                articles.insert({articleFilename + projectName, articleObject});
            }
        }
        else
        {
            auto articleFilename = article.path().filename().string();

            auto articleObject = Article::fromFilename(articleFilename);
            articleObject.path = article.path().string();

            articles.insert({articleFilename, articleObject});
        }
    }

    crow::SimpleApp app;

    // TODO: List project and articles
    CROW_ROUTE(app, "/")
    ([&articles]()
    {
        std::stringstream content;
        for(const auto& [key, article] : articles)
        {
            content << "<p>";

            auto link = std::string();
            auto linkText = std::string();
            if(article.projectName.empty())
            {
                link += "/articles";
            }
            else
            {
                link += "/projects/" + article.projectName;
                linkText += "Project " + article.projectName + ": ";
            }
            link += article.linkSuffix;
            linkText += article.name;

            // Adds link to opening tag
            content << "<a href=\"" << link << "\">";
            content << linkText << "</a>";

            content << "</p>\n";
        }

        auto page = crow::mustache::load("display_article.html");
        crow::mustache::context ctx;
        ctx["content"] = content.str();
        ctx["title"] = "Home";
        ctx["date"] = "";

        return page.render(ctx);
    });

    // TODO: "/projects/name/YYYY/MM/DD/NR/name" for project routing

    CROW_ROUTE(app, "/articles/<string>/<string>/<string>/<string>/<string>")
    ([&articles](const std::string& year, const std::string& month, const std::string& day,
                 const std::string& nr, const std::string& filename)
    {
       auto key = year + month + day + "_" + nr + "_" + filename;
       auto path = articles.find(key);
       auto content = std::string("The article could not be found.");
       if(path != articles.end())
       {
            content = readWholeFile(path->second.path);
       }

       auto title = transformFilenameToTitle(filename);
       auto date = year + "-" + month + "-" + day;

       auto page = crow::mustache::load("display_article.html");
       crow::mustache::context ctx;
       ctx["content"] = content;
       ctx["title"] = title;
       ctx["date"] = date;

       return page.render(ctx);
    });



    app.port(PORT_NUMBER).multithreaded().run();

    return 0;
}
