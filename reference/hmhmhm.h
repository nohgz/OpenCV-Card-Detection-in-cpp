#pragma once

class Thing
{
public:
    Thing(double hp);
    double getHP();
    void takeDamage(double amount);
private:
    double tHP;
};