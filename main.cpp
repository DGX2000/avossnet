#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#define CROW_MAIN
#include <crow_all.h>

using ArticleDictionary = std::map<std::string, std::filesystem::path>;
using ProjectDictionary = std::map<std::string, ArticleDictionary>;

int main()
{
    auto parentPath = std::filesystem::current_path().parent_path();
    auto articlesPath = parentPath;
    articlesPath += "/articles";
    auto templatesPath = parentPath;
    templatesPath += "/templates";

    ArticleDictionary articles;
    ProjectDictionary projects;

    for(const auto& article : std::filesystem::directory_iterator(articlesPath))
    {
        if(article.is_directory())
        {
            ArticleDictionary articlesInProject;
            for(const auto& articleInProject : std::filesystem::directory_iterator(article.path()))
            {
                articlesInProject.insert({articleInProject.path().filename(), articleInProject.path()});
            }
            projects.insert({article.path().filename(), articlesInProject});
        }
        else
        {
            articles.insert({article.path().filename(), article.path()});
        }
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([]()
    {
       return "Home";
    });

    CROW_ROUTE(app, "/articles/<int>/<int>/<int>/<string>")
    ([templatesPath](int year, int month, int day, const std::string& filename)
    {
       auto page = crow::mustache::load("display_article.html");

       crow::mustache::context ctx;

       ctx["date"] = "testdate";
       ctx["content"] = "<b>TEST CONTENT</b>";

       return page.render(ctx);
       //return std::to_string(year+month+day) + " " + filename;
    });

    app.port(18080).multithreaded().run();

    return 0;
}
