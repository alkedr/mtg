#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/preprocessor.hpp>




class ECardNotFound : public std::exception {
public:
	virtual const char * what() const noexcept { return "card not found"; }
};
class ECardClassNotFound : public std::exception {
	/*const Card::Id cardId;*/ 
public: 
	ECardClassNotFound(/*const Card::Id _cardId*/) /*: cardId(_cardId)*/ {} 
};
class ETooMuchLandsPerTurn : public std::exception {
public: 
	virtual const char * what() const noexcept { return "too much lands per turn"; }
};
class EWrongPhase : public std::exception {
public: 
	virtual const char * what() const noexcept { return "wrong phase"; }
};
class EWrongZone : public std::exception {
public: 
	virtual const char * what() const noexcept { return "wrong zone"; }
};
class ENotYourTurn : public std::exception {
public: 
	virtual const char * what() const noexcept { return "not your turn"; }
};


enum Color : unsigned char {
	  COLORLESS = 0
	, WHITE = (1 << 0)
	, BLUE =  (1 << 1)
	, BLACK = (1 << 2)
	, RED =   (1 << 3)
	, GREEN = (1 << 4)
	
	, ALL = WHITE | BLUE | BLACK | RED | GREEN
};


typedef std::vector< std::pair<Color, short int> > Cost;


class ManaPool { 
public: 
	Cost s; 

	void add(Color color, short int count = 1) {
		s.emplace_back(color, count);
	} 
	void subtract(const Cost & cost) {
		short int colorlessCount = 0;
		for (const auto & pair : cost) {
			if (pair.first == Color::COLORLESS) {
				colorlessCount += pair.second;
			} else {
				auto it = std::find_if(s.begin(), s.end(), [&](const std::pair<Color, short int> & p) { return p.first == pair.first; } );
				if (it == s.end()) throw "not enough mana";
				it->second -= pair.second;
				if (it->second < 0) throw "not enough mana";
			}
		}
		for (auto & pair : s) {
			int a = std::min(colorlessCount, pair.second);
			pair.second -= a;
			colorlessCount -= a;
		}
		if (colorlessCount > 0) throw "not enough mana";
	}

};



class Game {

public:

	class Player;

	typedef unsigned char PlayerId;
	typedef unsigned char TeamId;
	typedef size_t CardInGameId;

	class Effect {
	public:
		CardInGameId source;
	};

class Card {
	
public:

	typedef unsigned char SetId;
	typedef unsigned char CardInSetId;
	typedef unsigned short Id;

	enum Type : unsigned char {
		ARTIFACT,
		CREATURE,
		ENCHANTMENT,
		INSTANT,
		LAND,
		PLANESWALKER,
		SORCERY
	}; 
	enum Rarity : unsigned char {
		COMMON,
		UNCOMMON,
		RARE,
		MYTHIC_RARE
	}; 
	enum Position : unsigned char {
		LIBRARY,
		HAND,
		STACK,
		BATTLEFIELD,
		GRAVEYARD,
		EXILE
	};

public:

	Game & game;
	PlayerId ownerId;
	Position position;
	bool tapped_;

public:

	void setPosition(Position newPosition) {
		position = newPosition;
	}

	Card(Game & _game, PlayerId _ownerId)
	: game(_game)
	, ownerId(_ownerId)
	, tapped_(false)
	{
	}

	void moveToBattlefield() {
		setPosition(Position::BATTLEFIELD);
	}

	bool tapped() const { return tapped_; }
	void tap() { tapped_ = true; }
	void untap() { tapped_ = false; }

	virtual ~Card() {}


	// specific for card type

	// specific for card
	virtual Id getId() const = 0;
	virtual const char * getName() const = 0;
	virtual const char * getDescription() const = 0;
	virtual Type getType() const = 0;

	virtual const std::string getImageName() const {
		return "images/" + std::to_string(getId()) + ".jpg";
	}

	virtual Color getColor() const { return Color::COLORLESS; }


	virtual void playFromHand() = 0;
	virtual void activateAbility() {/* throw "card doesn't have abilities";*/}

	virtual void beforeTap() {}
	virtual void afterTap() {}
	virtual void beforeUntap() {}
	virtual void afterUntap() {}

};
class Player { 
public: 
	typedef unsigned short HP;

	std::vector<Card::Id> library;
	ManaPool manaPool;
	HP hp;
	bool passed;
	bool loser;

	Player() : hp(20), passed(false), loser(false) {
	} 
};
class Stack {

	class Entry {

		Card & card;
		void (Card::* const method)();

	public:

		Entry(Card & _card, void (Card::* const _method)())
		: card(_card)
		, method(_method)
		{
		}

		void resolve()
		{
			(card.*method)();
		}

	};

