#include "PokeHelper.h"

#include <algorithm>

#define SUIT(card) ((card)/100)
#define NUMBER(card) ((card)%100)

#define MASK(num) (1<<(num))

#define NUMBERS_COUNT 13
#define SUITS_COUNT 4

enum
{
    Number2 = 0,
    Number3 = 1,
    Number4 = 2,
    Number5 = 3,
    Number6 = 4,
    Number7 = 5,
    Number8 = 6,
    Number9 = 7,
    Number10 = 8,
    NumberJ = 9,
    NumberQ = 10,
    NumberK = 11,
    NumberA = 12,
    Joker = 0xFF
};

enum
{
    Suit0 = 0,
    Suit1 = 1,
    Suit2 = 2,
    Suit3 = 3
};

static int straight[10] = {
    MASK(NumberA) | MASK(Number2) | MASK(Number3) | MASK(Number4) | MASK(Number5),
    MASK(Number2) | MASK(Number3) | MASK(Number4) | MASK(Number5) | MASK(Number6),
    MASK(Number3) | MASK(Number4) | MASK(Number5) | MASK(Number6) | MASK(Number7),
    MASK(Number4) | MASK(Number5) | MASK(Number6) | MASK(Number7) | MASK(Number8),
    MASK(Number5) | MASK(Number6) | MASK(Number7) | MASK(Number8) | MASK(Number9),
    MASK(Number6) | MASK(Number7) | MASK(Number8) | MASK(Number9) | MASK(Number10),
    MASK(Number7) | MASK(Number8) | MASK(Number9) | MASK(Number10) | MASK(NumberJ),
    MASK(Number8) | MASK(Number9) | MASK(Number10) | MASK(NumberJ) | MASK(NumberQ),
    MASK(Number9) | MASK(Number10) | MASK(NumberJ) | MASK(NumberQ) | MASK(NumberK),
    MASK(Number10) | MASK(NumberJ) | MASK(NumberQ) | MASK(NumberK) | MASK(NumberA),
};

#define STRAIGHTCOUNT 10
#define STRAIGHTWOROYALCOUNT 9

#define ROYAL_FLUSH_MASK straight[9]

//TODO: replace with C++20 std::popcount
static int popcount8[256] = {
    //0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,

};

static int popcount(const int val)
{
    return popcount8[val & 0xFF]
        + popcount8[(val >> 8) & 0xFF];
}

static bool count_with_joker(const int val, int cnt, const bool hasJoker)
{
    if (hasJoker)
    {
        cnt -= 1;
    }

    return val >= cnt;
}

PokeHelper::PokeHelper(std::wstring bet) // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    size_t pos = bet.find(';');
    init(CardListFromString(bet.substr(0, pos)), CardListFromString(bet.substr(pos + 1)));
}

PokeHelper::PokeHelper(const std::vector<int>& hand, const std::vector<int>& discard) // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    init(hand, discard);
}

static bool sort_cards_by_numbers(int card1, int card2)
{
    return NUMBER(card1) < NUMBER(card2);
}

void PokeHelper::init(const std::vector<int>& hand, const std::vector<int>& discard)
{
    _deckSuits = {0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF};
    _deckNumbers = {0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF};
    _handCount = hand.size();
    _handMask = 0;
    _handSuitsCount = 0;
    _handNumbersCount = 0;

    _hand = {};
    _discard = {};
    _handLines = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
    _hasJoker = false;
    _jokerDiscarded = false;

    int suit = -1;
    for (auto card : hand)
    {
        if (card == Joker)
        {
            _hasJoker = true;
        }
        else
        {
            _hand.push_back(card);
            _handMask |= MASK(NUMBER(card));
            if (suit != SUIT(card))
            {
                suit = SUIT(card);
                _handSuitsCount += 1;
            }
        }
    }

    for (auto card : discard)
    {
        if (card == Joker)
        {
            _jokerDiscarded = true;
        }
        else
        {
            _discard.push_back(card);
            //remove card from deck
            const int number = NUMBER(card);
            const int suit = SUIT(card);

            _deckSuits[suit] = _deckSuits[suit] & ~MASK(number);
            _deckNumbers[number] = _deckNumbers[number] & ~MASK(suit);
        }
    }

    std::sort(_hand.begin(), _hand.end(), sort_cards_by_numbers);
    std::sort(_discard.begin(), _discard.end(), sort_cards_by_numbers);

    if (!_hand.empty())
    {
        int lineSize = 1;
        int number = NUMBER(_hand[0]);
        for (size_t x = 1; x < _hand.size(); x += 1, lineSize += 1)
        {
            if (NUMBER(_hand[x]) != number)
            {
                number = NUMBER(_hand[x]);
                _handLines[lineSize] += 1;
                _handNumbersCount += 1;
                lineSize = 0;
            }
        }
        _handLines[lineSize] += 1;
        _handNumbersCount += 1;
    }
}

