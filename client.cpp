
#include "magic.hpp"

#include <QtGui>
#include <QtWidgets>


namespace {


class CardWidget : public QWidget {
	Q_OBJECT

	Game::CardInGameId cardInGameId;
	QPixmap pixmap;
	QColor highlightColor_;
	bool tapped;

public:
	CardWidget(Game::CardInGameId _cardInGameId = (Game::CardInGameId)-1, const Game::Card * card = nullptr)
	: cardInGameId(_cardInGameId), highlightColor_(100, 0, 0, 255), tapped(false)
	{
		if (card != nullptr) {
			tapped = card->tapped();
			if (card->tapped()) {
				pixmap = QPixmap(card->getImageName().c_str()).transformed(QTransform().rotate(90));
				setSizeIncrement(88, 88);
			} else {
				pixmap = QPixmap(card->getImageName().c_str());
				setSizeIncrement(63, 88);
			}
		}
	}

	void setHighlightColor(QColor c) {
		highlightColor_ = c;
	}
	QColor highlightColor() const {
		return highlightColor_;
	}

	void mousePressEvent(QMouseEvent * event) override {
		std::cout << "w: " << width() << " h:" << height() << std::endl;
		if (event->button() == Qt::LeftButton) {
			emit cardActivated(cardInGameId);
		}
	}

	void paintEvent(QPaintEvent * event) override {
		std::cout << __FUNCTION__ << " w: " << width() << " h:" << height() << " r: " << (double)width()/height() << std::endl;
		QPainter p(this);
		//p.setBrush(QBrush(QColor(0, 0, 0, 255)));
		//p.drawRect(0, 0, width(), height());

		QPixmap scaled = pixmap.scaled(QSize(width()-10, height()-10), Qt::KeepAspectRatio, Qt::SmoothTransformation);

		int y = (tapped) ? (scaled.width() - scaled.height()) : (height() - scaled.height() - 10);

		if (highlightColor() != QColor(0, 0, 0, 0)) {
			p.setBrush(QBrush(highlightColor()));
			p.drawRect(0, y, scaled.width() + 10, scaled.height() + 10);
		}
		p.drawPixmap(5, y+5, scaled);
	}

signals:
	void cardActivated(Game::CardInGameId);
};
class CardListWidget : public QWidget {
private:
	class Layout : public QLayout {
	private:

		QList<QLayoutItem*> list;

		QSize sizeInUnits() const {
			if (list.empty()) return QSize(0, 0);

			QSize s(list.size()*10, 88);
			for (QLayoutItem * item : list) {
				s += QSize(item->widget()->sizeIncrement().width(), 0);
			}
			std::cout << __FUNCTION__ << " " << s.width() << "x" << s.height() << std::endl;
			s.scale(geometry().width(), geometry().height(), Qt::KeepAspectRatio);
			std::cout << __FUNCTION__ << " " << s.width() << "x" << s.height() << std::endl;
			return s;
		}

	public:
		Layout() {
		}
		~Layout() override {
			QLayoutItem *item;
			while ((item = takeAt(0))) delete item;
		}
		void addItem(QLayoutItem * item) override {
			list.append(item);
		}
		QSize sizeHint() const override {
			return QSize(100, 100);
		}
		QLayoutItem * itemAt(int index) const override {
			return list.value(index);
		}
		QLayoutItem * takeAt(int index) override {
			if (index >= 0 && index < list.size())
				return list.takeAt(index);
			else
				return 0;
		}
		int count() const {
			return list.size();
		}
		void setGeometry(const QRect & rect) override {
			QLayout::setGeometry(rect);

			double scaleRatio = (double)rect.height() / 88;
			std::cout << __FUNCTION__ << " ratio: " << scaleRatio << std::endl;

			int x = 0;
			for (QLayoutItem * item : list) {
				int itemWidth = (int)(item->widget()->sizeIncrement().width() * scaleRatio);
				int itemHeight = (int)(item->widget()->sizeIncrement().height() * scaleRatio);
				std::cout << __FUNCTION__ << " " << "w: " << itemWidth << " h: " << itemHeight << std::endl;
				item->setGeometry(QRect(x, 0, itemWidth, itemHeight));
				x += itemWidth+10;
			}
		}
	};

private:
	Q_OBJECT

	Layout layout;
	std::vector<std::unique_ptr<CardWidget>> cardWidgets;

private slots:
	void cardActivatedSlot(Game::CardInGameId cardInGameId) {
		std::cout << cardInGameId << std::endl;
		emit cardActivated(cardInGameId);
	}

public:
	CardListWidget() {
		setLayout(&layout);
	}
	template<class Predicate> void setCards(const std::vector<std::unique_ptr<Game::Card>> & cards, Predicate predicate) {
		cardWidgets.clear();
		for (size_t i = 0; i < cards.size(); i++) {
			if (predicate(cards[i])) {
				cardWidgets.emplace_back(new CardWidget(i, cards[i].get()));
				layout.addWidget(cardWidgets.back().get());
				connect(cardWidgets.back().get(), SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
			}
		}
	}

signals:
	void cardActivated(Game::CardInGameId);
};
class TeamWidget : public QGroupBox {
	Q_OBJECT

