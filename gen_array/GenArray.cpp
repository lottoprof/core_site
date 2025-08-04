#include <cstring>
#include <iostream>
#include <vector>

#include "ShipTicketInfo.h"

void print_ticket(ShipTicketInfo* ticketInfo)
{
    char array[90];
    std::memset(&array, '_', 90);

    for (auto ship_info : ticketInfo->ships_info)
    {
        if (ship_info.horizontal)
        {
            for (int column = ship_info.y; column < ship_info.y + ship_info.length; column += 1)
            {
                array[ship_info.x * 10 + column] = 'X';
            }
        }
        else
        {
            for (int row = ship_info.x; row < ship_info.x + ship_info.length; row += 1)
            {
                array[row * 10 + ship_info.y] = 'X';
            }
        }
    }

    for (int row = 0; row < 9; row += 1)
    {
        for (int column = 0; column < 10; column += 1)
        {
            std::wcout << array[row * 10 + column] << " ";
        }
        std::wcout << "z\n";
    }
}

#ifdef TEST_SHIPS
int main(int argc, char* argv[])
{
    std::vector<ShipTicketInfo> ships;
    ships.resize(100);

    for (int x = 0; x < 100; x += 1)
    {
        print_ticket(&ships[x]);
        std::cout << ships[x].print_mask();
        std::wcout << "z\n";
        std::wcout << "z\n";
    }
    return 0;
}
#endif