	std::vector<Entry> s;

public:

	Stack() {
	} 
	void push(Card & card, void (Card::* const method)()) {
		s.push_back(Entry(card, method));
	}
	void resolve() {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		while (s.empty() == false) {
			s.back().resolve();
			s.pop_back();
		}
	}
	bool empty() const {
		return s.empty();
	}
};
class Team {

	typedef std::vector<std::pair<PlayerId, Player>> Storage;

	Storage s;

public:

	typedef unsigned char Id;


	typedef Storage::iterator iterator;
	typedef Storage::const_iterator const_iterator;
	typedef Storage::reverse_iterator reverse_iterator;
	typedef Storage::const_reverse_iterator const_reverse_iterator;


	iterator begin() { return s.begin(); }
	iterator end() { return s.end(); }
	reverse_iterator rbegin() { return s.rbegin(); }
	reverse_iterator rend() { return s.rend(); }
	const_iterator begin() const { return s.begin(); }
	const_iterator end() const { return s.end(); }
	const_reverse_iterator rbegin() const { return s.rbegin(); }
	const_reverse_iterator rend() const { return s.rend(); }

	Storage::size_type size() const { return s.size(); }

	bool empty() const { return s.empty(); }


	Team()
	{
	}

};
class Turn {
public:
	enum Phase : unsigned char {
		UNTAP,
		UPKEEP,
		DRAW_CARD,
		FIRST_MAIN,
		COMBAT_BEGIN,
		COMBAT_ATTACK,
		COMBAT_BLOCK,
		COMBAT_DAMAGE,
		COMBAT_END,
		SECOND_MAIN,
		END,
		CLEANUP
	}; 

	static const char * phaseToString(Phase phase) {
		static const char * s[] = {
			[UNTAP] = "untap",
			[UPKEEP] = "upkeep",
			[DRAW_CARD] = "draw card",
			[FIRST_MAIN] = "first main",
			[COMBAT_BEGIN] = "combat begin",
			[COMBAT_ATTACK] = "combat attack",
			[COMBAT_BLOCK] = "combat block",
			[COMBAT_DAMAGE] = "combat damage",
			[COMBAT_END] = "combat end",
			[SECOND_MAIN] = "second main",
			[END] = "end",
			[CLEANUP] = "cleanup",
		};

		return s[phase];
	}


	PlayerId activePlayerId;
	PlayerId priorityPlayerId;
	Phase phase;
	// TODO: history


	Turn() : activePlayerId(0), priorityPlayerId(0), phase(FIRST_MAIN) {
	}
};

	static std::unique_ptr<Card> newCard(const Card::Id cardId, Game & game, PlayerId ownerId);
	static Card * newCardHelper(const Card::Id cardId, Game & game, Game::PlayerId ownerId);

	bool allPlayersPassed() const {
		return std::all_of(
			players.begin(),
			players.end(),
			[&](const Player & p) {
				return p.passed;
			}
		);
	}
	void startPhase(Turn::Phase phase) {
		switch (phase) {
			case (Turn::Phase::UNTAP): {
				for (std::unique_ptr<Card> & pCard : cards) {
					if (pCard->ownerId == turn.activePlayerId) pCard->untap();
				}
				goToNextPhase();
				break;
			}
			case (Turn::Phase::UPKEEP): {
				goToNextPhase();
				break;
			}
			case (Turn::Phase::DRAW_CARD): {
				drawCard(turn.activePlayerId);
				goToNextPhase();
				break;
			}
			case (Turn::Phase::FIRST_MAIN): {
				break;
			}
			case (Turn::Phase::COMBAT_BEGIN): {
				break;
			}
			case (Turn::Phase::COMBAT_ATTACK): {
				break;
			}
			case (Turn::Phase::COMBAT_BLOCK): {
				break;
			}
			case (Turn::Phase::COMBAT_DAMAGE): {
				break;
			}
			case (Turn::Phase::COMBAT_END): {
				break;
			}
			case (Turn::Phase::SECOND_MAIN): {
				break;
			}
			case (Turn::Phase::END): {
				break;
			}
			case (Turn::Phase::CLEANUP): {
				break;
			}
		}
	}
	void endPhase(Turn::Phase phase) {
		switch (phase) {
			case (Turn::Phase::UNTAP): {
				break;
			}
			case (Turn::Phase::UPKEEP): {
				break;
			}
			case (Turn::Phase::DRAW_CARD): {
				break;
			}
			case (Turn::Phase::FIRST_MAIN): {
				break;
			}
			case (Turn::Phase::COMBAT_BEGIN): {
				break;
			}
			case (Turn::Phase::COMBAT_ATTACK): {
				break;
			}
			case (Turn::Phase::COMBAT_BLOCK): {
				break;
			}
			case (Turn::Phase::COMBAT_DAMAGE): {
				break;
			}
			case (Turn::Phase::COMBAT_END): {
				break;
			}
			case (Turn::Phase::SECOND_MAIN): {
				break;
			}
			case (Turn::Phase::END): {
				break;
			}
			case (Turn::Phase::CLEANUP): {
				landsPlayedThisTurn = 0;
				break;
			}
		}
	}
	void goToNextPhase() {
		endPhase(turn.phase);
		turn.phase++;
		if (turn.phase > Turn::Phase::CLEANUP) {
			turn.phase = Turn::Phase::UNTAP;
			turn.activePlayerId = 1 - turn.activePlayerId;
			turn.priorityPlayerId = turn.activePlayerId;
		}
		startPhase(turn.phase);
	}
	void lose(PlayerId playerId) {
		player(playerId).loser = true;
	}
	void drawCard(PlayerId playerId) {
		if (player(playerId).library.empty()) {
			lose(playerId);
		} else {
			Card::Id cardId = player(playerId).library.back();
			player(playerId).library.pop_back();
			cards.push_back( newCard(cardId, *this, playerId) );
			cards.back()->position = Card::Position::HAND;
		}
	}
	void clearPlayerPassFlag() {
		for (Player & player : players) player.passed = false;
	}

public:

