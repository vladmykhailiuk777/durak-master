#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <cstdlib> 

using namespace std;

void clearConsole() {
    #if defined(_WIN32)
        system("cls");
    #else
        system("clear");
    #endif
}

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
    int deck_size;

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
    void addCard(Card card);
    void removeCard(int index);
    void showHand() const;
    int getHandSize() const;
    bool hasCards() const;
    string getName() const;
    bool isOut() const { return is_out; }
    void setOut(bool status) { is_out = status; }
    vector<Card> getHandCopy() const { return hand; }
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
    void addDefenseCard(Card card);
    bool canAddCard(Rank rank) const;
    bool isFullyDefended() const;
    vector<Card> getAllCards() const;
    void clear();
    void display() const;
    int getAttackCardsCount() const { return attack_cards.size(); }
    int getDefenseCardsCount() const { return defense_cards.size(); }
    const vector<Card>& getAttackCards() const { return attack_cards; }
};

class GameMode {
public:
    virtual ~GameMode();
    virtual bool canThrow(Player* player, const Table& table) = 0;
    virtual string getModeName() = 0;
    virtual void handleTurn(GameSession& session) = 0;
};

class ThrowingDurak : public GameMode {
public:
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
    void setLoser(string name) { loser_name = name; }
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
    GameSession(const vector<string>& playerNames, int deckSize, GameMode* mode);
    ~GameSession();

    void start();
    void nextTurn();
    void dealToPlayers();
    bool checkWinCondition();
    void displayState() const;
    GameResult getResult();
    int getNextActivePlayer(int current_idx);
    
    friend class ThrowingDurak;
};


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
    if (size == 24) minRank = Rank::NINE;
    
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
            if (card.getSuit() == trump_suit) card.setIsTrump(true);
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

Player::Player(string playerName) : name(playerName), is_out(false), cards_taken(0) {}
Player::~Player() {}

void Player::addCards(const vector<Card>& newCards) {
    hand.insert(hand.end(), newCards.begin(), newCards.end());
}
void Player::addCard(Card card) { hand.push_back(card); }

void Player::removeCard(int index) {
    if (index >= 0 && index < hand.size()) {
        hand.erase(hand.begin() + index);
    }
}

void Player::showHand() const {
    cout << "\nВаші карти:\n";
    for (size_t i = 0; i < hand.size(); ++i) {
        cout << "[" << i + 1 << "] " << hand[i] << "   ";
    }
    cout << "\n";
}

int Player::getHandSize() const { return hand.size(); }
bool Player::hasCards() const { return !hand.empty(); }
string Player::getName() const { return name; }

HumanPlayer::HumanPlayer(string playerName) : Player(playerName) {}