bool PokeHelper::canMakePoker() const
{
    //если джокер в сбросе то мы не можем собрать покер
    if (_jokerDiscarded)
    {
        return false;
    }

    if (_hand.empty())
    {
        for (int x = 0; x < NUMBERS_COUNT; x += 1)
        {
            if (_deckNumbers[x] == 0xF)
            {
                //джокер на руках или в колоде
                return true;
            }
        }

        return false;
    }

    if (_handNumbersCount > 1)
    {
        //карты на руках разного номинала
        return false;
    }

    //ни одной карты нужного номинала не сброшено
    return _deckNumbers[NUMBER(_hand[0])] == 0xF;
}

bool PokeHelper::canMakeRoyalFlush() const
{
    if (_hand.empty())
    {
        //если на руках нет карт или только джокер проверяем колоду
        for (int x = 0; x < SUITS_COUNT; x += 1)
        {
            const int pcnt = popcount(_deckSuits[x] & ROYAL_FLUSH_MASK);
            if (count_with_joker(pcnt, 5, !_jokerDiscarded))
            {
                //можем собрать
                return true;
            }
        }
        return false;
    }

    if (_handSuitsCount > 1)
    {
        //на руках карты разных мастей
        return false;
    }

    if(_handLines.at(1)!=_hand.size())
    {
        //на руках пара/тройка
        return false;
    }

    //проверяем что карты на руках подходят для РФ
    if ((_handMask & ROYAL_FLUSH_MASK) != _handMask)
    {
        return false;
    }

    //проверяем что в колоде есть карты для сбора ФР нашей масти
    const int pcnt = popcount(_deckSuits[SUIT(_hand[0])] & ROYAL_FLUSH_MASK);
    return count_with_joker(pcnt, 5, !_jokerDiscarded);
}

bool PokeHelper::canMakeStraightFlush() const
{
    if (_hand.empty())
    {
        for (int x = 0; x < SUITS_COUNT; x += 1)
        {
            for (int y = 0; y < STRAIGHTWOROYALCOUNT; y += 1)
            {
                const int pcnt = popcount(_deckSuits[x] & straight[y]);
                if (count_with_joker(pcnt, 5, !_jokerDiscarded))
                {
                    //можем собрать
                    return true;
                }
            }
        }
        //ни в одной масти нет возможности собрать СФ
        return false;
    }

    if (_handSuitsCount > 1)
    {
        //на руках карты разных мастей
        return false;
    }

    if(_handLines.at(1)!=_hand.size())
    {
        //на руках пара/тройка
        return false;
    }

    const int handSuit = SUIT(_hand[0]);

    for (int x = 0; x < STRAIGHTWOROYALCOUNT; x += 1)
    {
        //если стрит собирается из руки
        if ((_handMask & straight[x]) == _handMask)
        {
            //проверяем что для него достаточно карт в колоде
            const int pcnt = popcount(_deckSuits[handSuit] & straight[x]);
            if (count_with_joker(pcnt, 5, !_jokerDiscarded))
            {
                //можем собрать
                return true;
            }
        }
    }

    //нимагём
    return false;
}

