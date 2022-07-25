#pragma once

class HammersleySequence
{
public:
    HammersleySequence();
    const static size_t K_LENGTH = 64;
    float sequence[K_LENGTH * 2] = {};
};
