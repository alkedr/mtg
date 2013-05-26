#include "magic.hpp"





class CardWidget : public QWidget
{
	Q_OBJECT

    QPixmap source_;
    QPixmap current_;

	Game::CardInGameId cardInGameId;

public:

	CardWidget()
	: cardInGameId((Game::CardInGameId)-1)
	{
	}

	CardWidget(Game::CardInGameId _cardInGameId, const Game::Card & card)
	: cardInGameId(_cardInGameId)
	{
		setCard(&card);
	}

	void setCard(const Game::Card * card)
	{
		if (card == nullptr) {
			setPixmap(QPixmap());
			cardInGameId = (Game::CardInGameId)-1;
		} else {
			if (card->tapped()) {
				setPixmap(QPixmap(card->getImageName().c_str()).transformed(QTransform().rotate(90)));
			} else {
				setPixmap(QPixmap(card->getImageName().c_str()));
			}
		}
	}

	void setPixmap(QPixmap aPicture)
	{
    	source_ = current_ = aPicture;
    	repaint();
	}

	void paintEvent(QPaintEvent * event) override
	{
    	QWidget::paintEvent(event);

		if (source_.isNull()) return;

		int cw = width(), ch = height();
		int pw = current_.width(), ph = current_.height();

		if (
		    ((pw > cw) && (ph > ch) && (pw/cw > ph/ch))   //both width and height are bigger, ratio at height is bigger or
		 || ((pw > cw) && (ph <= ch))                     //only the width is bigger or
		 || ((pw < cw) && (ph < ch) && (cw/pw < ch/ph))   //both width and height is smaller, ratio at width is smaller
		) {
			current_ = source_.scaledToWidth(cw, Qt::TransformationMode::FastTransformation);
		} else 
		if (
		    ((pw > cw) && (ph > ch) && (pw/cw <= ph/ch))  //both width and height are bigger, ratio at width is bigger or
		 || ((ph > ch) && (pw <= cw))                     //only the height is bigger or
		 || ((pw < cw) && (ph < ch) && (cw/pw > ch/ph))   //both width and height is smaller, ratio at height is smaller
		) {
			current_ = source_.scaledToHeight(ch, Qt::TransformationMode::FastTransformation);
		}

		int x = (cw - current_.width()) / 2;
		int y = (ch - current_.height()) / 2;

		QPainter paint(this);
		paint.drawPixmap(x, y, current_);
	}

	void mousePressEvent(QMouseEvent * event) override
	{
		if (event->button() == Qt::LeftButton) {
			emit cardActivated(cardInGameId);
		}
	}

signals:

	void cardActivated(Game::CardInGameId);

};




class CardListWidget : public QWidget
{
	Q_OBJECT

	QHBoxLayout layout;
	std::vector<std::unique_ptr<CardWidget>> cardWidgets;

private slots:

	void cardActivatedSlot(Game::CardInGameId cardInGameId)
	{
		std::cout << cardInGameId << std::endl;
		emit cardActivated(cardInGameId);
	}

public:

	CardListWidget()
	{
		setLayout(&layout);
	}

	template<class Predicate> void setCards(const std::vector<std::unique_ptr<Game::Card>> & cards, Predicate predicate)
	{
		cardWidgets.clear();
		for (size_t i = 0; i < cards.size(); i++) {
			if (predicate(cards[i])) {
				cardWidgets.emplace_back(std::unique_ptr<CardWidget>(new CardWidget(i, *cards[i])));
				layout.addWidget(cardWidgets.back().get(), 10);
				connect(cardWidgets.back().get(), SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
			}
		}
	}

signals:

	void cardActivated(Game::CardInGameId);

};




class TeamWidget : public QWidget
{
	Q_OBJECT

	QHBoxLayout teamLayout;
		QGroupBox playerWidget;
		QVBoxLayout playerLayout;
			QLabel nameLabel;
			QLabel hpLabel;
			QLabel libraryLabel;
			QLabel graveLabel;
			QLabel activeAndPriorityLabel;
		QGroupBox battlefieldWidget;
		QVBoxLayout battlefieldLayout;
			CardListWidget handWidget;
			CardListWidget creaturesWidget;
			CardListWidget landsWidget;

private slots:

	void cardActivatedSlot(Game::CardInGameId cardInGameId)
	{
		std::cout << cardInGameId << std::endl;
		emit cardActivated(cardInGameId);
	}

public:

	TeamWidget()
	{
		setLayout(&teamLayout);
		teamLayout.addWidget(&playerWidget);
			playerWidget.setLayout(&playerLayout);
			playerLayout.addWidget(&nameLabel);
			playerLayout.addWidget(&hpLabel);
			playerLayout.addWidget(&libraryLabel);
			playerLayout.addWidget(&graveLabel);
			playerLayout.addWidget(&activeAndPriorityLabel);
			
		teamLayout.addWidget(&battlefieldWidget, 1);
			battlefieldWidget.setLayout(&battlefieldLayout);
			battlefieldLayout.addWidget(&landsWidget);
			battlefieldLayout.addWidget(&creaturesWidget);

		connect(&landsWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
		connect(&creaturesWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
	}


	void set(const Game & game, Game::PlayerId forPlayer)
	{
		std::cout << __PRETTY_FUNCTION__ << "  " << (int)forPlayer << std::endl;

		nameLabel.setText(QString("Player ") + QString::number(forPlayer+1));
		hpLabel.setText(QString("HP: ") + QString::number(game.player(forPlayer).hp));

		auto gravePredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::LIBRARY)
				    && (pCard->ownerId == forPlayer);
			};

		libraryLabel.setText(QString("L: ") + QString::number(game.player(forPlayer).library.size()));
		graveLabel.setText(QString("G: ") + QString::number(std::count_if(game.cards.begin(), game.cards.end(), gravePredicate)));
		activeAndPriorityLabel.setText(
			((game.player(forPlayer).loser) ? QString("L") : QString("")) + 
			((game.turn.activePlayerId == forPlayer) ? QString("A") : QString("")) + 
			((game.turn.priorityPlayerId == forPlayer) ? QString("P") : QString(""))
		);

		auto landsPredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::BATTLEFIELD)
 				    && (pCard->ownerId == forPlayer)
					&& (pCard->getType() == Game::Card::Type::LAND);
			};