bool PokeHelper::canMakeFourOfAKind() const
{
    if (_hand.empty())
    {
        for (int x = 0; x < NUMBERS_COUNT; x += 1)
        {
            //если в колоде есть 4 карты или 3+Джокер то можем
            if (count_with_joker(popcount(_deckNumbers[x]), 4, !_jokerDiscarded))
            {
                return true;
            }
        }
    }

    if (_handNumbersCount == 1)
    {
        //на руках карты одного номинала
        return count_with_joker(popcount(_deckNumbers[NUMBER(_hand[0])]), 4, !_jokerDiscarded);
    }

    if (_hand.size() <= 2)
    {
        //если на руках 2 карты разного номинала, то нужно проверить что мы можем собрать 4 с любой из них
        for (size_t x = 0; x < _hand.size(); x += 1)
        {
            if (count_with_joker(popcount(_deckNumbers[NUMBER(_hand[x])]), 4, !_jokerDiscarded))
            {
                //с одной из карт с рук можем собрать 4
                return true;
            }
        }

        return false;
    }

    if (_handLines.at(_hand.size() - 1) != 1)
    {
        //на руках больше одной неподходящей карты
        //на руках две пары или все вразнобой или 1+1+2
        return false;
    }


    //если в колоде есть 4 карты или 3+Джокер то можем
    return count_with_joker(popcount(_deckNumbers[NUMBER(_hand[1])]), 4, !_jokerDiscarded);
}

bool PokeHelper::canMakeFullHouse() const
{
    if (_hand.empty())
    {
        //мы не можем сбросить все пары и тройки
        return true;
    }

    if (_hand.size() > 2)
    {
        if (_handNumbersCount > 2)
        {
            //на руках больше 2 разных номиналов
            return false;
        }

        if (_handLines.at(4) == 1 || (_handLines.at(3) == 1 && _hasJoker))
        {
            //на руках 4
            return false;
        }
    }

    return true;
}

bool PokeHelper::canMakeFlush() const
{
    if (_hand.empty())
    {
        //мы не можем сбросить все масти
        return true;
    }

    const int handSuit = SUIT(_hand[0]);
    if (_handSuitsCount > 1)
    {
        //на руках разные масти
        return false;
    }

    //в колоде осталось больше 5 карт нашей масти
    return count_with_joker(popcount(_deckSuits[handSuit]), 5, _jokerDiscarded);
}