	Turn turn;
	std::vector<Player> players;    // TODO: vector<unique_ptr>  or const
	std::vector<std::unique_ptr<Card>> cards;  // all cards allocated here,  CardInGameId - index in this vector  DO NOT ERASE! DO NOT FREE! DO NOT NULL!
	Stack stack;

	unsigned char maxLandsPerTurn;
	unsigned char landsPlayedThisTurn;

public:

	Game() : maxLandsPerTurn(1), landsPlayedThisTurn(0) {
	}

	// getters
	Player & player(PlayerId id) {
		return players.at(id);
	}
	const Player & player(PlayerId id) const {
		return players.at(id);
	}
	std::unique_ptr<Card> & card(CardInGameId cardInGameId) {
		return cards.at(cardInGameId);
	}

	void start(PlayerId firstPlayer) {
		turn.activePlayerId = firstPlayer;
		for (PlayerId i = 0; i < players.size(); i++) {
			for (int j = 0; j < 7; j++) drawCard(i);
		}
	}

	// actions of player
	void playCardFromHand(PlayerId playerId, CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << std::endl; 
		// TODO: check errors 
		std::unique_ptr<Card> & pCard = card(cardInGameId);
		if (pCard->ownerId != playerId) throw "not your card";
		pCard->playFromHand();
		clearPlayerPassFlag();
	} 
	void tap(PlayerId playerId, CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << std::endl; 
		if (card(cardInGameId)->ownerId != playerId) throw "not your card";
		card(cardInGameId)->activateAbility();
		clearPlayerPassFlag();
	} 
	void pass(PlayerId playerId) {
		player(playerId).passed = true;
		if (allPlayersPassed()) {
			if (stack.empty()) 
				goToNextPhase();
			else
				stack.resolve();
			clearPlayerPassFlag();
			turn.priorityPlayerId = turn.activePlayerId;
		} else {
			turn.priorityPlayerId = 1 - turn.priorityPlayerId;
		}
	}


template <Card::Id _id> class CardHelper : public Card {

public:

	virtual Type getType() const override { return Type::LAND; }

	virtual Id getId() const override { return _id; }
	virtual const char * getName() const { return "Unknown Card"; }
	virtual const char * getDescription() const { return "Unknown"; }

	virtual const std::string getImageName() const { return "dsgfd"; }

	virtual void playFromHand() override {}

	virtual void activateAbility() override {}

	CardHelper(Game & game, PlayerId ownerId)
	: Card(game, ownerId) {}
};


#define CARD_BEGIN(BASE, ID, NAME, DESCRIPTION)                                                           \
template<> class Game::CardHelper<ID> : public BASE {                                                              \
public:                                                                                                               \
	CardHelper<ID>(Game & game, PlayerId & ownerId)                                    \
	: BASE(game, ownerId) {}                                                                           \
	GETTER_SIMPLE(Id, ID)                                                                                             \
	GETTER_TYPE_SIMPLE(const char *, Name, NAME)                                                                      \
	GETTER_TYPE_SIMPLE(const char *, Description, DESCRIPTION)

#define CARD_END  };



#define OVERRIDE(RETURN_VALUE, FUNCTION_NAME, ATTRIBUTES, ...) virtual RETURN_VALUE FUNCTION_NAME(__VA_ARGS__) ATTRIBUTES override final

#define GETTER_TYPE(TYPE_NAME, PROPERTY_NAME) OVERRIDE(TYPE_NAME, get ## PROPERTY_NAME, const, )
#define GETTER_TYPE_SIMPLE(TYPE_NAME, PROPERTY_NAME, VALUE) GETTER_TYPE(TYPE_NAME, PROPERTY_NAME) { return VALUE; }

#define GETTER(PROPERTY_NAME) GETTER_TYPE(PROPERTY_NAME, PROPERTY_NAME)
#define GETTER_SIMPLE(PROPERTY_NAME, VALUE) GETTER_TYPE_SIMPLE(PROPERTY_NAME, PROPERTY_NAME, VALUE)

#define COLOR(VALUE) GETTER_SIMPLE(Color, VALUE)
#define POWER(VALUE) GETTER_SIMPLE(Power, VALUE)
#define TOUGHNESS(VALUE) GETTER_SIMPLE(Toughness, VALUE)

#define COST(...) GETTER_SIMPLE(Cost, Cost { __VA_ARGS__ })

#define AFTER_TAP OVERRIDE(void, afterTap, , )




class Artifact : public Card {

public:

	virtual Type getType() const override { return Type::ARTIFACT; }

};
class Creature : public Card {

public:

	typedef short Power;
	typedef short Toughness;


	virtual Type getType() const override { return Type::CREATURE; }


	//virtual bool isFlying(const Player & blockerOwner, const Creature & blocker) const { return false; }
	virtual bool isUnblocable() const { return false; }

	virtual Power getPower() const = 0;
	virtual Toughness getToughness() const = 0;

	virtual Cost getCost() const = 0;

	Player & owner() { return game.player(ownerId); }

	virtual void playFromHand()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		owner().manaPool.subtract(getCost());
		game.stack.push(*this, &Card::moveToBattlefield);
		position = Position::STACK;
	}


	Creature(Game & game, PlayerId ownerId)
	: Card(game, ownerId) {}
};
class Enchantment : public Card {

public:

