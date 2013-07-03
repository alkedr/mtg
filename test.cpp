#include "magic.hpp"

#include <iostream>


namespace {


#define CHECK(COND) if ((COND) == false) { std::cout << "FAIL: " << #COND << std::endl; }
#define CHECK_EQUAL(X, Y) if ((X) != (Y)) { std::cout << "FAIL: " << #X << " != " << #Y << "  " << (X) << " != " << (Y) << std::endl; }


static void print_sizes() {
	std::cout << "sizeof(Game): " << sizeof(Game) << std::endl;
	std::cout << "sizeof(Game::Turn): " << sizeof(Game::Turn) << std::endl;
	std::cout << "sizeof(Game::Player): " << sizeof(Game::Player) << std::endl;
	std::cout << "sizeof(Game::Stack): " << sizeof(Game::Stack) << std::endl;
	std::cout << "sizeof(Game::Card): " << sizeof(Game::Card) << std::endl;
}

static void test_simple() {
	Game game;
	game.players().emplace_back();
	game.players().emplace_back();

	game.player(0).library = { 1, 2, 3, 4, 5, 6 };

	game.start(0);

	CHECK_EQUAL( game.card(0).id(), 6 );
	CHECK_EQUAL( game.card(1).id(), 5 );
	CHECK_EQUAL( game.card(2).id(), 4 );
	CHECK_EQUAL( game.card(3).id(), 3 );
	CHECK_EQUAL( game.card(4).id(), 2 );
	CHECK_EQUAL( game.card(5).id(), 1 );

	game.playCardFromHand(0, 5);

	game.activateAbility(0, 5);
	CHECK( game.player(0).manaPool.s.at(0) == std::make_pair(Color::WHITE, (short)1) );

	game.playCardFromHand(0, 1);

	game.activateAbility(0, 1);
	CHECK( game.player(0).manaPool.s.at(1) == std::make_pair(Color::GREEN, (short)1) );

	game.playCardFromHand(0, 0);
	CHECK( game.player(0).manaPool.s.at(0) == std::make_pair(Color::WHITE, (short)0) );
	CHECK( game.player(0).manaPool.s.at(1) == std::make_pair(Color::GREEN, (short)0) );

	game.pass(0);
	game.pass(1);
}

}


int main() {
	print_sizes();
	test_simple();
}
