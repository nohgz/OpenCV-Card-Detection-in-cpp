#include "hmhmhm.h"

Thing::Thing(double hp)
{
    tHP = hp;
}

double Thing::getHP()
{
    return tHP;
}

void Thing::takeDamage(double amount)
{
    tHP -= amount;

    if (tHP < 0)
    {
        tHP = 0;
    }
}