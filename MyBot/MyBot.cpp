#include "MyBot.h"   // Custom header for bot configuration (if any)
#include <dpp/dpp.h> // D++ library for Discord bot functionality
#include <dpp/nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>

/*
Place your bot token in the line below.
To create a bot token, follow the instructions here:
https://dpp.dev/creating-a-bot-application.html

Ensure that you invite the bot with both 'bot' and 'applications.commands' scopes.
Example invitation link (replace client_id with your bot's client ID):
https://discord.com/oauth2/authorize?client_id=YOUR_CLIENT_ID&scope=bot+applications.commands&permissions=139586816064
*/
const std::string BOT_TOKEN = ""; // Your bot token

// std string function for fetching wikipedia api
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::cout.write(static_cast<const char*>(contents), size * nmemb); // Print to stdout
    return size * nmemb;
}

std::string fetch(const std::string& topic) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://en.wikipedia.org/api/rest_v1/page/summary/" + topic;
        std::replace(url.begin(), url.end(), ' ', '_');

        std::cout << "Fetching URL: " << url << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // Set the callback
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Optional: Enable verbose output for debugging

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "cURL Easy Perform Failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Curl Init Fail" << std::endl;
    }

    return ""; // Return an empty string, since we don't have a buffer
}




// std string fucntion to extract summary

std::string extract(const std::string& jsonData) {
    std::cout << "Raw JSON: " << jsonData << std::endl;

    if (jsonData.empty()) {
        std::cerr << "Error: No data fetched from Wikipedia!" << std::endl;
        return "No information found.";
    }

    try {
        auto json = nlohmann::json::parse(jsonData);

        // Check if the response has an error message
        if (json.contains("title") && json["title"] == "Not found") {
            std::cerr << "Topic not found: " << json["title"] << std::endl;
            return "No information found.";
        }

        // Check for the "extract" key
        if (json.contains("extract")) {
            return json["extract"];
        }
        // If "extract" is missing, check if it might be in a different part of the JSON
        else {
            std::cerr << "Key 'extract' not found in JSON." << std::endl;
            std::cerr << "Full JSON response: " << json.dump(4) << std::endl; // Print full JSON for debugging
        }
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return "No information found.";
}





int main() {
    /*
    Create a bot cluster instance using the token.
    This cluster manages the bot's connection to Discord and handles events.
    */
    dpp::cluster bot(BOT_TOKEN);

    /*
    Set up basic logging to output events and errors to the console.
    This helps in debugging and tracking bot activity.
    */
    bot.on_log(dpp::utility::cout_logger());

    /*
    Event handler for when the bot is fully ready (connected to Discord and authenticated).
    This is where you register commands and perform other setup tasks.
    */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /*
        Use `run_once` to ensure that commands are registered only once, even if the bot reconnects.
        Without this, the commands could be registered multiple times on reconnects.
        */
        if (dpp::run_once<struct register_bot_commands>()) {
            /*
            Define a list of slash commands.
            In this example, we're registering a single "ping" command that replies with "Pong!".
            The `bot.me.id` is the bot's own ID, used to associate commands with this specific bot.
            */
            std::vector<dpp::slashcommand> commands{
                { "ping", "Ping pong!", bot.me.id },
                {"factcheck", "Checks facts on a topic.", bot.me.id}
                
                // Command name: "ping", description: "Ping pong!", bot's ID
            };

            /*
            Register the slash commands globally.
            This means they will be available in all servers the bot is part of.
            */
            commands[1].add_option(dpp::command_option(dpp::co_string, "topic", "The topic is a fact check", true));
            bot.global_bulk_command_create(commands);
        }
        });

    /*
    Event handler for slash commands.
    This checks if the command issued is "ping", and if so, replies with "Pong!".
    */
    bot.on_slashcommand([](const dpp::slashcommand_t& event) -> void {
        if (event.command.get_command_name() == "ping") {
            event.reply("Anorexic Piggy Bank!");  // Respond with "Pong!" to the "ping" command
        }
        else if (event.command.get_command_name() == "factcheck") {
            //event.thinking();
            std::string topic = std::get<std::string>(event.get_parameter("topic"));

            try
            {
                std::string wikiData = fetch(topic);
                std::string summary = extract(wikiData);
                if (!summary.empty()) {
                    event.reply(summary);
                }
                else {
                    event.reply("Sorry, we couldn't find information for this topic.");
                }
            }
            catch (const std::exception& e)
            {
                event.reply(std::string("An error has happen") + e.what());
            }
           
        }
        });

    /*
    Start the bot's event loop.
    This will keep the bot running and connected to Discord, listening for events (such as slash commands).
    The `st_wait` flag ensures the bot runs until manually stopped.
    */
    bot.start(dpp::st_wait);

    return 0;
}

//our fetch and extract files have many errors in them (JSON parsing is a major issue; check discord message history for the exact error)
//we need to locate everywhere where the API isn't getting info or where the parsing is inaccurate and fix it

//use wikipedia to make a factchecking bot
//we have to use the wikipedia.org/api
//we also need to use the queryswWw



//next week, we might need to change the way we're using the wikipedia api (it might not be being used correctly; use a tutorial online to figure out how to implement it)
//our fetch isn't working at all, we don't know why

//we are able to get the data from the writeCallback method, but we can't extract it properly. Must figure out how to fix next time