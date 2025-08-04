#include "ShipTicketInfo.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <memory.h>
#include "ShipMasks.h"
#include <string>

void randomize_mask(ShipTicketInfo* ticket_info)
{
    for (int x = 0; x < 90; x += 1)
    {
        ticket_info->mask[x] = x + 1;
    }

    std::shuffle(std::begin(ticket_info->mask), std::end(ticket_info->mask), std::mt19937(std::random_device()()));
}

bool gen_internal(ShipTicketInfo* out_ticket,
                  unsigned int (* arr)[3],
                  ship_mask_info const** ship_mask_infos,
                  const size_t* ship_mask_infos_length,
                  int total_depth,
                  int current_depth)
{
    const auto masks_length = ship_mask_infos_length[current_depth];
    const auto masks = new ship_mask_info[masks_length];

    auto isFound = false;

    memcpy(masks, ship_mask_infos[current_depth], sizeof(ship_mask_info) * masks_length);
    std::shuffle(&masks[0], &masks[masks_length], std::mt19937(std::random_device()()));

    for (int x = 0; x < masks_length; x += 1)
    {
        auto mask_info = masks[x];

        if (masks_intersect(*arr, mask_info.check_mask))
        {
            continue;
        }

        unsigned int new_mask[3];
        memcpy(new_mask, arr, sizeof(unsigned int) * 3);

        apply_mask(mask_info.apply_mask, new_mask);

        if (current_depth < total_depth - 1)
        {
            if (!gen_internal(out_ticket, &new_mask, ship_mask_infos, ship_mask_infos_length, total_depth,
                              current_depth + 1))
            {
                continue;
            }
        }

        out_ticket->ships_info[current_depth].x = mask_info.x;
        out_ticket->ships_info[current_depth].y = mask_info.y;
        out_ticket->ships_info[current_depth].length = mask_info.length;
        out_ticket->ships_info[current_depth].horizontal = mask_info.horizontal;

        isFound = true;
        break;
    }

    delete[] masks;

    return isFound;
}

bool gen_ticket(ShipTicketInfo* outTicket)
{
    ship_mask_info const* p[] = {
        masks_4,
        masks_3,
        masks_3,
        masks_2,
        masks_2,
        masks_2
    };
    size_t const pp[] = {
        masks_length_4,
        masks_length_3,
        masks_length_3,
        masks_length_2,
        masks_length_2,
        masks_length_2
    };
    unsigned int mask[3] = {0, 0, 0};
    auto found = gen_internal(outTicket, &mask, p, pp, 6, 0);
    if (found)
    {
        randomize_mask(outTicket);
    }
    return found;
}

ShipTicketInfo::ShipTicketInfo()
{
    if (!randomize())
    {
        printf("%s", "FUU");
    }
}

std::string ShipTicketInfo::print_mask()
{
    std::stringstream stream;
    stream << R"({"number":")" <<get_number()<< R"(","price":")" <<get_price()<< R"(","draw":")" <<get_draw()<<R"(",)";
    stream << R"("mask":[)";
    for (int x = 0; x < 90; x += 1)
    {
        if (x != 0)
        {
            stream << ", ";
        }
        stream << mask[x];
    }
    stream << R"(],"bet":[)";
    stream << R"({"shipId":1,"mask":[)";
    print_ship(stream, ships_info[0]);
    stream<<"]},";
    stream << R"({"shipId":2,"mask":[)";
    print_ship(stream, ships_info[1]);
    stream<<"]},";
    stream << R"({"shipId":3,"mask":[)";
    print_ship(stream, ships_info[2]);
    stream<<"]},";
    stream << R"({"shipId":4,"mask":[)";
    print_ship(stream, ships_info[3]);
    stream<<"]},";
    stream << R"({"shipId":5,"mask":[)";
    print_ship(stream, ships_info[4]);
    stream<<"]},";
    stream << R"({"shipId":6,"mask":[)";
    print_ship(stream, ships_info[5]);
    stream<<"]}]}\n";

    return stream.str();
}

bool ShipTicketInfo::randomize()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    this->number = 10000000L + now.count();
    return gen_ticket(this);
}

void ShipTicketInfo::set_price(int price)
{
    this->price = price;
}

void ShipTicketInfo::set_draw(int draw)
{
    this->draw = draw;
}

uint64_t ShipTicketInfo::get_number() const
{
    return number;
}

int ShipTicketInfo::get_draw() const
{
    return draw;
}

int ShipTicketInfo::get_price() const
{
    return price;
}

int ShipTicketInfo::copy_json(char* buff, int size)
{
    auto mask = print_mask();
    if(mask.length()>size)
    {
        return 0;
    }
    //strcpy_s(buff, size, mask.c_str());
    strncpy(buff, mask.c_str(),size);
    return mask.length();
}

int ShipTicketInfo::copy_mask(char* buff, int size)
{
    return copy_json(buff, size);
}


void ShipTicketInfo::print_ship(std::stringstream& stream, ship_info ship) const
{
    bool print_comma = false;
    if (ship.horizontal)
    {
        //horizontal
        for (int column = ship.y; column < ship.y + ship.length; column += 1)
        {
            if(print_comma)
            {
                stream<<", ";
            }
            print_comma = true;
            stream << mask[ship.x * 10 + column];
        }
    }
    else
    {
        //vertical
        for (int row = ship.x; row < ship.x + ship.length; row += 1)
        {
            if(print_comma)
            {
                stream<<", ";
            }
            print_comma = true;
            stream << mask[row * 10 + ship.y];
        }
    }
}
