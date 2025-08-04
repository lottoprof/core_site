#pragma once
#include <map>
#include <string>
#include <vector>
#include <stdint.h>
class PokeHelper
{
public:
    /**
     * \brief PokerHelper ctor
     * \param bet in format hand;discard - "hand_card,hand_card,hand_card;discard_card,discard_card,discard_card" ex: "♠10,♠J,Joker,♣2;♥3,♥5"
     */
    PokeHelper(std::wstring bet);
    PokeHelper(const std::vector<int>& hand, const std::vector<int>& discard);
    
#pragma region future combos
    //TODO: return probability instead of bool
    bool canMakePoker() const;
    bool canMakeRoyalFlush() const;
    bool canMakeStraightFlush() const;
    bool canMakeFourOfAKind() const;
    bool canMakeFullHouse() const;
    bool canMakeFlush() const;
    bool canMakeStraight() const;
    bool canMakeThreeOfAKind() const;
    bool canMakeTwoPair() const;
#pragma endregion

#pragma region check current hand
    bool hasPoker() const;
    bool hasRoyalFlush() const;
    bool hasStraightFlush() const;
    bool hasFourOfAKind() const;
    bool hasFullHouse() const;
    bool hasFlush() const;
    bool hasStraight() const;
    bool hasThreeOfAKind() const;
    bool hasTwoPair() const;
#pragma endregion

#pragma region create next combination
    //TODO: not implemented
    std::vector<int> makePoker() const;
    std::vector<int> makeRoyalFlush() const;
    std::vector<int> makeStraightFlush() const;
    std::vector<int> makeFourOfAKind() const;
    std::vector<int> makeFullHouse() const;
    std::vector<int> makeFlush() const;
    std::vector<int> makeStraight() const;
    std::vector<int> makeThreeOfAKind() const;
    std::vector<int> makeTwoPair() const;
#pragma endregion

#pragma region helpers
    static int CardFromString(std::wstring card);
    static std::wstring CardToString(int card);
    static std::vector<int> CardListFromString(std::wstring cards);
#pragma endregion
    
private:
    void init(const std::vector<int>& hand, const std::vector<int>& discard);
    std::vector<int> _hand;
    bool _hasJoker;
    int _handCount; //_hand.size()+_hasJoker?1:0;
    int _handMask;
    int _handSuitsCount;
    int _handNumbersCount;
    std::map<int,int> _handLines;
    
    std::vector<int> _discard;
    bool _jokerDiscarded;

    std::vector<uint16_t> _deckSuits;
    std::vector<uint8_t> _deckNumbers;
};
