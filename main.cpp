#include <iostream>
#include <vector>
#include <string>
#include <algorithm> 
#include <random>    
#include <chrono>    

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
    cout << "║                  DurakMaster                 ║\n";
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



Card::Card(Suit s, Rank r) : suit(s), rank(r), is_trump(false) {}

string Card::getInfo() const {
    string r_str;
    switch (rank) {
        case Rank::JACK: r_str = "J"; break;
        case Rank::QUEEN: r_str = "Q"; break;
        case Rank::KING: r_str = "K"; break;
        case Rank::ACE: r_str = "A"; break;
        default: r_str = to_string(static_cast<int>(rank)); break;
    }
    string s_str;
    switch (suit) {
        case Suit::SPADES: s_str = "♠"; break;
        case Suit::HEARTS: s_str = "♥"; break;
        case Suit::DIAMONDS: s_str = "♦"; break;
        case Suit::CLUBS: s_str = "♣"; break;
    }
    return r_str + s_str;
}

bool Card::getIsTrump() const { return is_trump; }
void Card::setIsTrump(bool status) { is_trump = status; }
Suit Card::getSuit() const { return suit; }
Rank Card::getRank() const { return rank; }

ostream& operator<<(ostream& os, const Card& card) {
    os << card.getInfo();
    if (card.is_trump) os << "(К)";
    return os;
}

bool Card::operator<(const Card& other) const { return rank < other.rank; }
bool Card::operator==(const Card& other) const { return rank == other.rank && suit == other.suit; }

bool Card::canBeat(const Card& attackingCard) const {
    if (suit == attackingCard.suit) {
        return rank > attackingCard.rank;
    }
    return is_trump && !attackingCard.is_trump;
}



Deck::Deck(int size) : deck_size(size) {
    Rank minRank = (size == 36) ? Rank::SIX : Rank::TWO;
    Suit suits[] = {Suit::SPADES, Suit::HEARTS, Suit::DIAMONDS, Suit::CLUBS};
    
    for (Suit s : suits) {
        for (int r = static_cast<int>(minRank); r <= static_cast<int>(Rank::ACE); ++r) {
            cards.push_back(Card(s, static_cast<Rank>(r)));
        }
    }
    shuffle();
    
    if (!cards.empty()) {
        trump_suit = cards.back().getSuit();
        for (auto& card : cards) {
            if (card.getSuit() == trump_suit) {
                card.setIsTrump(true);
            }
        }
    }
}

void Deck::shuffle() {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(cards.begin(), cards.end(), std::default_random_engine(seed));
}

vector<Card> Deck::dealCards(int n) {
    vector<Card> dealt;
    for (int i = 0; i < n && !cards.empty(); ++i) {
        dealt.push_back(cards.back());
        cards.pop_back();
    }
    return dealt;
}

Card Deck::drawCard() {
    Card c = cards.back();
    cards.pop_back();
    return c;
}

Suit Deck::getTrumpSuit() const { return trump_suit; }
bool Deck::isEmpty() const { return cards.empty(); }
int Deck::size() const { return cards.size(); }



int main(){

    displayStartMenu();
    return 0;
};