int HumanPlayer::chooseAttackCard(const Table& table) {
    showHand();
    cout << "\n" << name << ", виберіть карту для ходу (1-" << hand.size() << ") або 0 щоб завершити/пас: ";
    int choice;
    while (!(cin >> choice) || choice < 0 || choice > hand.size()) {
        cout << "Невірний вибір. Спробуйте ще раз: ";
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return choice - 1; 
}

int HumanPlayer::chooseDefenseCard(const Card& attackCard, const Table& table) {
    showHand();
    cout << "\n" << name << ", відбивайтесь від " << attackCard << " (1-" << hand.size() << ") або 0 щоб взяти карти: ";
    int choice;
    while (!(cin >> choice) || choice < 0 || choice > hand.size()) {
        cout << "Невірний вибір. Спробуйте ще раз: ";
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return choice - 1;
}

bool HumanPlayer::wantToPass() { return false; }

Table::Table() : max_cards(6) {}

void Table::addAttackCard(Card card) { attack_cards.push_back(card); }
void Table::addDefenseCard(Card card) { defense_cards.push_back(card); }

bool Table::canAddCard(Rank rank) const {
    if (attack_cards.empty()) return true;
    for (const auto& c : attack_cards) if (c.getRank() == rank) return true;
    for (const auto& c : defense_cards) if (c.getRank() == rank) return true;
    return false;
}

bool Table::isFullyDefended() const {
    return !attack_cards.empty() && attack_cards.size() == defense_cards.size();
}

vector<Card> Table::getAllCards() const {
    vector<Card> all = attack_cards;
    all.insert(all.end(), defense_cards.begin(), defense_cards.end());
    return all;
}

void Table::clear() {
    attack_cards.clear();
    defense_cards.clear();
}

void Table::display() const {
    cout << "\n================ СТІЛ ================\n";
    if (attack_cards.empty()) {
        cout << "            (Стіл порожній)           \n";
    } else {
        for (size_t i = 0; i < attack_cards.size(); ++i) {
            cout << "  Атака: " << attack_cards[i];
            if (i < defense_cards.size()) cout << "  -->  Захист: " << defense_cards[i];
            cout << "\n";
        }
    }
    cout << "======================================\n";
}

GameMode::~GameMode() {}
bool ThrowingDurak::canThrow(Player* player, const Table& table) { return true; }
string ThrowingDurak::getModeName() { return "Підкидний"; }

void ThrowingDurak::handleTurn(GameSession& session) {
    Player* attacker = session.players[session.attacker_idx];
    Player* defender = session.players[session.defender_idx];
    
    int left_neighbor_idx = session.attacker_idx;
    int right_neighbor_idx = session.getNextActivePlayer(session.defender_idx);
    
    vector<int> active_throwers;
    active_throwers.push_back(left_neighbor_idx);
    if (left_neighbor_idx != right_neighbor_idx && right_neighbor_idx != session.defender_idx) {
        active_throwers.push_back(right_neighbor_idx);
    }

    bool defense_failed = false;
    
    while (true) {
        session.displayState();
        int attack_idx = attacker->chooseAttackCard(session.table);
        
        if (attack_idx == -1) break; 
        
        Card attack_card = attacker->getHandCopy()[attack_idx];
        if (session.table.getAttackCardsCount() == 0 || session.table.canAddCard(attack_card.getRank())) {
            attacker->removeCard(attack_idx);
            session.table.addAttackCard(attack_card);
            break;
        } else {
            cout << "\n Не можна піти цією картою!\n";
        }
    }
    
    if (session.table.getAttackCardsCount() == 0) return; 
    
    while (!session.table.isFullyDefended() || !defense_failed) {
        session.displayState();
        
        Card current_attack = session.table.getAttackCards().back();
        bool defended = false;
        
        while (!defended) {
            int def_idx = defender->chooseDefenseCard(current_attack, session.table);
            if (def_idx == -1) {
                defense_failed = true;
                break; 
            }
            
            Card def_card = defender->getHandCopy()[def_idx];
            if (def_card.canBeat(current_attack)) {
                defender->removeCard(def_idx);
                session.table.addDefenseCard(def_card);
                defended = true;
            } else {
                cout << "\n Ця карта не б'є атакуючу!\n";
            }
        }
        
        if (defense_failed) break;
        
        if (session.table.getAttackCardsCount() < 6 && defender->getHandSize() > 0) {
            bool anyone_tossed = false;
            for (int thrower_idx : active_throwers) {
                if (session.table.getAttackCardsCount() >= 6 || defender->getHandSize() == 0) break;
                
                Player* thrower = session.players[thrower_idx];
                if (!thrower->hasCards()) continue;

                vector<Card> valid_cards;
                for (const auto& c : thrower->getHandCopy()) {
                    if (session.table.canAddCard(c.getRank())) {
                        valid_cards.push_back(c);
                    }
                }
                
                if (valid_cards.empty()) continue;

                cout << "\n--- Хід підкидання для " << thrower->getName() << " ---\n";
                cout << "Дозволені карти для підкидання: ";
                for (const auto& c : valid_cards) cout << c << " ";
                cout << "\n";

                int throw_idx = thrower->chooseAttackCard(session.table);
                
                if (throw_idx != -1) {
                    Card throw_card = thrower->getHandCopy()[throw_idx];
                    if (session.table.canAddCard(throw_card.getRank())) {
                        thrower->removeCard(throw_idx);
                        session.table.addAttackCard(throw_card);
                        anyone_tossed = true;
                        break; 
                    } else {
                        cout << "\nНе можна підкинути цю карту!\n";
                    }
                }
            }
            if (!anyone_tossed) break; 
        } else {
            break; 
        }
    }
    
    if (defense_failed) {
        cout << "\n======================================\n";
        cout << defender->getName() << " НЕ ВІДБИВСЯ І БЕРЕ КАРТИ!\n";
        cout << "======================================\n";
        defender->addCards(session.table.getAllCards());
        session.table.clear();
        session.attacker_idx = session.getNextActivePlayer(session.defender_idx); 
    } else {
        cout << "\n======================================\n";
        cout << "              🛡️ БИТО! 🛡️              \n";
        cout << "======================================\n";
        vector<Card> tableCards = session.table.getAllCards();
        session.discard.insert(session.discard.end(), tableCards.begin(), tableCards.end());
        session.table.clear();
        session.attacker_idx = session.defender_idx;
    }

    cout << "\n[Натисніть Enter для продовження]";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

GameResult::GameResult() : total_turns(0) {}
void GameResult::display() const {
    cout << "\n======================================\n";
    cout << "              КІНЕЦЬ ГРИ              \n";
    cout << "======================================\n";
    cout << "Дурень: " << loser_name << "!\n\n";
}

GameSession::GameSession(const vector<string>& playerNames, int deckSize, GameMode* mode) : deck(deckSize), game_mode(mode) {
    for (const string& name : playerNames) {
        players.push_back(new HumanPlayer(name));
    }
    attacker_idx = 0;
}

GameSession::~GameSession() {
    for (auto p : players) delete p;
    delete game_mode;
}

int GameSession::getNextActivePlayer(int current_idx) {
    int next_idx = (current_idx + 1) % players.size();
    while (players[next_idx]->isOut() && next_idx != current_idx) {
        next_idx = (next_idx + 1) % players.size();
    }
    return next_idx;
}

void GameSession::dealToPlayers() {
    int current = attacker_idx;
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[current]->isOut() && players[current]->getHandSize() < 6 && !deck.isEmpty()) {
            int needed = 6 - players[current]->getHandSize();
            players[current]->addCards(deck.dealCards(needed));
        }
        current = (current + 1) % players.size();
    }
}

bool GameSession::checkWinCondition() {
    int active_players = 0;
    string last_player_name = "";
    
    for (auto p : players) {
        if (p->getHandSize() == 0 && deck.isEmpty()) {
            p->setOut(true);
        }
        if (!p->isOut()) {
            active_players++;
            last_player_name = p->getName();
        }
    }
    
    if (active_players <= 1) {
        clearConsole();
        GameResult res;
        res.setLoser(active_players == 1 ? last_player_name : "Нічия");
        res.display();
        return true;
    }
    return false;
}

void GameSession::displayState() const {
    cout << "\n--- СТАТУС ГРИ -----------------------\n";
    cout << "Козир: ";
    switch (deck.getTrumpSuit()) {
        case Suit::SPADES: cout << "♠"; break;
        case Suit::HEARTS: cout << "♥"; break;
        case Suit::DIAMONDS: cout << "♦"; break;
        case Suit::CLUBS: cout << "♣"; break;
    }
    cout << " | Карт у колоді: " << deck.size() << "\n";
    table.display();
}

void GameSession::nextTurn() {
    clearConsole();
    cout << "\n======================================\n";
    cout << "Новий раунд! Ходить гравець: " << players[attacker_idx]->getName();
    cout << "\n======================================\n";
    
    defender_idx = getNextActivePlayer(attacker_idx);
    game_mode->handleTurn(*this);
    dealToPlayers();
}

void GameSession::start() {
    cout << "\nРоздача карт...\n";
    dealToPlayers();
    
    while (!checkWinCondition()) {
        nextTurn();
    }
}

void displayStartMenu(int num_players, int deck_size) {
    clearConsole();
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║                  DurakMaster                 ║\n";
    cout << "║                   «Дурень»                   ║\n";
    cout << "╠══════════════════════════════════════════════╣\n";
    cout << "║  1. Нова гра                                 ║\n";
    cout << "║  2. Налаштування                             ║\n";
    cout << "║     → Кількість гравців: [" << num_players << "]                 ║\n";
    cout << "║     → Колода: [" << deck_size << " карт]                      ║\n";
    cout << "║     → Вид гри: [Підкидний]                   ║\n";
    cout << "║  3. Правила гри                              ║\n";
    cout << "║  0. Вийти                                    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "Оберіть опцію: ";
}

int main() {
    #if defined(_WIN32)
        system("chcp 65001");
    #endif

    int num_players = 2;
    int deck_size = 36;
    bool running = true;

    while (running) {
        displayStartMenu(num_players, deck_size);
        int choice;
        if (!(cin >> choice)) { 
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            continue; 
        }

        switch (choice) {
            case 1: {
                clearConsole();
                vector<string> player_names;
                cout << "=== ВВЕДЕННЯ ІМЕН ГРАВЦІВ ===\n\n";
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                
                for (int i = 0; i < num_players; ++i) {
                    string name;
                    cout << "Введіть ім'я гравця " << (i + 1) << ": ";
                    getline(cin, name);
                    if (name.empty()) name = "Гравець " + to_string(i + 1);
                    player_names.push_back(name);
                }
                
                GameMode* mode = new ThrowingDurak();
                GameSession session(player_names, deck_size, mode);
                session.start();
                
                cout << "[Натисніть Enter для повернення в меню]";
                cin.get();
                break;
            }
            case 2: {
                clearConsole();
                cout << "=== НАЛАШТУВАННЯ ===\n\n";
                cout << "Введіть кількість гравців (2-6): ";
                cin >> num_players;
                if (num_players < 2 || num_players > 6) num_players = 2;
                
                cout << "Введіть розмір колоди (24, 36, 52): ";
                cin >> deck_size;
                if (deck_size != 24 && deck_size != 36 && deck_size != 52) deck_size = 36;
                break;
            }
            case 3:
                clearConsole();
                cout << "╔═══════════════════════════════════════════════════════╗\n";
                cout << "║               ПРАВИЛА: ПІДКИДНИЙ ДУРЕНЬ               ║\n";
                cout << "╠═══════════════════════════════════════════════════════╣\n";
                cout << "║ 1. Мета гри - першим позбутися всіх карт на руці.     ║\n";
                cout << "║ 2. На початку гри кожному роздається по 6 карт.       ║\n";
                cout << "║ 3. Перша карта в атаці може бути будь-якою.           ║\n";
                cout << "║ 4. Захисник має побити її старшою картою тієї ж масті ║\n";
                cout << "║    або будь-яким козирем (якщо атака не козирна).     ║\n";
                cout << "║ 5. Сусіди (атакуючий і наступний гравець) можуть      ║\n";
                cout << "║    ПІДКИДАТИ карти, якщо їхній ранг збігається з      ║\n";
                cout << "║    картами, що вже лежать на столі.                   ║\n";
                cout << "║ 6. Захисник повинен відбити всі підкинуті карти       ║\n";
                cout << "║    (але не більше 6 за один хід).                     ║\n";
                cout << "║ 7. Якщо він відбився — карти йдуть у відбій (Бито!),  ║\n";
                cout << "║    а він стає новим атакуючим.                        ║\n";
                cout << "║ 8. Якщо не відбився — забирає всі карти зі столу,     ║\n";
                cout << "║    а хід переходить до наступного за ним гравця.      ║\n";
                cout << "╚═══════════════════════════════════════════════════════╝\n";
                
                cout << "\n[Натисніть Enter, щоб повернутися в меню]";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            case 0:
                running = false;
                break;
            default:
                break;
        }
    }
    return 0;
}
