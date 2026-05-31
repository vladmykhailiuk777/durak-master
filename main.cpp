#include <iostream>
#include <vector>
#include <string>

using namespace std;


enum class Suit {
    SPADES,   // ♠
    HEARTS,   // ♥
    DIAMONDS, // ♦
    CLUBS     // ♣
};

enum class Rank {
    TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
    JACK, QUEEN, KING, ACE
};

class Table;
class GameSession;

class Card {
private:
    Suit suit;
    Rank rank;
    bool is_trump;

public:
    Card(Suit s, Rank r);

    string getInfo() const;
    bool getIsTrump() const;
    void setIsTrump(bool status);
    Suit getSuit() const;
    Rank getRank() const;

    friend ostream& operator<<(ostream& os, const Card& card);
    bool operator< (const Card& other) const;
    bool operator== (const Card& other) const;
    bool canBeat(const Card& attackingCard) const;
};

class Deck {
private:
    vector<Card> cards;
    Suit trump_suit;
    int deck_size; //24, 36 або 52

public:
    Deck(int size);

    void shuffle();
    vector<Card> dealCards(int n);
    Card drawCard();
    Suit getTrumpSuit() const;
    bool isEmpty() const;
    int size() const;
};


class Player {
protected:
    string name;
    vector<Card> hand;
    bool is_out;
    int cards_taken;

public:
    Player(string playerName);
    virtual ~Player();

    virtual int chooseAttackCard(const Table& table) = 0;
    virtual int chooseDefenseCard(const Card& attackCard, const Table& table) = 0;
    virtual bool wantToPass() = 0;

    void addCards(const vector<Card>& newCards);
    void removeCard(int index);
    void showHand() const;
    int getHandSize() const;
    bool hasCards() const;
    string getName() const;
};

class HumanPlayer : public Player {
public:
    HumanPlayer(string playerName);

    int chooseAttackCard(const Table& table) override;
    int chooseDefenseCard(const Card& attackCard, const Table& table) override;
    bool wantToPass() override;
};

class Table {
private:
    vector<Card> attack_cards;
    vector<Card> defense_cards;
    int max_cards;

public:
    Table();

    void addAttackCard(Card card);
    void addDefenseCard(Card card, int index);
    bool canAddCard(Rank rank) const;
    bool isFullyDefended() const;
    vector<Card> getAllCards();
    void clear();
    void display() const;
};

class GameMode {
public:
    virtual ~GameMode();
    
    virtual bool canTransfer(Player* defender, const Table& table) = 0;
    virtual bool canThrow(Player* player, const Table& table) = 0;
    virtual string getModeName() = 0;
    virtual void handleTurn(GameSession& session) = 0;
};

class ThrowingDurak : public GameMode {
public:
    bool canTransfer(Player* defender, const Table& table) override;
    bool canThrow(Player* player, const Table& table) override;
    string getModeName() override;
    void handleTurn(GameSession& session) override;
};

class TransferDurak : public GameMode {
public:
    bool canTransfer(Player* defender, const Table& table) override;
    bool canThrow(Player* player, const Table& table) override;
    string getModeName() override;
    void handleTurn(GameSession& session) override;
};

class GameResult {
private:
    string loser_name;
    vector<string> winner_names;
    int total_turns;

public:
    GameResult();
    
    void display() const;
    friend ostream& operator<<(ostream& os, const GameResult& result);
};

class GameSession {
private:
    vector<Player*> players;
    Deck deck;
    Table table;
    GameMode* game_mode;
    int attacker_idx;
    int defender_idx;
    vector<Card> discard;

public:
    GameSession(int deckSize, GameMode* mode);

    void start();
    void nextTurn();
    void dealToPlayers();
    bool checkWinCondition();
    void displayState() const;
    GameResult getResult();
};

class CommandConsole {
private:
    GameSession& session;

public:
    CommandConsole(GameSession& gameSession);

    void run();
    void execute(const string& command);
    void showHelp() const;
    void showTrumps() const;
    void showPlayers() const;
};

void displayStartMenu() {
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║                  DurakMaster                  ║\n";
    cout << "║                   «Дурень»                   ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  1. Нова гра                                 ║\n";
    cout << "║  2. Налаштування                             ║\n";
    cout << "║     → Кількість гравців: [2]                 ║\n";
    cout << "║     → Колода: [36 карт]                      ║\n";
    cout << "║     → Вид гри: [Підкидний]                   ║\n";
    cout << "║  3. Правила гри                              ║\n";
    cout << "║  0. Вийти                                    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "Оберіть опцію: ";
}

int main(){
    displayStartMenu();
    return 0;
};