		landsWidget.setCards(game.cards, landsPredicate);


		auto creaturesPredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::BATTLEFIELD)
 				    && (pCard->ownerId == forPlayer)
					&& (pCard->getType() == Game::Card::Type::CREATURE);
			};

		creaturesWidget.setCards(game.cards, creaturesPredicate);
	}

signals:

	void cardActivated(Game::CardInGameId);

}; 




class GameWidget : public QWidget {
	Q_OBJECT

	Game & game;
	Game::PlayerId forPlayer;

	QHBoxLayout mainLayout;
		QGroupBox teamsWidget;
		QVBoxLayout teamsLayout;
			TeamWidget myTeamWidget;
			TeamWidget enemyTeamWidget;
			CardListWidget myHandWidget;
		QGroupBox sideBarWidget;
		QVBoxLayout sideBarLayout;
			CardWidget bigCardWidget;
			QLabel phaseLabel;
			CardListWidget stackWidget;
			QPushButton passButton;

private slots:

	void cardFromHandActivated(Game::CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << "  " << cardInGameId << std::endl;
		game.playCardFromHand(forPlayer, cardInGameId);
		dataUpdated();
	}

	void cardFromBattlefieldActivated(Game::CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << "  " << cardInGameId << std::endl;
		game.tap(forPlayer, cardInGameId);
		dataUpdated();
	}

	void passButtonPressed() {
		game.pass(forPlayer);
		dataUpdated();
	}

public:

	GameWidget(Game & _game, Game::PlayerId _forPlayer)
	: game(_game)
	, forPlayer(_forPlayer)
	, passButton("pass") {
		setLayout(&mainLayout);
		mainLayout.addWidget(&teamsWidget, 1);
			teamsWidget.setLayout(&teamsLayout);
			teamsLayout.addWidget(&enemyTeamWidget);
			teamsLayout.addWidget(&myTeamWidget);
			teamsLayout.addWidget(&myHandWidget);
		mainLayout.addWidget(&sideBarWidget);
			sideBarWidget.setLayout(&sideBarLayout);
			sideBarLayout.addWidget(&bigCardWidget);
			sideBarLayout.addWidget(&phaseLabel);
			sideBarLayout.addWidget(&stackWidget);
			sideBarLayout.addWidget(&passButton);

		connect(&myHandWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardFromHandActivated(Game::CardInGameId)));
		connect(&myTeamWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardFromBattlefieldActivated(Game::CardInGameId)));

		connect(&passButton, SIGNAL(clicked()), this, SLOT(passButtonPressed()));
	}

	void dataUpdated() {

		forPlayer = game.turn.priorityPlayerId;

		static auto handPredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::HAND)
 				    && (pCard->ownerId == forPlayer);
			};

		static auto stackPredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::STACK);
			};

		myHandWidget.setCards(game.cards, handPredicate);

		myTeamWidget.set(game, forPlayer);
		enemyTeamWidget.set(game, 1-forPlayer);

		stackWidget.setCards(game.cards, stackPredicate);

		phaseLabel.setText(Game::Turn::phaseToString(game.turn.phase));
	}

};




class GameWindow : public QMainWindow
{
	Q_OBJECT

	GameWidget gameWidget;

public:

	GameWindow(Game & game, Game::PlayerId forPlayer)
	: gameWidget(game, forPlayer)
	{
		setCentralWidget(&gameWidget);
		dataUpdated();
	}

	void dataUpdated()
	{
		gameWidget.dataUpdated();
	}
};





int main(int argc, char ** argv)
{
	QApplication app(argc, argv);

	Game game;

	game.players.emplace_back();
	game.players.emplace_back();
	game.player(0).library = { 40, 258, 78, 77, 273, 268, 40, 40, 40 }; 
	game.player(1).library = { 40, 258, 78, 77, 273, 268, 40, 40, 40 }; 
	game.start(0);

	GameWindow gameWindow(game, 0);
	gameWindow.show();
	
	/*MainMenuWindow mainMenuWindow;
	mainMenuWindow.exec();

	MainMenuWindow::MenuItem selectedItem = mainMenuWindow.getSelectedItem();
	switch (selectedItem) {
		case (MainMenuWindow::MenuItem::NEW_GAME): {
			break;
		}
		case (MainMenuWindow::MenuItem::DECKS): {
			break;
		}
		case (MainMenuWindow::MenuItem::SETTINGS): {
			break;
		}
		case (MainMenuWindow::MenuItem::EXIT): {
			break;
		}
		default: {
			break;
		}
	};*/

	return app.exec();
}



#include "moc_client.cpp"
