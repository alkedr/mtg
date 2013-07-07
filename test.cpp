#include "magic.hpp"
#include "gtest/gtest.h"
#include <map>


namespace {

class GameTest : public ::testing::Test {
protected:

	void allPass() {
		Game::Turn::Phase phase = game.turn().phase;
		while (game.turn().phase == phase) {
			game.pass(game.turn().priorityPlayerId);
		}
	}
	
	void allPassUntilPhase(Game::Turn::Phase phase) {
		while (game.turn().phase != phase) {
			allPass();
		}
	}

	void allPassUntilNextTurn() {
		allPass();
		allPassUntilPhase(Game::Turn::Phase::UNTAP);
	}


	void checkEverything() {
		checkCards();
		checkTurn();
		checkPlayers();
	}


	struct Player {
		std::string name;

		void check(const Game::Player & player) const {
			if (!name.empty()) EXPECT_EQ( name, player.name );
		}
	};

	void playerChanged(Game::PlayerId id, struct Player player) {
		Player & p = players[id];
		if (!player.name.empty()) p.name = player.name;
		checkPlayers();
	}

	void playersChanged(std::vector<std::pair<Game::PlayerId, Player>> v) {
		for (const auto & pair : v) playerChanged(pair.first, pair.second);
		checkPlayers();
	}

	void checkPlayers() {
		for (const auto & pair : players) pair.second.check(game.player(pair.first));
	}


	struct Card {
		Game::Card::Id id;
		Game::PlayerId ownerId;
		Game::Card::Position position;

		void check(const Game::Card & card) const {
			if (id != 0) EXPECT_EQ( id, card.id() );
			if (ownerId != 0) EXPECT_EQ( ownerId, card.ownerId() );
			if (position != 0) EXPECT_EQ( position, card.position() );
		}
	};

	void cardChanged(Game::CardInGameId id, Card card) {
		Card & c = cards[id];
		if (card.id != 0) c.id = card.id;
		if (card.ownerId != 0) c.ownerId = card.ownerId;
		if (card.position != 0) c.position = card.position;
		checkCards();
	}

	void cardsChanged(std::vector<std::pair<Game::CardInGameId, Card>> v) {
		for (const auto & pair : v) cardChanged(pair.first, pair.second);
		checkCards();
	}

	void checkCards() {
		for (const auto & pair : cards) pair.second.check(game.card(pair.first));
	}

	struct Turn {
		Game::PlayerId activePlayerId;
		Game::PlayerId priorityPlayerId;
		Game::Turn::Phase phase;

		void check(Game::Turn t) {
			if (activePlayerId != 0) EXPECT_EQ( activePlayerId, t.activePlayerId );
			if (priorityPlayerId != 0) EXPECT_EQ( priorityPlayerId, t.priorityPlayerId );
			if (phase != 0) EXPECT_EQ( phase, t.phase );
		}
	};

	void turnChanged(Turn data) {
		if (data.phase != 0) turn.phase = data.phase;
		checkTurn();
	}

	void checkTurn() {
		turn.check(game.turn());
	}


	GameTest() {
		turn = Turn{ .activePlayerId = 0, .priorityPlayerId = 0, .phase = (Game::Turn::Phase)0 };
	}

	std::map<Game::PlayerId, Player> players;
	std::map<Game::CardInGameId, Card> cards;
	Turn turn;

	Game game;
};

TEST_F(GameTest, GrizzlyBears) {
	// add players
	auto player1Id = game.addPlayer("Player1", { 1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1 });
	auto player2Id = game.addPlayer("Player2", { 1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1 });

	playersChanged({
	  { player1Id, { .name = "Player1" } }
	, { player2Id, { .name = "Player2" } }
	});

	// start game
	game.start(player1Id);

	cardsChanged({
	  { 0, { .id = 1, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 1, { .id = 2, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 2, { .id = 3, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 3, { .id = 4, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 4, { .id = 5, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 5, { .id = 6, .ownerId = player1Id, .position = Game::Card::Position::HAND } }
	, { 6, { .id = 6, .ownerId = player1Id, .position = Game::Card::Position::HAND } }

	, { 7, { .id = 1, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 8, { .id = 2, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 9, { .id = 3, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 10, { .id = 4, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 11, { .id = 5, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 12, { .id = 6, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	, { 13, { .id = 6, .ownerId = player2Id, .position = Game::Card::Position::HAND } }
	});

	turnChanged({ .activePlayerId = player1Id, .priorityPlayerId = player1Id, .phase = Game::Turn::Phase::UNTAP });


	// turn 1, player 1
	allPassUntilPhase(Game::Turn::Phase::FIRST_MAIN);
	turnChanged({ .activePlayerId = player1Id, .priorityPlayerId = player1Id, .phase = Game::Turn::Phase::FIRST_MAIN });

	// play forest
	game.playCardFromHand(player1Id, 4);
	cardChanged(4, { .position = Game::Card::Position::BATTLEFIELD });

	allPassUntilNextTurn();


	// turn 1, player 2
	allPassUntilPhase(Game::Turn::Phase::FIRST_MAIN);
	turnChanged({ .activePlayerId = player2Id, .priorityPlayerId = player2Id, .phase = Game::Turn::Phase::FIRST_MAIN });

	// play plains
	game.playCardFromHand(player2Id, 8);
	cardChanged(8, { .position = Game::Card::Position::BATTLEFIELD });

	// try to play same land (plains) twice
	EXPECT_THROW( game.playCardFromHand(player2Id, 8), EWrongZone );
	checkEverything();

	// try to play second land (island)
	EXPECT_THROW( game.playCardFromHand(player2Id, 9), ETooMuchLandsPerTurn ) << "played second land";
	checkEverything();
}


TEST_F(GameTest, Phases) {
	auto player1Id = game.addPlayer("Player1", { 1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1 });
	game.addPlayer("Player2", { 1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1 });

	game.start(player1Id);

	turnChanged({ .phase = Game::Turn::Phase::UNTAP });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::UPKEEP });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::DRAW_CARD });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::FIRST_MAIN });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::COMBAT_BEGIN });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::COMBAT_ATTACK });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::COMBAT_BLOCK });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::COMBAT_DAMAGE });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::COMBAT_END });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::SECOND_MAIN });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::END });
	allPass();  turnChanged({ .phase = Game::Turn::Phase::CLEANUP });
}


}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

