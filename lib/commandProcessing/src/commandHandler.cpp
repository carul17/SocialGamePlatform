#include "commandHandler.h"

void CommandHandler::initializeMaps() {
    initializeCommandMap();
    initializeCommandResultMap();
}

std::deque<Message>
CommandHandler::getOutgoingMessages(const std::deque<ProcessedMessage> &incomingProcessedMessages) {
    outgoing.clear();

    for (auto processedMessage : incomingProcessedMessages) {
        User user = processedMessage.user;

        if (processedMessage.isCommand) {
            CommandResult commandResult = executeCommand(processedMessage);
            if (commandResult != CommandResult::SUCCESS) {
                outgoing.push_back({user, commandResultMap[commandResult]});
            }
        } else {
            if (globalState.isInGame(user)) {
                globalState.registerUserGameInput(user, processedMessage.input);
            } else if(globalState.isInLobby(user)) {
                broadcastLobbyMessage(processedMessage);
            } else {
                outgoing.push_back({user, commandResultMap[CommandResult::ERROR_NO_USERNAME]});
            }
        }
    }
    return outgoing;
}

std::deque<Message> 
CommandHandler::handleLostUsers(std::vector<User> &users) {
    outgoing.clear();
    for (auto user: users) {
        ProcessedMessage processedMessage{true, user, UserCommand::EXIT, std::vector<std::string>{}, ""};
        executeCommand(processedMessage);
    }
    users.clear();
    return outgoing;
}

CommandResult
CommandHandler::executeCommand(ProcessedMessage &processedMessage) {

    if (processedMessage.commandType != UserCommand::USERNAME && globalState.getName(processedMessage.user) == "error") {
        return CommandResult::ERROR_NO_USERNAME;    // TODO: Fix this into more informative message
    }

    return commandMap[processedMessage.commandType]->execute(processedMessage);
}

void CommandHandler::broadcastLobbyMessage(ProcessedMessage &processedMessage) {
    std::stringstream outgoingText;
    outgoingText << globalState.getName(processedMessage.user) << " : " << processedMessage.input << "\n";

    std::deque<Message> messages = globalState.buildMessagesForServerLobby(outgoingText.str());
    outgoing.insert(outgoing.end(), messages.begin(), messages.end());
}

void CommandHandler::registerCommand(UserCommand userCommand, commandPointer commandPointer) {
    commandMap[userCommand] = std::move(commandPointer);
}

void CommandHandler::initializeCommandMap() {
    registerCommand(UserCommand::CREATE, std::make_unique<CreateGameCommand>(globalState, outgoing));
    registerCommand(UserCommand::START, std::make_unique<StartGameCommand>(globalState, outgoing));
    registerCommand(UserCommand::JOIN, std::make_unique<JoinGameCommand>(globalState, outgoing));
    registerCommand(UserCommand::LEAVE, std::make_unique<LeaveGameCommand>(globalState, outgoing));
    registerCommand(UserCommand::END, std::make_unique<EndGameCommand>(globalState, outgoing));
    registerCommand(UserCommand::HELP, std::make_unique<ListHelpCommand>(globalState, outgoing));
    registerCommand(UserCommand::GAMES, std::make_unique<ListGamesCommand>(globalState, outgoing));
    registerCommand(UserCommand::EXIT, std::make_unique<ExitServerCommand>(globalState, outgoing));
    registerCommand(UserCommand::USERNAME, std::make_unique<UserNameCommand>(globalState, outgoing));
}

void CommandHandler::initializeCommandResultMap() {
    commandResultMap[CommandResult::ERROR_INCORRECT_COMMAND_FORMAT] = "Incorrect Command Format.\nPlease enter: <command> <arguments>. To list all commands, enter: help\n\n";
    commandResultMap[CommandResult::ERROR_INVALID_GAME_INDEX] = "Invalid Game Index.\nTo list all available games, enter: games\n\n";
    commandResultMap[CommandResult::ERROR_INVALID_INVITATION_CODE] = "Invalid invitation code entered: No game found.\n\n";
    commandResultMap[CommandResult::ERROR_GAME_HAS_STARTED] = "The game has already started.\n\n";
    commandResultMap[CommandResult::ERROR_NOT_AN_OWNER] = "Only an owner can start or end the game.\n\n";
    commandResultMap[CommandResult::ERROR_OWNER_CANNOT_LEAVE] = "Owner cannot leave their game. To end game for all players, enter : end\n\n";
    commandResultMap[CommandResult::ERROR_OWNER_CANNOT_JOIN_FROM_SAME_DEVICE] = "Owner cannot join the game from same window. Please open up a new window to join the game!\n\n";
    commandResultMap[CommandResult::ERROR_INVALID_COMMAND] = "Invalid Command!\n\n";
    commandResultMap[CommandResult::ERROR_GAME_NOT_STARTED] = "The game has not been started yet!\n\n";
    commandResultMap[CommandResult::ERROR_NOT_ENOUGH_PLAYERS] = "Game cannot be started, more players needed!\n\n";
    commandResultMap[CommandResult::ERROR_NO_USERNAME] = "To proceed, enter: name <your game name>\n\n";

    commandResultMap[CommandResult::SUCCESS_GAME_CREATION] = "Game Successfully Created with invitation Code : \nPlease enter \"start\" to start the game.\n\n";
    commandResultMap[CommandResult::SUCCESS_GAME_JOIN] = "Game successfully joined. Waiting for the owner to start\n\n";
    commandResultMap[CommandResult::SUCCESS_GAME_START] = "Game Started!\n\n";
    commandResultMap[CommandResult::SUCCESS_GAME_END] = "Game Ended. All players have been moved to server lobby.\n\n";
    commandResultMap[CommandResult::SUCCESS_GAME_LEAVE] = "Game Successfully left.\n\n";
    commandResultMap[CommandResult::SUCCESS_USERNAME] = "Name Successfully assigned.\n\n";

    commandResultMap[CommandResult::STRING_SERVER_HELP] =
        "\n"
        "  * To view the available games enter: games\n"
        "  * To create a new game from the options enter: create <game_index>\n"
        "  * To join a game with an invitation code enter: join <invitation_code>\n"
        "  * To exit the server enter: exit\n"
        "  * To display help information again enter: help\n\n";
    commandResultMap[CommandResult::STRING_INGAME_PLAYER_HELP] =
        "\n"
        "  * To leave the game enter: leave\n"
        "  * To exit the server enter: exit\n\n";
    commandResultMap[CommandResult::STRING_INGAME_OWNER_HELP] =
        "\n"
        "  * To start the game enter: start\n"
        "  * To end the game enter: end\n"
        "  * To exit the server enter: exit\n\n";
}