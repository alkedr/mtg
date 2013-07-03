#pragma once

#include <memory>
#include <string>
#include <vector>


// temporary {
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
	virtual const char * what() const noexcept { return "too much lands per turn_"; }
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
	virtual const char * what() const noexcept { return "not your turn_"; }
};
class ENotYourPriority : public std::exception {
public:
	virtual const char * what() const noexcept { return "not your priority"; }
};
class EWrongCardOwner : public std::exception {
public:
	virtual const char * what() const noexcept { return "wrong card owner"; }
};
class EWrongCardType : public std::exception {
public:
	virtual const char * what() const noexcept { return "wrong card type"; }
};
class ENotEnoughMana : public std::exception {
public:
	virtual const char * what() const noexcept { return "not enough mana"; }
};
class EAlreadyTapped : public std::exception {
public:
	virtual const char * what() const noexcept { return "already tapped"; }
};
// }




enum Color : unsigned char {
	  COLORLESS = 0
	, WHITE = (1 << 0)
	, BLUE =  (1 << 1)
	, BLACK = (1 << 2)
	, RED =   (1 << 3)
	, GREEN = (1 << 4)

	, ALL = WHITE | BLUE | BLACK | RED | GREEN
};




class Game {
public:
	class Impl;


	typedef std::vector< std::pair<Color, short int> > Cost;

	class ManaPool {
	public:
		Cost s;

		void add(Color color, short int count = 1);
		void subtract(const Cost & cost);
	};


	class Player;

	typedef unsigned char PlayerId;
	typedef size_t CardInGameId;


	class Effect {
	public:
		Effect(CardInGameId source);
		virtual ~Effect();

		CardInGameId source() const;

		void resolve(Impl & impl);

		virtual void onResolve(Impl & impl) {}

		// TODO: on*

	private:
		CardInGameId source_;
	};

	typedef std::vector<std::unique_ptr<Effect>> Effects;


	class Card {
	public:
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

		enum Position : unsigned char {
			LIBRARY,
			HAND,
			STACK,
			BATTLEFIELD,
			GRAVEYARD,
			EXILE
		};

	private:
		PlayerId ownerId_;
		Position position_;
		bool tapped_;

	public:
		Position position() const { return position_; }
		void setPosition(Position value) { position_ = value; }

		PlayerId ownerId() const { return ownerId_; }

		Card(PlayerId ownerId) : ownerId_(ownerId), tapped_(false) {
		}
		virtual ~Card() {}

		bool tapped() const { return tapped_; }
		void setTapped(bool value) { tapped_ = value; }
		void tap() { tapped_ = true; }
		void untap() { tapped_ = false; }

		// specific for card type

		// specific for card
		virtual Id id() const = 0;
		virtual const char * name() const = 0;
		virtual const char * description() const = 0;
		virtual Type type() const = 0;

		virtual const std::string getImageName() const {
			return std::string("images/") + name() + ".jpg";
		}

		virtual Color color() const { return Color::COLORLESS; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) = 0;
		virtual void activateAbility(Impl & impl, CardInGameId myInGameId) {/* throw "card doesn't have abilities";*/}

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

		Player() : hp(20), passed(false), loser(false) {}
	};

	typedef std::vector<Effects> Stack;

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

		Turn() : activePlayerId(0), priorityPlayerId(0), phase(FIRST_MAIN) {   // FIXME: phase(UNTAP)
		}
	};

	class Target {
	public:
		enum Type {
			PLAYER,
			CARD
		};

		Type type;
		union {
			CardInGameId cardInGameId;
			PlayerId playerId;
		};
	};

	static std::unique_ptr<Card> newCard(Card::Id cardId, PlayerId ownerId);

public:

	Game();
	~Game();

	Player & player(PlayerId id);
	const Player & player(PlayerId id) const;
	std::vector<Player> & players();

	Card & card(CardInGameId cardInGameId);
	std::vector<std::unique_ptr<Card>> & cards();
	const std::vector<std::unique_ptr<Card>> & cards() const;

	Turn & turn();
	const Turn & turn() const;

	void start(PlayerId firstPlayer);

	// actions of player
	void playCardFromHand(PlayerId playerId, CardInGameId cardInGameId);
	void activateAbility(PlayerId playerId, CardInGameId cardInGameId);
	void pass(PlayerId playerId);

private:
	std::unique_ptr<Impl> pimpl;
};





class EWrongPosition : public std::exception {
	const Game::Card::Position position_;
	const Game::Card::Position possiblePositions_;
public:
	EWrongPosition(Game::Card::Position position, Game::Card::Position possiblePositions)
	 : position_(position), possiblePositions_(possiblePositions) {}

	Game::Card::Position position() const { return position_; }
	Game::Card::Position possiblePositions() const { return possiblePositions_; }
};
