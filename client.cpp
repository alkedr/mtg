#include "client.hpp"


class Client::Impl {
private:

public:
	Game game;
	Game::PlayerId myId;

	Impl(std::string host, unsigned short port) {
	}

	void untapCard(Game::CardInGameId cardInGameId) {
	}

	void playCardFromHand(Game::CardInGameId cardInGameId) {
		game.playCardFromHand(myId, cardInGameId);

	}

	void activateAbility(Game::CardInGameId cardInGameId) {
		game.activateAbility(myId, cardInGameId);

	}

	void declareAttacker(Game::CardInGameId attacker, Game::Target target) {
		game.declareAttacker(attacker, target);

	}

	void declareBlocker(Game::CardInGameId blocker, Game::CardInGameId attacker) {
		game.declareBlocker(blocker, attacker);

	}

	void pass() {
		game.pass(myId);

	}
};

Client::Client(std::string host, unsigned short port): pimpl(new Impl(host, port)) {
}

Client::~Client() {
}


const Game & Client::game() const { return pimpl->game; }
Game::PlayerId Client::myId() const { return pimpl->myId; }

void Client::untapCard(Game::CardInGameId cardInGameId) { pimpl->untapCard(cardInGameId); onUpdate(); }
void Client::playCardFromHand(Game::CardInGameId cardInGameId) { pimpl->playCardFromHand(cardInGameId); onUpdate(); }
void Client::activateAbility(Game::CardInGameId cardInGameId) { pimpl->activateAbility(cardInGameId); onUpdate(); }
void Client::declareAttacker(Game::CardInGameId attacker, Game::Target target) { pimpl->declareAttacker(attacker, target); }
void Client::declareBlocker(Game::CardInGameId blocker, Game::CardInGameId attacker) { pimpl->declareBlocker(blocker, attacker); }
void Client::pass() { pimpl->pass(); onUpdate(); }

void Client::onUpdate() {
	if (game().turn().priorityPlayerId == myId()) onPriority();
}
