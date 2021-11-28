#include <filesystem>
#include <sstream>
#include <string>

#define CROW_MAIN
#include <crow_all.h>

#include "article.h"

constexpr std::uint16_t PORT_NUMBER = 19283;

int main()
{
    auto executablePath = std::filesystem::current_path();
    auto articlesPath = executablePath;
    articlesPath += "/articles";

    auto articles = buildIndexForArticles(articlesPath);

    // TODO: Implement simple keyword search

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([&articles]()
    {
        std::stringstream content;
        for(const auto& [key, article] : articles)
        {
            content << "<p>";

            // appendLinkToArticle
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
        ctx["title"] = "Articles and Projects";
        ctx["date"] = "";

        return page.render(ctx);
    });

    CROW_ROUTE(app, "/projects/<string>/<string>/<string>/<string>/<string>/<string>")
    ([&articles](const std::string& projectName, const std::string& year, const std::string& month,
                 const std::string& day, const std::string& nr, const std::string& filename)
    {
        auto key = buildArticleKey(year, month, day, nr, filename, projectName);
        auto [content, title, date] = retrieve(articles, key);

        auto page = crow::mustache::load("display_article.html");
        crow::mustache::context ctx;
        ctx["content"] = content;
        ctx["title"] = title;
        ctx["date"] = date;

        return page.render(ctx);
    });

    CROW_ROUTE(app, "/articles/<string>/<string>/<string>/<string>/<string>")
    ([&articles](const std::string& year, const std::string& month, const std::string& day,
                 const std::string& nr, const std::string& filename)
    {
       auto key = buildArticleKey(year, month, day, nr, filename);
       auto [content, title, date] = retrieve(articles, key);

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