	QHBoxLayout teamLayout;
		QGroupBox playerWidget;
		QVBoxLayout playerLayout;
			QLabel nameLabel;
			QLabel hpLabel;
			QLabel handLabel;
			QLabel libraryLabel;
			QLabel graveLabel;
			QLabel activeAndPriorityLabel;
		QGroupBox battlefieldWidget;
		QVBoxLayout battlefieldLayout;
			CardListWidget creaturesWidget;
			CardListWidget landsWidget;

private slots:
	void cardActivatedSlot(Game::CardInGameId cardInGameId) {
		std::cout << cardInGameId << std::endl;
		emit cardActivated(cardInGameId);
	}

public:
	TeamWidget(bool isMyTeam) {
		setContentsMargins(0, 0, 0, 0);
		setLayout(&teamLayout);
		teamLayout.addWidget(&playerWidget);
			playerWidget.setLayout(&playerLayout);
			playerLayout.addWidget(&nameLabel);
			playerLayout.addWidget(&hpLabel);
			if (!isMyTeam) playerLayout.addWidget(&handLabel);
			playerLayout.addWidget(&libraryLabel);
			playerLayout.addWidget(&graveLabel);
			playerLayout.addWidget(&activeAndPriorityLabel);

		teamLayout.addWidget(&battlefieldWidget, 1);
			battlefieldWidget.setLayout(&battlefieldLayout);
			if (isMyTeam) {
				battlefieldLayout.addWidget(&creaturesWidget);
				battlefieldLayout.addWidget(&landsWidget);
			} else {
				battlefieldLayout.addWidget(&landsWidget);
				battlefieldLayout.addWidget(&creaturesWidget);
			}

		connect(&landsWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
		connect(&creaturesWidget, SIGNAL(cardActivated(Game::CardInGameId)), this, SLOT(cardActivatedSlot(Game::CardInGameId)));
	}
	void set(const Game & game, Game::PlayerId forPlayer) {
		std::cout << __PRETTY_FUNCTION__ << "  " << (int)forPlayer << std::endl;

		nameLabel.setText(QString("Player ") + QString::number(forPlayer+1));
		hpLabel.setText(QString("HP: ") + QString::number(game.player(forPlayer).hp));

		static auto handPredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::HAND)
 				    && (pCard->ownerId == forPlayer);
			};

		handLabel.setText(QString("H: ") + QString::number(std::count_if(std::begin(game.cards), std::end(game.cards), handPredicate)));

		auto gravePredicate =
			[&](const std::unique_ptr<Game::Card> & pCard) {
				return (pCard->position == Game::Card::Position::LIBRARY)
				    && (pCard->ownerId == forPlayer);
			};

		libraryLabel.setText(QString("L: ") + QString::number(game.player(forPlayer).library.size()));
		graveLabel.setText(QString("G: ") + QString::number(std::count_if(std::begin(game.cards), std::end(game.cards), gravePredicate)));
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
class GameWidget : public QSplitter {
	Q_OBJECT

	Game & game;
	Game::PlayerId forPlayer;

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
		try {
			game.playCardFromHand(forPlayer, cardInGameId);
		} catch (const std::exception & e) {
			QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Close).exec();
		}
		dataUpdated();
	}
	void cardFromBattlefieldActivated(Game::CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << "  " << cardInGameId << std::endl;
		try {
			game.tap(forPlayer, cardInGameId);
		} catch (const std::exception & e) {
			QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Close).exec();
		}
		dataUpdated();
	}
	void passButtonPressed() {
		try {
			game.pass(forPlayer);
		} catch (const std::exception & e) {
			QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Close).exec();
		}
		dataUpdated();
	}

public:
	GameWidget(Game & _game, Game::PlayerId _forPlayer) : game(_game), forPlayer(_forPlayer), passButton("pass"), myTeamWidget(true), enemyTeamWidget(false) {
		setContentsMargins(0, 0, 0, 0);
		setFrameShape(QFrame::NoFrame);
		teamsWidget.setContentsMargins(0, 0, 0, 0);
		addWidget(&teamsWidget);
			teamsLayout.setSpacing(0);
			teamsWidget.setLayout(&teamsLayout);
			teamsLayout.addWidget(&enemyTeamWidget);
			teamsLayout.addWidget(&myTeamWidget);
			teamsLayout.addWidget(&myHandWidget);
		addWidget(&sideBarWidget);
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
class GameWindow : public QMainWindow {
	Q_OBJECT

	GameWidget gameWidget;

public:
	GameWindow(Game & game, Game::PlayerId forPlayer) : gameWidget(game, forPlayer) {
		setContentsMargins(0, 0, 0, 0);
		setCentralWidget(&gameWidget);
		dataUpdated();
	}

	void dataUpdated() {
		gameWidget.dataUpdated();
	}
};

}

int main(int argc, char ** argv) {
	QApplication app(argc, argv);

	Game game;

	game.players.emplace_back();
	game.players.emplace_back();
	game.player(0).library = { 1, 2, 3, 4, 5, 6, 1, 1, 1 };
	game.player(1).library = { 1, 2, 3, 4, 5, 6, 1, 1, 1 };
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
