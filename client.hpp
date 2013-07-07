#include "magic.hpp"


class Client : private Game {
public:

	Client(std::string host, unsigned short port);
	
	//const Game & game() const { return game_; }

	const Player & player(PlayerId id) const { return Game::player(id); }
	const std::vector<Player> & players() const { return Game::players(); }

	const std::vector<std::unique_ptr<Card>> & cards() const { return Game::cards(); }

	const Turn & turn() const { return Game::turn(); }

	// actions of player
	void untapCard();
	void playCardFromHand(CardInGameId cardInGameId);
	void activateAbility(CardInGameId cardInGameId);
	void pass();

	virtual void onPriority() {}
};