bool PokeHelper::canMakeStraight() const
{
    if (_hand.empty())
    {
        //мы не можем сбросить все стриты
        return true;
    }

    if(_handLines.at(1)!=_hand.size())
    {
        //на руках пара/тройка
        return false;
    }

    const int suit = SUIT(_hand[0]);

    const int deckMask = _deckSuits[Suit0] | _deckSuits[Suit1] | _deckSuits[Suit2] | _deckSuits[Suit3];
    for (int x = 0; x < STRAIGHTCOUNT; x += 1)
    {
        //если стрит собирается из руки
        if ((_handMask & straight[x]) == _handMask)
        {
            //проверяем что для него достаточно карт в колоде
            const int pcnt = popcount(deckMask & straight[x]);
            if (count_with_joker(pcnt, 5, !_jokerDiscarded))
            {
                //если разные масти на руках, то нет смысла проверять масти в колоде
                if (_handSuitsCount > 1)
                {
                    return true;
                }

                //проверяем что в колоде для сбора стрита есть карты другой масти иначе получим стрит-флеш
                int str = straight[x] & ~_handMask;
                for (int y = 0; str > 0; y += 1, str >>= 1)
                {
                    if (str & 1)
                    {
                        if (popcount(_deckNumbers[y] & ~suit))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool PokeHelper::canMakeThreeOfAKind() const
{
    if (_hand.empty())
    {
        //мы не можем сбросить все тройки
        return true;
    }

    //можем сфейлить если на руках 4 разных номинала или 4(3+Joker) или две пары
    if (_handNumbersCount > 3
        || _handLines.at(4) > 0
        || (_handLines.at(3) > 0 && _hasJoker)
        || _handLines.at(2) > 1)
    {
        return false;
    }
    return true;
}

bool PokeHelper::canMakeTwoPair() const
{
    if (_hand.empty())
    {
        //мы не можем сбросить все пары
        return true;
    }

    //можем сфейлить если на руках тройка и выше или 4 карты вразнобой
    if (_handNumbersCount > 3
        || _handLines.at(4) > 0
        || _handLines.at(3) > 0
        || (_handLines.at(2) > 0 && _hasJoker)
        || _handLines.at(1) >= 4)
    {
        return false;
    }

    return true;
}

bool PokeHelper::hasPoker() const
{
    return _handLines.at(4) == 1 && _hasJoker;
}

bool PokeHelper::hasRoyalFlush() const
{
    if (_handSuitsCount > 1)
    {
        return false;
    }

    return count_with_joker(popcount(_handMask & ROYAL_FLUSH_MASK), 5, _hasJoker);
}

bool PokeHelper::hasStraightFlush() const
{
    if (_handSuitsCount > 1)
    {
        return false;
    }

    for (int x = 0; x < STRAIGHTWOROYALCOUNT; x += 1)
    {
        if (count_with_joker(popcount(_handMask & straight[x]), 5, _hasJoker))
        {
            return true;
        }
    }

    return false;
}

bool PokeHelper::hasFourOfAKind() const
{
    return (_handLines.at(4) == 1 && !_hasJoker) || (_handLines.at(3) == 1 && _hasJoker);
}

bool PokeHelper::hasFullHouse() const
{
    return (_handLines.at(2) == 1 && _handLines.at(3) == 1) || (_handLines.at(2) == 2 && _hasJoker);
}

bool PokeHelper::hasFlush() const
{
    if (_handSuitsCount > 1)
    {
        return false;
    }

    //любая комбинация кроме РФ и СФ
    return !hasRoyalFlush() && !hasStraightFlush();
}

bool PokeHelper::hasStraight() const
{
    if (_handSuitsCount == 1)
    {
        return false;
    }

    for (int x = 0; x < STRAIGHTCOUNT; x += 1)
    {
        if (count_with_joker(popcount(_handMask & straight[x]), 5, _hasJoker))
        {
            return true;
        }
    }

    return false;
}

bool PokeHelper::hasThreeOfAKind() const
{
    return (_handLines.at(3) == 1 && !_hasJoker && _handLines.at(2) == 0)
        || (_handLines.at(2) == 1 && _handLines.at(3) == 0 && _hasJoker);
}

bool PokeHelper::hasTwoPair() const
{
    return _handLines.at(2) == 2 && !_hasJoker;
}

const std::map<std::wstring, int> cardStrToNumberMapping = {
    {L"2", Number2},
    {L"3", Number3},
    {L"4", Number4},
    {L"5", Number5},
    {L"6", Number6},
    {L"7", Number7},
    {L"8", Number8},
    {L"9", Number9},
    {L"10", Number10},
    {L"J", NumberJ},
    {L"Q", NumberQ},
    {L"K", NumberK},
    {L"A", NumberA}
};

const std::map<int, std::wstring> cardNumberToStrMapping = {
    {Number2, L"Number2"},
    {Number3, L"Number3"},
    {Number4, L"Number4"},
    {Number5, L"Number5"},
    {Number6, L"Number6"},
    {Number7, L"Number7"},
    {Number8, L"Number8"},
    {Number9, L"Number9"},
    {Number10, L"Number10"},
    {NumberJ, L"NumberJ"},
    {NumberQ, L"NumberQ"},
    {NumberK, L"NumberK"},
    {NumberA, L"NumberA"}
};

const std::map<wchar_t, int> cardStrToSuitMapping = {
    {L'♠', Suit0},
    {L'♥', Suit1},
    {L'♣', Suit2},
    {L'♦', Suit3}
};

const std::map<int, std::wstring> cardSuitToStrMapping = {
    {Suit0, L"♠"},
    {Suit1, L"♥"},
    {Suit2, L"♣"},
    {Suit3, L"♦"}
};

int PokeHelper::CardFromString(std::wstring card)
{
    if (card == L"Joker")
    {
        return Joker;
    }

    int ret = cardStrToSuitMapping.at(card[0]) * 100;
    ret += cardStrToNumberMapping.at(card.substr(1));

    return ret;
}

std::wstring PokeHelper::CardToString(int card)
{
    if (card == Joker)
    {
        return L"Joker";
    }

    return cardSuitToStrMapping.at(SUIT(card)) + cardNumberToStrMapping.at(NUMBER(card));
}

std::vector<int> PokeHelper::CardListFromString(std::wstring cards)
{
    std::vector<int> ret{};
    size_t pos;

    while ((pos = cards.find(',')) != std::wstring::npos)
    {
        ret.push_back(CardFromString(cards.substr(0, pos)));
        cards.erase(0, pos + 1);
    }
    if (cards.length() > 0)
    {
        ret.push_back(CardFromString(cards));
    }
    return ret;
}