	virtual Type getType() const override { return Type::ENCHANTMENT; }

	virtual void playFromHand()
	{
		//game.stack.push(new PlayCardEffect(*this));
		position = Position::STACK;
	}
};
class Instant : public Card {

public:

	virtual Type getType() const override { return Type::INSTANT; }

	virtual void playFromHand()
	{
		//game.stack.push(new PlayCardEffect(*this));
		position = Position::STACK;
	}
};
class Land : public Card {

public:

	virtual Type getType() const override { return Type::LAND; }

	Player & owner() { return game.player(ownerId); }

	virtual void playFromHand() {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (position != Card::Position::HAND) throw EWrongZone();
		if (ownerId != game.turn.activePlayerId) throw ENotYourTurn();
		if ((game.turn.phase != Turn::Phase::FIRST_MAIN) && (game.turn.phase != Turn::Phase::SECOND_MAIN)) throw EWrongPhase();
		if (game.landsPlayedThisTurn >= game.maxLandsPerTurn) throw ETooMuchLandsPerTurn();
		game.landsPlayedThisTurn++;
		position = Position::BATTLEFIELD;
	}

	virtual void activateAbility() override {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		tapped_ = true;
		afterTap();
	}


	Land(Game & game, PlayerId ownerId)
	: Card(game, ownerId) {}
};
class Planeswalker : public Card {

public:

	virtual Type getType() const override { return Type::PLANESWALKER; }

	virtual void playFromHand() {
		//game.stack.push(new PlayCardEffect(*this));
		position = Position::STACK;
	}
};
class Sorcery : public Card {

public:

	virtual Type getType() const override { return Type::SORCERY; }

	virtual void playFromHand() {
		//game.stack.push(new PlayCardEffect(*this));
		position = Position::STACK;
	}
};

};





CARD_BEGIN(Game::Land,  40, "Plains", "Plains")
	AFTER_TAP { owner().manaPool.add(Color::WHITE); }
CARD_END


CARD_BEGIN(Game::Land, 258, "Island", "Island")
	AFTER_TAP { owner().manaPool.add(Color::BLUE); }
CARD_END


CARD_BEGIN(Game::Land,  78, "Swamp", "Swamp")
	AFTER_TAP { owner().manaPool.add(Color::BLACK); }
CARD_END


CARD_BEGIN(Game::Land,  77, "Mountain", "Mountain")
	AFTER_TAP { owner().manaPool.add(Color::RED); }
CARD_END


CARD_BEGIN(Game::Land, 273, "Forest", "Forest")
	AFTER_TAP { owner().manaPool.add(Color::GREEN); }
CARD_END




CARD_BEGIN(Game::Creature, 268, "Grizzly Bears", "")
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