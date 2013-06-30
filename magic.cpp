#include "magic.hpp"

#include <boost/preprocessor.hpp>


CARD_BEGIN(Game::Land,   1, "Plains", "Plains")
	AFTER_TAP { owner().manaPool.add(Color::WHITE); }
CARD_END


CARD_BEGIN(Game::Land,   2, "Island", "Island")
	AFTER_TAP { owner().manaPool.add(Color::BLUE); }
CARD_END


CARD_BEGIN(Game::Land,   3, "Swamp", "Swamp")
	AFTER_TAP { owner().manaPool.add(Color::BLACK); }
CARD_END


CARD_BEGIN(Game::Land,   4, "Mountain", "Mountain")
	AFTER_TAP { owner().manaPool.add(Color::RED); }
CARD_END


CARD_BEGIN(Game::Land,   5, "Forest", "Forest")
	AFTER_TAP { owner().manaPool.add(Color::GREEN); }
CARD_END




CARD_BEGIN(Game::Creature,   6, "Grizzly Bears", "")
	POWER(2)
	TOUGHNESS(2)
	virtual Cost getCost() const {
		return Cost {
			{ Color::COLORLESS, 1 },
			{ Color::GREEN, 1 }
		};
	}
	/*COST(
		{ Color::COLORLESS, 1 },
		{ Color::GREEN, 1 }
	)*/
CARD_END




#define CARD(Z, ID, DATA) case(DATA+ID): { return new Game::CardHelper<DATA+ID>(game, ownerId); }

Game::Card * Game::newCardHelper(const Game::Card::Id cardId, Game & game, Game::PlayerId ownerId) {
	switch (cardId) {
		BOOST_PP_REPEAT(255, CARD, 0)
		BOOST_PP_REPEAT(255, CARD, 255)
		default: { return new Game::CardHelper<0>(game, ownerId); }
	}
}
std::unique_ptr<Game::Card> Game::newCard(const Game::Card::Id cardId, Game & game, Game::PlayerId ownerId) {
	return std::move(std::unique_ptr<Game::Card>(newCardHelper(cardId, game, ownerId)));
}

#undef CARD
