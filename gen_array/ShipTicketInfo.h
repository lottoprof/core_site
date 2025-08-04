#pragma once
#include <sstream>
#include <cstdint>

class ShipTicketInfo
{
    struct ship_info
    {
        int x;
        int y;
        bool horizontal;
        int length;
    };
public:
    ShipTicketInfo();
    ship_info ships_info[6]{};

    int mask[90]{};
    uint64_t number{};
    int price{};
    int draw{};

    std::string print_mask();

    bool randomize();

    void set_price(int price);
    void set_draw(int draw);

    int copy_json(char*, int);
    int copy_mask(char*, int);

    uint64_t get_number() const;
    int get_draw() const;
    int get_price() const;

private:
    void print_ship(std::stringstream&stream, ship_info ship) const;
};
