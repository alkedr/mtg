#pragma once

#include "magic.hpp"

#include <memory>


class Client {
public:

	Client(std::string host, unsigned short port);
	virtual ~Client();
	
	const Game & game() const;
	Game::PlayerId myId() const;

	// actions of player
	void untapCard(Game::CardInGameId cardInGameId);
	void playCardFromHand(Game::CardInGameId cardInGameId);
	void activateAbility(Game::CardInGameId cardInGameId);
	void declareAttacker(Game::CardInGameId attacker, Game::Target target);
	void declareBlocker(Game::CardInGameId blocker, Game::CardInGameId attacker);
	void pass();

protected:
	virtual void onPriority() {}

private:
	void onUpdate();

	class Impl;
	std::unique_ptr<Impl> pimpl;
};

