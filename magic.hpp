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
			ARTIFACT     = 0b00000001,
			CREATURE     = 0b00000010,
			ENCHANTMENT  = 0b00000100,
			INSTANT      = 0b00001000,
			LAND         = 0b00010000,
			PLANESWALKER = 0b00100000,
			SORCERY      = 0b01000000
		};

		enum Position : unsigned char {
			LIBRARY     = 0b00000001,
			HAND        = 0b00000010,
			STACK       = 0b00000100,
			BATTLEFIELD = 0b00001000,
			GRAVEYARD   = 0b00010000,
			EXILE       = 0b00100000
		};

	protected:
		struct Info {
			Id id;
			const char * name;
			const char * description;
		};
		virtual const Info & info() const = 0;

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


		Id id() const { return info().id; }
		const char * name() const { return info().name; }
		const char * description() { return info().description; }
		virtual Type type() const { return (Type)0; }

		virtual const std::string getImageName() const { return std::string("images/") + name() + ".jpg"; }

		virtual Color color() const { return Color::COLORLESS; }  // TODO: get colors from cost

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) {}
		virtual void activateAbility(Impl & impl, CardInGameId myInGameId) {}

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
		enum Phase : unsigned short {
			UNTAP         = 0b1,
			UPKEEP        = 0b10,
			DRAW_CARD     = 0b100,
			FIRST_MAIN    = 0b1000,
			COMBAT_BEGIN  = 0b10000,
			COMBAT_ATTACK = 0b100000,
			COMBAT_BLOCK  = 0b1000000,
			COMBAT_DAMAGE = 0b10000000,
			COMBAT_END    = 0b100000000,
			SECOND_MAIN   = 0b1000000000,
			END           = 0b10000000000,
			CLEANUP       = 0b100000000000
		};


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
			PLAYER = 0b1,
			CARD   = 0b10
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

